#include "obj_1865A0.h"
#include <stddef.h>
#include "../objects/obj_anim.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

/* ROM 0x0CC2BC: D0w = (A5+0x878) mod max(1, ((0x180 - A5+0x87E) * d0) >> 8) */
static uint16_t sub_0CC2BC(const f3mem_t *m, uint16_t d0w)
{
    uint16_t d1 = (uint16_t)(0x0180u - rd16(m, F3_A5 + 0x087Eu));
    uint32_t prod = (uint32_t)d0w * (uint32_t)d1;
    uint16_t mod = (uint16_t)((prod << 8) >> 16);
    if (mod == 0u)
        mod = 1u;
    return (uint16_t)(rd16(m, F3_A5 + 0x0878u) % mod);
}

/* ROM 0x185A0A: part callback used by 0x186972 */
static int sub_185A0A(const f3mem_t *m, uint32_t obj)
{
    if (rd16(m, F3_A5 + 0x4B38u) == 1u)
        return 0;
    if (rd16(m, obj + 0x59u) != 0u)
        return 0;
    if ((int16_t)rd16(m, obj + 0x3Eu) < 0) {
        uint16_t idx = rd16(m, 0x001869A2u);
        wr16(m, obj + 0x40u, idx);
        wr32(m, obj + 0x3Au, 0x001869A4u);
        if (Obj_AnimInit(m, obj, idx) < 0)
            return -1;
    }
    if ((rd8(m, F3_A5 + 0x4B3Au) & 0x08u) != 0u &&
        rd32(m, obj + 0x10u) == 0x00031FB0u &&
        sub_0CC2BC(m, 0x0014u) == 0u) {
        (void)Obj_Spawn(m, obj, 0x0017D912u, 9u, 2u, 0xFBF800F0u);
    }
    return 0;
}

/* ROM 0x17AFD4: steer angle (+0x20) toward target object (+0x4C) */
static void sub_17AFD4(const f3mem_t *m, uint32_t obj, uint32_t tgt,
                       uint16_t x, uint16_t y, uint16_t step)
{
    if ((int32_t)tgt <= 0)
        return;
    uint16_t d0 = Obj_AtanByte(m,
        (int16_t)((int16_t)rd16(m, tgt + 0x14u) + 0x18 - (int16_t)x),
        (int16_t)((int16_t)rd16(m, tgt + 0x18u) + 0x10 - (int16_t)y));
    int8_t diff = (int8_t)((uint8_t)d0 - rd8(m, obj + 0x20u));
    int16_t d1 = (int16_t)step;
    if (diff == 0)
        d1 = 0;
    else if (diff < 0)
        d1 = (int16_t)(-d1);
    wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + (uint16_t)d1));
}

/* ROM 0x185A94: part callback */
static int sub_185A94(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 + 0x4B3Au) & 0x40u) != 0u &&
        rd16(m, obj + 0x59u) == 0u) {
        uint16_t d0 = 0x000Au;
        if ((rd8(m, F3_A5 + 0x4B3Au) & 0x20u) != 0u) {
            wr8(m, obj + 0x20u, (uint8_t)(rd8(m, obj + 0x20u) + 4u));
            d0 = 0x0004u;
        }
        int16_t cnt = (int16_t)(rd16(m, obj + 0x62u) - 1);
        wr16(m, obj + 0x62u, (uint16_t)cnt);
        if (cnt >= 0) {
            if (sub_0CC2BC(m, d0) != 0u)
                return 0;
            uint32_t a1 = rd32(m, obj + 0x4Cu);
            int16_t dx = (int16_t)((int16_t)rd16(m, a1 + 0x14u) + 0x18 -
                                   (int16_t)rd16(m, obj + 0x50u));
            int16_t dy = (int16_t)((int16_t)rd16(m, a1 + 0x18u) + 0x10 -
                                   (int16_t)rd16(m, obj + 0x54u));
            uint16_t dist = Obj_DistScaled(m, dx, dy, NULL);
            if ((int16_t)dist <= 0x40)
                return 0;
            uint8_t b = rd8(m, obj + 0x20u);
            if ((int8_t)b >= 0) {
                b = (uint8_t)(b - 0x80u);
                if ((int8_t)b < 0)
                    b = (uint8_t)(-(int8_t)b);
                b = (uint8_t)(b + 0x80u);
            }
            b = (uint8_t)((b + 8u) & 0xF0u);
            (void)Obj_Spawn(m, obj, 0x0017D136u, 9u, 2u, 0xFE070C00u | b);
            return 0;
        }
    }
    wr8(m, F3_A5 + 0x4B3Au, (uint8_t)(rd8(m, F3_A5 + 0x4B3Au) & ~0x40u));
    wr16(m, obj + 0x62u, 0x0032u);
    uint32_t tgt = rd32(m, F3_A5 + 0x1D28u);
    wr32(m, obj + 0x4Cu, tgt);
    sub_17AFD4(m, obj, tgt, rd16(m, obj + 0x50u), rd16(m, obj + 0x54u), 0x0100u);
    return 0;
}

/* ROM 0x185B6E: part callback */
static int sub_185B6E(const f3mem_t *m, uint32_t obj)
{
    if (rd16(m, F3_A5 + 0x4B38u) == 3u && rd32(m, obj + 0x64u) == 0u) {
        uint32_t child = Obj_Spawn(m, obj, 0x0017A50Eu, 3u, 2u, 0x00000808u);
        wr32(m, obj + 0x64u, child);
    }
    if ((rd8(m, F3_A5 + 0x4B3Bu) & 0x02u) != 0u ||
        (int16_t)rd16(m, F3_A5 + 0x4B38u) >= 4) {
        wr32(m, obj + 0x24u, 0u);
    }
    return 0;
}

/* ROM 0x185C50: part callback */
static int sub_185C50(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 + 0x4B3Bu) & 0x02u) != 0u) {
        uint32_t a1 = rd32(m, obj + 0x0Cu);
        wr16(m, obj + 0x16u, rd16(m, a1 + 0x1Cu));
        wr16(m, obj + 0x1Au, rd16(m, a1 + 0x1Eu));
        wr8(m, obj + 0x34u, 0x14u);
    }
    return 0;
}

/* ROM 0x1865A0: boss part tick (record at +0x0C) */
int Obj_1865A0_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a3 = rd32(m, obj + 0x0Cu);
    if (rd16(m, F3_A5 + 0x4B38u) == 0xFFFFu) {
        Obj_Free(m, obj);
        return 0;
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0)
        return -1; /* dynamic jump (a3+0x24) on death */

    uint32_t cb = rd32(m, a3 + 0x14u);
    if (cb == 0x00185A0Au) {
        if (sub_185A0A(m, obj) < 0)
            return -1;
    } else if (cb == 0x00185A94u) {
        if (sub_185A94(m, obj) < 0)
            return -1;
    } else if (cb == 0x00185B6Eu) {
        if (sub_185B6E(m, obj) < 0)
            return -1;
    } else if (cb == 0x00185C50u) {
        if (sub_185C50(m, obj) < 0)
            return -1;
    } else if (cb != 0u) {
        return -1;
    }

    uint32_t a4 = rd32(m, obj + 0x08u);
    uint16_t x = (uint16_t)(rd16(m, a4 + 0x50u) + rd16(m, obj + 0x16u));
    uint16_t y = (uint16_t)(rd16(m, a4 + 0x54u) + rd16(m, obj + 0x1Au));
    wr16(m, obj + 0x50u, x);
    wr16(m, obj + 0x54u, y);
    wr16(m, obj + 0x14u, x);
    wr16(m, obj + 0x18u, y);
    wr16(m, obj + 0x1Eu, rd16(m, a4 + 0x1Eu));
    wr32(m, obj + 0x58u, rd32(m, a4 + 0x58u));

    if (rd8(m, obj + 0x1Du) == 0x06u)
        wr8(m, obj + 0x1Du, 0x08u);
    else if (rd8(m, obj + 0x35u) != 0u)
        wr8(m, obj + 0x1Du, 0x06u);
    else
        wr8(m, obj + 0x1Du, rd8(m, F3_A5 + 0x4B36u));
    wr8(m, obj + 0x35u, 0u);

    if (Obj_AnimTick(m, obj, rd16(m, obj + 0x40u)) < 0)
        return -1;

    uint32_t a1 = rd32(m, obj + 0x10u);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - (uint16_t)(rd16(m, a1 + 2u) << 3)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - (uint16_t)(rd16(m, a1) << 3)));

    Obj_BossMapScreenPos(m, obj);
    Obj_DrawSprite(m, obj);

    if ((rd8(m, F3_A5 + 0x4B3Bu) & 0x01u) == 0u) {
        uint32_t p1 = rd32(m, F3_A5 + 0x08FEu);
        wr32(m, p1, obj);
        wr32(m, F3_A5 + 0x08FEu, p1 + 4u);
        uint32_t p2 = rd32(m, F3_A5 + 0x0906u);
        wr32(m, p2, obj);
        wr32(m, F3_A5 + 0x0906u, p2 + 4u);
    }

    wr32(m, obj + 0x00u, 0x001865A0u);
    return 0;
}
