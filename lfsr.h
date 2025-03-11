#include <stdint.h>

#define XTAL		18432000
#define RNG_CLOCK	(XTAL / 3 * 2)
#define HSYNC		16000
#define NOISE_CLOCK	(HSYNC / 4)
#define RNG_BUF_SIZE	(RNG_CLOCK / NOISE_CLOCK / 32)
// Buffer needs to be about 80 bytes. Now we have 96 above


typedef struct {
    uint32_t shiftreg;
    uint8_t noiseBuf[RNG_BUF_SIZE];
    int read;
    int write;
    int size;
} lfsr_t;

void lfsr_init(lfsr_t*); 
uint8_t lfsr_update(lfsr_t*);
/*void lfsr_update(lfsr_t*); */
/*int8_t lfsr_read_byte(lfsr_t*);*/
/*int lfsr_read_buf(lfsr_t* lfsr, uint8_t* out);*/
