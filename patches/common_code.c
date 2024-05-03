#include "common_code.h"
#include "stubs.h"

float remap(float s, float start, float end, float new_start, float new_end) {
  return new_start + remap01(s, start, end) * (new_end - new_start);
}

float remapc(float s, float start, float end, float new_start, float new_end) {
  return new_start + remap01c(s, start, end) * (new_end - new_start);
}

float remap01(float s, float start, float end) {
   return (s - start)/(end-start);
}

float remap01c(float s, float start, float end) {
   return clamp( remap01(s, start, end), 0.0f, 1.0f );
}

float lerp(float from, float to, float coeff) {
   return from + (to - from) * coeff;
}

float pow(float base, int exp){
  if (exp == 0) { return 1; }
  if (exp % 2 == 0) {
    return pow(base, exp/2) * pow(base, exp/2);
  } else {
    return base * pow(base, exp/2) * pow(base, exp/2);
  }
}

// Based on this SO answer: https://stackoverflow.com/a/49991852
float sqrtf(float n) {
  float guess = 1.0f;
  for(int i=0; i <= 10; i++) {
    guess -= (guess*guess - n) / (2*guess);
  }
  return guess;
}

typedef struct {
  unsigned magic;
  void** pointers;
} magic_ptr_t;

// At the very start of a segment of memory addresses not used by the device.
// Other candidates include: 0be0, 2978, 2980, 4560
magic_ptr_t * const magic_ptr = (void*) (0x20000be0); 
const unsigned MAGIC = 0x07E49001;

void *get_pointer(ptr_index index, int size, void (*init_fn)(void*)) {
  const int max_pointers = __PTR_LAST;
  if (magic_ptr->magic != MAGIC) {
    magic_ptr->pointers = malloc(sizeof(void*) * max_pointers);
    magic_ptr->magic = MAGIC;
    for(int i=0; i<max_pointers; i++) {
      magic_ptr->pointers[i] = 0;
    }
  }
  if (magic_ptr->pointers[index] == 0) {
    magic_ptr->pointers[index] = malloc(size);
    init_fn(magic_ptr->pointers[index]);
  }
  return magic_ptr->pointers[index];
}


void init_history(history_t *hist) {
  for(int i=0; i<HISTORY_LENGTH; i++) {
    hist->flow[i] = 0.0f;
  }
  hist->tick = -1;
  hist->last_jitter = 0;
}

void update_history(history_t *hist) {
  const unsigned now = tim_read_tim5();
  // Initialize if it's the first time or more than 0.1s elapsed, suggesting that the therapy was stopped and re-started.
  if ((now - hist->last_time) > 100000) { init_history(hist); }
  
  hist->last_time = now; // Keep it updated so we don't reset the struct
  hist->tick += 1;
  hist->flow[hist->tick % HISTORY_LENGTH] = *flow_compensated;
}

history_t *get_history() { 
  return GET_PTR(PTR_HISTORY, history_t, init_history);
}


void apply_jitter(bool undo) {
  history_t *hist = get_history();
  if (undo) { // Undo the previous
    hist->last_jitter *= -1;
  } else { // Get new jitter value
    hist->last_jitter = 1 - (ivars[0]/4) % 2 * 2;
  }
  // FIXME: This needs to be relative to either EPAP or IPAP, the "is the PS sufficiently different to trigger a redraw?" routine is relative to pressure.
  const float amtf = 0.01f * hist->last_jitter;
  *cmd_ps += amtf; *cmd_epap_ramp -= amtf;
}


///////////////////////////////
// All-purpose tracking code //

void init_breath(breath_t *breath) {
  breath->volume = 0.0f;
  breath->volume_max = 0.0f;
  breath->exh_maxflow = 0.0f;
  breath->inh_maxflow = 0.0f;
  breath->t = -1;
  breath->ti = 0.0f;
  breath->te = 0.0f;
}
void init_settings(settings_proxy_t *sett) {
  sett->last_trigger = 0.0f;
  sett->real_trigger = sens_trigger;
  sett->last_cycle = 0.0f;
  sett->real_cycle = sens_cycle;
}
void init_tracking(tracking_t *tr) {
  tr->last_progress = breath_progress;
  tr->last_time = tim_read_tim5();
  tr->breath_count = 0;
  tr->tick = 0;
  tr->st_inhaling = false;
  tr->st_just_started = false;

  init_settings(&tr->settings);

  init_breath(&tr->recent);
  init_breath(&tr->last);
  init_breath(&tr->current);
}

tracking_t* get_tracking() {
  return GET_PTR(PTR_TRACKING, tracking_t, init_tracking);
}

void update_tracking(tracking_t *tr) {
  const unsigned now = tim_read_tim5();
  // Initialize if it's the first time or more than 0.1s elapsed, suggesting that the therapy was stopped and re-started.
  if ((now - tr->last_time) > 100000) { init_tracking(tr); }

  // Handle breaths and their stage
  tr->st_just_started = false;
  if ((tr->last_progress > 0.5f) && (breath_progress < 0.5f)) {
    tr->st_inhaling = true; tr->st_just_started = true;

    tr->last = tr->current;
    tr->breath_count += 1;
    init_breath(&tr->current);

    // Update recent breath representing the recent "weighted averages"
    // TODO: Expand checks for whether a breath was valid
    tr->st_valid_breath = tr->last.te > max(tr->recent.te * 0.6f, 0.7f);
    tr->st_valid_breath &= tr->last.ti > 0.7f;
    if (tr->st_valid_breath) {
      inplace(lerp, &tr->recent.volume_max, tr->last.volume_max, tr_coeff);
      inplace(lerp, &tr->recent.exh_maxflow, tr->last.exh_maxflow, tr_coeff);
      inplace(lerp, &tr->recent.inh_maxflow, tr->last.inh_maxflow, tr_coeff);
      inplace(lerp, &tr->recent.ti, tr->last.ti, tr_coeff);
      inplace(lerp, &tr->recent.te, tr->last.te, tr_coeff);
    }
  } else if ((tr->last_progress <= 0.5f) && (breath_progress > 0.5f)) {
    tr->st_inhaling = false; tr->st_just_started = true;
  }

  tr->tick += 1; tr->current.t += 1;
  if (tr->st_inhaling) { 
    tr->current.ti += 0.01f;
    inplace(max, &tr->current.inh_maxflow, *flow_compensated);
  } else { 
    tr->current.te += 0.01f;
    inplace(min, &tr->current.exh_maxflow, *flow_compensated);
  }

  tr->current.volume += (*flow_compensated / 60.0f) * 0.01f;
  inplace(max, &tr->current.volume, 0.0f);
  inplace(max, &tr->current.volume_max, tr->current.volume);

  tr->last_progress = breath_progress;
  tr->last_time = now;
}

#include "my_asv.c"