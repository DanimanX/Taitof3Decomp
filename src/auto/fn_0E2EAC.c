#include "fn_auto.h"
#include "fn_call_frames.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"
#include "../enemy/obj_E8B72.h"
#include "../player/player_move.h"

/* ROM 0x0E2EAC: scripted enemy/boss part tick (A0 = object).
 *
 * While +0x06 == 2 the position is scroll-absolute: the A5+0x4224 / A5+0x4228
 * scroll deltas are subtracted from +0x3A / +0x3E and copied to +0x14 / +0x18
 * (the +0x18 copy always happens). A bomb hit (A5+0x1D58 bit 1 with X in
 * [-0xA..0x11C]) recolours from +0x4F, credits 0x12C with 0x0D0E30, clears
 * +0x06 and hands over to 0x0CEF0E.
 *
 * HP <= 0 credits 0x12C through 0x0D0E04, while A5-0x144B bit 5 skips that;
 * both spawn the 0x0DC328 burst (group 3, count 2) at +0x10/+0x10 and then
 * clear +0x06 and run the 0x0CEE74 death hand-over.
 *
 * Otherwise (state != 2): a pending +0x07 bit 1 runs the script interpreter
 * 0x0CF1E4, a nonzero +0x20 runs the movement step 0x0CF4B8 with the aim
 * record 0x0E3054, 0x0CF7FE / 0x0CF85A arm and bounds-check the object, and
 * +0x07 bit 3 dies. The position is then resynced from +0x3A/+0x3E; with no
 * +0x23 countdown and +0x07 bit 5 set the object aims (0x0D07B4 / 0x0D0BBC,
 * record 0x0E3054), takes its direction record 0x0E31BC through 0x0D005A,
 * and when the +0x23 countdown runs out spawns 0x0E35BE (group 9, count 2) and
 * enters state 2. The tail steps the script frame (0x0D0690), registers on the
 * collision lists (0x0D0DC2) and blinks the hit colour (0x0CBE34). */

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0D0C8C: difficulty-scaled aim threshold offset from A5+0x87F. */
static uint16_t sub_0D0C8C(const f3mem_t *m)
{
    uint8_t d0 = (uint8_t)(rd8(m, F3_A5 + 0x087Fu) >> 3);
    return rd16(m, 0x000D0D02u + (uint32_t)d0 * 2u);
}

/* ROM 0x0D2D44: (D0, D1) = {0x0D2D64, 0x0D2ECC}[d0w] * d1w. */
static void sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t off = (uint32_t)((int32_t)(int16_t)d0w * 4);
    uint32_t d0 = rd32(m, 0x000D2D64u + off);
    uint32_t d1 = rd32(m, 0x000D2ECCu + off);
    *out_d0 = d0 * (uint32_t)d1w;
    *out_d1 = d1 * (uint32_t)d1w;
}

/* ROM 0x0CF4B8 with the D0/D1 left behind by 0x0D2D44 in modes 12..15. */
static int cf4b8_regs(const f3mem_t *m, uint32_t obj, uint32_t rec,
                      uint32_t *d0_io, uint32_t *d1_io)
{
    uint8_t op = rd8(m, obj + 0x20u);
    if (ObjScript_CF4B8(m, obj, rec) < 0)
        return -1;
    if (op >= 12u && op <= 15u)
        sub_0D2D44(m, rd16(m, obj + 0x58u), rd16(m, obj + 0x60u), d0_io, d1_io);
    return 0;
}

/* ROM 0x0D0E52: 256-step angle of (d0, d1) (quadrant offsets 0x0D0EB4). */
static uint16_t sub_0D0E52(const f3mem_t *m, uint16_t d0, uint16_t d1)
{
    uint16_t d2 = 0u;

    if ((int16_t)d0 < 0) {
        d2 = (uint16_t)(d2 + 2u);
        d0 = (uint16_t)(0u - d0);
    }
    if ((int16_t)d1 < 0) {
        d2 = (uint16_t)(d2 + 4u);
        d1 = (uint16_t)(0u - d1);
    }

    uint16_t src = (d1 > d0) ? d1 : d0;
    uint16_t d3 = 0x20u;
    for (uint16_t i = 0u; i < 16u; i++) {
        if (src & (uint16_t)(0x8000u >> i)) {
            d3 = (uint16_t)(0x10u + i);
            break;
        }
    }

    d3 = (uint16_t)(d3 - 0x1Au);
    if ((int16_t)d3 < 0) {
        d3 = (uint16_t)(0u - d3);
        d0 = (uint16_t)(d0 >> (d3 & 63u));
        d1 = (uint16_t)(d1 >> (d3 & 63u));
    }

    if (d2 != 2u && d2 != 4u) {
        uint16_t tmp = d0;
        d0 = d1;
        d1 = tmp;
    }

    d1 = (uint16_t)((uint16_t)(d1 << 6) + d0);
    d0 = (uint16_t)rd8(m, 0x0D0EBCu + (uint32_t)(int32_t)(int16_t)d1);
    d0 = (uint16_t)(d0 + rd16(m, 0x0D0EB4u + (uint32_t)(int32_t)(int16_t)d2));
    if (d0 == 0x0168u)
        d0 = 0u;
    return d0;
}

/* ROM 0x0D07B4: aim at the nearest live player inside the bounds/angle window
 * of the record at A1. */
static void sub_0D07B4(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    uint32_t muzzle = rd32(m, obj + 0x68u);
    int16_t mx = (int16_t)(uint16_t)(rd16(m, obj + 0x14u) + rd16(m, muzzle));
    if (mx < 0x28 || mx > 0x0127)
        return;
    int16_t my = (int16_t)(uint16_t)(rd16(m, obj + 0x18u) + rd16(m, muzzle + 2u));
    if (my < 0x10 || my > 0xDF)
        return;

    uint32_t players[2] = { 0x0041B190u, 0x0041B21Au };
    uint16_t adx[2] = { 0u, 0u };
    uint16_t ady[2] = { 0u, 0u };
    uint16_t angs[2] = { 0xFFFFu, 0xFFFFu };
    uint16_t near = rd16(m, a1 + 0x08u);
    uint16_t reach = rd16(m, a1 + 0x0Au);
    uint16_t lo = rd16(m, a1 + 0x0Cu);
    uint16_t hi = rd16(m, a1 + 0x0Eu);

    for (int p = 0; p < 2; p++) {
        uint32_t pl = players[p];
        if ((int16_t)rd16(m, pl + 0x4Cu) < 0)
            continue;

        uint32_t a4 = rd32(m, obj + 0x68u) + 2u;
        uint16_t dy = (uint16_t)(rd16(m, obj + 0x18u) - rd16(m, pl + 0x18u) + rd16(m, a4) - 0x000Fu);
        uint16_t a3w = dy;
        if ((int16_t)dy < 0)
            dy = (uint16_t)(0u - dy);
        ady[p] = dy;
        if (dy > reach)
            continue;

        a4 -= 2u;
        uint16_t dx = (uint16_t)(rd16(m, obj + 0x14u) - rd16(m, pl + 0x14u) + rd16(m, a4) - 0x0017u);
        uint16_t a4w = dx;
        if ((int16_t)dx < 0) {
            dx = (uint16_t)(0u - dx);
            adx[p] = dx;
            if (dx > reach)
                continue;
            if (dx < near && dy < near)
                continue;
        } else {
            adx[p] = dx;
            if (dx > reach)
                continue;
            uint16_t lim = (uint16_t)(near - 0x001Eu);
            if ((int16_t)dx < (int16_t)lim && dy < near)
                continue;
        }

        uint16_t ang = sub_0D0E52(m, a4w, a3w);
        if (lo > hi) {
            if (!(ang >= lo || ang <= hi))
                continue;
        } else {
            if (ang < lo || ang > hi)
                continue;
        }
        angs[p] = ang;
    }

    uint16_t d4 = angs[0];
    uint16_t d7 = angs[1];
    uint16_t d0 = d7;

    if ((int16_t)d4 < 0) {
        if ((int16_t)d7 < 0)
            return;
    } else if ((int16_t)d7 < 0) {
        d0 = d4;
    } else {
        uint16_t d2 = (ady[0] > adx[0]) ? ady[0] : adx[0];
        uint16_t d5 = (ady[1] > adx[1]) ? ady[1] : adx[1];
        if (d5 >= d2)
            d0 = d4;
    }

    wr16(m, obj + 0x42u, d0);
    uint16_t d1 = (uint16_t)(rd16(m, a1 + 0x10u) + sub_0D0C8C(m));
    uint16_t roll = Obj_Next4Way(m);
    if ((int16_t)d1 > (int16_t)roll)
        wr8(m, obj + 0x23u, (uint8_t)rd16(m, a1 + 0x12u));
}

/* ROM 0x0D0BBC: turn +0x08 toward +0x42 by the step in (A1+4). */
static void sub_0D0BBC(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    uint16_t d1 = rd16(m, obj + 0x42u);
    uint16_t d0 = (uint16_t)(d1 - rd16(m, obj + 0x08u));
    if ((int16_t)d0 < 0)
        d0 = (uint16_t)(d0 + 0x0168u);

    if (d0 <= 0x00B4u) {
        if (d0 <= rd16(m, a1 + 0x04u)) {
            wr16(m, obj + 0x08u, d1);
            return;
        }
        wr32(m, obj + 0x08u, rd32(m, obj + 0x08u) + rd32(m, a1 + 0x04u));
        if (rd16(m, obj + 0x08u) >= 0x0168u)
            wr16(m, obj + 0x08u, (uint16_t)(rd16(m, obj + 0x08u) - 0x0168u));
        return;
    }

    d0 = (uint16_t)((uint16_t)(0u - d0) + 0x0168u);
    if (d0 <= rd16(m, a1 + 0x04u)) {
        wr16(m, obj + 0x08u, d1);
        return;
    }
    uint32_t v = rd32(m, obj + 0x08u) - rd32(m, a1 + 0x04u);
    wr32(m, obj + 0x08u, v);
    if ((int32_t)v < 0)
        wr16(m, obj + 0x08u, (uint16_t)(rd16(m, obj + 0x08u) + 0x0168u));
}

/* ROM 0x0D005A: install the direction record base[d0] into +0x48/+0x4C/
 * +0x1C/+0x10/+0x68 (+0x28 of the +0x0C block remembers it). */
static void sub_0D005A(const f3mem_t *m, uint32_t obj, uint32_t a1, uint16_t d0)
{
    int16_t off = (int16_t)rd16(m, 0x000D00B4u + (uint32_t)(int32_t)(int16_t)d0 * 2u);
    a1 = rd32(m, a1 + (uint32_t)(int32_t)off);
    uint32_t a2 = rd32(m, obj + 0x0Cu);
    if (a1 == rd32(m, a2 + 0x28u))
        return;

    wr32(m, obj + 0x48u, a1);
    wr32(m, a2 + 0x28u, a1);
    wr16(m, obj + 0x4Cu, rd16(m, a1));
    a1 += 2u;
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x0D0690 with D0/A1 register tracking: one 12-byte script step when the
 * +0x4C timer runs out. */
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

/* ROM 0x0D0DC2 with A1 register tracking. */
static void add_coll_regs(const f3mem_t *m, uint32_t obj, uint32_t *a1_io)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x08FEu, a1 + 4u);
    a1 = rd32(m, F3_A5 + 0x0906u);
    wr32(m, a1, obj);
    a1 += 4u;
    wr32(m, F3_A5 + 0x0906u, a1);
    *a1_io = a1;
}

int Fn_0E2EAC(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    if (rd8(m, obj + 0x06u) == 0x02u) {                         /* 0x0E2EAC */
        d0 = rd32(m, F3_A5 + 0x4224u);                          /* 0x0E2EB6 */
        wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - d0);
        wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    }
    d0 = rd32(m, F3_A5 + 0x4228u);                              /* 0x0E2EC4 */
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - d0);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {              /* 0x0E2ED2 */
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x000A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));           /* 0x0E2EEA */
            d0 = 0x0000012Cu;                                   /* 0x0E303E */
            Obj_RelayTimer(m, d0);                              /* 0x0E3044 */
            wr8(m, obj + 0x06u, 0u);                            /* 0x0E304A */
            Obj_0CEF0E(m, obj);                                 /* 0x0E304E bra */
            r->d[0] = d0;
            r->d[1] = d1;
            r->a[1] = a1;
            return 0;
        }
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {                   /* 0x0E2EF6 */
        d0 = 0x0000012Cu;                                       /* 0x0E2FE0 */
        Obj_0D0E04(m, obj, d0);                                 /* 0x0E2FE6 */
        goto burst;
    }
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)                /* 0x0E2EFE */
        goto burst;
    if (rd8(m, obj + 0x06u) == 0x02u)                           /* 0x0E2F08 */
        goto tail;

    {
        uint8_t f7 = rd8(m, obj + 0x07u);                       /* 0x0E2F12 bclr.b #1 */
        wr8(m, obj + 0x07u, (uint8_t)(f7 & ~0x02u));
        if ((f7 & 0x02u) != 0u) {
            if (ObjScript_CF1E4(m, obj) < 0)
                return -1;
            /* 0x0CF1E8 leaves the executed opcode word in D0 (cases 9, 11, 12
             * and 13 overwrite it with their own scratch value, which its C
             * translation does not expose). */
            d0 = setw(d0, rd16(m, rd32(m, obj + 0x44u) - 6u));
        }
    }
    if (rd8(m, obj + 0x20u) != 0u) {                            /* 0x0E2F22 */
        a1 = 0x000E3054u;
        if (cf4b8_regs(m, obj, a1, &d0, &d1) < 0)
            return -1;
    }

    Obj_ArmOnScreen(m, obj);                                    /* 0x0E2F34 */
    Obj_FlagOffscreen(m, obj);                                  /* 0x0E2F3A */
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u)                    /* 0x0E2F40 */
        goto die;

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));                 /* 0x0E2F4A */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));                 /* 0x0E2F50 */

    if (rd8(m, obj + 0x23u) != 0u)                              /* 0x0E2F56 */
        goto countdown;
    if ((rd8(m, obj + 0x07u) & 0x20u) == 0u)                    /* 0x0E2F5E */
        goto tail;

    a1 = 0x000E3054u;                                           /* 0x0E2F68 */
    sub_0D07B4(m, obj, a1);
    {
        /* 0x0D0BBC's D0/D1: D1 carries the target angle (+0x42); D0 is the
         * angle difference on the "already turned" exit (0x0D0C1A) and the
         * step long (A1+4) on the two turning exits. The branch is decided on
         * the heading before the turn. */
        uint16_t ang = rd16(m, obj + 0x42u);
        int16_t diff = (int16_t)(uint16_t)(ang - rd16(m, obj + 0x08u));
        uint16_t d0w;
        int goal;
        if (diff < 0)
            diff = (int16_t)(diff + 0x0168);
        if ((uint16_t)diff <= 0x00B4u) {
            d0w = (uint16_t)diff;
        } else {
            d0w = (uint16_t)((uint16_t)(0u - (uint16_t)diff) + 0x0168u);
        }
        goal = (d0w <= rd16(m, a1 + 4u));
        sub_0D0BBC(m, obj, a1);
        d1 = setw(d1, ang);
        d0 = goal ? setw(d0, d0w) : rd32(m, a1 + 4u);
    }
    d0 = setw(d0, rd16(m, obj + 0x08u));                        /* 0x0E2F7C */
    sub_0D005A(m, obj, 0x000E31BCu, (uint16_t)d0);              /* 0x0E2F80 */
    d0 = setw(d0, rd16(m, 0x000D00B4u + (uint32_t)(int32_t)(int16_t)(uint16_t)d0 * 2u));
    if (rd8(m, obj + 0x06u) == 0x03u)                           /* 0x0E2F86 */
        wr8(m, obj + 0x23u, 0u);
    if (rd8(m, obj + 0x23u) == 0u)                              /* 0x0E2F94 */
        goto tail;

countdown:
    {
        uint8_t t = (uint8_t)(rd8(m, obj + 0x23u) - 1u);         /* 0x0E2F9C */
        wr8(m, obj + 0x23u, t);
        if (t != 0u)
            goto tail;
    }
    {
        uint32_t c = Obj_Spawn(m, obj, 0x000E35BEu, 9u, 2u, 0u); /* 0x0E2FA4 */
        d0 = c;
        if ((int32_t)c >= 0)                                    /* 0x0E2FC0 */
            wr8(m, obj + 0x06u, 0x02u);                          /* 0x0E2FC6 */
    }

tail:
    step_script_regs(m, obj, &d0, &a1);                          /* 0x0E2FCC */
    add_coll_regs(m, obj, &a1);                                  /* 0x0E2FD2 */
    Obj_BlinkColour(m, obj);                                     /* 0x0E2FD8 */
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;

burst:
    {
        uint32_t c = Obj_Spawn(m, obj, 0x000DC328u, 3u, 2u, 0u); /* 0x0E2FEC */
        d0 = c;
        if ((int32_t)c >= 0) {
            a1 = c;                                              /* 0x0E300E movea.l D0,A1 */
            wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));           /* 0x0E3010 */
            wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x0010u));
            wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));           /* 0x0E301C */
            wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x0010u));
            wr16(m, a1 + 0x58u, 0x00B4u);                        /* 0x0E3028 */
            wr16(m, a1 + 0x60u, rd16(m, obj + 0x60u));           /* 0x0E302E */
        }
    }

die:
    wr8(m, obj + 0x06u, 0u);                                     /* 0x0E3034 */
    Obj_StartDying(m, obj);                                      /* 0x0E3038 bra 0x0CEE74 */
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
