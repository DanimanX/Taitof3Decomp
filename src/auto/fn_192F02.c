/* ROM 0x192F02: stage-3 mid-boss spawn burst (A0 = object). Decrements the
 * delay word at +0x06; while it stays positive it just re-arms 0x192F02.
 * Otherwise it spawns five pairs of 0x17A8D0 objects (args 0x1949F0,
 * 0x194A00, 0x194A10, 0x194A20, 0x194A30) in group 3, six 0x194814 objects
 * with args 5..0, four 0x118D12 objects (args 0x195392, 0x1953A0, 0x1953AE,
 * 0x1953BC) and two 0x17A664 objects (args 0x195676, 0x19568A), all in
 * group 3; queues the 0x20-word palette 0x17300 at 0x6540 (0x001660) and
 * continues at 0x193096. D0 ends as the last spawned object, D1.w as 0xFFFF
 * (the dbf counter) and A0 is untouched. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"  /* Obj_Spawn (0x01057C) */
#include "../video/palette.h"      /* Pal_QueueUpload (0x001660) */

int Fn_192F02(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = r->a[0];
    uint32_t d0;

    /* 0x192F02 subq.w #1,(0x6,A0) ; 0x192F06 bgt 0x192EFA */
    uint16_t t = (uint16_t)(rd16(m, a0 + 0x06u) - 1u);
    wr16(m, a0 + 0x06u, t);
    if ((int16_t)t > 0) {
        wr32(m, a0, 0x00192F02u);              /* 0x192EFA move.l #0x192f02,(A0) */
        return 0;                              /* 0x192F00 rts */
    }

    /* 0x192F08..0x192F9A: five 0x17A8D0 spawns, count 2, group 3 */
    d0 = Obj_Spawn(m, a0, 0x0017A8D0u, 3u, 2u, 0x001949F0u);
    d0 = Obj_Spawn(m, a0, 0x0017A8D0u, 3u, 2u, 0x00194A00u);
    d0 = Obj_Spawn(m, a0, 0x0017A8D0u, 3u, 2u, 0x00194A10u);
    d0 = Obj_Spawn(m, a0, 0x0017A8D0u, 3u, 2u, 0x00194A20u);
    d0 = Obj_Spawn(m, a0, 0x0017A8D0u, 3u, 2u, 0x00194A30u);

    /* 0x192F9E moveq #5,D1 ; 0x192FA0..0x192FBA six 0x194814 spawns with
     * args D1.w = 5..0 (dbf decrements after each body) */
    for (uint16_t i = 0u; i < 6u; i++) {
        d0 = Obj_Spawn(m, a0, 0x00194814u, 3u, 2u, 5u - (uint32_t)i);
    }

    /* 0x192FBE..0x193032: four 0x118D12 spawns, count 2, group 3 */
    d0 = Obj_Spawn(m, a0, 0x00118D12u, 3u, 2u, 0x00195392u);
    d0 = Obj_Spawn(m, a0, 0x00118D12u, 3u, 2u, 0x001953A0u);
    d0 = Obj_Spawn(m, a0, 0x00118D12u, 3u, 2u, 0x001953AEu);
    d0 = Obj_Spawn(m, a0, 0x00118D12u, 3u, 2u, 0x001953BCu);

    /* 0x193036..0x19306E: two 0x17A664 spawns, count 2, group 3 */
    d0 = Obj_Spawn(m, a0, 0x0017A664u, 3u, 2u, 0x00195676u);
    d0 = Obj_Spawn(m, a0, 0x0017A664u, 3u, 2u, 0x0019568Au);

    /* 0x193072..0x19308A: long 0x20, long 0x6540, long 0x17300, jsr 0x1660 */
    Pal_QueueUpload(m, 0x00017300u, 0x6540u, 0x0020u);

    wr32(m, a0, 0x00193096u);                  /* 0x19308E move.l #0x193096,(A0) */
    r->d[0] = d0;                              /* last Obj_Spawn result */
    r->d[1] = 0x0000FFFFu;                     /* 0x192FBA dbf leaves D1.w = -1 */
    return 0;                                  /* 0x193094 rts */
}
