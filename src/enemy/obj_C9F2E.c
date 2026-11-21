#include "obj_C9F2E.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"
#include "../audio/sound_cmd.h"

#define ASR16(v, n) ((int16_t)(((int16_t)(v) < 0) \
    ? ~(uint16_t)((uint16_t)~(uint16_t)(v) >> (n)) \
    : ((uint16_t)(v) >> (n))))

/* ROM 0x006422 */
static uint16_t sub_006422(const f3mem_t *m, uint16_t d0, uint16_t d1)
{
    uint32_t a0 = 0x00026E40u;
    uint16_t d2 = 0u;

    if ((int16_t)d0 < 0) {
        d2 |= 0x0002u;
        d0 = (uint16_t)(0u - d0);
    }
    if ((int16_t)d1 < 0) {
        d2 |= 0x0004u;
        d1 = (uint16_t)(0u - d1);
    }

    uint16_t src = ((int16_t)d1 > (int16_t)d0) ? d1 : d0;
    uint16_t d3 = 0x0020u;
    for (uint16_t i = 0u; i < 16u; i++) {
        if ((src & (uint16_t)(0x8000u >> i)) != 0u) {
            d3 = (uint16_t)(0x0010u + i);
            break;
        }
    }

    d3 = (uint16_t)(d3 - 0x001Au);
    if ((int16_t)d3 < 0) {
        d3 = (uint16_t)(0u - d3);
        unsigned sh = (unsigned)(d3 & 63u);
        d0 = (sh >= 16u) ? 0u : (uint16_t)(d0 >> sh);
        d1 = (sh >= 16u) ? 0u : (uint16_t)(d1 >> sh);
    }

    if (d2 != 0u && d2 != 6u) {
        uint16_t tmp = d0;
        d0 = d1;
        d1 = tmp;
    }

    d1 = (uint16_t)(d1 << 6);
    a0 += (uint32_t)(int32_t)(int16_t)d1;
    uint8_t tbl = rd8(m, a0 + (uint32_t)(int32_t)(int16_t)d0);
    d0 = (uint16_t)((d0 & 0xFF00u) | (uint16_t)tbl);
    d0 = (uint16_t)(d0 + rd16(m, 0x00006492u + (uint32_t)(int32_t)(int16_t)d2));
    return (uint16_t)(d0 & 0x00FFu);
}

/* ROM 0x0CA022 */
static void sub_0CA022(const f3mem_t *m, uint32_t obj)
{
    if ((int16_t)rd16(m, obj + 0x46u) >= 0x24)
        return;

    wr16(m, obj + 0x46u, (uint16_t)(rd16(m, obj + 0x46u) + 1u));
    uint16_t d0 = rd16(m, obj + 0x44u);
    wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + d0));
    d0 = (uint16_t)ASR16(d0, 2u);
    wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + d0));
}

/* ROM 0x0CA040 */
static void sub_0CA040(const f3mem_t *m, uint32_t obj)
{
    uint32_t save_14 = rd32(m, obj + 0x14u);
    uint32_t save_18 = rd32(m, obj + 0x18u);
    uint32_t save_42 = rd32(m, obj + 0x42u);

    Obj_Animate(m, obj, 0x000CA116u);

    uint32_t a1 = rd32(m, obj + 0x10u);
    int16_t d1 = (int16_t)rd16(m, obj + 0x22u);
    wr32(m, obj + 0x10u, rd32(m, a1 + (uint32_t)((int32_t)d1 * 4)));

    uint16_t ang = (uint16_t)rd8(m, obj + 0x20u);
    wr16(m, obj + 0x42u, (uint16_t)(-0x38));
    Obj_MoveByAngle(m, obj, ang);
    Obj_DrawSprite(m, obj);

    wr32(m, obj + 0x42u, save_42);
    wr32(m, obj + 0x18u, save_18);
    wr32(m, obj + 0x14u, save_14);

    a1 = 0u;
    uint32_t a2 = 0x0041A988u;
    while (a2 != rd32(m, F3_A5 + 0x08FEu)) {
        uint32_t a3 = rd32(m, a2);
        a2 += 4u;
        if ((rd8(m, a3 + 0x34u) & 0x08u) != 0u)
            continue;
        a1 = a3;
        if ((rd8(m, a3 + 0x34u) & 0x04u) != 0u)
            continue;
        break;
    }

    if (a1 == 0u)
        return;

    uint16_t d2 = (uint16_t)(rd16(m, a1 + 0x2Eu) + rd16(m, a1 + 0x2Cu));
    d2 >>= 1;
    uint16_t d0 = (uint16_t)(rd16(m, a1 + 0x14u) + d2);
    d0 = (uint16_t)(d0 - rd16(m, obj + 0x14u));
    d0 = (uint16_t)(d0 - 8u);

    d2 = (uint16_t)(rd16(m, a1 + 0x32u) + rd16(m, a1 + 0x30u));
    d2 >>= 1;
    uint16_t dy = (uint16_t)(rd16(m, a1 + 0x18u) + d2);
    dy = (uint16_t)(dy - rd16(m, obj + 0x18u));
    dy = (uint16_t)(dy - 8u);

    d0 = sub_006422(m, d0, dy);
    d0 = (uint16_t)(d0 << 8);
    d0 = (uint16_t)(d0 - rd16(m, obj + 0x20u));

    if ((int16_t)d0 >= 0x0200)
        d0 = 0x0200u;
    else if ((int16_t)d0 <= -0x0200)
        d0 = (uint16_t)(-0x0200);
    else
        d0 = (uint16_t)ASR16(d0, 1u);

    wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + d0));
}

/* ROM 0x0CA18A */
static void sub_0CA18A(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0x00027E40u;
    uint16_t d0w = (uint16_t)(rd16(m, obj + 0x42u) << 1);
    uint16_t d1w = d0w;
    uint16_t d2w = (uint16_t)(rd16(m, obj + 0x20u) >> 8);

    uint32_t d0 = (uint32_t)((int32_t)(int16_t)d0w *
                             (int32_t)(int16_t)rd16(m, a1 + 0x80u + (uint32_t)d2w * 2u));
    uint32_t d1 = (uint32_t)((int32_t)(int16_t)d1w *
                             (int32_t)(int16_t)rd16(m, a1 + (uint32_t)d2w * 2u));

    d0 += rd32(m, obj + 0x14u);
    int16_t x = (int16_t)(uint16_t)((uint16_t)(d0 >> 16) + 8u);
    d1 += rd32(m, obj + 0x18u);
    int16_t y = (int16_t)(uint16_t)((uint16_t)(d1 >> 16) + 8u);

    if (y > 0 && y < 0xE8 && Terrain_TestPoint(m, x, y) == 0)
        return;

    wr8(m, obj + 0x4Cu, 0xFFu);
    wr8(m, obj + 0x4Eu, 0x00u);
    wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) >> 2));

    uint16_t idx = (uint16_t)rd8(m, rd32(m, obj + 0x0Cu) + 0x02u);
    wr16(m, obj + 0x20u, rd16(m, 0x000CA204u + (uint32_t)idx * 2u));
}

/* ROM 0x13E4D0 */
static uint32_t sub_13E4D0(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    uint32_t a1 = rd32(m, obj + 0x10u);
    uint16_t w0 = (uint16_t)(rd16(m, a1) << 3);
    a1 += 2u;
    d0 = (d0 & 0xFF00FFFFu) | (((uint32_t)w0 & 0xFFu) << 16);

    uint16_t w1 = (uint16_t)(rd16(m, a1) << 3);
    d0 = (d0 & 0x00FFFFFFu) | (((uint32_t)w1 & 0xFFu) << 24);
    return d0;
}

/* ROM 0x13F238 */
static int16_t sub_13F238(const f3mem_t *m, int16_t cmd)
{
    if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
        cmd = (int16_t)(uint16_t)((uint16_t)cmd + 0x53u);
    return cmd;
}

/* ROM 0x11819A */
static void sub_11819A(const f3mem_t *m, uint32_t obj, uint16_t d1w, uint16_t d2w)
{
    if (rd8(m, F3_A5 + 0x30F4u) == 0u)
        return;
    if ((int16_t)rd16(m, F3_A5 + 0x30C0u) >= 0x20)
        return;

    d1w = (uint16_t)(d1w - rd16(m, F3_A5 + 0x30D2u));
    d2w = (uint16_t)(d2w - rd16(m, F3_A5 + 0x30D6u));
    uint32_t d0 = ((uint32_t)d1w << 16) | (uint32_t)d2w;

    int16_t q1 = (int16_t)((int32_t)(int16_t)d1w / 16);
    int16_t q2 = (int16_t)((int32_t)(int16_t)d2w / 16);
    uint32_t d2 = ((uint32_t)(uint16_t)q2 << 16) | (uint32_t)(uint16_t)q1;

    uint32_t a1 = 0x0041B50Eu;
    int16_t cnt = (int16_t)rd16(m, F3_A5 + 0x30C0u);
    for (;;) {
        cnt = (int16_t)(uint16_t)((uint16_t)cnt - 1u);
        if (cnt < 0)
            break;
        uint32_t cur = rd32(m, a1);
        a1 += 4u;
        if (cur == d2)
            return;
    }

    wr32(m, a1, d2);
    uint32_t spawned = Obj_Spawn(m, obj, 0x00118242u, 3u, 2u, d0);
    if ((int32_t)spawned < 0)
        return;

    wr16(m, F3_A5 + 0x30C0u, (uint16_t)(rd16(m, F3_A5 + 0x30C0u) + 1u));

    int16_t cmd = sub_13F238(m, 0x0033);
    if (m->dpram) {
        Sound_PlayCmd(m, cmd);
    } else {
        uint8_t tmp_dpram[F3_DPRAM_SIZE] = {0};
        f3mem_t tmp_m = *m;
        tmp_m.dpram = tmp_dpram;
        Sound_PlayCmd(&tmp_m, cmd);
    }

    uint32_t arg2 = sub_13E4D0(m, obj, 2u);
    Obj_Spawn(m, obj, 0x0013E4EEu, 5u, 2u, arg2);
}

/* ROM 0x0CA514 */
static void sub_0CA514(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x35u) == 0u)
        return;

    uint32_t a1 = rd32(m, obj + 0x36u);
    int8_t grp = (int8_t)rd8(m, a1 + 0x04u);
    if (grp < 3 || grp >= 5)
        return;

    uint16_t d1w = (uint16_t)(rd16(m, obj + 0x2Cu) + rd16(m, obj + 0x2Eu));
    d1w = (uint16_t)ASR16(d1w, 1u);
    uint16_t d2w = (uint16_t)(rd16(m, obj + 0x30u) + rd16(m, obj + 0x32u));
    d2w = (uint16_t)ASR16(d2w, 1u);
    d1w = (uint16_t)(d1w + rd16(m, obj + 0x14u));
    d2w = (uint16_t)(d2w + rd16(m, obj + 0x18u));

    sub_11819A(m, obj, d1w, d2w);
    wr8(m, obj + 0x35u, 0x00u);
}

/* ROM 0x0C9F2E */
int Obj_C9F2E_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t d0 = 0u;

    sub_0CA514(m, obj);

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        /* LAB_000c9fe8 */
        uint32_t a1 = rd32(m, obj + 0x36u);
        d0 = (uint32_t)((rd8(m, a1 + 0x34u) >> 1) & 1u);
        goto lab_0c9ff8;
    }

    uint16_t ang = (uint16_t)rd8(m, obj + 0x20u);
    uint16_t d1 = ang;
    uint16_t d2 = (uint16_t)((d1 >> 6) & 3u);
    uint32_t a1 = rd32(m, obj + 0x48u);
    d1 = (uint16_t)(((uint16_t)(d1 + 8u) >> 4) & 7u);
    wr16(m, obj + 0x22u, d1);
    wr32(m, obj + 0x10u, rd32(m, a1 + (uint32_t)d1 * 4u));
    wr8(m, obj + 0x1Cu, rd8(m, 0x000CC10Eu + (uint32_t)d2));

    Obj_MoveByAngle(m, obj, ang);
    Obj_DrawSprite(m, obj);

    {
        int16_t tx = (int16_t)(uint16_t)(rd16(m, obj + 0x14u) + 8u);
        int16_t ty = (int16_t)(uint16_t)(rd16(m, obj + 0x18u) + 8u);
        if (Terrain_TestPoint(m, tx, ty) != 0) {
            /* LAB_000c9ff6 */
            d0 = 1u;
            goto lab_0c9ff8;
        }
    }

    {
        uint32_t p = rd32(m, F3_A5 + 0x0902u);
        wr32(m, p, obj);
        wr32(m, F3_A5 + 0x0902u, p + 4u);
    }

    if (rd8(m, obj + 0x4Cu) == 0u)
        sub_0CA022(m, obj);
    else
        sub_0CA040(m, obj);

    if (rd8(m, obj + 0x4Eu) != 0u)
        sub_0CA18A(m, obj);

    if ((int16_t)rd16(m, obj + 0x42u) < 0x20)
        wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) + 1u));

    {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        int16_t xlo = (int16_t)rd16(m, 0x000CC112u);
        int16_t xhi = (int16_t)rd16(m, 0x000CC114u);
        if (x < xlo || x > xhi)
            goto lab_0ca00c;

        int16_t y = (int16_t)rd16(m, obj + 0x18u);
        int16_t ylo = (int16_t)rd16(m, 0x000CC116u);
        int16_t yhi = (int16_t)rd16(m, 0x000CC118u);
        if (y < ylo || y > yhi)
            goto lab_0ca00c;
    }

    /* LAB_000c9f26 */
    wr32(m, obj, 0x000C9F2Eu);
    return 0;

lab_0c9ff8:
    if (rd8(m, obj + 0x4Du) != 0u) {
        /* LAB_000ca20c */
        wr8(m, obj + 0x22u, 0x05u);
        wr32(m, obj + 0x24u, 0x27100000u);
        wr16(m, obj + 0x1Eu, 0x0000u);

        uint32_t idx = (uint32_t)(rd8(m, obj + 0x4Du) & 0x07u);
        uint32_t rec = 0x000CA2ECu + idx * 4u;
        wr32(m, obj + 0x28u, (idx << 8) + 0x00001400u);

        uint16_t v0 = (uint16_t)(int16_t)(int8_t)rd8(m, rec++);
        wr16(m, obj + 0x2Cu, v0);
        wr16(m, obj + 0x30u, v0);

        uint16_t v1 = (uint16_t)(int16_t)(int8_t)rd8(m, rec++);
        wr16(m, obj + 0x2Eu, v1);
        wr16(m, obj + 0x32u, v1);

        uint8_t v2 = rd8(m, rec++);
        wr8(m, obj + 0x1Fu, v2);
        wr8(m, obj + 0x1Eu, v2);

        uint16_t off = (uint16_t)((uint16_t)v2 - 0x0100u);
        off = (uint16_t)((uint16_t)ASR16(off, 4u) + 8u);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + off));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + off));

        wr16(m, obj + 0x3Au, 0u);
        wr16(m, obj + 0x3Eu, 0u);
        wr16(m, obj + 0x06u, 0x0014u);

        /* LAB_000ca284 */
        wr32(m, obj, 0x000CA28Cu);
        return 0;
    }

    {
        uint32_t parent = rd32(m, obj + 0x08u);
        wr16(m, parent + 0x5Cu, (uint16_t)(rd16(m, parent + 0x5Cu) + 1u));
    }

    /* LAB_000caa26 */
    wr8(m, obj + 0x21u, rd8(m, obj + 0x20u));
    wr8(m, obj + 0x20u, 0x00u);
    {
        uint16_t move_ang = rd16(m, obj + 0x20u);
        wr16(m, obj + 0x42u, 0x0010u);
        Obj_MoveByAngle(m, obj, move_ang);
    }
    wr32(m, obj + 0x44u, rd32(m, obj + 0x14u));
    wr16(m, obj + 0x44u, (uint16_t)(rd16(m, obj + 0x44u) + 8u));
    wr32(m, obj + 0x48u, rd32(m, obj + 0x18u));
    wr16(m, obj + 0x48u, (uint16_t)(rd16(m, obj + 0x48u) + 8u));

    if ((uint16_t)d0 == 0u) {
        wr8(m, obj + 0x1Du, 0x00u);
        wr32(m, obj + 0x36u, 0x000CAAE8u);
        wr16(m, obj + 0x42u, 0x0000u);
        wr8(m, obj + 0x4Cu, 0x00u);
    } else {
        /* LAB_000caa7c */
        wr8(m, obj + 0x1Du, 0x00u);
        wr32(m, obj + 0x36u, 0x000CAB02u);
        uint16_t r = (uint16_t)Rng_NextMem(m);
        uint8_t delta = (uint8_t)((uint8_t)(r & 0x3Fu) + 0x60u);
        wr8(m, obj + 0x21u, (uint8_t)(rd8(m, obj + 0x21u) + delta));
        wr16(m, obj + 0x42u, 0x0000u);
        wr8(m, obj + 0x4Cu, 0x00u);
    }

    /* LAB_000cab22 */
    wr16(m, obj + 0x1Eu, 0x0000u);
    wr16(m, obj + 0x3Au, 0x0000u);
    wr16(m, obj + 0x3Eu, 0x0000u);
    wr32(m, obj, 0x000CAB36u);
    return 0;

lab_0ca00c:
    {
        uint32_t parent = rd32(m, obj + 0x08u);
        wr16(m, parent + 0x5Cu, (uint16_t)(rd16(m, parent + 0x5Cu) + 1u));
        Obj_Free(m, obj);
        return 0;
    }
}
