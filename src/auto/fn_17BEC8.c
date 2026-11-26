/* ROM 0x17BEC8: generic scripted boss/enemy object tick (A0 = object).
 * Clears +0x24 if A5-0x144C bit 4 is set; when +0x24 <= 0 jumps to the death
 * handler at +0x66 (0x17C59A, 0x183002, 0x17C704, 0x17C73E, 0x17C78E).
 * Otherwise steps 10-byte script (0x178982), frees via 0x17C78E if +0x50/+0x54
 * leave 0x1849F8/0x1849FC, syncs (+0x14, +0x18), ticks 0x179A5C animation,
 * centers by sprite dims * 8, invokes +0x0C sub-tick callback, appends to
 * hazard list A5+0x906, draws sprite (0x001282), explodes via 0x0CBB2C when
 * A5+0x1D58 bit 1 is set on-screen, and re-arms 0x17BEC8. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_script10.h"
#include "../objects/obj_shared.h"
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
    uint32_t spr_hdr = rd32(m, a2);

    Obj_DrawSprite(m, obj);
    r->a[1] = rd32(m, F3_A5 - 0x0CA0u);

    if (spr_hdr == 0x00010001u) {
        uint32_t d0 = (((uint32_t)rd16(m, obj + 0x14u) & 0x0FFFu) << 16)
                    | (uint32_t)rd16(m, a2 + 6u);
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u);
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 ^ d1w);
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        r->d[0] = d0;
        r->d[1] = setw(r->d[1], d1w);
    } else {
        uint16_t cols = (uint16_t)(spr_hdr >> 16);
        r->d[0] = 0x0000FFFFu;
        r->d[1] = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

/* ROM 0x00649A with full D0 result. */
static uint32_t dist_full(const f3mem_t *m, uint16_t dx_in, uint16_t dy_in)
{
    uint32_t d0 = (uint32_t)((int32_t)(int16_t)dx_in * (int32_t)(int16_t)dx_in);
    uint32_t d1 = (uint32_t)((int32_t)(int16_t)dy_in * (int32_t)(int16_t)dy_in);
    d0 += d1;
    d0 += d0;

    uint16_t shift = 4u;
    for (int i = 0; i < 3; i++) {
        if ((int32_t)d0 < 0x200)
            break;
        d0 >>= 8;
        shift = (uint16_t)(shift + 4u);
    }

    int32_t idx = (int32_t)(int16_t)(uint16_t)d0;
    d0 = (d0 & 0xFFFF0000u) | (uint32_t)rd16(m, 0x028340u + (uint32_t)(idx * 2));
    d0 <<= (shift & 63u);
    return (d0 << 16) | (d0 >> 16);
}

/* ROM 0x0CBB2C with register tracking. */
static void explode_tick_regs(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t dx = (uint16_t)(rd16(m, obj + 0x14u) - 0x00A0u);
    uint16_t dy = (uint16_t)(rd16(m, obj + 0x18u) - 0x0074u);

    uint32_t a1 = rd32(m, obj + 0x10u);
    int16_t off_y = (int16_t)((int32_t)(int16_t)((uint16_t)rd8(m, obj + 0x1Eu) - 0x0100u)
                              * (int32_t)(int16_t)rd16(m, a1)) >> 5;
    int16_t off_x = (int16_t)((int32_t)(int16_t)((uint16_t)rd8(m, obj + 0x1Fu) - 0x0100u)
                              * (int32_t)(int16_t)rd16(m, a1 + 2u)) >> 5;
    dx = (uint16_t)(dx - (uint16_t)off_x);
    dy = (uint16_t)(dy - (uint16_t)off_y);

    uint32_t d0_dist = dist_full(m, dx, dy);
    uint16_t d0w = (uint16_t)((uint16_t)d0_dist << 2);
    uint16_t thresh = rd16(m, F3_A5 + 0x1D5Cu);
    int is_lt = (int16_t)d0w < (int16_t)thresh;
    uint16_t diff = (uint16_t)(d0w - thresh);

    Obj_ExplodeTick(m, obj);

    if (is_lt) {
        r->a[1] = a1 + 4u;
        r->d[0] = setw(d0_dist, diff);
        r->d[1] = setw(r->d[1], dy);
        return;
    }

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) == 0u) {
        uint16_t ang = Obj_Atan2(m, dx, dy);
        r->a[1] = a1 + 4u;
        r->d[0] = setw(d0_dist, (uint16_t)(ang << 8));
        r->d[1] = setw(r->d[1], dy);
        return;
    }

    draw_sprite_regs(m, r);
}

/* ROM 0x17B02E: resolve score receiver from +0x35/+0x36 (or -1). */
static uint32_t sub_17B02E(const f3mem_t *m, uint32_t obj)
{
    uint8_t k = rd8(m, obj + 0x35u);
    if (k == 1u)
        return rd32(m, rd32(m, obj + 0x36u) + 8u);
    if (k == 2u)
        return rd32(m, obj + 0x36u);
    return 0xFFFFFFFFu;
}

/* ROM 0x17C614: shared post-death animation + script step. */
static int sub_17C614(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    if (Obj_Script10(m, obj) < 0)
        return -1;
    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));
    if (Obj_AnimTick(m, obj, rd16(m, obj + 0x40u)) < 0)
        return -1;
    {
        uint32_t spr = rd32(m, obj + 0x10u);
        uint16_t d1w = (uint16_t)(rd16(m, spr) << 3);
        uint16_t d0w = (uint16_t)(rd16(m, spr + 2u) << 3);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0w));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d1w));
    }
    if ((int16_t)rd16(m, obj + 0x3Eu) < 0) {
        Obj_Free(m, obj);
        return 0;
    }
    draw_sprite_regs(m, r);
    wr32(m, obj, 0x0017C660u);
    return 0;
}

/* ROM 0x17C59A: death handler (shrink-away if +0x34 bit 1, else anim + 0x17C614). */
static int sub_17C59A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    if ((rd8(m, obj + 0x34u) & 0x02u) != 0u) {
        /* 0x17C662 */
        if (rd8(m, obj + 0x1Fu) >= 0xF0u || rd8(m, obj + 0x1Eu) >= 0xF0u) {
            Obj_Free(m, obj);
            return 0;
        }
        if (Obj_Script10(m, obj) < 0)
            return -1;
        wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
        wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));

        wr8(m, obj + 0x1Fu, (uint8_t)(rd8(m, obj + 0x1Fu) + 0x10u));
        {
            uint32_t spr = rd32(m, obj + 0x10u);
            uint16_t d1w = (uint16_t)(rd16(m, spr + 2u) << 4);
            uint16_t d0w = (uint16_t)(((uint32_t)d1w * (uint32_t)rd8(m, obj + 0x1Fu)) >> 9);
            wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + d0w));
        }
        wr8(m, obj + 0x1Eu, (uint8_t)(rd8(m, obj + 0x1Eu) + 0x10u));
        {
            uint32_t spr = rd32(m, obj + 0x10u);
            uint16_t d1w = (uint16_t)(rd16(m, spr) << 4);
            uint16_t d0w = (uint16_t)(((uint32_t)d1w * (uint32_t)rd8(m, obj + 0x1Eu)) >> 9);
            wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + d0w));
        }
        if (Obj_AnimTick(m, obj, rd16(m, obj + 0x40u)) < 0)
            return -1;
        {
            uint32_t spr = rd32(m, obj + 0x10u);
            wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - (rd16(m, spr + 2u) << 3)));
            wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - (rd16(m, spr) << 3)));
        }
        draw_sprite_regs(m, r);
        wr32(m, obj, 0x0017C700u);
        return 0;
    }

    uint32_t d1 = rd16(m, obj + 0x62u);
    uint32_t rcv = sub_17B02E(m, obj);
    if ((int32_t)rcv >= 0)
        wr32(m, rcv + 0x48u, rd32(m, rcv + 0x48u) + d1);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x1Cu, 0u);

    uint32_t a1 = ((int16_t)(rd16(m, obj + 0x2Eu) - rd16(m, obj + 0x2Cu)) <= 8)
                ? 0x0017C79Cu : 0x0017C7DCu;
    uint16_t idx = rd16(m, a1);
    a1 += 2u;
    wr16(m, obj + 0x40u, idx);
    wr32(m, obj + 0x3Au, a1);
    if (Obj_AnimInit(m, obj, idx) < 0)
        return -1;
    return sub_17C614(m, r);
}

/* Dispatch death callback stored at (+0x66, A0). */
static int dispatch_death66(const f3mem_t *m, f3regs_t *r, uint32_t fn)
{
    uint32_t obj = r->a[0];
    switch (fn) {
    case 0x0017C78Eu:
        Obj_Free(m, obj);
        return 0;
    case 0x00183002u:
        Obj_Free(m, rd32(m, obj + 0x58u));
        return sub_17C59A(m, r);
    case 0x0017C59Au:
        return sub_17C59A(m, r);
    case 0x0017C704u: {
        uint32_t d1 = rd16(m, obj + 0x62u);
        uint32_t rcv = sub_17B02E(m, obj);
        if ((int32_t)rcv >= 0)
            wr32(m, rcv + 0x48u, rd32(m, rcv + 0x48u) + d1);
        if (Obj_AnimList179B06(m, obj) < 0)
            return -1;
        wr8(m, obj + 0x1Du, 0u);
        Sound_PlayHooked(m, 0x0035u);
        return sub_17C614(m, r);
    }
    case 0x0017C73Eu: {
        uint32_t d1 = rd16(m, obj + 0x62u);
        uint32_t rcv = sub_17B02E(m, obj);
        if ((int32_t)rcv >= 0)
            wr32(m, rcv + 0x48u, rd32(m, rcv + 0x48u) + d1);
        uint32_t a1 = 0x0017C81Cu;
        uint16_t idx = rd16(m, a1);
        a1 += 2u;
        wr16(m, obj + 0x40u, idx);
        wr32(m, obj + 0x3Au, a1);
        if (Obj_AnimInit(m, obj, idx) < 0)
            return -1;
        wr8(m, obj + 0x1Du, 0u);
        Sound_PlayHooked(m, 0x0035u);
        return sub_17C614(m, r);
    }
    default:
        return -1;
    }
}

/* ROM 0x1849AA: scale D0w by difficulty nibble table at 0x1849D6. */
static uint16_t sub_1849AA(const f3mem_t *m, uint16_t d0w)
{
    uint16_t sel = (uint16_t)((rd16(m, F3_A5 + 0x087Eu) >> 4) & 0x0Fu);
    uint32_t d1 = (uint32_t)rd8(m, 0x001849D6u + sel);
    int32_t d2 = (int32_t)((uint32_t)d0w << 16);
    d2 = (d2 / 0x64) * (int32_t)d1;
    uint32_t u2 = ((uint32_t)d2 << 16) | ((uint32_t)d2 >> 16);
    return (uint16_t)(d0w + (uint16_t)u2);
}

/* ROM 0x182E56..0x182FFE sub-tick helpers for 182D24 objects. */
static int sub_182E56(const f3mem_t *m, uint32_t obj)
{
    uint32_t parent = rd32(m, obj + 0x08u);
    uint16_t x = (uint16_t)(rd16(m, parent + 0x50u) + rd16(m, obj + 0x16u));
    uint16_t y = (uint16_t)(rd16(m, parent + 0x54u) + rd16(m, obj + 0x1Au));
    wr16(m, obj + 0x50u, x);
    wr16(m, obj + 0x54u, y);
    uint8_t cnt = (uint8_t)(rd8(m, obj + 0x06u) - 1u);
    wr8(m, obj + 0x06u, cnt);
    if ((int8_t)cnt <= 0) {
        wr32(m, obj + 0x44u, 0x001831E2u);
        wr16(m, obj + 0x48u, rd16(m, 0x001831E2u + 4u));
        wr16(m, obj + 0x4Au, 0u);
        wr32(m, obj + 0x0Cu, 0x00182EB2u);
        wr8(m, obj + 0x06u, 0x03u);
        Sound_PlayHooked(m, 0x0011u);
    }
    return 0;
}

static int sub_182EB2(const f3mem_t *m, uint32_t obj)
{
    uint8_t cnt = (uint8_t)(rd8(m, obj + 0x06u) - 1u);
    wr8(m, obj + 0x06u, cnt);
    if ((int8_t)cnt <= 0) {
        wr32(m, obj + 0x44u, 0x001831F6u);
        wr16(m, obj + 0x48u, rd16(m, 0x001831F6u + 4u));
        wr16(m, obj + 0x4Au, 0u);
        wr32(m, obj + 0x66u, 0x0017C59Au);
        wr32(m, obj + 0x0Cu, 0x00182F08u);
        uint32_t a2 = rd32(m, obj + 0x58u);
        wr32(m, a2, rd32(m, obj));
        wr32(m, a2 + 0x0Cu, rd32(m, obj + 0x0Cu));
        for (int i = 0; i < 0x17; i++)
            wr32(m, a2 + 0x10u + (uint32_t)i * 4u, rd32(m, obj + 0x10u + (uint32_t)i * 4u));
        wr32(m, obj + 0x66u, 0x0017C78Eu);
        wr32(m, obj + 0x24u, 0u);
    }
    return 0;
}

static int sub_182F08(const f3mem_t *m, uint32_t obj)
{
    int32_t p = (int32_t)rd32(m, F3_A5 + 0x1D28u);
    if (p <= 0)
        return 0;
    uint32_t a1 = (uint32_t)p;
    int16_t d2 = (int16_t)(rd16(m, a1 + 0x14u) + 0x18 - rd16(m, obj + 0x50u));
    if (d2 < 0)
        d2 = (int16_t)(-d2);
    if (d2 > (int16_t)sub_1849AA(m, 0x0020u))
        return 0;

    int16_t py = (int16_t)(rd16(m, a1 + 0x18u) + 0x10);
    if (py > (int16_t)rd16(m, obj + 0x54u)) {
        wr32(m, obj + 0x44u, 0x00183228u);
        wr16(m, obj + 0x48u, rd16(m, 0x00183228u + 4u));
        wr16(m, obj + 0x4Au, 0u);
        a1 = ((rd8(m, obj + 0x1Cu) & 1u) != 0u) ? 0x001831ACu : 0x00183120u;
    } else {
        wr32(m, obj + 0x44u, 0x0018320Au);
        wr16(m, obj + 0x48u, rd16(m, 0x0018320Au + 4u));
        wr16(m, obj + 0x4Au, 0u);
        a1 = ((rd8(m, obj + 0x1Cu) & 1u) != 0u) ? 0x00183184u : 0x001830C0u;
    }
    wr32(m, obj + 0x58u, a1);
    wr32(m, obj + 0x0Cu, 0x00182FB4u);
    wr8(m, obj + 0x06u, 0x0Au);
    return 0;
}

static int sub_182FC8(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x58u);
    uint16_t idx = rd16(m, a1);
    a1 += 2u;
    wr16(m, obj + 0x40u, idx);
    wr32(m, obj + 0x3Au, a1);
    if (Obj_AnimInit(m, obj, idx) < 0)
        return -1;
    uint32_t spr = rd32(m, obj + 0x10u);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - (rd16(m, spr + 2u) << 3)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - (rd16(m, spr) << 3)));
    wr32(m, obj + 0x0Cu, 0x00183000u);
    return 0;
}

/* Dispatch sub-tick callback stored at (+0x0C, A0). */
static int dispatch_cb0c(const f3mem_t *m, uint32_t obj, uint32_t fn)
{
    switch (fn) {
    case 0x00180FDCu:
        if (rd8(m, obj + 0x1Eu) != 0u) {
            wr8(m, obj + 0x1Fu, (uint8_t)(rd8(m, obj + 0x1Fu) - 8u));
            wr8(m, obj + 0x1Eu, (uint8_t)(rd8(m, obj + 0x1Eu) - 8u));
            uint32_t spr = rd32(m, obj + 0x10u);
            uint16_t d1w = (uint16_t)(rd16(m, spr) << 4);
            uint16_t d0w = (uint16_t)(((uint32_t)d1w * (uint32_t)rd8(m, obj + 0x1Fu)) >> 9);
            wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + d0w));
            wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + d0w));
        } else {
            wr32(m, obj + 0x2Cu, 0x00080018u);
            wr32(m, obj + 0x30u, 0x00080018u);
            wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~0x01u));
        }
        return 0;
    case 0x00180D96u:
        if (rd8(m, obj + 0x1Fu) != 0u) {
            wr8(m, obj + 0x1Fu, (uint8_t)(rd8(m, obj + 0x1Fu) - 0x10u));
            uint32_t spr = rd32(m, obj + 0x10u);
            uint16_t d1w = (uint16_t)(rd16(m, spr + 2u) << 4);
            uint16_t d0w = (uint16_t)(((uint32_t)d1w * (uint32_t)rd8(m, obj + 0x1Fu)) >> 9);
            wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + d0w));
        }
        return 0;
    case 0x0018363Cu: {
        uint32_t spr = rd32(m, obj + 0x10u);
        uint16_t d1w = (uint16_t)(rd16(m, spr) << 4);
        uint16_t d0w = (uint16_t)(((uint32_t)d1w * (uint32_t)rd8(m, obj + 0x1Fu)) >> 9);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + d0w));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + d0w));
        return 0;
    }
    case 0x001838E8u:
        wr16(m, obj + 0x06u, (uint16_t)(rd16(m, obj + 0x06u) - 1u));
        if ((int16_t)rd16(m, obj + 0x06u) <= 0)
            wr32(m, obj + 0x24u, 0u);
        return 0;
    case 0x00182DD4u: {
        uint32_t parent = rd32(m, obj + 0x08u);
        uint16_t x = (uint16_t)(rd16(m, parent + 0x50u) + rd16(m, obj + 0x16u));
        uint16_t y = (uint16_t)(rd16(m, parent + 0x54u) + rd16(m, obj + 0x1Au));
        wr16(m, obj + 0x50u, x);
        wr16(m, obj + 0x54u, y);
        uint8_t cnt = (uint8_t)(rd8(m, obj + 0x06u) - 1u);
        wr8(m, obj + 0x06u, cnt);
        if ((int8_t)cnt <= 0) {
            wr16(m, obj + 0x14u, x);
            wr16(m, obj + 0x18u, y);
            uint32_t a1 = ((rd8(m, obj + 0x1Cu) & 1u) != 0u) ? 0x00183088u : 0x00183050u;
            uint16_t idx = rd16(m, a1);
            a1 += 2u;
            wr16(m, obj + 0x40u, idx);
            wr32(m, obj + 0x3Au, a1);
            if (Obj_AnimInit(m, obj, idx) < 0)
                return -1;
            uint32_t spr = rd32(m, obj + 0x10u);
            wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - (rd16(m, spr + 2u) << 3)));
            wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - (rd16(m, spr) << 3)));
            wr32(m, obj + 0x0Cu, 0x00182E56u);
            wr8(m, obj + 0x06u, 0x14u);
        }
        return 0;
    }
    case 0x00182E56u:
        return sub_182E56(m, obj);
    case 0x00182EB2u:
        return sub_182EB2(m, obj);
    case 0x00182F08u:
        return sub_182F08(m, obj);
    case 0x00182FB4u: {
        uint8_t cnt = (uint8_t)(rd8(m, obj + 0x06u) - 1u);
        wr8(m, obj + 0x06u, cnt);
        if ((int8_t)cnt <= 0) {
            wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
            wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));
            return sub_182FC8(m, obj);
        }
        return 0;
    }
    case 0x00182FC8u:
        return sub_182FC8(m, obj);
    case 0x00183000u:
        return 0;
    case 0x001828E6u: {
        uint16_t n = (uint16_t)(rd16(m, obj + 0x64u) + 1u);
        wr16(m, obj + 0x64u, n);
        uint32_t a1 = ((n % 3u) == 0u) ? 0x001829B8u : 0x00182956u;
        uint16_t idx = rd16(m, a1);
        a1 += 2u;
        wr16(m, obj + 0x40u, idx);
        wr32(m, obj + 0x3Au, a1);
        return Obj_AnimInit(m, obj, idx);
    }
    default:
        return -1;
    }
}

int Fn_17BEC8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((rd8(m, F3_A5 - 0x144Cu) & 0x10u) != 0u)       /* 0x17BEC8..0x17BED2 */
        wr32(m, obj + 0x24u, 0u);

    if ((int32_t)rd32(m, obj + 0x24u) <= 0)              /* 0x17BED6..0x17BEE2 */
        return dispatch_death66(m, r, rd32(m, obj + 0x66u));

    if (Obj_Script10(m, obj) < 0)                        /* 0x17BEE4 */
        return -1;

    {
        int16_t x = (int16_t)rd16(m, obj + 0x50u);       /* 0x17BEEA */
        if (x < (int16_t)rd16(m, 0x001849F8u) || x > (int16_t)rd16(m, 0x001849FAu)) {
            Obj_Free(m, obj);                            /* 0x17C78E */
            r->d[0] = setw(r->d[0], (uint16_t)x);
            return 0;
        }
        int16_t y = (int16_t)rd16(m, obj + 0x54u);       /* 0x17BEFA */
        if (y < (int16_t)rd16(m, 0x001849FCu) || y > (int16_t)rd16(m, 0x001849FEu)) {
            Obj_Free(m, obj);                            /* 0x17C78E */
            r->d[0] = setw(r->d[0], (uint16_t)y);
            return 0;
        }
    }

    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));          /* 0x17BF0A */
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));          /* 0x17BF10 */

    {
        uint16_t anim_idx = rd16(m, obj + 0x40u);        /* 0x17BF16 */
        if (Obj_AnimTick(m, obj, anim_idx) < 0)          /* 0x17BF24 */
            return -1;
    }

    {
        uint32_t spr = rd32(m, obj + 0x10u);             /* 0x17BF26 */
        uint16_t d1w = (uint16_t)(rd16(m, spr) << 3);    /* 0x17BF2A..0x17BF30 */
        uint16_t d0w = (uint16_t)(rd16(m, spr + 2u) << 3); /* 0x17BF2C..0x17BF2E */
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0w)); /* 0x17BF32 */
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d1w)); /* 0x17BF36 */
    }

    if (dispatch_cb0c(m, obj, rd32(m, obj + 0x0Cu)) < 0) /* 0x17BF3A..0x17BF3E */
        return -1;

    {
        uint32_t p = rd32(m, F3_A5 + 0x0906u);           /* 0x17BF40 */
        wr32(m, p, obj);                                 /* 0x17BF44 */
        wr32(m, F3_A5 + 0x0906u, p + 4u);                /* 0x17BF46 */
    }

    draw_sprite_regs(m, r);                              /* 0x17BF4A */

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {       /* 0x17BF50 */
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x000A && x <= 0x011C) {               /* 0x17BF58..0x17BF66 */
            explode_tick_regs(m, r);                     /* 0x17BF68 -> 0x0CBB2C */
            return 0;
        }
    }

    wr32(m, obj, 0x0017BEC8u);                           /* 0x17BF6E -> 0x17BEC0 */
    return 0;
}
