/* ROM 0x130E3A: (injected capture fnj_, NOIRQ): object tick that steps the
 * {dur.w, sprdef.l} animation table 0x130ED4, appends the object to the list
 * at A5+0x906, draws the sprite (0x001282), counts +0x06 down and re-arms:
 *
 *   130E3A  A1 = 0x130ED4
 *   130E40  jsr 0x0CBD52            step the animation (Obj_Animate)
 *   130E46  jsr 0x0CBDAE            append A0 to *(A5+0x906), advance it
 *   130E4C  jsr 0x001282            draw the sprite
 *   130E52  subq.w #1,(A0+6)
 *   130E56  bgt 0x130E32 -> fn = 0x130E3A (keep ticking)
 *   130E58  bra 0x130E9E -> fn = 0x130EA6 (drop to the free state)
 *
 * The only capture pair spans 0x39 frames, so tests/test_fn.c skips it as
 * VBlank-interrupted; the routine is translated from the bytes. */
#include "fn_auto.h"
#include "../video/sprite.h"

/* ROM 0x001282 wrapper preserving D0/D1/A1 return values
 * (same helper as src/auto/fn_0C12AA.c). */
static void draw_sprite_regs(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a2 = rd32(m, obj + 0x10u);
    uint32_t spr_hdr = rd32(m, a2);

    Obj_DrawSprite(m, obj);
    r->a[1] = rd32(m, F3_A5 - 0x0CA0u);

    if (spr_hdr == 0x00010001u) {
        uint32_t d0 = (((uint32_t)rd16(m, obj + 0x14u) & 0x0FFFu) << 16)
                    | (uint32_t)rd16(m, a2 + 6u);
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u);
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 ^ d1w);
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        r->d[0] = d0;
        r->d[1] = (r->d[1] & 0xFFFF0000u) | d1w;
    } else {
        uint16_t cols = (uint16_t)(spr_hdr >> 16);
        r->d[0] = 0x0000FFFFu;
        r->d[1] = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

/* ROM 0x0CBD52: step the {dur.w, sprdef.l} table at `tbl` (index is the byte
 * offset in +0x3A, frame timer in +0x3E). Leaves D0w = offset, D1w = timer. */
static void anim_step(const f3mem_t *m, f3regs_t *r, uint32_t obj, uint32_t tbl)
{
    for (;;) {
        uint16_t off = rd16(m, obj + 0x3Au);
        uint32_t entry = tbl + (uint32_t)(int32_t)(int16_t)off;
        int16_t dur = (int16_t)rd16(m, entry);

        r->d[0] = (r->d[0] & 0xFFFF0000u) | off;
        if (dur < 0) {                                   /* 0x0CBD56 tst.w ; bge */
            wr16(m, obj + 0x3Au, 0u);                    /* 0x0CBD5E */
            continue;
        }
        wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) + 1u)); /* 0x0CBD64 */
        uint16_t timer = rd16(m, obj + 0x3Eu);           /* 0x0CBD68 */
        r->d[1] = (r->d[1] & 0xFFFF0000u) | timer;
        if ((int16_t)timer <= dur) {                     /* 0x0CBD70 ble */
            wr32(m, obj + 0x10u, rd32(m, entry + 2u));   /* 0x0CBD7E */
            return;
        }
        wr16(m, obj + 0x3Eu, 0u);                        /* 0x0CBD74 */
        wr16(m, obj + 0x3Au, (uint16_t)(off + 6u));      /* 0x0CBD78 */
    }
}

int Fn_130E3A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a5 = r->a[5];
    uint16_t count;

    anim_step(m, r, obj, 0x00130ED4u);                   /* 0x130E3A/0x130E40 -> 0x0CBD52 */

    uint32_t p = rd32(m, a5 + 0x0906u);                  /* 0x0CBDAE jsr */
    wr32(m, p, obj);
    wr32(m, a5 + 0x0906u, p + 4u);
    r->a[1] = p + 4u;

    draw_sprite_regs(m, r);                              /* 0x130E4C jsr 0x001282 */

    count = (uint16_t)(rd16(m, obj + 6u) - 1u);          /* 0x130E52 subq.w #1 */
    wr16(m, obj + 6u, count);
    if ((int16_t)count > 0)
        wr32(m, obj, 0x00130E3Au);                       /* 0x130E32 keep ticking */
    else
        wr32(m, obj, 0x00130EA6u);                       /* 0x130E9E free state */
    return 0;
}
