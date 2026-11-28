#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x11A994: moving sprite bullet for the 0x117C28 family (A0 = object).
 *
 * +0x22 indexes a {countdown.w, spawnarg.w} table at ROM 0x11AA3C; when the
 * +0x06 tick matches the countdown the object spawns 0x117C28 (group 7,
 * count 2, arg from the table) and advances +0x22 by 4. Then +0x06 is bumped,
 * the 16.16 position at +0x14 gains 0x48000 (4.5 pixels), the animation table
 * 0x11AA22 steps (0x0CBD52), and the sprite is drawn twice through 0x001282 --
 * once 5 pixels left / 7 down, then at the real position with sprite
 * 0x0002A0EC. A +0x14 x below 0x140 re-arms 0x11A994 (0x11A98C thunk),
 * otherwise the object is freed (0x0105EC). */

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x001282 with the D0/D1/A1 register outputs the capture sees. */
static void draw_sprite_regs(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a2 = rd32(m, obj + 0x10u);
    uint32_t spr_hdr = rd32(m, a2);

    Obj_DrawSprite(m, obj);
    r->a[1] = rd32(m, r->a[5] - 0x0CA0u);

    if (spr_hdr == 0x00010001u) {                    /* 0x0013CE */
        uint32_t d0 = (((uint32_t)rd16(m, obj + 0x14u) & 0x0FFFu) << 16)
                    | (uint32_t)rd16(m, a2 + 6u);
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u);
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 ^ d1w);
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        r->d[0] = d0;
        r->d[1] = setw(r->d[1], d1w);
    } else {                                         /* 0x001294 */
        uint16_t cols = (uint16_t)(spr_hdr >> 16);
        r->d[0] = 0x0000FFFFu;
        r->d[1] = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

int Fn_11A994(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    uint16_t off = rd16(m, obj + 0x22u);                          /* 0x11A994 */
    uint32_t a1 = 0x0011AA3Cu + off;
    uint16_t tick = rd16(m, obj + 0x06u);
    uint16_t want = rd16(m, a1);
    a1 += 2u;
    if (tick == want) {                                           /* 0x11A9A4 */
        uint16_t arg = rd16(m, a1);
        r->d[0] = arg;
        (void)Obj_Spawn(m, obj, 0x00117C28u, 0x0007u, 2u, arg);   /* 0x11A9AE */
        wr16(m, obj + 0x22u, (uint16_t)(off + 4u));               /* 0x11A9C8 */
    }

    wr16(m, obj + 0x06u, (uint16_t)(tick + 1u));                  /* 0x11A9CC */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + 0x00048000u);     /* 0x11A9D0 */
    Obj_Animate(m, obj, 0x0011AA22u);                             /* 0x11A9D8 */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 5u));  /* 0x11A9E2 */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 7u));  /* 0x11A9E8 */
    draw_sprite_regs(m, r);                                       /* 0x11A9EC -> 0x001282 */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 5u));  /* 0x11A9F2 */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 7u));  /* 0x11A9F8 */
    wr32(m, obj + 0x10u, 0x0002A0ECu);                            /* 0x11A9FC */
    draw_sprite_regs(m, r);                                       /* 0x11AA04 -> 0x001282 */

    if ((int16_t)rd16(m, obj + 0x14u) < 0x0140) {                 /* 0x11AA0A */
        wr32(m, obj + 0x00u, 0x0011A994u);                        /* 0x11A98C */
    } else {
        Obj_Free(m, obj);                                         /* 0x11AA14 */
    }
    return 0;
}
