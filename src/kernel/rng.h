#ifndef TAITOF3_KERNEL_RNG_H
#define TAITOF3_KERNEL_RNG_H
#include <stdint.h>

extern uint32_t rng_seed;            /* 0x406BAA */
uint32_t rng_step(uint32_t seed);    /* pure LCG step */
uint32_t Rng_Next(void);             /* ROM 0x001D8A; returns swapped seed (D0) */

#endif
