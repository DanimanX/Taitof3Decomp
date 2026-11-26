/* ROM 0x110756: boss/large-enemy main body tick (A0 = object).
 * Applies scroll deltas (A5+0x4224/4228) to +0x14/+0x18 and companion scroll
 * fields (A5-0x5D98/-0x5D94, A5+0x4214/+0x4218). While state (+0x06) != 3 and
 * HP (+0x24) <= 0, transitions 1 -> 2 (resetting +0x24 = 0x80000) or 2 -> 3
 * (queuing fade ramp 0x262C0 -> 0x110B9C, adding 0x1770 score timer, setting
 * +0x21 = 0xF0, and spawning 5 explosion objects 0xDC456/0xDC36C). Otherwise
 * dies on bomb flag (A5-0x144B bit 5) or X <= -0x118 (clearing three tilemap
 * regions with 0x66D20 via 0x0053B2 and dying via 0x0CEE74), or dispatches by
 * +0x06 through jump table 0x1109B6 (states 1/2: hitbox toggle, collision
 * register, hit-flash palette uploads, cyclic palette ramp; state 3: sinking
 * death sequence spawning random 0xDC456 explosions). */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../video/palette.h"
#include "../video/sprite.h"

static uint32_t setl_w(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0D0E04 with A1 register tracking. */
static void add_score_timer(const f3mem_t *m, f3regs_t *r, uint32_t obj, uint32_t d0)
{
    uint8_t k = rd8(m, obj + 0x35u);
    r->d[0] = d0;
    if (k == 1u)
        r->a[1] = rd32(m, rd32(m, obj + 0x36u) + 0x08u);
    else if (k == 2u)
        r->a[1] = rd32(m, obj + 0x36u);
    Obj_0D0E04(m, obj, d0);
}

/* ROM 0x1109C6: active state (1 or 2). */
static void state_active(const f3mem_t *m, f3regs_t *r, uint32_t obj)
{
    wr8(m, obj + 0x43u, (uint8_t)(rd8(m, obj + 0x43u) ^ 0x01u)); /* 0x1109C6 */

    uint16_t idx = rd16(m, obj + 0x42u);                          /* 0x1109CC */
    r->d[0] = setl_w(r->d[0], idx);
    uint32_t tbl = 0x00110B78u + (uint32_t)(int32_t)(int16_t)idx * 8u; /* 0x1109D0 */
    wr16(m, obj + 0x2Cu, rd16(m, tbl));                           /* 0x1109D8 */
    wr16(m, obj + 0x2Eu, rd16(m, tbl + 2u));                      /* 0x1109DC */
    wr16(m, obj + 0x30u, rd16(m, tbl + 4u));                      /* 0x1109E0 */
    wr16(m, obj + 0x32u, rd16(m, tbl + 6u));                      /* 0x1109E4 */

    Obj_0D0DC2(m, obj);                                           /* 0x1109E8 */
    r->a[1] = rd32(m, F3_A5 + 0x0906u);

    if (rd8(m, obj + 0x35u) != 0u) {                              /* 0x1109EE */
        wr8(m, obj + 0x35u, 0u);                                  /* 0x1109F6 */
        Pal_QueueUpload(m, 0x00016B00u, 0x1F80u, 0x001Au);        /* 0x110A0C */
        wr8(m, obj + 0x20u, 1u);                                  /* 0x110A16 */
    } else {
        int8_t flash = (int8_t)rd8(m, obj + 0x20u);               /* 0x110A20 */
        if (flash > 0) {                                          /* 0x110A24..0x110A28 */
            Pal_QueueUpload(m, 0x00016B80u, 0x1F80u, 0x001Au);    /* 0x110A3E */
            wr8(m, obj + 0x20u, 0xFFu);                           /* 0x110A48 */
        } else if (flash < 0) {
            Pal_QueueUpload(m, 0x000262C0u, 0x1F80u, 0x001Au);    /* 0x110A64 */
            wr8(m, obj + 0x20u, 0u);                              /* 0x110A6E */
        }
    }

    uint8_t cnt = (uint8_t)(rd8(m, obj + 0x23u) - 1u);            /* 0x110A72 */
    wr8(m, obj + 0x23u, cnt);
    if (cnt == 0u) {                                              /* 0x110A76 */
        uint8_t tog = (uint8_t)(~rd8(m, obj + 0x22u));            /* 0x110A7A */
        wr8(m, obj + 0x22u, tog);
        uint32_t slot;
        if (tog == 0u) {                                          /* 0x110A7E */
            wr8(m, obj + 0x23u, 0x23u);                           /* 0x110A82 */
            slot = Pal_QueueRamp(m, 0x00110B88u, 0x00026328u, 0x1FE8u, 0x19u, 5u); /* 0x110AA6 */
        } else {
            wr8(m, obj + 0x23u, 0x1Eu);                           /* 0x110AB4 */
            slot = Pal_QueueRamp(m, 0x00026328u, 0x00110B88u, 0x1FE8u, 0x1Cu, 5u); /* 0x110AD8 */
        }
        r->d[0] = (slot == 0u) ? 0xFFFFFFFFu : slot;
    }
}

/* ROM 0x110AE4: dying/sinking state (3). */
static void state_sinking(const f3mem_t *m, f3regs_t *r, uint32_t obj)
{
    if ((int16_t)rd16(m, obj + 0x18u) >= 0x0100)                  /* 0x110AE4 */
        return;

    const uint32_t dy = 0x000061A8u;                              /* 0x110AEE */
    r->d[0] = dy;
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + dy);              /* 0x110AF4 */
    wr32(m, F3_A5 - 0x5D94u, rd32(m, F3_A5 - 0x5D94u) - dy);      /* 0x110AF8 */
    wr32(m, F3_A5 + 0x4218u, rd32(m, F3_A5 + 0x4218u) - dy);      /* 0x110AFC */

    if (rd8(m, obj + 0x21u) == 0u)                                /* 0x110B00 */
        return;

    wr8(m, obj + 0x21u, (uint8_t)(rd8(m, obj + 0x21u) - 1u));     /* 0x110B08 */
    uint16_t rng = Obj_Next4Way(m);                               /* 0x110B0C */
    r->d[0] = setl_w(r->d[0], rng);
    if (rng > 0x1Eu)                                              /* 0x110B12..0x110B16 */
        return;

    uint32_t child = Obj_Spawn(m, obj, 0x000DC456u, 3u, 2u, 0u);  /* 0x110B2C */
    r->d[0] = child;
    if ((int32_t)child < 0)                                       /* 0x110B36..0x110B38 */
        return;

    r->a[1] = child;                                              /* 0x110B3C */
    wr32(m, child + 0x14u, rd32(m, obj + 0x14u));                 /* 0x110B3E */
    wr32(m, child + 0x18u, rd32(m, obj + 0x18u));                 /* 0x110B44 */
    wr16(m, child + 0x58u, 0u);                                   /* 0x110B4A */
    wr16(m, child + 0x60u, 0u);                                   /* 0x110B4E */

    uint16_t rx = (uint16_t)((Obj_Next4Way(m) & 0x7Fu) + 0x6Au);  /* 0x110B52..0x110B5C */
    wr16(m, child + 0x14u, (uint16_t)(rd16(m, child + 0x14u) + rx)); /* 0x110B60 */
    uint16_t ry = (uint16_t)((Obj_Next4Way(m) & 0x3Fu) + 0x6Cu);  /* 0x110B64..0x110B6E */
    r->d[0] = setl_w(r->d[0], ry);
    wr16(m, child + 0x18u, (uint16_t)(rd16(m, child + 0x18u) + ry)); /* 0x110B72 */
}

/* ROM 0x1107C6: transition state 2 -> 3 when HP runs out. */
static void enter_sinking(const f3mem_t *m, f3regs_t *r, uint32_t obj)
{
    static const struct { uint32_t fn; int16_t dx, dy; } booms[5] = {
        { 0x000DC456u, -0x09, 0x2D },
        { 0x000DC36Cu,  0x14, 0x33 },
        { 0x000DC36Cu,  0x32, 0x2A },
        { 0x000DC36Cu,  0x4E, 0x1A },
        { 0x000DC36Cu,  0x6D, 0x13 },
    };

    (void)Pal_QueueRamp(m, 0x000262C0u, 0x00110B9Cu, 0x1F80u, 0x3Cu, 0x20u); /* 0x1107E4 */
    add_score_timer(m, r, obj, 0x00001770u);                      /* 0x1107F4 */
    wr8(m, obj + 0x21u, 0xF0u);                                   /* 0x1107FA */
    wr8(m, obj + 0x06u, 3u);                                      /* 0x110800 */

    uint32_t d7 = rd32(m, obj + 0x14u) + 0x00780000u;             /* 0x110806..0x11080A */
    uint32_t d6 = rd32(m, obj + 0x18u) + 0x00880000u;             /* 0x110810..0x110814 */

    for (int i = 0; i < 5; i++) {
        uint32_t child = Obj_Spawn(m, obj, booms[i].fn, 3u, 2u, 0u);
        r->d[0] = child;
        if ((int32_t)child < 0)
            return;
        r->a[1] = child;
        wr32(m, child + 0x14u, d7);
        wr16(m, child + 0x14u, (uint16_t)((int16_t)rd16(m, child + 0x14u) + booms[i].dx));
        wr32(m, child + 0x18u, d6);
        wr16(m, child + 0x18u, (uint16_t)((int16_t)rd16(m, child + 0x18u) + booms[i].dy));
        wr16(m, child + 0x58u, 0u);
        wr16(m, child + 0x60u, 0u);
    }
}

int Fn_110756(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    uint32_t dx = rd32(m, F3_A5 + 0x4224u);                       /* 0x110756 */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - dx);              /* 0x11075A */
    wr32(m, F3_A5 - 0x5D98u, rd32(m, F3_A5 - 0x5D98u) + dx);      /* 0x11075E */
    wr32(m, F3_A5 + 0x4214u, rd32(m, F3_A5 + 0x4214u) + dx);      /* 0x110762 */

    uint32_t dy = rd32(m, F3_A5 + 0x4228u);                       /* 0x110766 */
    r->d[0] = dy;
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - dy);              /* 0x11076A */
    wr32(m, F3_A5 - 0x5D94u, rd32(m, F3_A5 - 0x5D94u) + dy);      /* 0x11076E */
    wr32(m, F3_A5 + 0x4218u, rd32(m, F3_A5 + 0x4218u) + dy);      /* 0x110772 */

    if (rd8(m, obj + 0x06u) != 3u && (int32_t)rd32(m, obj + 0x24u) <= 0) { /* 0x110776..0x110784 */
        if (rd8(m, obj + 0x06u) != 2u) {                          /* 0x1107AC */
            wr32(m, obj + 0x24u, 0x00080000u);                    /* 0x1107B6 */
            wr8(m, obj + 0x06u, 2u);                              /* 0x1107BE */
            return 0;                                             /* 0x1107C4 */
        }
        enter_sinking(m, r, obj);
        return 0;
    }

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u
        || (int16_t)rd16(m, obj + 0x14u) <= -0x118) {             /* 0x110788..0x110798 */
        Gfx_DrawCompressedTilemap(m, 0x00066D20u, 0x00612400u, 0u); /* 0x11096E */
        Gfx_DrawCompressedTilemap(m, 0x00066D20u, 0x00612020u, 0u); /* 0x110988 */
        Gfx_DrawCompressedTilemap(m, 0x00066D20u, 0x00612420u, 0u); /* 0x1109A2 */
        wr8(m, obj + 0x06u, 0u);                                  /* 0x1109AC */
        Obj_StartDying(m, obj);                                   /* 0x1109B0 */
        return 0;
    }

    uint16_t state = rd8(m, obj + 0x06u);                         /* 0x11079C..0x11079E */
    r->d[0] = setl_w(r->d[0], state);
    switch (state) {                                              /* 0x1107A2 */
    case 1u:
    case 2u:
        state_active(m, r, obj);
        return 0;
    case 3u:
        state_sinking(m, r, obj);
        return 0;
    default:
        return -1;
    }
}
