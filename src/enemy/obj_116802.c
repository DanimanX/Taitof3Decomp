#include "obj_116802.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"

/* ROM 0x116802: the Ameoba boss's entry tick (armed by 0x1166EA and by the
 * segment spawners, which reload 0x1167FA / 0x1167EE). It holds the boss alive
 * while +0x24 has HP and +0x3A the player target, then walks a three-phase
 * descent driven by the vertical scale at A5+0x30BC and the "diving" phase
 * counter +0x22. */

/* ROM 0x113456: per-frame body colour. While the global bit A5+0x1D58.1 is
 * set and the boss is on screen, the body colour follows the angle between the
 * boss (+0x14/+0x18) and the target (+0x2C/+0x30, biased by -0xA0/-0x74) with
 * the segment timer held at angle+0x58 for the duration of the call. */
static void sub_113456(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) == 0u)
        return;
    if ((int16_t)rd16(m, obj + 0x14u) < -0x0A || (int16_t)rd16(m, obj + 0x14u) > 0x11C)
        return;

    uint16_t saved = rd16(m, obj + 0x42u);
    uint16_t rng = Rng_NextMem(m);
    wr16(m, obj + 0x42u, (uint16_t)((rng & 0x0Fu) - 5u));

    uint16_t dx = (uint16_t)(rd16(m, obj + 0x14u) - 0xA0u);
    uint16_t dy = (uint16_t)(rd16(m, obj + 0x18u) - 0x74u);
    uint16_t ang = Obj_Atan2(m, dx, dy);
    wr16(m, obj + 0x42u, (uint16_t)(ang + 0x58u));
    Obj_BlinkColour(m, obj);
    wr16(m, obj + 0x42u, saved);
}

/* ROM 0x1169D8: one step of the phase +0x22 state machine. When bit 1 of
 * +0x51 is clear the object swings between "hide" (phase 0, aim angle latched
 * into +0x20) and "dive" (phases 1..0x14) while the shared vertical scale
 * A5+0x30BC is driven; when the bit is set it climbs back out again. */
static void sub_1169D8(const f3mem_t *m, uint32_t obj)
{
    int16_t phase;
    if (rd8(m, obj + 0x51u) & 0x02u) {
        /* 0x116AAA: rising phase, fade +0x3E, scale clamped at 0x3A00 */
        wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) + 0x10u));
        if ((int16_t)rd16(m, obj + 0x3Eu) > 0x400)
            wr16(m, obj + 0x3Eu, 0x0400u);
        wr16(m, F3_A5 + 0x30BCu, (uint16_t)(rd16(m, F3_A5 + 0x30BCu) + 0x100u));
        if ((int16_t)rd16(m, F3_A5 + 0x30BCu) > 0x3A00)
            wr16(m, F3_A5 + 0x30BCu, 0x3A00u);
        wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - 0x4000u);
        wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + 0x4000u);
        phase = (int16_t)(rd16(m, obj + 0x40u) + 1u);
        wr16(m, obj + 0x40u, (uint16_t)phase);
        if (phase > 0xDC) {
            wr16(m, obj + 0x40u, 0u);
            wr8(m, obj + 0x51u, (uint8_t)(rd8(m, obj + 0x51u) & (uint8_t)~0x02u));
        }
    } else {
        /* 0x1169E2: shrink the fade first */
        wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) - 0x10u));
        if ((int16_t)rd16(m, obj + 0x3Eu) < 0)
            wr16(m, obj + 0x3Eu, 0u);

        phase = (int16_t)rd16(m, obj + 0x22u);
        if (phase > 0) {
            if (phase <= 1) {
                /* 0x116A3E: 0x200/step toward the target until scale 0x5000 */
                wr16(m, F3_A5 + 0x30BCu, (uint16_t)(rd16(m, F3_A5 + 0x30BCu) + 0x200u));
                wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - 0x4000u);
                wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + 0x2000u);
                if ((int16_t)rd16(m, F3_A5 + 0x30BCu) >= 0x5000) {
                    wr16(m, F3_A5 + 0x30BCu, 0x5000u);
                    wr16(m, obj + 0x22u, (uint16_t)(phase + 1u));
                }
            } else {
                /* 0x116A6C: sink the scale, blink on alternate frames, then
                 * run the +0x42 timer down to -0x14 */
                wr16(m, F3_A5 + 0x30BCu, (uint16_t)(rd16(m, F3_A5 + 0x30BCu) - 0x400u));
                if ((int16_t)rd16(m, F3_A5 + 0x30BCu) < -0x800)
                    wr16(m, F3_A5 + 0x30BCu, 0xF800u);
                Obj_BlinkColour(m, obj);
                if ((rd16(m, F3_A5 + 0x878u) & 1u) == 0u) {
                    int16_t t = (int16_t)(rd16(m, obj + 0x42u) - 1u);
                    wr16(m, obj + 0x42u, (uint16_t)t);
                    if (t <= 0)
                        wr16(m, obj + 0x22u, 0xFFECu);
                }
            }
        } else if (phase < 0) {
            /* 0x116A1C: rise back out */
            wr16(m, obj + 0x22u, (uint16_t)(phase + 1u));
            wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - 0x4000u);
            wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + 0x4000u);
        } else {
            /* 0x1169FC: latch the aim angle at the target and start a dive */
            uint16_t dx = (uint16_t)(rd16(m, obj + 0x2Cu) - rd16(m, obj + 0x14u));
            uint16_t dy = (uint16_t)(rd16(m, obj + 0x30u) - rd16(m, obj + 0x18u));
            uint16_t ang = Obj_Atan2(m, dx, dy);
            wr16(m, obj + 0x20u, ang);
            wr16(m, obj + 0x42u, 0x14u);
            wr16(m, obj + 0x22u, (uint16_t)(rd16(m, obj + 0x22u) + 1u));
            wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - 0x4000u);
            wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + 0x4000u);
        }
    }

    /* 0x116B02: the fade word feeds the global vertical scale */
    wr16(m, F3_A5 + 0x30BAu, (uint16_t)(rd16(m, F3_A5 + 0x30BAu) + rd16(m, obj + 0x3Eu)));
}

/* ROM 0x1167EE: re-arm the boss with the 0xB4 timer and the target record */
static void lab_1167EE(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x06u, 0x00B4u);
    wr32(m, obj + 0x3Au, rd32(m, F3_A5 + 0x1D28u));
    wr32(m, obj + 0x00u, 0x00116802u);          /* 0x1167FA */
}

/* ROM 0x1169A0: retire the boss -- both sprite chains released, the "gone"
 * bit latched on +0x50 and the 0x1169CA delay state installed */
static void lab_1169A0(const f3mem_t *m, uint32_t obj)
{
    wr8(m, obj + 0x50u, (uint8_t)(rd8(m, obj + 0x50u) | 0x20u));
    Obj_Free(m, rd32(m, obj + 0x08u));
    Obj_Free(m, rd32(m, obj + 0x0Cu));
    wr32(m, obj + 0x00u, 0x001169CAu);
}

/* ROM 0x11697A (reached from 0x1169CA): pause the boss and park it for 0x78. */
static void lab_11697A(const f3mem_t *m, uint32_t obj)
{
    wr8(m, obj + 0x51u, 0u);
    wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
    wr8(m, obj + 0x50u, (uint8_t)(rd8(m, obj + 0x50u) | 0x10u));
    wr16(m, obj + 0x06u, 0x0078u);
    wr32(m, obj + 0x00u, 0x00116992u);          /* 0x116990 beq -> 0x116992 */
}

/* ROM 0x1168E2: the boss died -- hand the body to the death effect 0x1176B4
 * and switch to the 0x4B0 delay before the pause state. */
static void lab_1168E2(const f3mem_t *m, uint32_t obj)
{
    Obj_Spawn(m, obj, 0x001176B4u, 3u, 2u, rd32(m, obj + 0x36u));
    wr8(m, obj + 0x51u, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr16(m, obj + 0x40u, 0u);
    wr16(m, obj + 0x22u, 0u);
    wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
    wr16(m, obj + 0x06u, 0x04B0u);
    wr32(m, obj + 0x00u, 0x00116922u);          /* 0x11691A */
}

int Obj_116802_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x116802: paused, or without HP / target -> pause the boss */
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) || (int32_t)rd32(m, obj + 0x24u) <= 0) {
        lab_11697A(m, obj);
        return 0;
    }

    /* 0x116814: bit 3 of +0x50 is the dying state */
    if (rd8(m, obj + 0x50u) & 0x08u) {
        lab_1168E2(m, obj);
        return 0;
    }

    /* 0x11681E */
    Obj_BlinkColour(m, obj);

    if (rd8(m, obj + 0x50u) & 0x01u) {
        /* 0x11682E: dive -- pull the shared scale down to -0x800 */
        wr16(m, F3_A5 + 0x30BCu, (uint16_t)(rd16(m, F3_A5 + 0x30BCu) - 0x100u));
        if ((int16_t)rd16(m, F3_A5 + 0x30BCu) < -0x800)
            wr16(m, F3_A5 + 0x30BCu, 0xF800u);
    } else {
        /* 0x116848: shadow the target at +0x8C/+0x10 and count the relatch
         * timer before starting a fresh dive toward 0x258/0x50 */
        uint32_t a1 = rd32(m, obj + 0x3Au);
        wr32(m, obj + 0x2Cu, rd32(m, a1 + 0x14u) + 0x8Cu);
        wr32(m, obj + 0x30u, rd32(m, a1 + 0x18u) + 0x10u);
        int16_t t = (int16_t)(rd16(m, obj + 0x56u) - 1u);
        wr16(m, obj + 0x56u, (uint16_t)t);
        if (t <= 0) {
            if ((int16_t)rd16(m, obj + 0x14u) >= 0x1B8) {
                lab_1169A0(m, obj);             /* 0x116872 bge -> 0x1169A0 */
                return 0;
            }
            wr16(m, obj + 0x2Cu, 0x0258u);
            wr16(m, obj + 0x30u, 0x0050u);
        }
        sub_1169D8(m, obj);                      /* 0x116882 */
    }

    /* 0x116886: bit 2 of +0x50 raises the scale toward -0x8000 */
    if (rd8(m, obj + 0x50u) & 0x04u) {
        wr16(m, F3_A5 + 0x30BEu, (uint16_t)(rd16(m, F3_A5 + 0x30BEu) + 0x555u));
        if ((uint16_t)(rd16(m, F3_A5 + 0x30BEu) - 0x8000u) != 0u)
            wr16(m, F3_A5 + 0x30BEu, 0x8000u);
    }

    /* 0x1168A6 */
    sub_113456(m, obj);
    if ((int16_t)rd16(m, obj + 0x14u) > 0xA0 && (rd16(m, F3_A5 + 0x878u) & 0x1FFu) == 0u)
        wr8(m, obj + 0x51u, (uint8_t)(rd8(m, obj + 0x51u) | 0x02u));

    /* 0x1168C8: lifetime timer */
    int16_t life = (int16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)life);
    if (life > 0) {
        wr32(m, obj + 0x00u, 0x00116802u);       /* 0x1167FA */
        return 0;
    }
    if ((int16_t)rd16(m, obj + 0x56u) > 0)
        wr8(m, obj + 0x51u, (uint8_t)(rd8(m, obj + 0x51u) | 0x01u));
    lab_1167EE(m, obj);
    return 0;
}
