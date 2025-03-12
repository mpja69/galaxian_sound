#include <stdint.h>
#include "lfsr.h"
#include "ring_buffer.h"

typedef struct {
    int enable;		// Is hold....
    double time;	// The current time (in seconds) 
    double envelope;	// The voltage on capacitor C25
    double advance;	// The delta time each sample @ the sample rate
    int sampleRate;	// The sound device's sample rate
    lfsr_t* lfsr;
    double amplitude;
    
    // Has an actual ringbuffer struct
    rb_t rb;
} hit_t;


void hit_init(hit_t* p, int sampleRate, lfsr_t* lfsr);
void hit_set_enable(hit_t* p, int enable);
void hit_update(hit_t* p);
float hit_wavefunc(hit_t* p);

