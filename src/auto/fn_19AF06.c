/* ROM 0x19AF06: scripted boss/enemy effect or projectile tick (A0 = object).
 * Steps 10-byte script (+0x44) via 0x178982, copies (+0x50, +0x54) to
 * (+0x14, +0x18), runs 0x179A5C[+0x40].tick, centers (+0x14, +0x18) by
 * sprite {w,h}*8, draws sprite (0x001282), and either re-arms 0x19AF06 (when
 * +0x3E >= 0) or frees self via 0x0105EC. */
#include "fn_auto.h"
#include "../enemy/obj_179550.h"
#include "../enemy/obj_17955E.h"
#include "../enemy/obj_179572.h"
#include "../enemy/obj_179588.h"
#include "../enemy/obj_1795A4.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_script10.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x001282 wrapper preserving D0/D1/A1 return values. */
static void draw_sprite_regs(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a2 = rd32(m, obj + 0x10u);
    uint32_t hdr = rd32(m, a2);

    Obj_DrawSprite(m, obj);
    r->a[1] = rd32(m, F3_A5 - 0x0CA0u);

    if (hdr == 0x00010001u) {
        uint32_t d0 = (((uint32_t)rd16(m, obj + 0x14u) & 0x0FFFu) << 16)
                    | (uint32_t)rd16(m, a2 + 6u);
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u);
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 ^ d1w);
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        r->d[0] = d0;
        r->d[1] = setw(r->d[1], d1w);
    } else {
        uint16_t cols = (uint16_t)(hdr >> 16);
        r->d[0] = 0x0000FFFFu;
        r->d[1] = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

/* ROM 0x1795CA: index-13 anim init. */
static void anim13_init(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x3Au);
    wr16(m, obj + 0x3Eu, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x2Cu, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x30u, rd32(m, a1)); a1 += 4u;
    int16_t dx = (int16_t)(int8_t)rd8(m, a1);
    int16_t dy = (int16_t)(int8_t)rd8(m, a1 + 1u);
    wr16(m, obj + 0x14u, (uint16_t)((int16_t)rd16(m, obj + 0x14u) + dx));
    wr16(m, obj + 0x18u, (uint16_t)((int16_t)rd16(m, obj + 0x18u) + dy));
}

static int anim_init_local(const f3mem_t *m, uint32_t obj, uint16_t idx)
{
    if (idx == 13u) {
        anim13_init(m, obj);
        return 0;
    }
    return Obj_AnimInit(m, obj, idx);
}

/* Shared frame-list step for indices 3 (stride 0x0E) and 11 (stride 0x10). */
static void anim_list_tick(const f3mem_t *m, uint32_t obj, uint32_t stride, int idx)
{
    int16_t t = (int16_t)rd16(m, obj + 0x3Eu);
    if (t < 0)
        return;
    if (t == 0) {
        uint32_t a1 = rd32(m, obj + 0x3Au) + stride;
        int16_t d = (int16_t)rd16(m, a1);
        if (d > 0) {
            wr32(m, obj + 0x3Au, a1);
            if (idx == 3)
                Obj_179572_Tick(m, obj);
            else
                Obj_179588_Tick(m, obj);
        } else if (d == 0) {
            a1 = rd32(m, a1 + 2u);
            uint16_t next = rd16(m, a1);
            a1 += 2u;
            wr16(m, obj + 0x40u, next);
            wr32(m, obj + 0x3Au, a1);
            if (anim_init_local(m, obj, next) < 0)
                wr16(m, obj + 0x40u, next);
        }
    }
    wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) - 1u));
}

/* ROM 0x17980E: index-13 anim tick. */
static int anim13_tick(const f3mem_t *m, uint32_t obj)
{
    int16_t timer = (int16_t)rd16(m, obj + 0x3Eu);
    if (timer == 0) {
        uint32_t a1 = rd32(m, obj + 0x3Au) + 0x12u;
        int16_t d = (int16_t)rd16(m, a1);
        if (d > 0) {
            wr32(m, obj + 0x3Au, a1);
            anim13_init(m, obj);
            wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) - 1u));
            return 0;
        } else if (d == 0) {
            a1 = rd32(m, a1 + 2u);
            uint16_t next = rd16(m, a1);
            a1 += 2u;
            wr16(m, obj + 0x40u, next);
            wr32(m, obj + 0x3Au, a1);
            if (anim_init_local(m, obj, next) < 0)
                return -1;
            wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) - 1u));
            return 0;
        }
        wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) - 1u));
    } else if (timer > 0) {
        wr16(m, obj + 0x3Eu, (uint16_t)(timer - 1));
    }
    uint32_t a1 = rd32(m, obj + 0x3Au);
    int16_t dx = (int16_t)(int8_t)rd8(m, a1 + 0x10u);
    int16_t dy = (int16_t)(int8_t)rd8(m, a1 + 0x11u);
    wr16(m, obj + 0x14u, (uint16_t)((int16_t)rd16(m, obj + 0x14u) + dx));
    wr16(m, obj + 0x18u, (uint16_t)((int16_t)rd16(m, obj + 0x18u) + dy));
    return 0;
}

static int anim_tick_local(const f3mem_t *m, uint32_t obj, uint16_t idx)
{
    if (idx == 3u) {
        anim_list_tick(m, obj, 0x0Eu, 3);
        return 0;
    }
    if (idx == 11u) {
        anim_list_tick(m, obj, 0x10u, 11);
        return 0;
    }
    if (idx == 13u)
        return anim13_tick(m, obj);
    return Obj_AnimTick(m, obj, idx);
}

/* Additional 0x178982 script commands not yet in Obj_Script10. */
static void scr_178B02(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x20u, rd16(m, rd32(m, obj + 0x44u) + 8u));
}

static void scr_178BB4(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x4Cu, rd32(m, F3_A5 + 0x1D28u));
}

static void scr_178DB6(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x44u);
    uint16_t d1 = (uint16_t)(rd16(m, obj + 0x42u) + rd16(m, a1 + 6u));
    uint8_t ang = (uint8_t)((uint8_t)Obj_RandRange(m, (uint16_t)rd8(m, a1 + 9u))
                  + rd8(m, obj + 0x20u));
    wr8(m, obj + 0x20u, ang);
    wr16(m, obj + 0x42u, d1);
    int16_t mag = (int16_t)(uint16_t)(d1 + 0x20u) >> 6;
    uint32_t base = 0x00027E40u + (uint32_t)ang * 2u;
    int32_t vx = (int32_t)mag * (int16_t)rd16(m, base + 0x80u);
    int32_t vy = (int32_t)mag * (int16_t)rd16(m, base);
    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + (uint32_t)vx);
    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + (uint32_t)vy);
}

static void scr_178FCE(const f3mem_t *m, uint32_t obj)
{
    int16_t d2 = (int16_t)(int8_t)rd8(m, obj + 0x42u);
    if (d2 < 0) {
        wr16(m, obj + 0x42u, 0u);
        wr16(m, obj + 0x48u, 0u);
        return;
    }
    int16_t dx = (int16_t)(rd16(m, obj + 0x4Cu) - rd16(m, obj + 0x50u));
    int16_t dy = (int16_t)(rd16(m, obj + 0x4Eu) - rd16(m, obj + 0x54u));
    int16_t ax = (dx < 0) ? (int16_t)(0u - (uint16_t)dx) : dx;
    int16_t ay = (dy < 0) ? (int16_t)(0u - (uint16_t)dy) : dy;
    if (ax <= d2 && ay <= d2) {
        wr16(m, obj + 0x48u, 0u);
        return;
    }
    uint16_t d0 = Obj_Atan2(m, (uint16_t)dx, (uint16_t)dy);
    uint32_t a1 = rd32(m, obj + 0x44u);
    uint16_t step = rd16(m, a1 + 6u);
    uint16_t turn = rd16(m, a1 + 8u);
    int8_t diff = (int8_t)(uint8_t)((uint8_t)d0 - rd8(m, obj + 0x20u));
    if (diff == 0)
        turn = 0u;
    else if (diff < 0)
        turn = (uint16_t)(0u - turn);
    wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) + step));
    wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + turn));
    int16_t mag = (int16_t)(uint16_t)(rd16(m, obj + 0x42u) + 0x20u) >> 6;
    uint16_t idx = (uint16_t)(rd16(m, obj + 0x20u) >> 8);
    uint32_t base = 0x00027E40u + (uint32_t)idx * 2u;
    int32_t vx = (int32_t)mag * (int16_t)rd16(m, base + 0x80u);
    int32_t vy = (int32_t)mag * (int16_t)rd16(m, base);
    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + (uint32_t)vx);
    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + (uint32_t)vy);
}

static int script10_local(const f3mem_t *m, uint32_t obj)
{
    for (;;) {
        uint32_t a1 = rd32(m, obj + 0x44u);
        uint32_t fn = rd32(m, a1);
        if (fn == 0u)
            goto loop_marker;
        if (fn == 0x00178B02u)
            scr_178B02(m, obj);
        else if (fn == 0x00178BB4u)
            scr_178BB4(m, obj);
        else if (fn == 0x00178DB6u)
            scr_178DB6(m, obj);
        else if (fn == 0x00178FCEu)
            scr_178FCE(m, obj);
        else
            return Obj_Script10(m, obj);

        int16_t n = (int16_t)rd16(m, obj + 0x48u);
        if (n < 0)
            return 0;
        if (n > 0) {
            n = (int16_t)(n - 1);
            wr16(m, obj + 0x48u, (uint16_t)n);
            if (n > 0)
                return 0;
            a1 = rd32(m, obj + 0x44u) + 0x0Au;
            if ((int32_t)rd32(m, a1) > 0)
                goto store;
            goto loop_marker;
        }
        a1 = rd32(m, obj + 0x44u) + 0x0Au;
        wr32(m, obj + 0x44u, a1);
        wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
        continue;

    loop_marker: {
            int16_t c = (int16_t)rd16(m, obj + 0x4Au);
            if (c >= 0) {
                if (c == 0)
                    wr16(m, obj + 0x4Au, rd16(m, a1 + 4u));
                c = (int16_t)(rd16(m, obj + 0x4Au) - 1u);
                wr16(m, obj + 0x4Au, (uint16_t)c);
                if (c == 0) {
                    a1 += 0x0Au;
                    goto store;
                }
            }
            a1 = rd32(m, a1 + 6u);
        }
    store:
        wr32(m, obj + 0x44u, a1);
        wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
        return 0;
    }
}

int Fn_19AF06(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if (script10_local(m, obj) < 0)                      /* 0x19AF06 */
        return -1;

    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));          /* 0x19AF0C */
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));          /* 0x19AF12 */

    uint16_t idx = rd16(m, obj + 0x40u);                 /* 0x19AF18 */
    if (anim_tick_local(m, obj, idx) < 0)                /* 0x19AF26 */
        return -1;

    uint32_t spr = rd32(m, obj + 0x10u);                 /* 0x19AF28 */
    uint16_t d1w = (uint16_t)(rd16(m, spr) << 3);        /* 0x19AF2C, 0x19AF32 */
    uint16_t d0w = (uint16_t)(rd16(m, spr + 2u) << 3);   /* 0x19AF2E, 0x19AF30 */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0w)); /* 0x19AF34 */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d1w)); /* 0x19AF38 */

    draw_sprite_regs(m, r);                              /* 0x19AF3C */

    if ((int16_t)rd16(m, obj + 0x3Eu) >= 0)              /* 0x19AF42..0x19AF46 */
        wr32(m, obj, 0x0019AF06u);                       /* 0x19AEFE */
    else
        Obj_Free(m, obj);                                /* 0x19AF48..0x19AF50 */

    return 0;
}
