#include "stubs.h"
#include "common_code.h"

#include "my_asv.h" // Include the asv_data_t definition

const float INSTANT_PS = 0.45f;
const float EPS = 1.2f;

typedef struct {
  float eps; // EPS (cmH2O) - used to prevent instant jumps in pressure in case of autotriggering
  float ips_fa; // Flow-Assist IPS (cmH2O) - currently used to augment pretrigger effort
} features_t;

STATIC void init_features(features_t *feat) {
  feat->eps = 0.0f;
  feat->ips_fa = 0.0f;
}

STATIC float get_delta_flow(history_t *hist, int bin_size) {
  const int t = hist->tick;
  if (t < 2*bin_size) { return 0.0f; }
  float avgf[3] = {0.0f, 0.0f, 0.0f}; // I don't think it overflows, but just in case it does, let's have padding.
  for (int i=0; i<2*bin_size; i++) {
    avgf[i/bin_size] += hist->flow[(t-i) % HISTORY_LENGTH];
  }
  return (avgf[0] - avgf[1]) / (float)(bin_size*bin_size);
}


// +1 pointer address: 0x000f93d0. Original function address: 0x080bc992
extern void pressure_limit_max_difference();

// Reshapes PS in 0.0-1.0 format to differently shaped slopes with `mult` times the AUC, first increasing slope before magnitude
// Only using ^4 shape, because going to ^8 and above is very jarring and results in bad premature cycling
STATIC float reshape_vauto_ps(float ps1, float mult) {
  // ^2 - 1.330, ^6 - 1.707, ^8 - 1.770
  float ps4 = 1.0f - pow(1.0f - ps1, 4);  // ~1.594x the AUC
  ps4 = ps4 * 0.5f + ps1 * 0.5f; // ~1.297x the AUC
  if (mult <= 1.0) { 
    return ps1; 
  } else if ((mult > 1.0) && (mult <= 2.0)) {
    return remap(mult, 1.0f, 2.0f, ps1, ps4 * 1.542f);
  } else {
    return ps4 * (mult / 1.297f);
  }

  return ps1;
}

STATIC void handle_sensitivities(tracking_t *tr, float asv_factor) {
  // If the value is changed, it means it was changed by the UI code due to user input. Update the reference value
  if (sens_trigger != tr->settings.last_trigger) {
    tr->settings.real_trigger = sens_trigger;
    tr->settings.last_trigger = sens_trigger;
  }
  if (sens_cycle != tr->settings.last_cycle) {
    tr->settings.real_cycle = sens_cycle;
    tr->settings.last_cycle = sens_cycle;
  }
  if (tr->st_inhaling) {
    const float cti = tr->current.ti;
    const float s = tr->settings.real_cycle;
    // There should be no dynamic collapse this early into the breath, if flow drops at all, it was likely an autotriggered breath.
    if (cti <= 0.4f) { sens_cycle = (s + 0.8f) / 2.0f; }
    else {
      // My ASV changes shape of the flow curve, so that it peaks higher, and goes low earlier. This (maybe) accounts for that.
      sens_cycle = s * remapc(asv_factor, 1.0f, 2.0f, 1.0f, 0.5f);
    }
    tr->settings.last_cycle = sens_cycle;
  } else {
    // Increase trigger threshold early into the exhale period to make autotriggering less likely.
    const float cte = tr->current.te;
    const float rte = tr->recent.te;
    const float sens = tr->settings.real_trigger;
    sens_trigger = remapc(cte, rte - 0.4f, rte - 0.15f, sens * 1.2f + 2.0f, sens * 1.0f);
    tr->settings.last_trigger = sens_trigger;
  }
}

void MAIN start() {
  history_t *hist = get_history();
  update_history(hist);
  tracking_t *tr = get_tracking();
  update_tracking(tr);
  asv_data_t *asv = get_asv_data();
  update_asv_data(asv, tr);

  const float flow = *flow_compensated; // (L/min)
  const float dflow = get_delta_flow(hist, 4);
  const float flow2 = max(flow, flow + dflow*8.0f); // Flow extrapolated 80ms into the future

  features_t *feat = GET_PTR(PTR_FEATURES, features_t, init_features);

  apply_jitter(true);


  float dps = 0.0f;
  bool toggle = (ti_min <= 150);
  handle_sensitivities(tr, toggle ? asv->asv_factor : 1.0f);

  if (*therapy_mode == 3) {

    float current_eps = clamp((*cmd_epap - vauto_ps) * 0.2f, 0.4f, 1.6f);

    int t = hist->tick;
    const float ps = *cmd_ps + vauto_ps/2.0f;
    const float ps1 = (ps/vauto_ps); // 0.0 to 1.0

    if (tr->st_inhaling) {
      float new_ps = ps;
      new_ps = remap(ps1, 0.0f, 1.0f, feat->eps, vauto_ps-INSTANT_PS) + INSTANT_PS;
      if (toggle) { // Disable if Ti min is set to above 0.1s
        float new_ps1 = reshape_vauto_ps(ps1, asv->asv_factor);
        new_ps = remap(new_ps1, 0.0f, 1.0f, feat->eps, vauto_ps - INSTANT_PS) + INSTANT_PS*asv->asv_factor;
      }
      dps = (new_ps - ps);

      feat->ips_fa = 0.0f;
      feat->eps = min(feat->eps + 0.01f * current_eps, 0.0f);

      asv->final_ips = max(asv->final_ips, ps + dps);
    } else { // Exhaling
      if (tr->current.ti >= 0.7f) {
        current_eps = max(0.0f, current_eps - (asv->final_ips - vauto_ps) * 0.25f);
        if (tr->st_just_started) { feat->eps = -current_eps; }
        else {
          float eps1 = remap01c(tr->current.volume / tr->current.volume_max, 0.05f, 0.6f);
          eps1 = min(eps1, remap01c(tr->current.te, 1.2f, 0.4f));
          feat->eps = max(feat->eps, -current_eps * eps1);
        }
      }
      float new_ps1 = ps1*ps1 * 0.75f + 0.25f * ps1;
      float new_ps = remap(new_ps1, 0.0f, 1.0f, feat->eps, asv->final_ips);
      dps = (new_ps - ps);

      feat->ips_fa = remapc(flow2, 1.0f, 8.0f, 0.0f, 0.4f);
      if (flow <= -2.0f) { feat->ips_fa = 0.0f; }
      dps += feat->ips_fa;
    }
  }

  const float orig_ps = *cmd_ps;
  *cmd_ps += dps;
  pressure_limit_max_difference(); // Execute the original function
  *cmd_ps = orig_ps;

  apply_jitter(false);
}
