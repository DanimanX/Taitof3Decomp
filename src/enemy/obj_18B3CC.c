/* ROM 0x18B3CC: per-flash tick of the 0x18B230 object. It copies the 52-byte
 * record at 0x18B59C into the object's state block, selects the 0x179A5C
 * frame-animation index from 0x18B5D4, runs that index's init half and arms the
 * 0x18B41C tick on the spare group-3 slot it allocates through 0x1057C. */
#include "obj_18B3CC.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_anim.h"

/* ROM 0x1848D6: the 13-field state-block copy. Every field below +0x34 is a
 * word, so the clr.w at the end clears the whole longword. */
static void sub_1848D6(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
    wr16(m, obj + 0x1Cu, rd16(m, a1)); a1 += 2u;
    wr16(m, obj + 0x1Eu, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x24u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x28u, rd32(m, a1)); a1 += 4u;
    wr16(m, obj + 0x2Cu, rd16(m, a1)); a1 += 2u;
    wr16(m, obj + 0x30u, rd16(m, a1)); a1 += 2u;
    wr16(m, obj + 0x2Eu, rd16(m, a1)); a1 += 2u;
    wr16(m, obj + 0x32u, rd16(m, a1)); a1 += 2u;
    wr16(m, obj + 0x16u, rd16(m, a1)); a1 += 2u;
    wr16(m, obj + 0x1Au, rd16(m, a1));
    wr16(m, obj + 0x34u, 0u);                 /* 0x184902 */
}

/* ROM 0x18B3CC */
int Obj_18B3CC_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x18B3CC: movea.l #0x18B59C,A1 ; jsr 0x1848D6 */
    sub_1848D6(m, obj, 0x0018B59Cu);

    /* 0x18B3D8: move.l A1,#0x18B5D4 ; move.w (A1)+,D0w ;
     *           move.w D0w,(A0+0x40) ; move.l A1,(A0+0x3A) */
    const uint32_t kAnimTbl = 0x0018B5D4u;
    uint16_t idx = rd16(m, kAnimTbl);
    wr16(m, obj + 0x40u, idx);
    wr32(m, obj + 0x3Au, kAnimTbl + 2u);

    /* 0x18B3E8: lea (0x179A5C).l,A1 ; movea.l (0x0,A1,D0w*0x8),A1 ; jsr (A1)
     * -- the +0 offset selects the index's INIT half. An index without a
     * translation leaves the state untouched and is reported, not guessed. */
    if (Obj_AnimInit(m, obj, idx) < 0)
        return -1;

    /* 0x18B3F4: pea (0x0).w ; move.w #2 ; move.w #3 ;
     *           move.l #0x18B4B8 ; jsr 0x1057C ; move.l D0,(A0+0xC) */
    wr32(m, obj + 0x0Cu, Obj_Spawn(m, obj, 0x0018B4B8u, 3u, 2u, 0x00000000u));

    /* 0x18B414: move.l #0x18B41C,(A0) */
    wr32(m, obj + 0x00u, 0x0018B41Cu);
    return 0;
}
