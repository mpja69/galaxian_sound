#include <stdint.h>

#ifndef LFSR_H_INCLUDED
#define LFSR_H_INCLUDED


/*#define XTAL		18432000*/
/*#define RNG_CLOCK	(XTAL / 3 * 2)*/
/*#define HSYNC		16000*/
/*#define NOISE_CLOCK	(HSYNC / 4)*/

typedef struct {
    uint32_t shiftreg;
    uint8_t noise;
} lfsr_t;


void lfsr_init(lfsr_t* p); 
void lfsr_update(lfsr_t* p);
uint8_t lfsr_get_noise(lfsr_t* p);

#endif
