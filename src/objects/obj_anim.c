#include "obj_anim.h"
#include "../enemy/obj_179550.h"
#include "../enemy/obj_179600.h"
#include "../enemy/obj_179614.h"
#include "../enemy/obj_179600.h"
#include "../enemy/obj_179614.h"

/* The 0x179A5C engine. Bodies are lifted from the ROM listings (see
 * work/disasm/bundle_179664.s and the copies in src/enemy/obj_17E05A.c); only
 * index 0 is translated, the rest return -1 before any side effect. */

/* ROM 0x179550 (index 0 init): plain frame record at +0x3A -- {duration.w,
 * sprite.l} into the countdown (+0x3E) and the sprite pointer (+0x10). Lives in
 * src/enemy/obj_179550.c; call it through its header rather than duplicating. */

/* ROM 0x179664 */
void Obj_AnimTick0(const f3mem_t *m, uint32_t obj)
{
    int16_t timer = (int16_t)rd16(m, obj + 0x3Eu);

    if (timer > 0) {
        /* 0x179668 bgt -> 0x1796AC: just count down */
    } else if (timer < 0) {
        return;                                     /* 0x17966C blt / 0x179670 rts */
    } else {
        /* 0x179672: zero duration -- take the next record */
        uint32_t a1 = rd32(m, obj + 0x3Au);
        a1 += 6u;                                   /* 0x179676 lea (0x6,A1),A1 */
        int16_t d = (int16_t)rd16(m, a1);
        if (d > 0) {
            /* 0x17967C bgt -> 0x1796A4: rewind to this record and reload */
            wr32(m, obj + 0x3Au, a1);
            Obj_179550_Tick(m, obj);                /* 0x1796A8 bsr 0x179550 */
        } else if (d < 0) {
            /* 0x179680 blt -> 0x1796AC */
        } else {
            /* 0x179684: jump the pointer record, fetch the next index */
            a1 = rd32(m, a1 + 2u);                  /* 0x179684 movea.l (0x2,A1),A1 */
            uint16_t d0w = rd16(m, a1);             /* 0x17968A move.w (A1)+,D0w */
            a1 += 2u;
            wr16(m, obj + 0x40u, d0w);              /* 0x17968C */
            wr32(m, obj + 0x3Au, a1);               /* 0x179690 */
            /* 0x17969A: movea.l (0x0,0x179A5C,D0w*0x8),A1; jsr (A1)
             * The +0x0 offset selects the INIT slot of the pair (the +4 slot is
             * the tick), so D0w == 0 re-runs index 0's init, 0x179550. */
            if (d0w == 0u) {
                Obj_179550_Tick(m, obj);
            } else {
                /* the index's tick is not translated: leave 0x179A5C state
                 * untouched rather than guessing */
                wr16(m, obj + 0x40u, d0w);
            }
            /* 0x1796A0 bra.w 0x1796AC */
        }
    }
    wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) - 1u));  /* 0x1796AC */
}

int Obj_AnimTick(const f3mem_t *m, uint32_t obj, uint16_t idx)
{
    if (idx != 0u)
        return -1;
    Obj_AnimTick0(m, obj);
    return 0;
}

int Obj_AnimInit(const f3mem_t *m, uint32_t obj, uint16_t idx)
{
    switch (idx) {
    case 0u: return Obj_179550_Tick(m, obj);
    case 9u: return Obj_179600_Tick(m, obj);
    case 10u: return Obj_179614_Tick(m, obj);
    default: return -1;
    }
}
