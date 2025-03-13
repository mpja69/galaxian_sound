// FIRE - SOUND 
//  - Samla ihop de kretsar som krävs
//	- Noise
//	- 
//
//  - Har en callback för att generera vågformen
//	-
//

#include "galaxian_board.h"
#include "fire_sound.h"
#include "timer_555.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>


// TODO: ATTACK time....and use these
#define AMPLITUDE_DECAY_TIME	(R41 * C25)	// 0.1 second

#define CV_ATTACK_TIME		(R47 * C28)	// 0.1 second 
#define CV_DECAY_TIME		(R48 * C28)	// 0.1 second 

static timer_555_astable_t timer = {0};

void fire_init(fire_t* p, int sampleRate, lfsr_t* lfsr) {
    
    timer_555_init(&timer, sampleRate, R44, R45, C27);
    timer.enabled = 1;
    timer.amplitude = 0.2;

    /*lfsr_init(&lfsr);*/
    
    p->sampleRate = sampleRate;
    p->advance = (1.0 / sampleRate);
    p->time = 5;
    p->lfsr = lfsr;
}


void fire_set_enable(fire_t* p, int enable) {
    if (p->enable != enable) {
	p->enable = enable;
	p->time = 0;
    }
}

void fire_update(fire_t* p) {
    if (p->time > 1) {return;}
    
    if (p->enable) {
	p->envelope =  (1 - exp(-p->time/0.001));   // enable,  T = R90 * C25 = 1k * 1u 
	p->c28_voltage = 4 * exp(-p->time/0.05);	    // !enable, T = Par(R47, R48) * C28 = 1.1k * 47u 
    } else {
	p->envelope =  exp(-p->time/0.1);	    // !enable, T = R41 * C25 = 100k * 1u
	p->c28_voltage = 4 * (1 - exp(-p->time/0.1)); // enable,  T = R47 * C28 = 2.2k * 47u 
    }
}


float fire_wavefunc(fire_t *p) {
    if (p->time > 1) {
	return 0;
    }
    p->time += p->advance; 

    double cv  = lfsr_get_noise(p->lfsr) + p->c28_voltage;
    timer_555_update(&timer, cv); // 1 -> High freq,  4 -> mid freq, 4.999 ->low
    return timer_555_wavefunc(&timer) * p->envelope;
}
