#ifndef TAITOF3_COMMON_F3REGS_H
#define TAITOF3_COMMON_F3REGS_H

#include <stdint.h>

typedef struct {
    uint32_t d[8];
    uint32_t a[8]; /* a[7] is the active SP */
    uint32_t pc;
    uint16_t sr;
} f3regs_t;

#endif
