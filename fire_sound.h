#include <stdint.h>
#include "lfsr.h"

typedef struct {
    int enable;		// Is hold high for 8 ticks when the user fire.
    double time;	// The current time (in seconds) 
    double envelope;	// The voltage on capacitor C25
    double c28_voltage;	// The voltage on capacitor C28,  for the CV-Decay
    double advance;	// The delta time each sample @ the sample rate
    int sampleRate;	// The sound device's sample rate
} fire_t;


void fire_init(fire_t* pf, int sampleRate);
void fire_set_enable(fire_t* p, int enable);
void fire_update(fire_t* pf);
float fire_wavefunc(fire_t* pf);
