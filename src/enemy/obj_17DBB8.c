#include "obj_17DBB8.h"
#include "obj_17DC1E.h"
#include "obj_179550.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x17DBB8: one-frame wait of the 0x17DB68/0x17D912 families. When the
 * +0x06 counter reaches zero it seeds the 0x17DD50 stat record via 0x184888,
 * loads the 0x17DD66 frame record (and its 0x179A5C animation index 0 init),
 * points the movement script at 0x17DD9E, draws its first record's duration,
 * arms the +0x64 lifetime timer from the A5+0x4B3C LCG and copies the
 * A5+0xEBAA byte into +0x5C before parking on 0x17DC1E.
 *
 * The countdown path (subq/bgt -> 0x17DBB0) is byte-for-byte the 0x17DBB0
 * target of the 0x17DB68 tick, so it is folded in here. Verified bit-exact
 * against work/ram/obj_17DBB8 (60 ticks) once the harness hands the tick the
 * ROM image its 0x17DD50/0x179A5C decodes need. */

int Obj_17DBB8_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x17DBB8: subq.w #1,(0x6,A0) ; bgt.b 0x17DBB0 */
    {
        int16_t n = (int16_t)(uint16_t)(rd16(m, obj + 0x06u) - 1u);
        wr16(m, obj + 0x06u, (uint16_t)n);
        if (n > 0) {
            /* 0x17DBB0: bra.b 0x17DBB8 -- re-arm, nothing else */
            wr32(m, obj + 0x00u, 0x0017DBB8u);
            return 0;
        }
    }

    /* 0x17DBBE: movea.l #0x17DD50,A1 ; jsr 0x184888 */
    Obj_184888(m, obj, 0x0017DD50u);

    /* 0x17DBCA: movea.l #0x17DD66,A1 ; move.w (A1)+,D0 ; move.w D0,(0x40,A0) ;
     * move.l A1,(0x3A,A0) */
    wr16(m, obj + 0x40u, rd16(m, 0x0017DD66u));
    wr32(m, obj + 0x3Au, 0x0017DD68u);

    /* 0x17DBDA: lea 0x179A5C ; movea.l (A1,D0*8),A1 ; jsr (A1) -- animation
     * index 0 init (0x179550) */
    if (Obj_179550_Tick(m, obj) < 0)
        return -1;

    /* 0x17DBE6: movea.l #0x17DD9E,A1 ; move.l A1,(0x44,A0) ;
     * move.w (0x4,A1),(0x48,A0) ; clr.w (0x4A,A0) */
    wr32(m, obj + 0x44u, 0x0017DD9Eu);
    wr16(m, obj + 0x48u, rd16(m, 0x0017DDA2u));
    wr16(m, obj + 0x4Au, 0u);

    /* 0x17DBFA: move.l #0xC8,D0 ; jsr 0x1794F0 ; addi.w #0x50,D0 ;
     * move.w D0,(0x64,A0) */
    wr16(m, obj + 0x64u, (uint16_t)(Obj_RandRange(m, 0x00C8u) + 0x0050u));

    /* 0x17DC0E: move.w (A5-0x1456),D0 ; move.b D0,(0x5C,A0) */
    wr8(m, obj + 0x5Cu, (uint8_t)rd16(m, F3_A5 - 0x1456u));

    /* 0x17DC16: move.l #0x17DC1E,(A0) */
    wr32(m, obj + 0x00u, 0x0017DC1Eu);
    return 0;
}
