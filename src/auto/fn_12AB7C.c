#include "fn_auto.h"
/* ROM 0x12AB7C: UNVERIFIED (weak capture): Consume a linked object and reverse its motion when its state allows. */
#include "fn_sound_frames.h"

int Fn_12AB7C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    push_long(m, r, r->a[1]); /* 0x12AB7C MOVEM */
    push_long(m, r, r->d[0]);
    uint32_t linked = rd32(m, obj + 0x36u);
    r->d[0] = linked;
    if (linked == 0)
        goto done;

    wr32(m, obj + 0x36u, 0);
    if (rd8(m, obj + 0x35u) == 2)
        goto done;
    r->a[1] = linked;
    uint8_t state = rd8(m, linked + 0x22u);
    if (state == 0 || state == 4 || state == 5 || state == 6)
        goto done;

    int32_t speed = (int32_t)rd32(m, linked + 0x48u);
    r->d[0] = (uint32_t)speed;
    if (speed <= 0)
        goto done;
    wr32(m, linked + 0x48u, (uint32_t)(-speed >> 3)); /* 0x12ABC8 NEG.L, ASR.L */
    push_long(m, r, 0x12ABD6u);
    r->d[0] = (uint32_t)(int32_t)(int16_t)Rng_NextMem(m);
    r->a[7] += 4u;
    wr32(m, linked + 0x4Cu, r->d[0]);
    wr8(m, linked + 0x1Cu, rd8(m, linked + 0x1Cu) | 1u);
    call_hooked_sound(m, r, 0x0Fu, 0x12ABECu, 0x12ABF2u);

done: /* 0x12ABF6 MOVEM and RTS */
    r->d[0] = rd32(m, r->a[7]);
    r->a[1] = rd32(m, r->a[7] + 4u);
    r->a[7] += 12u;
    return 0;
}
