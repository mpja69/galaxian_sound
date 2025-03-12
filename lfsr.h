#include <stdint.h>

#ifndef LFSR_H_INCLUDED
#define LFSR_H_INCLUDED


/*#define XTAL		18432000*/
/*#define RNG_CLOCK	(XTAL / 3 * 2)*/
/*#define HSYNC		16000*/
/*#define NOISE_CLOCK	(HSYNC / 4)*/
/*#define RNG_BUF_SIZE	(RNG_CLOCK / NOISE_CLOCK / 32)*/

typedef struct {
    uint32_t shiftreg;
    int read;
    int write;
    int size;
} lfsr_t;


void lfsr_init(lfsr_t*); 
uint8_t lfsr_update(lfsr_t*);

#endif
