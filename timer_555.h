#include <stdint.h>
#include "miniaudio.h"


typedef struct {
    double tau1;
    double tau2;
    double cv;			// The control voltage based on the frequency bits, that controls the 3 555-timers
    int enabled;		// HANDLE_ALIEN_AGGRESSIVENESS, enables/dissables the 3 different 555-timers
    //
    // These are related to the waveform
    double time_high;
    double time_low;
    double duty_cycle;
    double amplitude;
    double time;
    double advance;
    int sampleRate;
} timer_555_astable_t;


void timer_555_init(timer_555_astable_t* pt, int sampleRate, double r1, double r2, double c);

// Should be called as ofter as possible?
void timer_555_update(timer_555_astable_t* pt, double cv);


// Called from the "callback"
float timer_555_wavefunc(timer_555_astable_t* pt);
