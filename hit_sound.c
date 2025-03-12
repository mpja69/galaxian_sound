#include "galaxian_board.h"
#include "hit_sound.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>


#define ATTACK_TIME		(R90 * C21)	// 0.1 second 
#define DECAY_TIME		((R35+R36) * C21)	// 0.1 second 

#define RB_SIZE 128
static float ringBuffer[RB_SIZE];

void hit_init(hit_t* p, int sampleRate, lfsr_t* lfsr) {

    p->lfsr = lfsr; 
    p->sampleRate = sampleRate;
    p->advance = (1.0 / sampleRate);
    p->time = 9;
    p->amplitude = 0.9;

    rb_init(&(p->rb), (float*)&ringBuffer, RB_SIZE);

}


void hit_set_enable(hit_t* p, int enable) {
    if (p->enable != enable) {
	p->enable = enable;
	p->time = 0;
    }
}

void hit_update(hit_t* p) {
    if (p->time > 2) {return;}
    
    if (p->enable) {
	p->envelope =  (1 - exp(-p->time/0.0022));   // enable,  T = R89 * C21 = 1k * 2.2u 
    } else {
	p->envelope =  exp(-p->time/0.3784);	    // !enable, T = (R35+R36) * C21 = 172k * 2.2u
    }

    printf("Env: %f\n", p->envelope);
}


float hit_wavefunc(hit_t *p) {
    if (p->time > 2) {
	return 0;
    }
    p->time += p->advance; 

    // The acutal noise,  (together with envelope)
    float data =  lfsr_update(p->lfsr) * p->envelope * p->amplitude;

    // Using a floating window buffer, a low pass filter, by smoothiing it out by calculating the average.
    rb_write(&(p->rb), data);
    return rb_average(&(p->rb));
}

