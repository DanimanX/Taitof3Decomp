#include "obj_1795A4.h"

/* +0x1C in a frame record holds the sprite palette word; the init reads it as
 * a word and stores it as a byte. The body is copied verbatim from
 * obj_17A8E0.c (frame script engine, op table 0x179A5C), which the 0x17C05E
 * family follows too; this record's palette word is the first one only this
 * function consumes. */
static void scr_palette(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));  /* move.w (a1)+, d0 / move.b d0, $1c(a0) */
}

/* ROM 0x1795A4 (0x179A5C animation index 5 init): timed frame with a hit box
 * and the per-frame screen offset at +0x3A -- {duration.w, sprite.l, 2c.l,
 * 30.l, palette.w, dx.b, dy.b}. The two offset bytes are signed. */
int Obj_1795A4_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x3Au);
    wr16(m, obj + 0x3Eu, rd16(m, a1));          /* move.w (a1)+, $3e(a0) */
    wr32(m, obj + 0x10u, rd32(m, a1 + 2u));     /* move.l (a1)+, $10(a0) */
    wr32(m, obj + 0x2Cu, rd32(m, a1 + 6u));     /* move.l (a1)+, $2c(a0) */
    wr32(m, obj + 0x30u, rd32(m, a1 + 0x0Au));  /* move.l (a1)+, $30(a0) */
    scr_palette(m, obj, a1 + 0x0Eu);            /* move.w (a1)+, d0 / move.b d0, $1c(a0) */
    {
        /* bfexts (a1){0:8},d0 / bfexts (a1){8:8},d1: dx at +0x10, dy at +0x11 */
        int16_t dx = (int16_t)(int8_t)rd8(m, a1 + 0x10u);
        int16_t dy = (int16_t)(int8_t)rd8(m, a1 + 0x11u);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)dx));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)dy));
    }
    return 0;
}
