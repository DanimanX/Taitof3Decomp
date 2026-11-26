/* ROM 0x0F61C8: sub-part enemy tick (A0 = object).
 * Handles bomb-hit check (0x0F6324 -> 0x0CEE9A/0x0CEEA2/0x0CEE74),
 * HP death / bomb-clear (0x0F626C -> 0x0DC486 + parent 0x22 decrement + 0x117B44),
 * movement script (0x0CF1E4 / 0x0CF4B8), off-screen cull (0x0CF85A),
 * state dispatch (0x0F6382[0..3]), script/hazard/blink/draw. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0CF82C: clear bit 7 and set bit 1 of +0x07 when +0x3E is in [-64, 247]. */
static void sub_0CF82C(const f3mem_t *m, uint32_t obj)
{
    uint8_t f7 = rd8(m, obj + 0x07u);
    if ((f7 & 0x80u) == 0u)
        return;
    int16_t y = (int16_t)rd16(m, obj + 0x3Eu);
    if (y < -0x40 || y > 0x00F7)
        return;
    wr8(m, obj + 0x07u, (uint8_t)((f7 & ~0x80u) | 0x02u));
}

/* ROM 0x0D0690 with D0/A1 register tracking. */
static void step_script_regs(const f3mem_t *m, uint32_t obj, uint32_t *d0_io, uint32_t *a1_io)
{
    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x4Cu) - 1u);
    wr16(m, obj + 0x4Cu, cnt);
    if (cnt != 0u)
        return;

    uint32_t a1 = rd32(m, obj + 0x48u) + 0x0Cu;
    wr32(m, obj + 0x48u, a1);
    uint16_t d0 = rd16(m, a1);
    a1 += 2u;
    if ((int16_t)d0 < 0) {
        int16_t back = (int16_t)rd16(m, a1);
        if (back == 0) {
            *d0_io = setw(*d0_io, d0);
            *a1_io = a1;
            return;
        }
        a1 -= (uint32_t)(int32_t)back;
        wr32(m, obj + 0x48u, a1);
        wr16(m, obj + 0x4Cu, rd16(m, a1));
        a1 += 2u;
        uint16_t pal = rd16(m, a1);
        a1 += 2u;
        *d0_io = setw(*d0_io, pal);
        wr8(m, obj + 0x1Cu, (uint8_t)pal);
        wr32(m, obj + 0x10u, rd32(m, a1));
        a1 += 4u;
        wr32(m, obj + 0x68u, rd32(m, a1));
        *a1_io = a1;
        return;
    }

    wr16(m, obj + 0x4Cu, d0);
    uint16_t pal = rd16(m, a1);
    a1 += 2u;
    *d0_io = setw(*d0_io, pal);
    wr8(m, obj + 0x1Cu, (uint8_t)pal);
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
    *a1_io = a1;
}

/* ROM 0x001282 wrapper preserving D0/D1/A1 return values. */
static void draw_sprite_regs(const f3mem_t *m, uint32_t obj,
                             uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    uint32_t a2 = rd32(m, obj + 0x10u);
    uint32_t hdr = rd32(m, a2);

    Obj_DrawSprite(m, obj);
    *a1_io = rd32(m, F3_A5 - 0x0CA0u);

    if (hdr == 0x00010001u) {
        uint32_t d0 = (((uint32_t)rd16(m, obj + 0x14u) & 0x0FFFu) << 16)
                    | (uint32_t)rd16(m, a2 + 6u);
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u);
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 ^ d1w);
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        *d0_io = d0;
        *d1_io = setw(*d1_io, d1w);
    } else {
        uint16_t cols = (uint16_t)(hdr >> 16);
        *d0_io = 0x0000FFFFu;
        *d1_io = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

/* ROM 0x0CEE9A / 0x0CEEA2 */
static void sub_0CEE9A(const f3mem_t *m, uint32_t obj, int check_bomb, uint32_t *d0_io)
{
    if (check_bomb && (rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        Obj_StartDying(m, obj);
        return;
    }
    uint8_t arg = rd8(m, obj + 0x4Eu);
    *d0_io = (*d0_io & 0xFFFFFF00u) | arg;
    if (arg != 0u) {
        *d0_io = Obj_Spawn(m, obj, 0x00117C28u, 0u, 1u, *d0_io);
    }
    wr32(m, obj, 0x000CEEBCu);
}

int Fn_0F61C8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {      /* 0x0F61C8 */
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x000A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));   /* 0x0F61E0 */
            d0 = 0x0000012Cu;                           /* 0x0F6324 */
            Obj_RelayTimer(m, d0);                      /* 0x0F632A */
            a1 = rd32(m, obj + 0x08u);                  /* 0x0F6330 */
            uint8_t rem = (uint8_t)(rd8(m, a1 + 0x22u) - 1u);
            wr8(m, a1 + 0x22u, rem);                    /* 0x0F6334 */
            if (rem != 0u || (rd8(m, a1 + 0x07u) & 0x01u) != 0u) {
                sub_0CEE9A(m, obj, 1, &d0);
            } else if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
                Obj_StartDying(m, obj);
            } else {
                d0 = 0x00000BB8u;
                Obj_RelayTimer(m, d0);
                d0 = Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, d0);
                sub_0CEE9A(m, obj, 0, &d0);
            }
            r->d[0] = d0;
            r->d[1] = d1;
            r->a[1] = a1;
            return 0;
        }
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {           /* 0x0F61EC */
        d0 = 0x0000012Cu;                               /* 0x0F626C */
        Obj_0D0E04(m, obj, d0);                         /* 0x0F6272 */
        goto death_seq;
    }

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)        /* 0x0F61F4 */
        goto death_seq;

    d0 = rd32(m, F3_A5 + 0x4228u);                      /* 0x0F61FE */
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - d0);    /* 0x0F6202 */

    {
        uint8_t f7 = rd8(m, obj + 0x07u);               /* 0x0F6206 */
        wr8(m, obj + 0x07u, (uint8_t)(f7 & ~0x02u));
        if ((f7 & 0x02u) != 0u) {
            if (ObjScript_CF1E4(m, obj) < 0)            /* 0x0F6210 */
                return -1;
        }
    }

    if (rd8(m, obj + 0x20u) != 0u) {                    /* 0x0F6216 */
        a1 = 0x000F64D4u;                               /* 0x0F621E */
        if (ObjScript_CF4B8(m, obj, a1) < 0)            /* 0x0F6222 */
            return -1;
    }

    Obj_FlagOffscreen(m, obj);                          /* 0x0F6228 */
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {          /* 0x0F622E */
        a1 = rd32(m, obj + 0x08u);                      /* 0x0F6310 */
        wr8(m, a1 + 0x22u, (uint8_t)(rd8(m, a1 + 0x22u) - 1u));
        wr8(m, a1 + 0x07u, (uint8_t)(rd8(m, a1 + 0x07u) | 0x01u));
        Obj_StartDying(m, obj);                         /* 0x0F631E */
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        return 0;
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));         /* 0x0F6238 */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));         /* 0x0F623E */

    d0 = setw(d0, (uint16_t)rd8(m, obj + 0x06u));       /* 0x0F6244..0x0F6246 */
    switch ((uint16_t)d0) {                             /* 0x0F624A -> 0x0F6382 */
    case 0u:                                            /* 0x0F6392 */
        sub_0CF82C(m, obj);
        if ((rd8(m, obj + 0x07u) & 0x80u) == 0u) {
            a1 = 0x000F6590u;
            Obj_StartBehaviour(m, obj, a1);
            a1 += 8u;
            wr8(m, obj + 0x06u, 1u);
        }
        break;
    case 1u:                                            /* 0x0F63B4 */
        if (rd8(m, obj + 0x35u) != 0u) {
            a1 = 0x000F6554u;
            Obj_StartScript(m, obj, a1);
            a1 = 0x000F65CCu;
            Obj_StartBehaviour(m, obj, a1);
            a1 += 8u;
            wr8(m, obj + 0x06u, 2u);
        }
        break;
    case 2u:                                            /* 0x0F63D8 */
        a1 = 0x000F64D4u;
        Obj_AimAtPlayers(m, obj, a1);                   /* 0x0F63DC: note 0F64D4 */
        if (rd8(m, obj + 0x23u) != 0u) {
            wr8(m, obj + 0x23u, 0u);
            uint32_t sx = rd32(m, obj + 0x14u) + 0x00080000u;
            uint32_t sy = rd32(m, obj + 0x18u) + 0x00080000u;
            d0 = Obj_Spawn(m, obj, 0x000D9092u, 9u, 2u, 0u);
            if ((int32_t)d0 < 0) {
                wr8(m, obj + 0x06u, 3u);
                break;
            }
            a1 = d0;
            wr32(m, a1 + 0x14u, sx);
            wr32(m, a1 + 0x18u, sy);
            wr16(m, a1 + 0x10u, 0u);
            wr16(m, a1 + 0x58u, rd16(m, obj + 0x42u));

            d0 = Obj_Spawn(m, obj, 0x000D9092u, 9u, 2u, 0u);
            if ((int32_t)d0 < 0) {
                wr8(m, obj + 0x06u, 3u);
                break;
            }
            a1 = d0;
            wr32(m, a1 + 0x14u, sx);
            wr32(m, a1 + 0x18u, sy);
            wr16(m, a1 + 0x10u, 0u);
            int16_t a_lo = (int16_t)(rd16(m, obj + 0x42u) - 0x14u);
            if (a_lo < 0)
                a_lo = (int16_t)(a_lo + 0x0168);
            wr16(m, a1 + 0x58u, (uint16_t)a_lo);

            d0 = Obj_Spawn(m, obj, 0x000D9092u, 9u, 2u, 0u);
            if ((int32_t)d0 < 0) {
                wr8(m, obj + 0x06u, 3u);
                break;
            }
            a1 = d0;
            wr32(m, a1 + 0x14u, sx);
            wr32(m, a1 + 0x18u, sy);
            wr16(m, a1 + 0x10u, 0u);
            uint16_t a_hi = (uint16_t)(rd16(m, obj + 0x42u) + 0x14u);
            if (a_hi >= 0x0168u)
                a_hi = (uint16_t)(a_hi - 0x0168u);
            wr16(m, a1 + 0x58u, a_hi);
        }
        wr8(m, obj + 0x06u, 3u);
        break;
    case 3u:                                            /* 0x0F64CC */
        wr8(m, obj + 0x06u, 1u);
        break;
    }

    step_script_regs(m, obj, &d0, &a1);                 /* 0x0F6252 */
    Obj_0D0DE4(m, obj);                                 /* 0x0F6258 */
    Obj_BlinkColour(m, obj);                            /* 0x0F625E */
    draw_sprite_regs(m, obj, &d0, &d1, &a1);            /* 0x0F6264 */
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;

death_seq:
    d0 = Obj_Spawn(m, obj, 0x000DC486u, 3u, 2u, 0u);    /* 0x0F6278 */
    if ((int32_t)d0 >= 0) {
        a1 = d0;
        wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
        wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 8u));
        wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
        wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x14u));
        wr16(m, a1 + 0x58u, rd16(m, obj + 0x58u));
        wr16(m, a1 + 0x60u, rd16(m, obj + 0x60u));
    }
    a1 = rd32(m, obj + 0x08u);                          /* 0x0F62BE */
    {
        uint8_t rem = (uint8_t)(rd8(m, a1 + 0x22u) - 1u);
        wr8(m, a1 + 0x22u, rem);
        if (rem != 0u || (rd8(m, a1 + 0x07u) & 0x01u) != 0u) {
            Obj_0CEE48(m, obj);
        } else if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
            Obj_StartDying(m, obj);
        } else {
            d0 = 0x00000BB8u;
            Obj_0D0E04(m, obj, d0);
            d0 = Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, d0);
            uint8_t arg = rd8(m, obj + 0x4Eu);
            d0 = (d0 & 0xFFFFFF00u) | arg;
            if (arg != 0u)
                d0 = Obj_Spawn(m, obj, 0x00117C28u, 0u, 1u, d0);
            Obj_StartDying(m, obj);
        }
    }
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
