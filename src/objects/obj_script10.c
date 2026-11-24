#include "obj_script10.h"
#include "obj_shared.h"

/* The 10-byte movement script at obj+0x44, as walked by ROM 0x178982.
 * Bodies lifted from src/enemy/obj_17C05E.c (same ROM addresses), where this
 * interpreter is already exercised by a passing capture. */

/* ROM 0x178B20 */
void Obj_Script10_SetHeading(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x42u, rd16(m, rd32(m, obj + 0x44u) + 6u));
}

/* ROM 0x178E4A: aim at the object at +0x4C. The entry's arg0 is the heading
 * step and arg1 the speed step; the heading error's sign picks the direction
 * of the speed change, then the position advances along 0x27E40. */
void Obj_Script10_SteerTo(const f3mem_t *m, uint32_t obj)
{
    uint32_t tgt = rd32(m, obj + 0x4Cu);
    if ((int32_t)tgt <= 0)
        return;

    uint16_t dx = (uint16_t)(rd16(m, tgt + 0x14u) + 0x18u - rd16(m, obj + 0x50u));
    uint16_t dy = (uint16_t)(rd16(m, tgt + 0x18u) + 0x10u - rd16(m, obj + 0x54u));
    uint16_t d0 = Obj_Atan2(m, dx, dy);

    uint32_t a1 = rd32(m, obj + 0x44u);
    uint16_t step = rd16(m, a1 + 6u);
    uint16_t spd = rd16(m, a1 + 8u);

    int8_t diff = (int8_t)(uint8_t)((uint8_t)d0 - rd8(m, obj + 0x20u));
    if (diff == 0)
        spd = 0u;
    else if (diff < 0)
        spd = (uint16_t)(0u - spd);

    wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) + step));
    wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + spd));

    int16_t mag = (int16_t)(uint16_t)(rd16(m, obj + 0x42u) + 0x20u);
    mag = (int16_t)(mag >> 6);
    uint16_t ang = (uint16_t)(rd16(m, obj + 0x20u) >> 8);
    int32_t vx = (int32_t)mag * (int16_t)rd16(m, 0x00027E40u + 0x80u + ang * 2u);
    int32_t vy = (int32_t)mag * (int16_t)rd16(m, 0x00027E40u + ang * 2u);
    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + (uint32_t)vx);
    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + (uint32_t)vy);
}

/* Dispatch one command by its ROM address. 0 on success, -1 if untranslated. */
static int script10_call(const f3mem_t *m, uint32_t obj, uint32_t fn)
{
    switch (fn) {
    case 0x00178B20u: Obj_Script10_SetHeading(m, obj); return 0;
    case 0x00178E4Au: Obj_Script10_SteerTo(m, obj);    return 0;
    default:          return -1;
    }
}

/* ROM 0x178982 */
int Obj_Script10(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1;

    for (;;) {
        a1 = rd32(m, obj + 0x44u);
        uint32_t fn = rd32(m, a1);

        if (fn == 0u)
            goto loop_marker;
        if (script10_call(m, obj, fn) < 0)
            return -1;

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
    }

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
