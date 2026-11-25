#include "obj_17EA7E.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_script10.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_shared.h"
#include "obj_17C4D4.h"
#include "../video/sprite.h"

/* ROM 0x17EA7E: the burst child's flight step. Free when it leaves the
 * 0x184A00 region or its +0x24 life runs out, otherwise run the 0x44
 * behaviour script, follow the parent's +0x16/+0x1A velocity, animate, centre
 * the frame, draw and append to the hazard list. Once it is on screen and the
 * alternate sound mode is off it becomes the 0x0CBB2C explosion. */

/* ROM 0x17C78E: free the object */
static void lab_17C78E(const f3mem_t *m, uint32_t obj)
{
    Obj_Free(m, obj);
}

/* ROM 0x17EAA0: the flight body 0x17EA9A reaches after its jsr 0x178982.
 * 0x17F260's 0x17F43C body jumps back into exactly this address, so it lives
 * here as an internal entry; the script walk before it is the caller's. */
static int lab_17EAA0(const f3mem_t *m, uint32_t obj)
{
    /* 0x17EAA0: +0x50/+0x54 += the parent's +0x16/+0x1A velocity */
    {
        uint32_t a4 = rd32(m, obj + 0x08u);
        uint16_t dx = rd16(m, a4 + 0x16u);
        uint16_t dy = rd16(m, a4 + 0x1Au);
        wr16(m, obj + 0x50u, (uint16_t)(rd16(m, obj + 0x50u) + dx));
        wr16(m, obj + 0x54u, (uint16_t)(rd16(m, obj + 0x54u) + dy));
    }

    /* 0x17EAB4: cmp2.w (0x184A00)/(0x184A04); bcs 0x17C78E */
    {
        int16_t x = (int16_t)rd16(m, obj + 0x50u);
        int16_t y = (int16_t)rd16(m, obj + 0x54u);
        if (!(x > (int16_t)rd16(m, 0x00184A00u) && x < (int16_t)rd16(m, 0x00184A02u)) ||
            !(y > (int16_t)rd16(m, 0x00184A04u) && y < (int16_t)rd16(m, 0x00184A06u))) {
            lab_17C78E(m, obj);
            return 0;
        }
    }

    /* 0x17EAD4: +0x14/+0x18 = +0x50/+0x54 */
    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));

    /* 0x17EAE0: A1 = 0x179A5C + idx*8 + 4 (the TICK half), jsr (A1) */
    if (Obj_AnimTick(m, obj, rd16(m, obj + 0x40u)) < 0)
        return -1;

    /* 0x17EAF0: movea.l (0x10,A0),A1; move.w (A1)+,D1w; move.w (A1)+,D0w;
     * asl.w #3 both; sub.w D0w,(0x14,A0); sub.w D1w,(0x18,A0) */
    {
        uint32_t a1 = rd32(m, obj + 0x10u);
        uint16_t d1 = rd16(m, a1);
        uint16_t d0 = rd16(m, a1 + 2u);
        d1 = (uint16_t)(d1 << 3);
        d0 = (uint16_t)(d0 << 3);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d1));
    }

    /* 0x17EB04: append the object to the hazard list at A5+0x906 */
    {
        uint32_t p = rd32(m, F3_A5 + 0x906u);
        wr32(m, p, obj);
        wr32(m, F3_A5 + 0x906u, p + 4u);
    }

    Obj_DrawSprite(m, obj);                         /* 0x17EB0E jsr 0x1282 */

    /* 0x17EB14: alternate sound mode off and x inside [-0xA, 0x11C]:
     * jmp 0x0CBB2C */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x000A && x <= 0x011C) {
            Obj_ExplodeTick(m, obj);
            return 0;
        }
    }

    /* 0x17EB32: bra.w 0x17EA76 */
    wr32(m, obj + 0x00u, 0x0017EA7Eu);
    return 0;
}

/* ROM 0x17EA7E */
int Obj_17EA7E_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x17EA7E: btst.b #4,(-0x144C,A5); beq; clr.l (0x24,A0) */
    if ((rd8(m, F3_A5 - 0x144Cu) & 0x10u) != 0u)
        wr32(m, obj + 0x24u, 0u);

    /* 0x17EA8C: tst.l (0x24,A0); bgt; movea.l (0x66,A0),A1; jmp (A1) */
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        uint32_t target = rd32(m, obj + 0x66u);
        /* jmp (A1): the +0x66 handler is 0x17C4D4 in every capture */
        if (target != 0x0017C4D4u)
            return -1;                              /* handler not translated */
        Obj_17C4D4_Tick(m, obj);
    }

    /* 0x17EA9A: jsr 0x178982, then 0x17EAA0 */
    if (Obj_Script10(m, obj) < 0)
        return -1;
    return lab_17EAA0(m, obj);
}
