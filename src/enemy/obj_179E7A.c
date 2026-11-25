#include "obj_179E7A.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_script10.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x179E7A: entrance tick of the 0x179EBC scripted variant. Budget gate,
 * parent-relative offset, the 0x179B06 frame-script pick, then the 10-byte
 * movement script 0x179EBC ({0x178B20, 0}, the loop marker {0x178CCE, -1}),
 * a hold flag and a random heading byte. */

/* ROM 0x179AE4: (dx.w, dy.w) at +0x0C rides on the parent's +0x50/+0x54 */
static void sub_179AE4(const f3mem_t *m, uint32_t obj)
{
    uint32_t a4 = rd32(m, obj + 0x08u);
    int16_t d0 = (int16_t)rd16(m, obj + 0x0Cu);
    int16_t d1 = (int16_t)rd16(m, obj + 0x0Eu);
    wr16(m, obj + 0x50u, (uint16_t)(d0 + rd16(m, a4 + 0x50u)));
    wr16(m, obj + 0x54u, (uint16_t)(d1 + rd16(m, a4 + 0x54u)));
}

/* ROM 0x179B06: pick one of two frame lists with Obj_RandRange(1), latch the
 * index into +0x40, point +0x3A at the record body and run that INIT half. */
static int sub_179B06(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0x00179B3Au;
    if (Obj_RandRange(m, 1u) != 0u)
        a1 = 0x00179B8Cu;

    uint16_t d0 = rd16(m, a1);
    a1 += 2u;
    wr16(m, obj + 0x40u, d0);
    wr32(m, obj + 0x3Au, a1);
    return Obj_AnimInit(m, obj, d0); /* 0x179B32 movea.l (0,A1,D0w*8),A1 ; jsr */
}

/* ROM 0x179D18: drop the slot counter and free the object */
static void sub_179D18(const f3mem_t *m, uint32_t obj)
{
    wr16(m, F3_A5 + 0x4B24u, (uint16_t)(rd16(m, F3_A5 + 0x4B24u) - 1u));
    Obj_Free(m, obj); /* 0x0105EC */
}

int Obj_179E7A_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x179E7A: cmpi.w #0x1E,(0x4B24,A5) ; bgt -> 0x179D18 */
    if ((int16_t)rd16(m, F3_A5 + 0x4B24u) > 0x1E) {
        sub_179D18(m, obj);
        return 0;
    }
    wr16(m, F3_A5 + 0x4B24u, (uint16_t)(rd16(m, F3_A5 + 0x4B24u) + 1u)); /* 0x179E84 */

    sub_179AE4(m, obj); /* 0x179E88 */
    if (sub_179B06(m, obj) < 0) /* 0x179E8C */
        return -1;

    /* 0x179E90: movea.l #0x179EBC,A1 ; move.l A1,(0x44,A0) ;
     * move.w (0x4,A1),(0x48,A0) ; clr.w (0x4A,A0) */
    {
        uint32_t a1 = 0x00179EBCu;
        wr32(m, obj + 0x44u, a1);
        wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
    }
    wr16(m, obj + 0x4Au, 0u);

    wr16(m, obj + 0x1Eu, 0u); /* 0x179EA4 */
    wr16(m, obj + 0x1Cu, 0u); /* 0x179EA8 */

    /* 0x179EAC: moveq #2,D0 ; jsr 0x1794A4 ; move.b D0b,(0x5C,A0) */
    wr8(m, obj + 0x5Cu, (uint8_t)Obj_RandRange(m, 2u));

    wr32(m, obj + 0x00u, 0x00179C6Eu); /* 0x179EB8 bra.w 0x179C66 */
    return 0;
}
