#include "obj_17C660.h"
#include "../objects/obj_script10.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_pool.h"
#include "../audio/sound_cmd.h"
#include "../video/sprite.h"

/* ROM 0x17C660: the tail of the 0x17C614 body that 0x17C56C / 0x17C700
 * install. 0x17C660 itself is a single `bra 0x17C614`; the entry here is the
 * full body so a capture taken with PC = 0x17C660 replays every step:
 *
 *   bit 1 of +0x34 clear -> 0x17C59A: score credit, +0x1C/+0x1E cleared, the
 *     frame list re-picked from +0x2E - +0x2C (<= 8: 0x17C79C, else 0x17C7DC)
 *   bit 1 set             -> 0x17C662: the dithering fade, retired at 0xF0
 *
 * Both feed 0x17C614 and are re-installed there, so the tick stays on
 * 0x17C660. */

/* ROM 0x17B02E: score receiver of the object, or -1 when +0x35 selects
 * neither mode 1 (receiver = (+0x36)+0x08) nor mode 2 (receiver = +0x36). */
static int32_t sub_17B02E(const f3mem_t *m, uint32_t obj)
{
    uint8_t mode = rd8(m, obj + 0x35u);
    if (mode == 0x01u)
        return (int32_t)rd32(m, rd32(m, obj + 0x36u) + 0x08u);
    if (mode == 0x02u)
        return (int32_t)rd32(m, obj + 0x36u);
    return -1;
}

/* ROM 0x17C614: script step, animation, the frame-centre correction, the
 * offscreen test and then the draw call. */
static int sub_17C614(const f3mem_t *m, uint32_t obj, uint16_t idx)
{
    if (Obj_Script10(m, obj) < 0)
        return -1;

    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));

    if (Obj_AnimTick(m, obj, idx) < 0)
        return -1;

    {
        uint32_t a1 = rd32(m, obj + 0x10u);
        uint16_t d1 = (uint16_t)(rd16(m, a1) << 3);
        uint16_t d0 = (uint16_t)(rd16(m, a1 + 2u) << 3);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d1));
    }

    /* 0x17C64A: a negative animation timer means the object goes away */
    if ((int16_t)rd16(m, obj + 0x3Eu) < 0) {
        Obj_Free(m, obj);
        return 0;
    }
    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x00u, 0x0017C660u);
    return 0;
}


/* ROM 0x17C5A4: credit the score at the 0x17B02E receiver with the +0x62
 * word, then pick the frame list and run its INIT half. */
static int sub_17C5A4(const f3mem_t *m, uint32_t obj)
{
    uint32_t d1 = rd16(m, obj + 0x62u);
    int32_t rec = sub_17B02E(m, obj);
    if (rec >= 0)
        wr32(m, (uint32_t)rec + 0x48u, rd32(m, (uint32_t)rec + 0x48u) + d1);

    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x1Cu, 0u);

    /* 0x17C5C4: `cmpi.w #8,D0; bgt` -- D0 = +0x2E - +0x2C */
    uint16_t d0 = (uint16_t)(rd16(m, obj + 0x2Eu) - rd16(m, obj + 0x2Cu));
    uint32_t list = ((int16_t)d0 > 8) ? 0x0017C7DCu : 0x0017C79Cu;
    uint16_t idx = rd16(m, list);
    wr16(m, obj + 0x40u, idx);
    wr32(m, obj + 0x3Au, list + 2u);
    if (Obj_AnimInit(m, obj, idx) < 0)
        return -1;
    return sub_17C614(m, obj, idx);
}

/* ROM 0x17C662: the dithering fade. +0x1F and +0x1E step by 0x10 until either
 * reaches 0xF0, which retires the object at 0x17C78E. The X step reads the
 * sprite record's SECOND size word, the Y step the first (0x17C69A vs
 * 0x17C6BA). */
static int sub_17C662(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x1Fu) >= 0xF0u || rd8(m, obj + 0x1Eu) >= 0xF0u) {
        Obj_Free(m, obj);                           /* 0x17C78E */
        return 0;
    }

    if (Obj_Script10(m, obj) < 0)
        return -1;

    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));

    wr8(m, obj + 0x1Fu, (uint8_t)(rd8(m, obj + 0x1Fu) + 0x10u));
    {
        uint32_t a1 = rd32(m, obj + 0x10u);
        uint16_t d1 = (uint16_t)(rd16(m, a1 + 2u) << 4);
        uint16_t d0 = (uint16_t)(((uint16_t)rd8(m, obj + 0x1Fu) * d1) >> 8);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)(d0 >> 1)));
    }
    wr8(m, obj + 0x1Eu, (uint8_t)(rd8(m, obj + 0x1Eu) + 0x10u));
    {
        uint32_t a1 = rd32(m, obj + 0x10u);
        uint16_t d1 = (uint16_t)(rd16(m, a1) << 4);
        uint16_t d0 = (uint16_t)(((uint16_t)rd8(m, obj + 0x1Eu) * d1) >> 8);
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)(d0 >> 1)));
    }

    if (Obj_AnimTick(m, obj, rd16(m, obj + 0x40u)) < 0)
        return -1;

    {
        uint32_t a1 = rd32(m, obj + 0x10u);
        uint16_t d1 = (uint16_t)(rd16(m, a1) << 3);
        uint16_t d0 = (uint16_t)(rd16(m, a1 + 2u) << 3);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d1));
    }

    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x00u, 0x0017C700u);              /* 0x17C6F8 -> 0x17C700 */
    return 0;
}

int Obj_17C660_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x17C59A: bit 1 of +0x34 selects the fade; otherwise 0x17C5A4 runs */
    if ((rd8(m, obj + 0x34u) & 0x02u) == 0u)
        return sub_17C5A4(m, obj);
    return sub_17C662(m, obj);
}
