#include "pitch.h"
#include <stdint.h>
#include <stdio.h>

void pitch_init(pitch_t* pp, int sampleRate)
{
    pp->pitch = 0xFF;
    pp->sampleRate = sampleRate;
    pp->amplitude = 0.1;
}

static double getFrequency(int pitch) {
    if (pitch >= 256) {
	return 0;
    }
	// XTAL:	18.432 MHz
	// CPU_CLOCK:	XTAL / 6	(3.072 MHz)
	// SOUND_CLOCK:	CPU_CLOCK / 2	(1.536 MHz) // 2 is 2H
	// SOUND_CLOCK:	1_536_000
    // frequency = SOUND_CLOCK / 16 / (256 - pitch) // 16 is a 16 bits counter
    return 1536000.0 / 16.0 / (256 - pitch);
}

void pitch_set_pitch(pitch_t* pp, uint8_t pitch)
{
    if (pitch == pp->pitch) { return; }
    pp->pitch = pitch;

    if (pp->pitch >= 0xFF) { 
	pp->advance = 0;
	return;
    }
    double frequency = getFrequency(pitch);
    pp->advance = 1.0 / (pp->sampleRate / frequency);
}

void pitch_set_vol(pitch_t* pp, uint8_t vol1, uint8_t vol2) {
	pp->vol1 = vol1;
	pp->vol2 = vol2;
}

// ======================== To generate the waveform ======================

#define R34	RES_K(5.1) 
#define R49	RES_K(10) 
#define R50	RES_K(22) 
#define R51	RES_K(33) 
#define R52	RES_K(15) 


// Calculate audio voltages from resistors
#define V_A   (R34 / (R51 + R34))
#define V_C0  (R34 / (R50 + R34))
#define V_C1  (R34 / (R50*R51/(R50*R51) + R34))
#define V_D   (R34 / (R52 + R34))

// Emulating the value from the 4-bit binary counter 74393, (6T), in conbination with the voltage dividers 
// Each clock pulse will increment the binary output, BUT here we just give a value 0..15
// MSB			LSB
// QD	QC	CB	QA
// 0	0   0   0
// 0	0   0   1
// 0	0   1   0
// 0	0   1	1	
// 0	1	0	0	
// ...and so on
// ====== Bilateral Analog Switch 4066, (7R) ====
// "Vol1" controlls QC: 0 -> 22kOhm,	1 -> (22*10)/(22+10) == 220/32 ≈ 7kOhm
// "Vol2" controlls QD: 0 -> 0Ohm,		1 -> 15kOhm
//
// TODO: Räkna om värdena för a, b och c
static float pitch_get_sample(pitch_t* pp, uint8_t i) {
	int qa = i & 1;
	int qb;	// not used
	int qc = i>>2 & 1;
	int qd = i>>3 & 1;
	
	double a = qa * V_A;
	double c = qc * (pp->vol1 * (V_C1-V_C0) + V_C0);
	double d = qd * pp->vol2 * V_D;

	double out = (a+c+d);// * 6.0 - 1.0;
	return out * pp->amplitude;
}

float pitch_wavefunc(pitch_t* pp)
{
    float val = 0;
    int i = (pp->time - (int)pp->time) * 16;
    val = pitch_get_sample(pp, i);
    pp->time += pp->advance;
    return val;
}



//vim: ts=4
