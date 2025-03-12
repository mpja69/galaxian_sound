#include "ring_buffer.h"

void rb_init(rb_t* p, float* pBuf, int bufSize) {
    p->buf = pBuf;
    p->bufSize = bufSize;
}

/*uint8_t rb_empty(rb_t* p) {*/
/*    return p->sz == 0;*/
/*}*/

/*uint8_t rb_full(rb_t* p) {*/
/*    return p->sz == p->bufSize;*/
/*}*/

void rb_write(rb_t* p, float data)  {
    /*if (rb_full(p)) { return; }*/

    p->buf[p->wr] = data;
    p->wr = (p->wr+1) % p->bufSize;
    p->sz++;
}

float rb_average(rb_t* p) {
    float tot = 0;
    for (int i=0; i<p->bufSize; i++) {
	tot += p->buf[i]; 
    }
    return tot / p->bufSize;
}

/*float rb_read(rb_t* p) {*/
/*    if (rb_empty(p))  { return 0; }*/
/**/
/*    int8_t data = p->buf[p->rd];*/
/*    p->rd = (p->rd+1) % p->bufSize;*/
/*    p->sz--;*/
/*    return data;*/
/*}*/



//vim: ts=4
