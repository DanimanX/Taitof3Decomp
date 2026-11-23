#include "obj_F5AFE.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"

/* ROM 0x006422 */

/* ROM 0x00649A */

/* ROM 0x0CBB2C */

/* ROM 0x0CBE34 */

/* ROM 0x0CEE52 */
static void sub_0CEE52(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    uint8_t b = rd8(m, obj + 0x4Eu);
    if (b != 0u) {
        d0 = (d0 & 0xFFFFFF00u) | b;
        Obj_Spawn(m, obj, 0x00117C28u, 0u, 1u, d0);
    }
    Obj_StartDying(m, obj);
}

/* ROM 0x0CEE48 */
static void sub_0CEE48(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        Obj_StartDying(m, obj);
        return;
    }
    sub_0CEE52(m, obj, d0);
}

/* ROM 0x0CEEA2 */
static void sub_0CEEA2(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    uint8_t b = rd8(m, obj + 0x4Eu);
    if (b != 0u) {
        d0 = (d0 & 0xFFFFFF00u) | b;
        Obj_Spawn(m, obj, 0x00117C28u, 0u, 1u, d0);
    }
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            Obj_ExplodeTick(m, obj);
            return;
        }
    }
    wr32(m, obj, 0x000CEEF0u);
}

/* ROM 0x0CEE9A */
static void sub_0CEE9A(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        Obj_StartDying(m, obj);
        return;
    }
    sub_0CEEA2(m, obj, d0);
}

/* ROM 0x0D0690 */
static void sub_0D0690(const f3mem_t *m, uint32_t obj)
{
    uint16_t t = (uint16_t)(rd16(m, obj + 0x4Cu) - 1u);
    wr16(m, obj + 0x4Cu, t);
    if (t != 0u)
        return;

    uint32_t a1 = rd32(m, obj + 0x48u) + 0x0Cu;
    wr32(m, obj + 0x48u, a1);
    uint16_t d0 = rd16(m, a1);
    a1 += 2u;
    if ((int16_t)d0 < 0) {
        uint16_t back = rd16(m, a1);
        if (back == 0u)
            return;
        a1 -= (uint32_t)(int32_t)(int16_t)back;
        wr32(m, obj + 0x48u, a1);
        wr16(m, obj + 0x4Cu, rd16(m, a1));
        a1 += 2u;
    } else {
        wr16(m, obj + 0x4Cu, d0);
    }
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x0D0DC2 */
static void sub_0D0DC2(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x8FEu);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x8FEu, a1 + 4u);
    Coll_AddHazard(m, obj);
}

/* ROM 0x0D0E04 */
static void sub_0D0E04(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    uint8_t hit = rd8(m, obj + 0x35u);
    if (hit == 1u) {
        uint32_t a1 = rd32(m, rd32(m, obj + 0x36u) + 0x08u);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
    } else if (hit == 2u) {
        uint32_t a1 = rd32(m, obj + 0x36u);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
    }
}

/* ROM 0x0D0E30 */

/* ROM 0x0F5AFE */
int Obj_F5AFE_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            Obj_RelayTimer(m, 0x1388u);
            wr8(m, obj + 0x06u, 0xFFu);
            sub_0CEE9A(m, obj, 0x1388u);
            return 0;
        }
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        sub_0D0E04(m, obj, 0x1388u);
    } else if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) == 0u) {
        wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - rd32(m, F3_A5 + 0x4228u));

        uint8_t f7 = rd8(m, obj + 0x07u);
        int had_bit1 = (f7 & 0x02u) != 0u;
        wr8(m, obj + 0x07u, (uint8_t)(f7 & ~0x02u));
        if (had_bit1) {
            if (ObjScript_CF1E4(m, obj) < 0)
                return -1;
        }

        if (rd8(m, obj + 0x20u) != 0u) {
            if (ObjScript_CF4B8(m, obj, 0x000F5C12u) < 0)
                return -1;
        }

        int16_t y3e = (int16_t)rd16(m, obj + 0x3Eu);
        if (y3e < -0x30 || y3e > 0x0118) {
            wr8(m, obj + 0x06u, 0xFFu);
            Obj_StartDying(m, obj);
            return 0;
        }

        wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
        wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
        sub_0D0690(m, obj);
        sub_0D0DC2(m, obj);
        Obj_BlinkColour(m, obj);
        return 0;
    }

    uint32_t child = Obj_Spawn(m, obj, 0x000DC486u, 3u, 2u, 0u);
    if ((int32_t)child >= 0) {
        wr32(m, child + 0x14u, rd32(m, obj + 0x14u));
        wr16(m, child + 0x14u, (uint16_t)(rd16(m, child + 0x14u) + 0x10u));
        wr32(m, child + 0x18u, rd32(m, obj + 0x18u));
        wr16(m, child + 0x18u, (uint16_t)(rd16(m, child + 0x18u) + 0x08u));
        wr16(m, child + 0x58u, rd16(m, obj + 0x58u));
        wr16(m, child + 0x60u, rd16(m, obj + 0x60u));
    }
    wr8(m, obj + 0x06u, 0u);
    sub_0CEE48(m, obj, child);
    return 0;
}
