#ifndef _feat_triggercycle_c_
#define _feat_triggercycle_c_

#include "feat_triggercycle.h"

void init_triggercycle(triggercycle_t *trc) {
  trc->last_trigger = 0.0f;
  trc->real_trigger = sens_trigger;
  trc->last_cycle = 0.0f;
  trc->real_cycle = sens_cycle;

  trc->custom_trigger = false;
  trc->custom_cycle = false;
}

triggercycle_t* get_triggercycle() {
  return GET_PTR(PTR_TRIGGERCYCLE, triggercycle_t, init_triggercycle);
}

void update_triggercycle(triggercycle_t *trc, tracking_t *tr) {
  // If the value is changed, it means it was changed by the UI code due to user input. Update the reference value
  if (sens_trigger != trc->last_trigger) {
    trc->real_trigger = sens_trigger;
  }
  if (sens_cycle != trc->last_cycle) {
    trc->real_cycle = sens_cycle;
  }

  if (tr->st_inhaling) {
    const float cti = tr->current.ti;
    const float s = trc->real_cycle;
    // There should be no dynamic collapse this early into the breath, if flow drops at all, it was likely an autotriggered breath.
    if (cti <= 0.4f) { sens_cycle = (s + 0.8f) / 2.0f; }

    if (trc->custom_cycle) {
      float s2 = -0.225f + 0.5f * s; // Results in thresholds of: 0.025, -0.05, -0.1, -0.15, -0.185
      if ( (flow_unfucked < (tr->current.inh_maxflow * s2)) || (tr->st_pre_cycle >= PRE_CYCLE_MAX_TICKS) ) {
        sens_cycle = 0.95; // Guarantee cycling
      } else {
        sens_cycle = -0.5; // Do not cycle (unless flow is very negative)
      }
    }
  } else {
    // Increase trigger threshold early into the exhale period to make autotriggering less likely.
    const float cte = tr->current.te;
    const float rte = tr->recent.te;
    const float s = trc->real_trigger;
    sens_trigger = remapc(cte, rte - 0.4f, rte - 0.15f, s * 1.2f + 2.0f, s * 1.0f);

    // TODO: Custom trigger that combines flow-based, pressure error-based, cumulative volume-based thresholds
  }

  trc->last_cycle = sens_cycle;
  trc->last_trigger = sens_trigger;
}


#endif

    // else {
    //   // My ASV changes shape of the flow curve, so that it peaks higher, and goes low earlier. This (maybe) accounts for that.
    //   sens_cycle = s * remapc(asv_factor, 1.0f, 2.0f, 1.0f, 0.5f);
    // }

// 7. Apply EPS to S mode

// PAP - lower trigger threshold with cumulative volume - most non-inhales only reach 10-15mL, most ineffective efforts reach 25-30mL, and with p_error(only if recent commanded volume was constant)
// 20-40mL for -0%-25% threshold 5-15 for +0%-25%
// Previously did 0.05-0.3cmH2O p_error to 3cmH2O, or -62.5% threshold, though baseline was 1.5cmH2O higher, so more like -30% ?



// Function pointer wizardry - probably/definitely won't use, overkill and bug-prone
/*
void *array[9];
bool (*trigger_fn[9])();
bool (*cycle_fn[9])();
trigger_fn[4] = ...;
bool result = (*trigger_fn[4])();
*/