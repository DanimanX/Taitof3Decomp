/* ROM 0x0E4EBC: boss-part initialiser (A0 = object). Seeds the movement
 * record from ROM 0x0E513A (0x0CEC4C), reparents +0xC to the source object in
 * +0x8, sets the flags +0x7 bit 7 / +0x34 bit 1, the shape word +0x42 = 0x6D,
 * loads frame 0x6D of the 0x0E5182 table (0x0CFA7C -> 0x0CFA86), copies the
 * source position (+0x14/+0x18) and arms 0x0E4F0A. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"

int Fn_0E4EBC(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t a1;

    Obj_InitMover(m, obj, 0x000E513Au);              /* 0x0E4EBC lea (0x27C,PC) */
                                                     /* 0x0E4EC0 jsr 0x0CEC4C */
    wr32(m, obj + 0x0Cu, rd32(m, obj + 0x08u));      /* 0x0E4EC6 */
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x80u)); /* 0x0E4ECC bset #7 */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & 0xFDu)); /* 0x0E4ED2 bclr #1 */

    d0 = (d0 & 0xFFFF0000u) | 0x006Du;               /* 0x0E4ED8 move.w #0x6D,D0 */
    wr16(m, obj + 0x42u, (uint16_t)d0);              /* 0x0E4EDC */
    wr32(m, obj + 0x08u, 0x006D0000u);               /* 0x0E4EE0 */

    /* 0x0E4EE8 lea (0x298,PC) = 0x0E5182 frame table; 0x0E4EEC jsr 0x0CFA7C
     * (word offset table 0x0CFA96 indexed by D0.w, `adda.w d0,a1`). */
    a1 = 0x000E5182u
       + (uint32_t)(int32_t)(int16_t)rd16(m, 0x000CFA96u + 2u * 0x006Du);
    d0 = (d0 & 0xFFFF0000u) | rd16(m, a1);           /* 0x0CFA86 move.w (a1)+,d0 */
    Obj_SetFrame(m, obj, a1);                        /* 0x0CFA86 */

    a1 = rd32(m, obj + 0x0Cu);                       /* 0x0E4EF2 movea.l (0xC,A0),A1 */
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));       /* 0x0E4EF6 */
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));       /* 0x0E4EFC */
    wr32(m, obj + 0x00u, 0x000E4F0Au);               /* 0x0E4F02 */

    r->d[0] = d0;
    r->a[1] = a1;
    return 0;                                        /* 0x0E4F08 rts */
}
