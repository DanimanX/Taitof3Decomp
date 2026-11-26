/* ROM 0x184BAE: one-shot fuse, the two-frame variant of 0x184A20. Counts
 * obj+6 down (the capture enters with +6 = 2, so a single call falls to 1 and
 * re-enters the top), and at zero builds the boss fight with 14 0x1057C calls:
 *   0x184BB4  0x186972  -> spawner 0x18657E, child kept at A5+0x4B5E
 *   0x184BD6  0x186908  -> 0x18657E, result dropped
 *   0x184BF4  0x186930  -> 0x18657E, dropped
 *   0x184C12  0x186B3A  -> spawner 0x186782, child kept at A5+0x4B62
 *   0x184C34  0x186A42  -> 0x18657E, dropped
 *   0x184C52  0x186A1A  -> 0x18657E, dropped
 *   0x184C70  0x1869F2  -> 0x18657E, dropped
 *   0x184C8E  0x186876  -> 0x18657E, child kept at A5+0x4B56
 *   0x184CB0  0x18689E  -> 0x18657E, child kept at A5+0x4B5A
 *   0x184CD2  0x187246  -> 0x187246, arg 0, dropped   (move.l #0 ; pea via 0)
 *   0x184CEE  0x187024  -> 0x187024, arg 0, dropped
 *   0x184D0A  {0x118D12, 0x187008}      -> dropped
 *   0x184D28  {0x118D12, 0x187016}      -> dropped
 *   0x184D46  {0x17A664, 0x184F56}      -> dropped
 *   0x184D64  installs 0x184D6C as the next tick in obj+0.
 * "dropped": the child address returned in D0 is not stored. */
#include "obj_184BAE.h"
#include "../objects/obj_pool.h"

/* ROM 0x184BAE */
int Obj_184BAE_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x184BAE: subq.w #1,(0x6,A0) ; bgt.b 0x184BB4 */
    uint16_t d = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, d);
    if ((int16_t)d > 0)
        return 0;                       /* 0x184BB4 */

    /* Each site: move.l arg,-(a7) ; move.w #2,-(a7) ; move.w #3,-(a7) ;
     * move.l fn,-(a7) ; jsr 0x1057C ; lea (0xc,a7),a7 */
    wr32(m, F3_A5 + 0x4B5Eu, Obj_Spawn(m, obj, 0x0018657Eu, 3u, 2u, 0x00186972u)); /* 0x184BB4 */
    (void)Obj_Spawn(m, obj, 0x0018657Eu, 3u, 2u, 0x00186908u);                      /* 0x184BD6 */
    (void)Obj_Spawn(m, obj, 0x0018657Eu, 3u, 2u, 0x00186930u);                      /* 0x184BF4 */
    wr32(m, F3_A5 + 0x4B62u, Obj_Spawn(m, obj, 0x00186782u, 3u, 2u, 0x00186B3Au)); /* 0x184C12 */
    (void)Obj_Spawn(m, obj, 0x0018657Eu, 3u, 2u, 0x00186A42u);                      /* 0x184C34 */
    (void)Obj_Spawn(m, obj, 0x0018657Eu, 3u, 2u, 0x00186A1Au);                      /* 0x184C52 */
    (void)Obj_Spawn(m, obj, 0x0018657Eu, 3u, 2u, 0x001869F2u);                      /* 0x184C70 */
    wr32(m, F3_A5 + 0x4B56u, Obj_Spawn(m, obj, 0x0018657Eu, 3u, 2u, 0x00186876u)); /* 0x184C8E */
    wr32(m, F3_A5 + 0x4B5Au, Obj_Spawn(m, obj, 0x0018657Eu, 3u, 2u, 0x0018689Eu)); /* 0x184CB0 */
    (void)Obj_Spawn(m, obj, 0x00187246u, 3u, 2u, 0u);                               /* 0x184CD2: pea 0 */
    (void)Obj_Spawn(m, obj, 0x00187024u, 3u, 2u, 0u);                               /* 0x184CEE: pea 0 */
    (void)Obj_Spawn(m, obj, 0x00118D12u, 3u, 2u, 0x00187008u);                      /* 0x184D0A */
    (void)Obj_Spawn(m, obj, 0x00118D12u, 3u, 2u, 0x00187016u);                      /* 0x184D28 */
    (void)Obj_Spawn(m, obj, 0x0017A664u, 3u, 2u, 0x00184F56u);                      /* 0x184D46 */

    /* 0x184D64: move.l #0x184D6C,(A0) ; rts */
    wr32(m, obj + 0x00u, 0x00184D6Cu);
    return 0;
}
