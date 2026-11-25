/* ROM 0x18675E: child entrance tick. Seeds the object from its record through
 * 0x184908, copies the record's hit-box width and angle into the object, then
 * installs the 0x1867A4 behaviour. */
#include "obj_18675E.h"

/* ROM 0x18495E: scale the object's +0x24 by one of the 0x1849E4 rate bytes
 * (divu #0x64 then mulu.w). Jumped to with D1 = the +0x24 script value. */
static void sub_18495E(const f3mem_t *m, uint32_t obj)
{
    uint8_t sel = (uint8_t)(rd8(m, F3_A5 + 0x087Eu) & 0x0Fu);
    uint16_t rate = rd8(m, 0x001849E4u + (uint32_t)sel);
    uint32_t d1 = (uint32_t)(rd16(m, obj + 0x24u) / 0x0064u) & 0xFFFFu;
    d1 = (d1 * (uint32_t)rate) & 0xFFFFu;
    wr16(m, obj + 0x24u, (uint16_t)(rd16(m, obj + 0x24u) + (uint16_t)d1));
}

int Obj_18675E_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x18675E: movea.l (0xC,A0),A1 ; jsr 0x184908 */
    uint32_t a1 = rd32(m, obj + 0x0Cu);

    /* ROM 0x184908: seed the leaf fields from the record at A1 */
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr32(m, obj + 0x24u, 0u);
    wr32(m, obj + 0x28u, 0u);
    wr16(m, obj + 0x34u, 0u);
    wr16(m, obj + 0x24u, rd16(m, a1));
    a1 += 2u;
    wr16(m, obj + 0x28u, rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x2Cu, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x30u, rd32(m, a1));
    a1 += 4u;
    wr16(m, obj + 0x16u, rd16(m, a1));
    a1 += 2u;
    wr16(m, obj + 0x1Au, rd16(m, a1));
    a1 += 2u;
    a1 = rd32(m, a1);                          /* movea.l (A1)+,A1 */
    uint16_t d0 = rd16(m, a1);
    a1 += 2u;
    wr16(m, obj + 0x40u, d0);
    wr32(m, obj + 0x3Au, a1);

    /* 0x184940: lea 0x179A5C,A1 ; movea.l (0x0,A1,D0w*8),A1 ; jsr (A1).
     * Entry 0 is the frame-list INIT half 0x179550 (src/enemy/obj_179550.c);
     * 0x0E73C4-0x0E73E2 of the parent family show the same [idx], not
     * [idx+4], dispatch. */
    if (d0 == 0u) {
        /* 0x179550: move.w (A1)+,(0x3E,A0) ; move.l (A1)+,(0x10,A0) */
        uint32_t frame = rd32(m, obj + 0x3Au);
        wr16(m, obj + 0x3Eu, rd16(m, frame));
        wr32(m, obj + 0x10u, rd32(m, frame + 2u));
    } else {
        return -1;                             /* other lists are not entered */
    }

    /* 0x18494C: bsr.w 0x18495E */
    sub_18495E(m, obj);

    /* 0x184950: clear the frame-list leftovers */
    wr16(m, obj + 0x62u, 0u);
    wr32(m, obj + 0x64u, 0u);
    wr32(m, obj + 0x68u, 0u);

    /* 0x186768: A3 = (0xC,A0) ; move.w (0x20,A3),(0x34,A0) ;
     * move.w (0x22,A3),(0x22,A0) */
    {
        uint32_t a3 = rd32(m, obj + 0x0Cu);
        wr16(m, obj + 0x34u, rd16(m, a3 + 0x20u));
        wr16(m, obj + 0x22u, rd16(m, a3 + 0x22u));
    }

    /* 0x186778: move.l (0x4B6A,A5),(0x10,A0) */
    wr32(m, obj + 0x10u, rd32(m, F3_A5 + 0x4B6Au));

    /* 0x18677E bra.w 0x18679C -> move.l #0x1867A4,(A0) ; rts */
    wr32(m, obj, 0x001867A4u);
    return 0;
}
