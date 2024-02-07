#ifndef _common_code_h_
#define _common_code_h_

// Mandatory definitions
#define INLINE inline __attribute__((always_inline))
#define MAIN __attribute__((section(".text.0.main")))
#define STATIC static __attribute__((section(".text.x.nonmain")))


// Types
typedef unsigned int uint32;
typedef signed short int16;
typedef signed char int8;
typedef __fp16 float16;
typedef enum { false, true } bool;


// PAP memory addresses
static float * const fvars = (void*) 0x2000e948;
static int * const ivars = (void*) 0x2000e750;

static float *cmd_ps = &fvars[0x29]; // (cmH2O)
static float *cmd_epap = &fvars[0x28]; // (cmH2O)
static float *cmd_ipap = &fvars[0x2a]; // (cmH2O) This is set to epap+ps somewhere else, no point in writing directly to it

static float *cmd_epap_ramp = &fvars[0x2d]; // (cmH2O) 

static const float *leak_basal = &fvars[0x22]; // I believe this to be basal unintentional leak (L/min)
static const float *leak = &fvars[0x24]; // Unintentional leak (L/min) - this is what flow_compensated incorporates
// const float *leak_b = &fvars[0x23]; // Also unintentional leak, but smaller amplitude, unsure of significance

static const float *flow_raw = &fvars[0x3]; // (L/min)
static const float *flow_patient = &fvars[0x0]; // (L/min)
static const float *flow_compensated = &fvars[0x25]; // (L/min)
// const float *flow_delayed = &fvars[0x26]; // Slightly delayed 0x25 ?

static const float *actual_pressure = &fvars[1]; // (cmH2O) Actual current pressure in the circuit
static const   int *therapy_mode = &ivars[0x6f]; // It's 0 when device is inactive
// Values of therapy_mode:
// 0 - Inactive
// 1 - CPAP
// 2 - APAP / AutoSet / AutoSet For Her
// 3 - VAuto
// 4 - S (EasyBreathe=Off) / ST / PAC
// 6 - iVAPS
// 8 - S (EasyBreathe=On)
// 9 - ASV / ASVAuto
static const   int *pap_timer = &ivars[0];

#define breath_progress (fvars[0x20]) // Inhale(0.0 to 0.5), Exhale(0.5 to 1.0). Breath duration-dependent. Works in S, VAuto modes, but not ASV

#define f_patient (fvars[0x0])
#define f_compensated (fvars[0x25])
#define f_unfucked (fvars[0x0] - fvars[0x22])

#define p_actual  (fvars[1])
#define p_command (fvars[0x2a])

#define p_error (fvars[1] - fvars[0x2a]) // Positive when above target

// These seem to be universal across modes, e.g. S, VAuto, etc.
#define sens_trigger fvars[0x7] // (L/min). Possible values: 13.2, 8.1, 4.8, 3.3, 1.8
#define sens_cycle fvars[0x8] // (%). Possible values: 0.5, 0.35, 0.25, 0.15, 0.08

// S mode configuration
#define s_ipap fvars[0xe]
#define s_epap fvars[0xf]
#define s_ips (s_ipap - s_epap)
#define s_rise_time_i ivars[0xD] // (80ms). 1, 18, 25, ... 106, 112. Seems to be in an unit of 80ms
#define s_rise_time_f (s_rise_time_i * 0.008f) // (1s). Ranges from 0.08 to 0.896
// VAuto mode configuration
#define vauto_max_ipap fvars[0x9]
#define vauto_epap fvars[0xa]
#define vauto_ps fvars[0xb]
#define vauto_ipap (vauto_epap + vauto_ps)
// 0x91 seems to contain epap-derived value

#define ti_min (ivars[0x5]*10) // (ms) integer
#define ti_max (ivars[0x6]*10) // (ms) integer

// I believe these are the reported EPAP and IPAP written to EDF files.
// However, they're probably written somewhere else(before end of inspiration), which needs to be debugged to write them
// Or used to figure out how to add extra signals..?
// float *report_epap = &fvars[0xC5];
// float *report_ipap = &fvars[0xC4];
// b9, be might be TV, ba, bb might be MV 

// const float *asv_epap_min = &fvars[0x11];
// const float *asv_epap_max = &fvars[0x10];
// const float *asv_ips_min  = &fvars[0x14];
// const float *asv_ips_max  = &fvars[0x13];

// Utility functions
#define min(a,b) ({ \
  __typeof__ (a) _a = (a); \
  __typeof__ (b) _b = (b); \
  _a < _b ? _a : _b; \
})

#define max(a,b) ({ \
  __typeof__ (a) _a = (a); \
  __typeof__ (b) _b = (b); \
  _a > _b ? _a : _b; \
})

#define clamp(a, _min, _max) ({ \
  __typeof__ (a) _a = (a); \
  __typeof__ (_min) __min = (_min); \
  __typeof__ (_max) __max = (_max); \
  _a > __max ? __max : (_a < __min ? __min : a); \
})

#define clamp01(a) ({ clamp(a, 0.0f, 1.0f) })

#define sign(a) ({a >= 0 ? 1 : -1; })

// Usage example: `inplace(max, &a, b)`
#define inplace(fn, ptr, args...) ({ \
  __typeof__ (ptr) _ptr = (ptr); \
  *_ptr = fn(*_ptr, args); \
})

#define GET_PTR(index, type, init_fn) ({ \
  get_pointer(index, sizeof(type), (void (*)(void*))&init_fn);\
})

//////////////////////////////////////
// Functions implemented in .c file //

float map(float s, float start, float end, float new_start, float new_end);
float mapc(float s, float start, float end, float new_start, float new_end); // Clamped to new_start-new_end
float map01(float s, float start, float end);
float map01c(float s, float start, float end); // Version that clamps to 0-1
float interp(float from, float to, float coeff);
float pow(float base, int exp);

typedef enum {
  PTR_HISTORY,
  PTR_SQUAREWAVE_DATA,
  PTR_TRACKING,
  PTR_ASV_DATA,

  __PTR_LAST,
} ptr_index;

void *get_pointer(ptr_index index, int size, void (*init_fn)(void*));


///////////////////////////
// History functionality //

#define HISTORY_LENGTH 16

// TODO: Expand history, make graph.c use it
typedef struct {
  int tick;
  int8 last_jitter;
  float last_time;
  float16 flow[HISTORY_LENGTH];
  // float16 cmd_ipap[HISTORY_LENGTH];
  // float16 pressure[HISTORY_LENGTH];
} history_t;

void init_history(history_t *hist);
void update_history(history_t *hist);
history_t *get_history();

void apply_jitter(bool undo);


///////////////////////////////
// All-purpose tracking code //

// Setup storage for important data
typedef struct {
  float volume;
  float volume_max;

  // New stuff
  float exh_maxflow;
  float inh_maxflow;
  int16 t; // (10ms ticks)
  float ti; // (s)
  float te; // (s)
} breath_t;

void init_breath(breath_t *breath);

typedef struct {
  float last_progress;
  uint32 last_time;
  uint32 breath_count;
  uint32 tick;

  bool st_inhaling : 1;
  bool st_just_started : 1;

  breath_t last;
  breath_t current;
} tracking_t;

void init_tracking(tracking_t *tr);
tracking_t* get_tracking();
void update_tracking(tracking_t *tr); 

#include "my_asv.h"

#endif