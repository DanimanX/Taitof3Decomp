#include "obj_17C14C.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_script10.h"
#include "../objects/obj_shared.h"
#include "../video/sprite.h"

/* ROM 0x17C14C: flight step of the 0x17C0xx family (the capture starts it
 * with PC = 0x17C14C, so this entry is the whole body). It runs the 0x44
 * movement script, re-centres on the 0x50/0x54 position, the 0x179A5C frame
 * tick and the +0x0C callback, registers the object in the hazard (A5+0x906)
 * and sort (A5+0x8FE) lists, resolves the +0x1D colour, draws, and either
 * parks on the 0x17C144 thunk or -- with the alternate sound bit
 * A5+0x1D58.1 -- credits +0x62 to the 0x1D50 timer and becomes the 0x0CBB2C
 * explosion. */

/* ROM 0x17C78E: free the object */
static void lab_17C78E(const f3mem_t *m, uint32_t obj)
{
    Obj_Free(m, obj);
}

/* cmp2.w (base)/(base+2) < D0w (a `bcs` after CMP2): true when the word is
 * outside the (lo, hi) pair. */
static int outside_box(const f3mem_t *m, uint32_t base, uint16_t v)
{
    int16_t lo = (int16_t)rd16(m, base), hi = (int16_t)rd16(m, base + 2u);
    int16_t x = (int16_t)v;
    return !(x > lo && x < hi);
}

/* ROM 0x17C144: re-arm the same tick */
static void lab_17C144(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x00u, 0x0017C14Cu);
}

/* ROM 0x17C1D8: the +0x1D colour byte. 6 means "blinking", +0x35 = 0 takes
 * the flying colour from +0x22, +0x35 != 0 forces 6 for this frame, and the
 * +0x35 request is always cleared afterwards. */
static void sub_17C1D8(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x1Du) == 0x06u) {
        wr8(m, obj + 0x1Du, 0x08u);
    } else if (rd8(m, obj + 0x35u) != 0u) {
        wr8(m, obj + 0x1Du, 0x06u);
    } else {
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x22u));
    }
    wr8(m, obj + 0x35u, 0u);
}

/* The two ROM jumps (0x17C166 `jmp (0x66,A0)` and 0x17C1BE `jsr (0x0C,A0)`)
 * go straight to another tick. The reachable targets are the ones with a
 * translation in this tree; anything else is reported as an undecompiled
 * path so the pair neither passes nor applies half of the step. */
static int dispatch_tick(const f3mem_t *m, uint32_t obj, uint32_t fn)
{
    switch (fn) {
    case 0x0CEBEE: {
        /* 0x0CEBEE: restore the pushed position after drawing the 0x23 body */
        uint32_t a1 = rd32(m, obj + 0x08u);
        wr16(m, obj + 0x18u, rd16(m, a1 + 0x18u));
        wr16(m, obj + 0x14u, rd16(m, a1 + 0x14u));
        return 0;
    }
    case 0x17D9CE: {
        /* 0x17D9CE: every third call spawn one 0x17A3F4 shot (group 5, one
         * slot... pushed #0, #2, #5, #0x17A3F4) and restart the +0x64 count */
        int16_t n = (int16_t)(rd16(m, obj + 0x64u) + 1u);
        wr16(m, obj + 0x64u, (uint16_t)n);
        if (n >= 3) {
            (void)Obj_Spawn(m, obj, 0x0017A3F4u, 5u, 2u, 0u);
            wr16(m, obj + 0x64u, 0u);
        }
        return 0;
    }
    default:
        return -1;
    }
}

int Obj_17C14C_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x17C14C: the clear bit, then the "no HP" hand-off through +0x66 */
    if ((rd8(m, F3_A5 - 0x144Cu) & 0x10u) != 0u)
        wr32(m, obj + 0x24u, 0u);
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        /* jmp (0x66) -- a bare jump with A0 still the object */
        uint32_t fn = rd32(m, obj + 0x66u);
        if (fn == 0u)
            return -1;
        return dispatch_tick(m, obj, fn);
    }

    /* 0x17C168: the 0x44 script */
    if (Obj_Script10(m, obj) < 0)
        return -1;

    /* 0x17C16E / 0x17C17E: the 0x1849F8 / 0x1849FC window; outside it the
     * object is retired */
    if (outside_box(m, 0x001849F8u, rd16(m, obj + 0x50u)) ||
        outside_box(m, 0x001849FCu, rd16(m, obj + 0x54u))) {
        lab_17C78E(m, obj);
        return 0;
    }

    /* 0x17C18E / 0x17C19A: centre = position, then the frame centre */
    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));

    if (Obj_AnimTick(m, obj, rd16(m, obj + 0x40u)) < 0)
        return -1;

    {
        uint32_t a1 = rd32(m, obj + 0x10u);
        uint16_t d1 = (uint16_t)(rd16(m, a1) << 3);
        uint16_t d0 = (uint16_t)(rd16(m, a1 + 2u) << 3);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d1));
    }

    /* 0x17C1BE: the +0x0C callback */
    {
        uint32_t fn = rd32(m, obj + 0x0Cu);
        if (fn == 0u)
            return -1;
        if (dispatch_tick(m, obj, fn) < 0)
            return -1;
    }

    /* 0x17C1C4 / 0x17C1CE: hazard and sort lists */
    {
        uint32_t p = rd32(m, F3_A5 + 0x0906u);
        wr32(m, p, obj);
        wr32(m, F3_A5 + 0x0906u, p + 4u);
    }
    {
        uint32_t p = rd32(m, F3_A5 + 0x08FEu);
        wr32(m, p, obj);
        wr32(m, F3_A5 + 0x08FEu, p + 4u);
    }

    /* 0x17C1D8 */
    sub_17C1D8(m, obj);

    Obj_DrawSprite(m, obj);                          /* 0x17C20C */

    /* 0x17C212: alternate sound bit -> credit +0x62 and become the explosion */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        Obj_RelayTimer(m, rd16(m, obj + 0x62u));
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x22u));
        Obj_ExplodeTick(m, obj);
        wr32(m, obj + 0x00u, 0x000CBB2Cu);
        return 0;
    }

    lab_17C144(m, obj);                              /* 0x17C234 */
    return 0;
}
