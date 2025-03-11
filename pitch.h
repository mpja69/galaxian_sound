#include "miniaudio.h"
#include "galaxian_board.h"
#include <stdint.h>

// This is one part of the audio system, responsible for:
//		1. Playing melodies:
//			a) Start Game
//			b) Alien Death
//			c) Flagship Death
//		2. Credit inserted
//		3. Extra Life,
//		4. Alien Attack
//			a) Wheee sound, whenever an alien starts to attack
//			b) Ocsillating bells, when alien reaches bottom of screen
//
//	Only one of these can play simultaneoysly!??
//	(They seem not to be able to be mixed, since each of them sets a specific pitch...in the shared circuit.)
typedef struct {
	int vol1;
	int vol2;
	int pitch;
	double amplitude;
    double time;
    double advance;
    int sampleRate;
} pitch_t;

void pitch_init(pitch_t* pp, int sampleRate);
float pitch_wavefunc(pitch_t* pp);
void pitch_set_pitch(pitch_t* pp, uint8_t pitch);



// ======================== To generate the waveform ======================

/*#define R34			RES_K(5.1) */
/*#define R49			RES_K(10) */
/*#define R50			RES_K(22) */
/*#define R51			RES_K(33) */
/*#define R52			RES_K(15) */


// Calculate audio voltages from resistors
/*#define V_A   (R34 / (R51 + R34))*/
/*#define V_C0  (R34 / (R50 + R34))*/
/*#define V_C1  (R34 / (R50*R51/(R50*R51) + R34))*/
/*#define V_D   (R34 / (R52 + R34))*/


// ========================== Internal stuff that isn't needed ================

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



//void player_generateWavedata(player_complex *p) {
//	for (int i = 0; i < 16; i++ ){
//		p->waveData[i] = player_resistor_dac(p, i);
//	}
//}

/*int getPitch(int note);*/
/*int getFrequency(int pitch);*/
/*int getToneLength(int note);*/

