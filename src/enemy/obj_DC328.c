#include "obj_DC328.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../audio/sound_cmd.h"

/* ROM 0x0D2D44: D0 = speed.w, D1 = angle (0..0x168). Reads the 16.16
 * (dx,dy) vector for the angle from tables 0x0D2D64/0x0D2ECC and scales it by
 * a 51-entry angle-dependent factor (jump table 0x0D4F26). Returns -1 for an
 * angle index past 50 (the table's end), leaving the caller's state alone. */
static int sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                      uint32_t *out_d0, uint32_t *out_d1)
{
    uint16_t d2w = d1w;
    uint32_t off = (uint32_t)(int32_t)(int16_t)d0w * 4u;
    uint32_t d1 = rd32(m, 0x000D2ECCu + off);
    uint32_t d0 = rd32(m, 0x000D2D64u + off);

    /* Jump table 0x0D4F26 [0..50] -> 0x0D4FF2..0x0D538A: exact (D0*k, D1*k)
     * mod 2^32. */
        /* Jump table 0x0D4F26 [0..50] -> 0x0D4FF2..0x0D538A: block k is the
         * constant 0..50 (k=0 at 0x0D4FF2 clears both), so (D0*k, D1*k)
         * mod 2^32 is exact for every k the table has. */

    *out_d0 = (uint32_t)(d0 * (uint32_t)d2w);
    *out_d1 = (uint32_t)(d1 * (uint32_t)d2w);
    return 0;
}

/* ROM 0x0CEDE4: init the animation/state block of obj from the template at
 * A1 (fields live 0x14 bytes into the template). */
static void sub_0CEDE4(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    a1 += 0x14u;
    wr32(m, obj + 0x1Cu, 0u);
    wr32(m, obj + 0x24u, 0u);
    wr16(m, obj + 0x24u, rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x28u, 0u);
    wr16(m, obj + 0x28u, rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x2Cu, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x30u, rd32(m, a1));
    wr16(m, obj + 0x34u, 0u);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u));
    wr16(m, obj + 0x06u, 0u);
    wr16(m, obj + 0x20u, 0u);
    wr16(m, obj + 0x22u, 0u);
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x14u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x18u));
    wr8(m, obj + 0x4Fu, 0u);
}

/* ROM 0x0D0E3A: 16-bit LCG step at A5+0x1D60 (n = n*5+1), D1 preserved;
 * returns the high byte of the new seed. */
static uint16_t sub_0D0E3A(const f3mem_t *m)
{
    uint16_t d0 = rd16(m, F3_A5 + 0x1D60u);
    uint16_t d1 = (uint16_t)(d0 << 2);
    d0 = (uint16_t)(d0 + d1 + 1u);
    wr16(m, F3_A5 + 0x1D60u, d0);
    return (uint16_t)(d0 >> 8);
}

/* ROM 0x0DC076: second half of the 0x0DC328 tail. Copy the current player
 * (A0+0x08) into obj+0x3A/0x3E i.e. move obj+/+ world position 0x14/0x18 to
 * screen (-0x18 each, the bullet is centred on the player), pick one of three
 * 0x0D0E3A-weighted animation templates by size (D0 > 0x33 / > 0x99), start
 * it, set the 16.16 velocity from the animation angle obj+0x58,obj+0x60 via
 * 0x0D2D44, un-scroll Y and draw; then hand over to 0x0DC0F8. */
static int tail_0DC076(const f3mem_t *m, uint32_t obj)
{
    /* 0x0DC076 */
    if (rd16(m, obj + 0x58u) > 0x0167u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    if (rd16(m, obj + 0x60u) > 0x0032u) {
        Obj_StartDying(m, obj);
        return 0;
    }

    /* 0x0DC08E */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x0018u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 0x0018u));

    /* 0x0DC09A: lea (-0x30ba,PC),A1 -> PC = 0x0DC09C -> 0x0D8FE2 */
    sub_0CEDE4(m, obj, 0x000D8FE2u);

    /* 0x0DC0A4: lea (0x7e,PC),A1 -> 0x0DC124 */
    uint16_t d0 = sub_0D0E3A(m);
    uint32_t a1 = 0x000DC124u;
    if (d0 > 0x0033u) {
        /* 0x0DC0B6: lea (0x114,PC),A1 -> 0x0DC1CC */
        a1 = 0x000DC1CCu;
        if (d0 > 0x0099u) {
            /* 0x0DC0C2: lea (0x174,PC),A1 -> 0x0DC238 */
            a1 = 0x000DC238u;
        }
    }

    /* 0x0DC0C6: start the animation sequence at A1 */
    wr32(m, obj + 0x48u, a1);
    wr16(m, obj + 0x4Cu, rd16(m, a1));
    a1 += 2u;
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));

    /* 0x0DC0CC */
    uint16_t d1w = rd16(m, obj + 0x60u);
    uint32_t dd0, dd1;
    if (sub_0D2D44(m, rd16(m, obj + 0x58u), d1w, &dd0, &dd1) < 0)
        return -1;
    wr32(m, obj + 0x50u, dd0);
    wr32(m, obj + 0x54u, dd1);

    /* 0x0DC0E2: draw at the world Y and then undo the frame scroll */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - rd32(m, F3_A5 + 0x4228u));
    Obj_DrawSprite(m, obj);

    /* 0x0DC0F0 */
    wr32(m, obj + 0x00u, 0x000DC0F8u);
    return 0;
}

/* ROM 0x0DC328 */
int Obj_DC328_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0DC328: pea #0x33; jsr 0x13F238 (sound pre-cmd hook);
     * jsr 0x39D8 (SFX 0x33) */
    int16_t cmd = 0x0033;
    if (rd8(m, F3_A5 + 0x3306u) != 0u)                /* 0x13F238 */
        cmd = (int16_t)(cmd + 0x53);
    if (m->dpram) {
        Sound_PlayCmd(m, cmd);
    } else {
        uint8_t tmp_dpram[F3_DPRAM_SIZE] = {0};
        f3mem_t tmp_m = *m;
        tmp_m.dpram = tmp_dpram;
        Sound_PlayCmd(&tmp_m, cmd);
    }

    /* 0x0DC33C: bra 0x0DC076 */
    return tail_0DC076(m, obj);
}
