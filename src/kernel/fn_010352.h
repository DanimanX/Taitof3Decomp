#ifndef TAITOF3_KERNEL_FN_010352_H
#define TAITOF3_KERNEL_FN_010352_H
#include "../common/f3mem.h"
/* ROM 0x010352: draw the FREE PLAY / CREDIT line on the title text layer
 * (0x61CE3E) and the credit counter after it. */
void Kernel_CreditLine010352(const f3mem_t *m);

/* ROM 0x005666: {col*2, char} pairs up to a NUL byte into text RAM at dst;
 * returns the address after the last pair. */
uint32_t Kernel_005666(const f3mem_t *m, uint32_t src, uint32_t dst, uint16_t col);
#endif
