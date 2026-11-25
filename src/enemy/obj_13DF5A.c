#include "obj_13DF5A.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* divs.w D2w,D0 (32/16): on overflow D0 is left untouched. */
static uint32_t divs_w(uint32_t d0, uint16_t d2w)
{
    int32_t num = (int32_t)d0, den = (int16_t)d2w;
    int32_t q = num / den, r = num % den;
    if (q > 32767 || q < -32768) return d0;
    return ((uint32_t)(uint16_t)r << 16) | (uint16_t)q;
}

/* ROM 0x13E080: perspective-project the 3D position (+0x4C/+0x50/+0x54) to
 * screen +0x14/+0x18 and derive the zoom byte (+0x1E/+0x1F). */
static void sub_13E080(const f3mem_t *m, uint32_t obj)
{
    uint32_t d0 = (uint32_t)((int32_t)rd32(m, obj + 0x4Cu) >> 8);
    uint32_t d1 = (uint32_t)((int32_t)rd32(m, obj + 0x50u) >> 8);
    uint16_t d2w = (uint16_t)((rd16(m, obj + 0x54u) >> 1) + 0x64u);
    d0 = divs_w(d0, d2w);
    d1 = divs_w(d1, d2w);
    wr16(m, obj + 0x14u, (uint16_t)(d0 + 0xA0u));
    wr16(m, obj + 0x18u, (uint16_t)(d1 + 0x74u));
    uint16_t z = (uint16_t)divs_w(0x639Cu, d2w);
    uint8_t zb = (uint8_t)(0u - (uint16_t)(z - 0xFFu));
    wr8(m, obj + 0x1Fu, zb);
    wr8(m, obj + 0x1Eu, zb);
}

/* ROM 0x13E0C4: offset +0x14/+0x18 by the sprite def's first two words,
 * scaled by (zoom - 0x100). */
static void sub_13E0C4(const f3mem_t *m, uint32_t obj)
{
    int16_t d1 = (int16_t)(rd8(m, obj + 0x1Eu) - 0x100);
    uint32_t a1 = rd32(m, obj + 0x10u);
    int32_t p = (int32_t)(int16_t)(uint16_t)(rd16(m, a1) << 3) * d1;
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)(p >> 8)));
    p = (int32_t)(int16_t)(uint16_t)(rd16(m, a1 + 2u) << 3) * d1;
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)(p >> 8)));
}

/* ROM 0x13DF5A: flying-in sprite. Integrates the velocity words (+0x58/5A/5C
 * << 6) into +0x4C/+0x50/+0x54, projects, culls and draws; frees itself when
 * hidden, too close, or off the 0xCC112 screen bounds. */
int Obj_13DF5A_Tick(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, F3_A5 - 0x144Bu) & 0x04u) goto done;

    wr32(m, obj + 0x4Cu, rd32(m, obj + 0x4Cu) + (uint32_t)((int32_t)(int16_t)rd16(m, obj + 0x58u) << 6));
    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + (uint32_t)((int32_t)(int16_t)rd16(m, obj + 0x5Au) << 6));
    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + (uint32_t)((int32_t)(int16_t)rd16(m, obj + 0x5Cu) << 6));
    if ((int16_t)rd16(m, obj + 0x54u) <= 0) goto done;

    sub_13E080(m, obj);
    if (rd8(m, obj + 0x1Eu) >= 0xE8u) goto done;
    {
        int16_t x = (int16_t)rd16(m, obj + 0x14u), y = (int16_t)rd16(m, obj + 0x18u);
        if (x < (int16_t)rd16(m, 0x000CC112u) || x > (int16_t)rd16(m, 0x000CC114u)) goto done;
        if (y < (int16_t)rd16(m, 0x000CC116u) || y > (int16_t)rd16(m, 0x000CC118u)) goto done;
    }

    uint32_t tbl = 0x0013E032u;                                  /* ROM 0x13DFE6 */
    if (rd8(m, obj + 0x22u) == 0u) {
        uint16_t d0 = rd8(m, obj + 0x1Eu);
        if (d0 < 0x80u) {
            tbl = 0x0013E018u;                                   /* ROM 0x13DFF0 */
        } else {
            d0 = (uint16_t)((uint16_t)(d0 - 0x100u) * 2u + 0x100u);
            wr8(m, obj + 0x1Eu, (uint8_t)d0);
            wr8(m, obj + 0x1Fu, (uint8_t)d0);
        }
    }
    Obj_Animate(m, obj, tbl);                                    /* ROM 0x13DFF6 */
    sub_13E0C4(m, obj);
    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x00u, 0x0013DF5Au);                           /* ROM 0x13DF52 */
    return 0;

done:
    Obj_Free(m, obj);                                            /* ROM 0x13E00A */
    return 0;
}
