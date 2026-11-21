/* Darius Gaiden (Taito F3) — kernel PRNG.
 * ROM: 0x001D8A  Rng_Next
 *
 *   move.l  (-0x1456,A5),D0      ; rng_seed @ 0x406BAA
 *   muls.l  #7,D0                ; low 32 bits of signed product
 *   addq.l  #5,D0
 *   move.l  D0,(-0x1456,A5)
 *   swap    D0                   ; D0.w = high half of new seed
 *   rts
 *
 * Called from the idle loop (Kernel_IdleHook, 0x001A72), so the number of
 * steps per frame depends on spare CPU time.
 */
#include "rng.h"

uint32_t rng_seed; /* 0x406BAA */

uint32_t rng_step(uint32_t seed)
{
    /* muls.l keeps the low 32 bits; identical to unsigned wraparound. */
    return seed * 7u + 5u;
}

uint32_t Rng_Next(void)
{
    rng_seed = rng_step(rng_seed);
    return (rng_seed << 16) | (rng_seed >> 16); /* swap D0 */
}
