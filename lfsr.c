#include "lfsr.h"
#include <stdio.h>

// Main clock: XTAL:    18.432 MHz
// Z80 Clock: XTAL/6:   3.072 MHz
// Horz Count Bus CLOCK: XTAL / 3 = 6.144 MHz
// 1H:                  3.072 Mhz -> Z80 clock
// 2H:                  1.536 Mhz -> Pitch sound
// Horizontal video frequency: HSYNC = XTAL/3/192/2 = 16 kHz
// 
// Video frequency: VSYNC = HSYNC/132/2 = 60.606060 Hz
// HSYNC:       16 kHz
// 1V:          8 kHz
// 2V:          4 kHz
// -> samplar 4000 ggr per sekund: rng -> XTAL/3*2 Hz
//
// VBlank duration: 1/VSYNC * (20/132) = 2500 us
//
// VSYNC: (every new frame, get back to the top)
// HSYNC: (every new scanline, get back to the left)
//
// CPU_CLOCL: XTAL/6
// 

    	/************************************************/
    	/* NOISE                                        */
    	/************************************************/

    	/* since only a sample of the LFSR is latched @V2 we let the lfsr
    	 * run at a lower speed
    	 */
    	/*DISCRETE_LFSR_NOISE(NODE_150, 1, 1, RNG_RATE.dvalue()/100, 1.0, 0, 0.5, &galaxian_lfsr)*/
    	/*DISCRETE_SQUAREWFIX(NODE_151,1,60*264/2,1.0,50,0.5,0)  // 2V signal */
    	/*DISCRETE_LOGIC_DFLIPFLOP(NODE_152,1,1,NODE_151,NODE_150)*/

// 7474 (dual) D-FlipFlop, CLK: 2V, IN: shiftreg & 1, OUT: noise-bit -> extra -> CV
//



void lfsr_init(lfsr_t* lfsr) 
{
    // Fill the shift register before using it.
    for (int i=0; i<17; i++) {
	lfsr->shiftreg = (lfsr->shiftreg >> 1) | ((((lfsr->shiftreg >> 12) ^ ~lfsr->shiftreg) & 1) << 16);
    }
}


uint8_t lfsr_update(lfsr_t* lfsr) {
    lfsr->shiftreg = (lfsr->shiftreg >> 1) | ((((lfsr->shiftreg >> 12) ^ ~lfsr->shiftreg) & 1) << 16);
    return lfsr->shiftreg & 1;
}

/////////////////////////// Tried a ring buffer ... but scrapped the idea ///////////////////////

// Tick rate:	@60Hz
// Every tick, fill a buffer, (RNG_CLOCK/NOISE_CLOCK = 3072 bytes), with lfsr samples.
// From the callback, get samples from this buf
/*void lfsr_update(lfsr_t* lfsr) */
/*{*/
/*    // How much data do we want to write to have max 100 item*/
/*    int idx = lfsr->write;*/
/*    int d = RNG_BUF_SIZE - lfsr->size;*/
/*    for (int dd=0; dd < d; dd++) {*/
/*	//							    _   _     _   _*/
/*	// The RNG CLOCKK is: XTAL/3*2:  ~12MHz assymetric form:  _| |_| |___| |_| |___*/
/*	// the LFSR is fed based on the XOR of bit 12 and the inverse of bit 0 */
/*	lfsr->shiftreg = (lfsr->shiftreg >> 1) | ((((lfsr->shiftreg >> 12) ^ ~lfsr->shiftreg) & 1) << 16); */
/*	// Write to the ringbuffer*/
/*	int noise = lfsr->shiftreg & 1;*/
/*	lfsr->noiseBuf[idx] = noise; //lfsr->shiftreg & 1;*/
/*	idx = (idx+1) % RNG_BUF_SIZE;*/
/*	lfsr->size++;*/
/*    }*/
/*    lfsr->write = idx;*/
/*}*/

// Reads 1 byte from the noise buffer
// Returns -1 if the buffer is empty
/*int8_t lfsr_read_byte(lfsr_t* lfsr) {*/
/*    if (lfsr->size > 0) {*/
/*	// Read from the ringbuffer*/
/*	int8_t data = lfsr->noiseBuf[lfsr->read];*/
/*	lfsr->read = (lfsr->read+1) % RNG_BUF_SIZE;*/
/*	lfsr->size--;*/
/*	return data;*/
/*    }*/
/*    return -1;*/
/*}*/

// Reads the noise to a buffef of 96 bytes, RNG_BUF_SIZE 
// Returns the number of read bytes
/*int lfsr_read_buf(lfsr_t* lfsr, uint8_t* out) {*/
/*    int idx = lfsr->read;*/
/*    int d = lfsr->size;*/
/*    for (int dd=0; dd < d; dd++) {*/
/*	// Read from the ringbuffer*/
/*	out[dd] = lfsr->noiseBuf[idx];*/
/*	idx = (idx+1) % RNG_BUF_SIZE;*/
/*	lfsr->size--;*/
/*    }*/
/*    lfsr->read = idx;*/
/*    return d;*/
/*}*/

// vim: ts=4
