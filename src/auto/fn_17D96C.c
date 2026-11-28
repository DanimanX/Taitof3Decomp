/* ROM 0x17D96C: short-lived "hit burst" object tick (A0 = object).
 * Counts +0x06 down and re-arms 0x17D96C while it stays > 0. On expiry it plays
 * sound 0x11 (0x13F238 + 0x39D8), seeds the hitbox/stats record 0x17DAB4
 * (0x184888), latches the animation index word from 0x17DACA (= 9, so the
 * 0x179A5C table entry 0x179600 runs the 16-direction record body 0x179898),
 * keeps the record cursor at 0x17DACC in +0x3A, installs script 0x17DB2C at
 * +0x44 with its timer word at +0x48, clears +0x4A and 0x17D9CE at +0x0C, and
 * parks the tick at the do-nothing 0x17C14C (0x17C144). */
#include "fn_auto.h"
#include "../objects/obj_anim.h"        /* Obj_AnimInit (0x179A5C dispatch) */
#include "../objects/obj_shared_ext.h"   /* Obj_184888 */
#include "../enemy/obj_E8B72.h"          /* Sound_PlayHooked */

#define TBL_17DAB4 0x0017DAB4u   /* stats/hitbox record */
#define IDX_17DACA 0x0017DACAu   /* {anim index.w, record body...} */
#define SCR_17DB2C 0x0017DB2Cu

int Fn_17D96C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);   /* 0x17D96C subq.w */
    wr16(m, obj + 0x06u, (uint16_t)t);
    if (t > 0) {                                        /* 0x17D970 bgt */
        wr32(m, obj + 0x00u, 0x0017D96Cu);              /* 0x17D964 */
        return 0;
    }

    Sound_PlayHooked(m, 0x0011u);                       /* 0x17D972..0x17D982 */
    Obj_184888(m, obj, TBL_17DAB4);                     /* 0x17D986..0x17D98C */

    uint32_t a1 = IDX_17DACA;                           /* 0x17D992 */
    uint16_t anim = rd16(m, a1);                        /* 0x17D998 */
    a1 += 2u;
    wr16(m, obj + 0x40u, anim);                         /* 0x17D99A */
    wr32(m, obj + 0x3Au, a1);                           /* 0x17D99E */

    /* 0x17D9A2..0x17D9AC: movea.l (0x179A5C,D0w*8),A1 ; jsr (A1). The index
     * word comes from ROM and is 9 (0x179600 -> 0x179898), so D0 after the
     * call is that record's palette word: 6*(angle+8)>>4 selects a 6-byte
     * {spr.l, pal.w} record at +0x3A. */
    if (Obj_AnimInit(m, obj, anim) < 0)
        return -1;
    uint16_t d0;
    {
        uint16_t sel = (uint16_t)((uint8_t)(rd8(m, obj + 0x20u) + 8u) >> 4);
        uint32_t rec = rd32(m, obj + 0x3Au) + ((uint32_t)sel * 6u & 0xFFFFu);
        d0 = rd16(m, rec + 4u);
    }

    a1 = SCR_17DB2C;                                    /* 0x17D9AE */
    wr32(m, obj + 0x44u, a1);                           /* 0x17D9B4 */
    wr16(m, obj + 0x48u, rd16(m, a1 + 4u));             /* 0x17D9B8 */
    wr16(m, obj + 0x4Au, 0u);                           /* 0x17D9BE */
    wr32(m, obj + 0x0Cu, 0x0017D9CEu);                  /* 0x17D9C2 */
    wr32(m, obj + 0x00u, 0x0017C14Cu);                  /* 0x17D9CA -> 0x17C144 */

    r->d[0] = (r->d[0] & 0xFFFF0000u) | d0;
    r->a[1] = a1;
    return 0;
}
