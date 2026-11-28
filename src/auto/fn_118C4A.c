/* ROM 0x118C4A: enemy sprite animation/deploy tick (A0 = object).
 * Steps the 20-entry frame table at 0x118C7C (word duration + long sprite
 * header pointer) through 0x0CBD24, which writes the current sprite pointer
 * into +0x10. While the current entry's duration is still positive the owner
 * is drawn via 0x001282 after +0x18 is offset by -0x8000, and the object
 * re-arms 0x118C4A; once the table runs out (duration 0xFFFF) the object is
 * freed through 0x0105EC. */
#include "fn_auto.h"
#include "batch_obj_regs.h"

/* ROM 0x0CBD24: walk the (duration.w, ptr.l) 6-byte table at 0x118C7C from
 * object offset +0x3A, counting frames in +0x3E; store the current sprite
 * pointer into +0x10. Exits with D0.w = +0x3A and D1.w = frame counter. */
static void sub_0CBD24(const f3mem_t *m, uint32_t obj, uint32_t tbl,
                       uint32_t *d0_io, uint32_t *d1_io)
{
    for (;;) {
        uint16_t off = rd16(m, obj + 0x3Au);                  /* 0x0CBD24 */
        *d0_io = batch_set_word(*d0_io, off);
        int16_t dur = (int16_t)rd16(m, tbl + (uint32_t)(int32_t)(int16_t)off);
        if (dur <= 0)                                         /* 0x0CBD2C */
            return;
        uint16_t cnt = (uint16_t)(rd16(m, obj + 0x3Eu) + 1u); /* 0x0CBD30 */
        wr16(m, obj + 0x3Eu, cnt);
        *d1_io = batch_set_word(*d1_io, cnt);
        if ((int16_t)cnt <= dur) {                            /* 0x0CBD3C */
            wr32(m, obj + 0x10u,                              /* 0x0CBD4A */
                 rd32(m, tbl + (uint32_t)(int32_t)(int16_t)off + 2u));
            return;
        }
        wr16(m, obj + 0x3Eu, 0u);                             /* 0x0CBD40 */
        wr16(m, obj + 0x3Au, (uint16_t)(off + 6u));           /* 0x0CBD44 */
    }
}

/* ROM 0x001282 at this call site: the shared 0x10001 header path (0x0013CE)
 * keeps D0's high word from the caller (0x0013EA only loads the low word),
 * unlike the generic model which fills it from +0x14. */
static void draw_sprite_keep_d0hi(const f3mem_t *m, f3regs_t *r, uint32_t ret)
{
    uint32_t obj = r->a[0];
    uint32_t src = rd32(m, obj + 0x10u);
    uint32_t d0hi = r->d[0] & 0xFFFF0000u;

    batch_draw_sprite(m, r, ret);                             /* 0x001282 */
    if (rd32(m, src) == 0x00010001u)                          /* 0x00128A */
        r->d[0] = d0hi | (r->d[0] & 0x0000FFFFu);             /* 0x0013EA */
}

int Fn_118C4A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    r->a[1] = 0x00118C7Cu;                                    /* 0x118C4A lea */
    sub_0CBD24(m, obj, 0x00118C7Cu, &r->d[0], &r->d[1]);      /* 0x118C50 */

    if ((int16_t)rd16(m, 0x00118C7Cu + (int32_t)(int16_t)(uint16_t)r->d[0]) <= 0)
        {                                                     /* 0x118C56 */
            batch_free_object(m, r, 0x00118C76u);             /* 0x118C70 */
            return 0;
        }

    /* 0x118C5E */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - 0x8000u);
    draw_sprite_keep_d0hi(m, r, 0x00118C6Cu);                 /* 0x118C66 */
    wr32(m, obj, 0x00118C4Au);                                /* 0x118C42 */
    return 0;
}
