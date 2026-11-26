#ifndef TAITOF3_LINEFX_176390_H
#define TAITOF3_LINEFX_176390_H
#include "../common/f3mem.h"
/* ROM 0x176390: perspective band into the line tables A0..A3 (D0 = height,
 * D1 = depth, D7 = first row). */
void LineFx_176390(const f3mem_t *m, uint32_t d0, uint32_t d1, uint32_t d7,
                   uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3);
#endif
