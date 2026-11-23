#include "obj_E077A.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"

#define ASR16(v, n) ((int16_t)(((int16_t)(v) < 0) \
    ? ~(uint16_t)((uint16_t)~(uint16_t)(v) >> (n)) \
    : ((uint16_t)(v) >> (n))))

#define ASR32(v, n) ((int32_t)(((int32_t)(v) < 0) \
    ? ~(uint32_t)((uint32_t)~(uint32_t)(v) >> (n)) \
    : ((uint32_t)(v) >> (n))))

/* ROM 0x006422 */

/* ROM 0x00649A */

/* ROM 0x0CBB2C */

/* ROM 0x0CBE34 */

/* ROM 0x0CF7FE */
static void sub_0CF7FE(const f3mem_t *m, uint32_t obj)
{
    uint8_t flags = rd8(m, obj + 0x07u);
    if (!(flags & 0x80u))
        return;

    int16_t x = (int16_t)rd16(m, obj + 0x3Au);
    if (x < -0x40 || x > 0x014F)
        return;

    flags = (uint8_t)((flags & (uint8_t)~0x80u) | 0x02u);
    wr8(m, obj + 0x07u, flags);
}

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

/* ROM 0x0D0DC2 */
static void sub_0D0DC2(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x08FEu, a1 + 4u);
    Coll_AddHazard(m, obj);
}

/* ROM 0x0D0E04 */
static void sub_0D0E04(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    uint8_t hit_by = rd8(m, obj + 0x35u);
    if (hit_by == 1u) {
        uint32_t a1 = rd32(m, obj + 0x36u);
        a1 = rd32(m, a1 + 0x08u);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
    } else if (hit_by == 2u) {
        uint32_t a1 = rd32(m, obj + 0x36u);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
    }
}

/* ROM 0x0D0E30 */

/* ROM 0x0CEF0E: leave the screen (or detonate during a bomb) */
static void sub_0CEF0E(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, F3_A5 - 0x144Bu) & 0x20u) {
        Obj_StartDying(m, obj);
        return;
    }
    if (rd8(m, F3_A5 + 0x1D58u) & 0x02u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            Obj_ExplodeTick(m, obj);
            return;
        }
    }
    wr32(m, obj + 0x00u, 0x000CEF44u);
}

/* ROM 0x0E077A */
int Obj_E077A_Tick(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, F3_A5 + 0x1D58u) & 0x02u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            /* 0x0E08E6 */
            Obj_RelayTimer(m, 0x0000012Cu);
            wr8(m, obj + 0x06u, 0u);
            sub_0CEF0E(m, obj);
            return 0;
        }
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        sub_0D0E04(m, obj, 0x0000012Cu);
        goto death_spawn;
    }
    if (rd8(m, F3_A5 - 0x144Bu) & 0x20u)
        goto death_spawn;

    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - rd32(m, F3_A5 + 0x4228u));
    {
        uint8_t f7 = rd8(m, obj + 0x07u);
        wr8(m, obj + 0x07u, (uint8_t)(f7 & (uint8_t)~0x02u));
        if (f7 & 0x02u) {
            if (ObjScript_CF1E4(m, obj) != 0)
                return -1;
        }
    }
    if (rd8(m, obj + 0x20u) != 0u) {
        /* lea (0x12a,PC),A1 at 0xE07D0 -> 0xE08FC */
        if (ObjScript_CF4B8(m, obj, 0x000E08FCu) != 0)
            return -1;
    }

    sub_0CF7FE(m, obj);
    Obj_FlagOffscreen(m, obj);
    if (rd8(m, obj + 0x07u) & 0x08u)
        goto die;

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
    if (rd8(m, obj + 0x07u) & 0x80u)
        return 0;
    if (rd16(m, obj + 0x60u) < 0x000Au) {
        uint16_t d0 = (uint16_t)(0x000Au - rd16(m, obj + 0x60u));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d0));
    }

    sub_0D0690(m, obj);
    {
        uint16_t d1 = rd16(m, obj + 0x68u);
        if ((int16_t)d1 >= 0) {
            uint32_t a1 = Obj_Spawn(m, obj, 0x000D9092u, 9u, 2u, 0u);
            if ((int32_t)a1 >= 0) {
                wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
                wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x0008u));
                wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
                wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x0020u));
                wr16(m, a1 + 0x58u, d1);
                wr16(m, a1 + 0x10u, 0x0012u);
                if (rd16(m, obj + 0x58u) != 0u)
                    wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x0010u));
            }
        }
    }
    sub_0D0DC2(m, obj);
    Obj_BlinkColour(m, obj);
    return 0;

death_spawn:
    {
        uint32_t a1 = Obj_Spawn(m, obj, 0x000DC328u, 3u, 2u, 0u);
        if ((int32_t)a1 >= 0) {
            wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
            wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x0010u));
            wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x0024u));
            wr16(m, a1 + 0x58u, 0x00B4u);
            wr16(m, a1 + 0x60u, rd16(m, obj + 0x60u));
        }
    }
die:
    wr8(m, obj + 0x06u, 0u);
    Obj_StartDying(m, obj);
    return 0;
}
