#include "obj_125D44.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"
#include "obj_11B09A.h"

/* Obj_11B09A_Tick (ROM 0x11B09A) is declared in obj_11B09A.h. */

/* ROM 0x125D44: parent-driven tilt sprite. While the parent's byte at +0x59 is
 * clear it re-derives its velocity from the signed angle byte at +0x22 (scaled
 * by the {sin, cos} pair at (0x10,A0)), integrates through 0x11B09A, and
 * re-arms 0x125D44. If A5+0x30EE bit 0xF is set, the angle byte is stepped by
 * 0x400 (one 8-step per frame) as long as it is below 0x400; once it reaches
 * 0x400 the object switches to 0x125DAA. A set parent byte at +0x59 frees the
 * object (0x125DB4 -> 0x105EC). */
int Obj_125D44_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);

    /* 0x125D48: tst.b (0x59,A1); bne 0x125DB4 */
    if (rd8(m, a1 + 0x59u) != 0u) {
        Obj_Free(m, obj);                              /* ROM 0x125DB4 -> 0x0105EC */
        return 0;
    }

    /* 0x125D50: bftst (0x30EE,A5){0xF:1} -- literal bit 15, MSB of 0x30EE */
    if (rd8(m, F3_A5 + 0x30EEu) & 0x01u) {
        /* 0x125D5A: cmpi.w #-0x400,(0x22,A0); bcc 0x125DA2 */
        if ((uint16_t)rd16(m, obj + 0x22u) >= 0xFC00u) {
            wr32(m, obj + 0x00u, 0x00125DAAu);         /* 0x125DA2 */
            return 0;
        }
        /* 0x125D64: addi.w #0x400,(0x22,A0) */
        wr16(m, obj + 0x22u, (uint16_t)(rd16(m, obj + 0x22u) + 0x0400u));
    }

    /* 0x125D6A */
    wr16(m, obj + 0x50u, 0xFF90u);                     /* move.w #-0x70 */
    wr16(m, obj + 0x52u, 0xFFC8u);                     /* move.w #-0x38 */

    /* 0x125D76: moveq #0,D0; move.b (0x22,A0),D0b; bytes into +0x1F/+0x1E */
    uint16_t d0 = (uint16_t)(int16_t)(int8_t)rd8(m, obj + 0x22u);
    wr8(m, obj + 0x1Fu, (uint8_t)d0);
    wr8(m, obj + 0x1Eu, (uint8_t)d0);
    uint16_t d1 = d0;                                  /* move.w D0w,D1w */

    /* 0x125D86: muls.w (A1)+,D1 ; asr.w #5 ; add.w D1w,(0x52,A0) */
    a1 = rd32(m, obj + 0x10u);
    int16_t m0 = (int16_t)rd16(m, a1);                 /* (A1)+ */
    int16_t m1 = (int16_t)rd16(m, a1 + 2u);            /* (A1)+ */
    d1 = (uint16_t)(((int32_t)(int16_t)d1 * (int32_t)m0) >> 5);
    wr16(m, obj + 0x52u, (uint16_t)(rd16(m, obj + 0x52u) + d1));
    d0 = (uint16_t)(((int32_t)(int16_t)d0 * (int32_t)m1) >> 5);
    wr16(m, obj + 0x50u, (uint16_t)(rd16(m, obj + 0x50u) + d0));

    /* 0x125D9A: jsr 0x11B09A (integrate + draw + un-integrate) */
    (void)Obj_11B09A_Tick(m, obj);

    wr32(m, obj + 0x00u, 0x00125D44u);                 /* 0x125D3C */
    return 0;
}
