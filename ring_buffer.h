#include <stdint.h>

#ifndef RING_BUFFER_H_INCLUDED
#define RING_BUFFER_H_INCLUDED


typedef struct {
    float *buf;
    int bufSize;
    int rd;
    int wr;
    int sz;
} rb_t;


void rb_init(rb_t* p, float* pBuf, int bufSize);
void rb_write(rb_t* p, float data);
float rb_average(rb_t* p);
/*float rb_read(rb_t* p);*/
/*uint8_t rb_empty(rb_t* p);*/
/*uint8_t rb_full(rb_t* p);*/

#endif

