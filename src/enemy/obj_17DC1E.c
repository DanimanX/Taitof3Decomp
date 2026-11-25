#include "obj_17DC1E.h"
#include "obj_179DEE.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_script10.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x17DC1E: main body of the 17DB68 cluster shot. A hit (+0x24 > 0)
 * routes into the 0x0066 handler; otherwise the movement script runs, the
 * object leaves the 0x184A00/0x184A04 play area by freeing itself, registers
 * itself with both object lists (A5+0x906 / A5+0x8FE), picks its colour byte
 * and lands in the shared 0x179DEE death state.
 *
 * The 0x66 dispatch (0x17DD50: hurt / death / explosion) has one translated
 * target, the shared 0x179DEE death entrance; every other target returns -1
 * before any side effect. */

/* ROM 0x17C78E: free the object */
static void lab_17C78E(const f3mem_t *m, uint32_t obj)
{
    Obj_Free(m, obj);
}

/* ROM 0x179DEE: shared death entrance of the 0x17C78E family */
static int sub_179DEE(const f3mem_t *m, uint32_t obj)
{
    return Obj_179DEE_Tick(m, obj);
}

int Obj_17DC1E_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x17DC1E: btst.b #4,(0xEBB4,A5) ; beq ; clr.l (0x24,A0) -- hit recovery
     * (A5+0xEBB4 == the word at 0x408EB4) */
    if (rd16(m, F3_A5 + 0x0EB4u) & 0x0010u)
        wr32(m, obj + 0x24u, 0u);

    /* 0x17DC2C: tst.l (0x24,A0) ; bgt -> 0x17DC3A ; else jmp (0x66,A0) */
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        /* 0x17DC34: movea.l (0x66,A0),A1 ; jmp (A1) -- hurt / death /
         * explosion routine, none of which is translated yet. The one
         * exception is the 0x179DEE death entrance the family shares. */
        uint32_t a1 = rd32(m, obj + 0x66u);
        if (a1 == 0x00179DEEu)
            return sub_179DEE(m, obj);
        return -1;
    }

    /* 0x17DC3A jsr 0x178982: one pass of the movement script */
    if (Obj_Script10(m, obj) < 0)
        return -1;

    /* 0x17DC40..0x17DC5C: cmp2.w (0x184A00)/(0x184A04) ; bcs -> 0x17C78E */
    {
        int16_t x = (int16_t)rd16(m, obj + 0x50u);
        int16_t y = (int16_t)rd16(m, obj + 0x54u);
        if (x < (int16_t)rd16(m, 0x00184A00u) || x > (int16_t)rd16(m, 0x00184A02u) ||
            y < (int16_t)rd16(m, 0x00184A04u) || y > (int16_t)rd16(m, 0x00184A06u)) {
            lab_17C78E(m, obj);
            return 0;
        }
    }

    /* 0x17DC60: append to the A5+0x906 list */
    Coll_AddHazard(m, obj);

    /* 0x17DC6A: movea.l (0x8FE,A5),A1 ; move.l A0,(A1)+ ; move.l A1,(0x8FE,A5)
     * -- the second registration list, same append pattern */
    {
        uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);
        wr32(m, a1, obj);
        wr32(m, F3_A5 + 0x08FEu, a1 + 4u);
    }

    /* 0x17DC74: move.b (0x22,A0),D1; cmpi.w #0x3C,(0x64,A0); bgt ;
     * move.b #0x30,D1 -- the flash colour while +0x64 has run low */
    {
        uint8_t d1 = rd8(m, obj + 0x22u);
        if (!((int16_t)rd16(m, obj + 0x64u) > 0x3C))
            d1 = 0x30u;

        /* 0x17DC86..0x17DCB0: colour override state machine on +0x1D/+0x35 */
        if (rd8(m, obj + 0x1Du) == 0x06u)
            wr8(m, obj + 0x1Du, 0x08u);
        else if (rd8(m, obj + 0x35u) != 0u)
            wr8(m, obj + 0x1Du, 0x06u);
        else
            wr8(m, obj + 0x1Du, d1);
    }
    wr8(m, obj + 0x35u, 0u);                       /* 0x17DCB4 */

    /* 0x17DCB8: +0x14/+0x18 := +0x50/+0x54 */
    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));

    /* 0x17DCC4: move.w (0x40,A0),D0 ; movea.l (0x179A5E,D0*8),A1 ; jsr (A1) */
    if (Obj_AnimTick(m, obj, rd16(m, obj + 0x40u)) < 0)
        return -1;

    /* 0x17DCD4: centre the sprite by the frame's (w, h) */
    {
        uint32_t a1 = rd32(m, obj + 0x10u);
        uint16_t d1 = (uint16_t)(rd16(m, a1) << 3);
        uint16_t d0 = (uint16_t)(rd16(m, a1 + 2u) << 3);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d1));
    }
    Obj_DrawSprite(m, obj);                        /* 0x17DCE8 */

    /* 0x17DCEE: btst.b #1,(0x1D58,A5) ; beq -> 0x17DD10 */
    if (rd16(m, F3_A5 + 0x1D58u) & 0x0002u) {
        /* 0x17DCF6: bfextu (0x62,A0){0:16},D0 ; add.l D0,(A5+0x1D50)->+0x48 ;
         * move.b (0x22,A0),(0x1D,A0) ; jmp 0x0CBB2C (explosion) */
        uint32_t p = rd32(m, F3_A5 + 0x1D50u);
        wr32(m, p + 0x48u, rd32(m, p + 0x48u) + rd16(m, obj + 0x62u));
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x22u));
        /* 0x0CBB2C: sf.b (0x32,A0) ; bra 0x0CBB50 -- the 0x0CBB50 body is not
         * translated, so the state is set up but the tick stays unobservable */
        wr8(m, obj + 0x32u, 0u);
        wr32(m, obj + 0x00u, 0x000CBB50u);
        return -1;
    }

    /* 0x17DD10: subq.w #1,(0x64,A0) ; bgt -> 0x17DC16 (re-arm 0x17DC1E) */
    {
        int16_t n = (int16_t)(uint16_t)(rd16(m, obj + 0x64u) - 1u);
        wr16(m, obj + 0x64u, (uint16_t)n);
        if (n > 0) {
            wr32(m, obj + 0x00u, 0x0017DC1Eu);
            return 0;
        }
    }

    /* 0x17DD18: move.l #0x80,-(SP) ; move.w #2,-(SP) ; move.w #9,-(SP) ;
     * move.l #0x17DDD0,-(SP) ; jsr 0x1057C (spawn, group 9, 2 groups) ;
     * 0x17DD36 move.w #1,(0x6,A0) ; beq never branches ; 0x17DD3E install */
    (void)Obj_Spawn(m, obj, 0x0017DDD0u, 0x0009u, 0x0002u, 0x0080u);
    wr16(m, obj + 0x06u, 0x0001u);
    wr32(m, obj + 0x00u, 0x0017DD46u);
    return 0;
}
