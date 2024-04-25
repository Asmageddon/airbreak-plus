#ifndef _my_asv_h_
#define _my_asv_h_

///////////////////
// Config values //

#define ASV_STEP_LENGTH 5 // (10ms ticks)
#define ASV_STEP_COUNT 20 // (steps)
#define ASV_STEP_SKIP 3 // (steps) MUST be at least 1 for the padding code to work correctly

const float asv_low = 0.95f;
const float asv_high = 0.98f;
const float asv_pid_max = 1.5f;

const float asv_coeff1 = 0.025f; // ~40% from last 20 breaths, ~66% from 40, ~77% from 60
const float asv_coeff2 = 0.3f;

const float asv_dampen_max = 0.3f;

/////////////////////////
// PID controller code //

typedef struct {
  float last_error;
  float cumulative_error;
  float current_error;
  float kp;
  float ki;
  float kd;
  float output_min;
  float output_max;
} pid_t;

void pid_init(pid_t *pid, float p, float i, float d, float _min, float _max);
float pid_get_signal(pid_t *pid);
void pid_update(pid_t *pid, float current_error);

////////////////////////
// ASV algorithm code //

typedef struct {
  uint32 last_time;
  uint32 breath_count;

  int16 ticks; // Starts at 0, +1 each call
  pid_t pid;
  float asv_factor;
  float final_ips; // Final max IPS value, used to maintain correct downslope

  float asv_dampen; // How much to drop the target by, other than after a hyperpnea, it's usually 0

  float target_vol; // The real target vol
  float target_vol2; // Intermediate for a second lerp that smoothes the changes out 

  float16 targets_recent[ASV_STEP_COUNT+1]; // +1 for flow calculations
  float16 targets_recent2[ASV_STEP_COUNT+1]; // sadly need a 2nd copy for the two-stage lerp
  float16 targets_current[ASV_STEP_COUNT+1];
} asv_data_t;

asv_data_t *get_asv_data();
void init_asv_data(asv_data_t *data);
void update_asv_data(asv_data_t* asv, tracking_t* tr);

#endif