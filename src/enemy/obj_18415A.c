#include "obj_18415A.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x17C78E: off-screen / dead, free the object */
static void lab_17C78E(const f3mem_t *m, uint32_t obj)
{
    Obj_Free(m, obj);
}

/* cmp2.w (0x1849F8) / (0x1849FC): inside the on-screen box (bcs -> 0x17C78E) */
static int in_bounds(const f3mem_t *m, uint32_t obj)
{
    int16_t x = (int16_t)rd16(m, obj + 0x50u);
    int16_t y = (int16_t)rd16(m, obj + 0x54u);
    if (x < (int16_t)rd16(m, 0x001849F8u) || x > (int16_t)rd16(m, 0x001849FAu))
        return 0;
    if (y < (int16_t)rd16(m, 0x001849FCu) || y > (int16_t)rd16(m, 0x001849FEu))
        return 0;
    return 1;
}

/* ROM 0x18418A: common body; A2 = the 16-byte difficulty record
 * {child fn, 0x184888 arg, anim record, script} */
static int body_18418A(const f3mem_t *m, uint32_t obj, uint32_t a2)
{
    (void)Obj_PolarPlace(m, obj);
    if (!in_bounds(m, obj)) {
        lab_17C78E(m, obj);
        return 0;
    }

    (void)Obj_Spawn(m, obj, rd32(m, a2), 0x0Bu, 1u, 0u);
    a2 += 4u;

    wr16(m, obj + 0x06u, 0u);
    Obj_184888(m, obj, rd32(m, a2));
    a2 += 4u;

    uint32_t anim_ptr = rd32(m, a2);
    a2 += 4u;
    uint16_t anim_idx = rd16(m, anim_ptr);
    anim_ptr += 2u;
    wr16(m, obj + 0x40u, anim_idx);
    wr32(m, obj + 0x3Au, anim_ptr);
    if (Obj_AnimInit(m, obj, anim_idx) < 0)
        return -1;

    uint32_t script_ptr = rd32(m, a2);
    wr32(m, obj + 0x44u, script_ptr);
    wr16(m, obj + 0x48u, rd16(m, script_ptr + 4u));
    wr16(m, obj + 0x4Au, 0u);

    /* bra.l 0x17BE14 */
    wr32(m, obj + 0x00u, 0x0017BE1Cu);
    return 0;
}

/* ROM 0x18415A: difficulty-branched spawn/init (bfextu A5+0x87E{8:4}),
 * then hand off to 0x17BE14 (0x17BE1C). */
int Obj_18415A_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t d0 = (uint16_t)((rd16(m, F3_A5 + 0x87Eu) >> 4) & 0x0Fu);
    if ((int16_t)d0 >= 8)
        return body_18418A(m, obj, 0x00184234u);         /* 0x184184 */
    if ((int16_t)d0 >= 4)
        return Obj_18417A_Tick(m, obj);
    return body_18418A(m, obj, 0x00184214u);             /* 0x184170 */
}

/* ROM 0x18417A: difficulty 4..7 case, lea 0x184224(pc),a2 ; bra 0x18418A */
int Obj_18417A_Tick(const f3mem_t *m, uint32_t obj)
{
    return body_18418A(m, obj, 0x00184224u);
}
