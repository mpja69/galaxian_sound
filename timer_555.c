#include "galaxian_board.h"
#include "timer_555.h"
#include <math.h>
#include <stdio.h>


// TODO: Check mixing stage. Right now we just add the signals
#define C20	  CAP_U(0.1)
#define R34	  RES_K(5.1) // (R34 exists on 2 places in the schematics: 15C and 14A

void timer_555_init(timer_555_astable_t* p, int sampleRate, double r1, double r2, double c) {
    p->tau1 =   (r1+r2) * c;
    p->tau2 =   r2 * c;
    p->time_low = p->tau2 * log(2);
    p->sampleRate = sampleRate;
    p->amplitude = 0.01;
}

static void timer_555_calculate_waveform(timer_555_astable_t* p) {
    if (p->cv >= 5) {
        p->duty_cycle = 0;
        p->advance  = 0;
        return;
    }
    double time_high = p->tau1 * log(1 + (p->cv / (2*(V_CC-p->cv))));
    p->duty_cycle = time_high / (time_high + p->time_low);
    p->advance = 1.0 / (p->sampleRate * (p->time_low + time_high));
}


void timer_555_update(timer_555_astable_t* p, double cv) {
    if (!p->enabled) { return; }  
    if (p->cv == cv) { return; }

    p->cv = cv;
    timer_555_calculate_waveform(p);
}

float timer_555_wavefunc(timer_555_astable_t* p)
{
    if (p->advance == 0) { return 0; }

    double f = p->time - (ma_int64)p->time;
    double r;

    if (f < p->duty_cycle) {
        r =  p->amplitude;
    } else {
        r = -p->amplitude;
    }

    p->time += p->advance;

    return (float)r;
}







// From $1898
// Sets the freq-bits depending on the time. Make the sound of the aliens "angrier" as the level goes on.
// Start at 15...slowly go to 0...and stay.


// NOTE: Withput pin 5 !!!
// 4 555 timer: -> square wave
// 1. Controlled by lfoFreqBits...but they also goes to OP-amp which then controls CV (control voltage) on the next 3!
// 2. Astable oscillating depending on voltage divider: 100/(100+470) and 0.01uF (Reset by FS1) Output over 10K res. 138Hz
// 3. Astable oscillating depending on voltage divider: 100/(100+330) and 0.01uF (Reset by FS2) ...189Hz
// 4. Astable oscillating depending on voltage divider: 100/(100+220) and 0.01uF (Reset by FS3) ... 267Hz
// Th = 0.693*(R1 + R2)*C1
// Tl = 0.693*R2*C1
//f = 1.44 / ((R1 + 2R2) * C1)
//
// NOTE: When the CV voltage increase -> the frequency decrease
// Usually the 
//
// Equations including cv:
// https://electronics.stackexchange.com/questions/101530/what-is-the-equation-for-the-555-timer-control-voltage

// Returns the value at i. From the Squarewave based on the parameters
//float timer_555_get_sample(lfoplayer *p, uint8_t i) {
    // Simulate a square wave, based on a 555-timer, and it's controllling resistors and capacitors, (and the control voltage)
    //
//}

