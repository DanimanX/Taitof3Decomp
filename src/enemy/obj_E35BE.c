#include "obj_E35BE.h"
#include "../objects/obj_shared_ext.h"
#include "obj_E8B72.h"

/* ROM 0x0D0090: install the animation record (base A1 + 0xD00B4[D0.w]) into
 * +0x48/+0x4C/+0x1C/+0x10/+0x68, the same layout as 0x0D0678. */
static void sub_0D0090(const f3mem_t *m, uint32_t obj, uint32_t a1, uint16_t d0_in)
{
    /* 0x0D0090: move.w (0xD00B4,PC,D0.w*2),D0 ; adda.w D0,A1 (sign-extended) ; movea.l (A1),A1
     * -- extension word at 0x0D0092, word table at 0x0D00B4 */
    a1 += (uint32_t)(int32_t)(int16_t)rd16(m, 0x000D00B4u + (uint32_t)d0_in * 2u);
    a1 = rd32(m, a1);

    /* 0x0D009C: move.l A1,(0x48,A0) */
    wr32(m, obj + 0x48u, a1);

    /* 0x0D00A0: move.w (A1)+,(0x4C,A0) */
    uint16_t d = rd16(m, a1);
    a1 += 2u;
    wr16(m, obj + 0x4Cu, d);

    /* 0x0D00A4: move.w (A1)+,D0 ; move.b D0b,(0x1C,A0) -- low byte */
    uint16_t f = rd16(m, a1);
    a1 += 2u;
    wr8(m, obj + 0x1Cu, (uint8_t)f);

    /* 0x0D00AA: move.l (A1)+,(0x10,A0) */
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;

    /* 0x0D00AE: move.l (A1),(0x68,A0) */
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x0E35BE */
int Obj_E35BE_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0E35BE..0x0E35D2: inherit the mover record and the position from the
     * parent (A1 = +0x08) */
    uint32_t a1 = rd32(m, obj + 0x08u);
    wr32(m, obj + 0x58u, rd32(m, a1 + 0x08u));
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));

    /* 0x0E35D4: lea (-0x6D88,PC),A1 (ext word 0x0E35D6 -> 0x0DC84E) ;
     * jsr 0x0CEDE4 (move.w +0x58 keeps only the low word for the index) */
    Obj_0CEDE4(m, obj, 0x000DC84Eu);

    /* 0x0E35DE: lea (0x15C,PC),A1 (ext word 0x0E35E0 -> 0x0E373C) ;
     * 0x0E35E2: move.w (0x58,A0),D0 ; jsr 0x0D0090 */
    sub_0D0090(m, obj, 0x000E373Cu, rd16(m, obj + 0x58u));

    /* 0x0E35EC: clr.w D0 ; move.b (0x87F,A5),D0b ; lsr.b #4,D0b ;
     * 0x0E35F4: move.l (0x1C6,PC,D0.w*4),D0 (full-format ext at 0x0E35F6 -> 0x0E37BC)
     * (the nop at 0x0E35FA is padding) ; add.l D0,(0x48,A0) */
    uint32_t d0 = (uint32_t)rd8(m, F3_A5 + 0x087Fu) >> 4;
    wr32(m, obj + 0x48u, rd32(m, obj + 0x48u) + rd32(m, 0x000E37BCu + d0 * 4u));

    /* 0x0E3600: movea.l (0x68,A0),A1 ; move.w (A1)+,D0 ; add.w D0,(0x14,A0) ;
     * move.w (A1),D0 ; add.w D0,(0x18,A0) -- low words of the position */
    a1 = rd32(m, obj + 0x68u);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + rd16(m, a1)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + rd16(m, a1 + 2u)));

    /* 0x0E3610: move.b #6,(0x21,A0) ; 0x0E3616: move.b #1,(0x23,A0) */
    wr8(m, obj + 0x21u, 0x06u);
    wr8(m, obj + 0x23u, 0x01u);

    /* 0x0E361C: move.l #0xE3624,(A0) */
    wr32(m, obj + 0x00u, 0x000E3624u);
    return 0;
}
