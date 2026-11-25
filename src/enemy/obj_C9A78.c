#include "obj_C9A78.h"
#include "../objects/obj_pool.h"

/* ROM 0x0C9A78: bullet emitter. The first tick arms +0x00 with 0x0C9A9E, a
 * 19-byte record in ROM (x/y offset longs, a word added to the position long,
 * four signed-byte velocities and two word speeds shifted left 8), then falls
 * into 0x0C9BCA on the next tick. Each tick decrements the parent's fire
 * counter (+0x5A) and, while it is negative or the parent is flagged at +0x65,
 * re-reads the record, copies the parent's position and re-arms 0x0C9C6A.
 * With the counter spent it hands +0x5A back to the parent and frees itself. */
int Obj_C9A78_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0C9A78: init -- move.b #2,(0x22,A0) ; move.l #0xC9A9E,(0x44,A0) ;
     * move.l #0x27100000,(0x24,A0) ; move.l #0xC800,(0x28,A0) ;
     * clr.w (0x1E,A0) ; bra.w 0x0C9BCA */
    wr8(m, obj + 0x22u, 0x02u);
    wr32(m, obj + 0x44u, 0x000C9A9Eu);
    wr32(m, obj + 0x24u, 0x27100000u);
    wr32(m, obj + 0x28u, 0x0000C800u);
    wr16(m, obj + 0x1Eu, 0u);

    for (;;) {
        /* 0x0C9BCA: A1 = (0x8,A0) ; subq.w #1,(0x5A,A1) ; blt.w 0x0C9D68 */
        uint32_t parent = rd32(m, obj + 0x08u);
        int16_t cnt = (int16_t)(rd16(m, parent + 0x5Au) - 1u);
        wr16(m, parent + 0x5Au, (uint16_t)cnt);
        if (cnt < 0) {
            /* 0x0C9D68: addq.w #1,(0x5A,A1) ; jsr 0x105EC ; rts */
            wr16(m, parent + 0x5Au, (uint16_t)(rd16(m, parent + 0x5Au) + 1u));
            Obj_Free(m, obj);
            return 0;
        }

        /* 0x0C9BD6: btst.b #0,(0x65,A1) ; bne.w 0x0C9D68 */
        if ((rd8(m, parent + 0x65u) & 0x01u) != 0u) {
            /* 0x0C9D68: addq.w #1,(0x5A,A1) ; jsr 0x105EC ; rts */
            wr16(m, parent + 0x5Au, (uint16_t)(rd16(m, parent + 0x5Au) + 1u));
            Obj_Free(m, obj);
            return 0;
        }

        /* 0x0C9BDE: clr.l (0x54,A0) ; move.l (0x28,A0),(0x50,A0) ;
         * move.l (0x14,A1),(0x14,A0) ; move.l (0x18,A1),(0x18,A0) ;
         * addi.w #0xF,(0x18,A0) */
        wr32(m, obj + 0x54u, 0u);
        wr32(m, obj + 0x50u, rd32(m, obj + 0x28u));
        wr32(m, obj + 0x14u, rd32(m, parent + 0x14u));
        wr32(m, obj + 0x18u, rd32(m, parent + 0x18u));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x000Fu)); /* .w = high word */

        /* 0x0C9BFC: movea.l (0x44,A0),A1 ; move.w (A1)+,D0 ;
         * add.w D0,(0x18,A0) ; then four move.b (A1)+ / ext.w / move.w to
         * +0x2C, +0x30, +0x2E, +0x32 ; move.l A1,(0x44,A0) */
        uint32_t a1 = rd32(m, obj + 0x44u);
        uint32_t d0 = rd16(m, a1);
        a1 += 2u;
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)d0));
        int8_t v = (int8_t)rd8(m, a1);
        a1 += 1u;
        wr16(m, obj + 0x2Cu, (uint16_t)(int16_t)v);
        v = (int8_t)rd8(m, a1);
        a1 += 1u;
        wr16(m, obj + 0x30u, (uint16_t)(int16_t)v);
        v = (int8_t)rd8(m, a1);
        a1 += 1u;
        wr16(m, obj + 0x2Eu, (uint16_t)(int16_t)v);
        v = (int8_t)rd8(m, a1);
        a1 += 1u;
        wr16(m, obj + 0x32u, (uint16_t)(int16_t)v);
        wr32(m, obj + 0x44u, a1);

        /* 0x0C9C2A: clr.w +0x1C / +0x34 / +0x3A / +0x3E ;
         * 0x0C9C3A: A1 = (0xC,A0) ; add.b byte to +0x14, +0x18 ;
         * then two word speeds ext.l/asl.l #8 into +0x48, +0x4C */
        wr16(m, obj + 0x1Cu, 0u);
        wr16(m, obj + 0x34u, 0u);
        wr16(m, obj + 0x3Au, 0u);
        wr16(m, obj + 0x3Eu, 0u);

        uint32_t a2 = rd32(m, obj + 0x0Cu);
        int8_t b = (int8_t)rd8(m, a2);
        a2 += 1u;
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)(int16_t)b));
        b = (int8_t)rd8(m, a2);
        a2 += 1u;
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)(int16_t)b));
        int32_t w = (int16_t)rd16(m, a2);
        a2 += 2u;
        wr32(m, obj + 0x48u, (uint32_t)w << 8);
        w = (int16_t)rd16(m, a2);
        a2 += 2u;
        wr32(m, obj + 0x4Cu, (uint32_t)w << 8);

        /* 0x0C9C62: move.l #0x0C9C6A,(A0) ; rts */
        wr32(m, obj + 0x00u, 0x000C9C6Au);
        return 0;
    }
}
