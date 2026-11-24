#include "obj_125016.h"

/* ROM 0x11B2F6: clear the per-life counters and flags. */
static void sub_11B2F6(const f3mem_t *m, uint32_t a0)
{
    wr16(m, a0 + 0x1Cu, 0u);
    wr16(m, a0 + 0x1Eu, 0u);
    wr32(m, a0 + 0x28u, 0x00010000u);
    wr32(m, a0 + 0x24u, 0u);
    wr16(m, a0 + 0x34u, 0u);
    wr16(m, a0 + 0x3Au, 0u);
    wr16(m, a0 + 0x3Eu, 0u);
    wr8(m, a0 + 0x4Fu, 0u);
}

/* ROM 0x11B31C: load the spawn record at A1 into the object. */
static void sub_11B31C(const f3mem_t *m, uint32_t a0, uint32_t a1)
{
    wr16(m, a0 + 0x50u, rd16(m, a1));
    wr16(m, a0 + 0x52u, rd16(m, a1 + 2u));
    wr16(m, a0 + 0x54u, 0u);
    wr16(m, a0 + 0x56u, 0u);
    wr16(m, a0 + 0x2Cu, (uint16_t)(int16_t)(int8_t)rd8(m, a1 + 4u));
    wr16(m, a0 + 0x30u, (uint16_t)(int16_t)(int8_t)rd8(m, a1 + 5u));
    wr16(m, a0 + 0x2Eu, (uint16_t)(int16_t)(int8_t)rd8(m, a1 + 6u));
    wr16(m, a0 + 0x32u, (uint16_t)(int16_t)(int8_t)rd8(m, a1 + 7u));
}

/* ROM 0x11B2F6 + 0x11B31C + move.l A1,(0xC,A0): the shared spawn prelude.
 * The two jsr's preserve every register but A0, so A1 still points past the
 * 8-byte record when the caller stores it. */
static void spawn_prelude(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1;
    sub_11B2F6(m, obj);
    a1 = rd32(m, obj + 0x0Cu);
    sub_11B31C(m, obj, a1);
    wr32(m, obj + 0x0Cu, a1 + 8u);
}

/* ROM 0x125016: spawn initialiser. Leaves the +0x24/+0x34/+0x3A/+0x3E state
 * that 0x11B2F6 cleared, and hands control to 0x125046. */
int Obj_125016_Tick(const f3mem_t *m, uint32_t obj)
{
    /* ROM 0x125016 */
    spawn_prelude(m, obj);
    /* ROM 0x12502A */
    wr32(m, obj + 0x28u, 0x00010000u);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u));   /* bset.b #1 */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x08u));   /* bset.b #3 */
    /* ROM 0x12503E */
    wr32(m, obj + 0x00u, 0x00125046u);
    return 0;
}
