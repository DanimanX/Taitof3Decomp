#include "obj_104F26.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"

/* ROM 0x0D0690 */
static void sub_0D0690(const f3mem_t *m, uint32_t obj)
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
        if (back == 0)
            return;
        a1 -= (uint32_t)(int32_t)back;
        wr32(m, obj + 0x48u, a1);
        wr16(m, obj + 0x4Cu, rd16(m, a1));
        a1 += 2u;
        wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
        a1 += 2u;
        wr32(m, obj + 0x10u, rd32(m, a1));
        a1 += 4u;
        wr32(m, obj + 0x68u, rd32(m, a1));
        return;
    }

    wr16(m, obj + 0x4Cu, d0);
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x105084 */
static void lab_105084(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x14u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x18u));
    wr8(m, obj + 0x1Du, 0u);
    wr8(m, obj + 0x06u, 2u);
}

/* ROM 0x10504C / 0x10509C common head: follow the parent; returns parent */
static uint32_t follow_parent(const f3mem_t *m, uint32_t obj)
{
    uint32_t p = rd32(m, obj + 0x08u);
    wr8(m, obj + 0x1Du, rd8(m, p + 0x1Du));
    wr32(m, obj + 0x14u, rd32(m, p + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, p + 0x18u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x18u));
    return p;
}

/* jump table 0x10503C */
static int state_10503C(const f3mem_t *m, uint32_t obj, uint8_t st)
{
    switch (st) {
    case 0: { /* 0x10504C */
        uint32_t p = follow_parent(m, obj);
        if (rd8(m, p + 0x06u) >= 3u) {
            lab_105084(m, obj);
            return 0;
        }
        if ((int16_t)rd16(m, obj + 0x14u) <= 0x12C)
            wr8(m, obj + 0x06u, 1u);
        return 0;
    }
    case 1: { /* 0x10509C */
        uint32_t p = follow_parent(m, obj);
        if (rd8(m, p + 0x06u) >= 3u) {
            lab_105084(m, obj);
            return 0;
        }
        if ((int16_t)rd16(m, obj + 0x68u) >= 0)
            return 0;
        uint32_t c = Obj_Spawn(m, obj, 0x000DB074u, 9u, 2u, 0u);
        if ((int32_t)c < 0)
            return 0;
        wr32(m, c + 0x14u, rd32(m, obj + 0x14u));
        wr16(m, c + 0x14u, (uint16_t)(rd16(m, c + 0x14u) + 0x0Fu));
        wr32(m, c + 0x18u, rd32(m, obj + 0x18u));
        wr16(m, c + 0x18u, (uint16_t)(rd16(m, c + 0x18u) + 0x22u));
        wr16(m, c + 0x58u, 0x00B4u);
        wr16(m, c + 0x10u, 0u);
        return 0;
    }
    case 2: { /* 0x105110 */
        uint8_t f7 = rd8(m, obj + 0x07u);
        wr8(m, obj + 0x07u, (uint8_t)(f7 & (uint8_t)~0x02u));
        if (f7 & 0x02u) {
            if (ObjScript_CF1E4(m, obj) < 0)
                return -1;
        }
        if (rd8(m, obj + 0x20u) != 0u) {
            if (ObjScript_CF4B8(m, obj, 0x001086FCu) < 0)
                return -1;
        }
        wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
        wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
        if ((int16_t)rd16(m, obj + 0x18u) >= 0xF0)
            wr8(m, obj + 0x06u, 3u);
        return 0;
    }
    default: /* 3: 0x10514E */
        return 0;
    }
}

/* ROM 0x104F26 */
int Obj_104F26_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t parent = rd32(m, obj + 0x08u);
    if (rd8(m, parent + 0x06u) == 0u) {
        /* 0x104FE4 */
        if (rd8(m, obj + 0x06u) != 3u) {
            uint32_t c = Obj_Spawn(m, obj, 0x000DC076u, 3u, 2u, 0u);
            if ((int32_t)c >= 0) {
                wr32(m, c + 0x14u, rd32(m, obj + 0x14u));
                wr16(m, c + 0x14u, (uint16_t)(rd16(m, c + 0x14u) + 0x14u));
                wr32(m, c + 0x18u, rd32(m, obj + 0x18u));
                wr16(m, c + 0x18u, (uint16_t)(rd16(m, c + 0x18u) + 0x18u));
                wr16(m, c + 0x58u, 0u);
                wr16(m, c + 0x60u, 0u);
            }
        }
        Obj_StartDying(m, obj);
        return 0;
    }

    uint8_t st = rd8(m, obj + 0x06u);
    if (st > 3u)
        return -1; /* jump table 0x10503C has 4 entries */
    sub_0D0690(m, obj);
    if (state_10503C(m, obj, st) < 0)
        return -1;

    /* 0x104F46 */
    Obj_DrawSprite(m, rd32(m, obj + 0x08u));
    Obj_DrawSprite(m, obj);
    uint32_t a4 = rd32(m, obj + 0x10u);
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x14u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x18u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 3u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x1Bu));
    if (rd16(m, obj + 0x68u) != 0u) {
        wr32(m, obj + 0x10u, ((int16_t)rd16(m, obj + 0x68u) < 0) ? 0x0002B460u : 0x0002B458u);
        Obj_DrawSprite(m, obj);
    }
    /* 0x104F9A */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 6u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 0x1Cu));
    uint8_t t = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
    wr8(m, obj + 0x23u, t);
    if (t == 0u) {
        wr8(m, obj + 0x23u, 0x0Au);
        wr8(m, obj + 0x22u, (uint8_t)((rd8(m, obj + 0x22u) + 1u) & 3u));
    }
    wr32(m, obj + 0x10u, rd32(m, 0x00105150u + (uint32_t)rd8(m, obj + 0x22u) * 4u));
    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x10u, a4);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
    return 0;
}
