/* ROM 0x179354: root pop-up tick. Copies the frame-list record into the object
 * and walks the 0x178982 movement script with the +0x50/+0x54 screen offset;
 * when the script ends at -1 the object is freed, while a repeat re-installs
 * 0x179354. */
#include "obj_179354.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_script10.h"

#define A5_4B26 (F3_A5 + 0x4B26u)

/* ROM 0x17934C */
static void sub_17934C(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj, 0x00179354u);
}

/* ROM 0x178982: one pass of the 10-byte script at obj+0x44. Every step's
 * command is an absolute address that is not translated at this level, so the
 * walk is handed to Obj_Script10, which applies the two translated commands
 * and reports -1 for the rest. Returns -1 when a step was not observable. */
static int sub_178982(const f3mem_t *m, uint32_t obj)
{
    return Obj_Script10(m, obj);
}

/* ROM 0x179354 */
int Obj_179354_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x179354: bftst (0x4B26,A5){0x5:1} ; bne.w 0x1793A4 (free) */
    if ((rd8(m, A5_4B26) & 0x04u) != 0u) {
        Obj_Free(m, obj);
        return 0;
    }

    /* 0x17935E: take the frame record's position into +0x50/+0x54 and the
     * position pair into +0x14/+0x18 */
    {
        uint32_t a4 = rd32(m, obj + 0x08u);
        wr16(m, obj + 0x50u, rd16(m, a4 + 0x50u));
        wr16(m, obj + 0x54u, rd16(m, a4 + 0x54u));
    }
    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));

    /* 0x17937A: jsr 0x178982 -- walk the movement script once */
    if (sub_178982(m, obj) < 0)
        return -1;

    /* 0x17937E: the script's residue moves the parent by the offset the object
     * accumulated */
    {
        int16_t dx = (int16_t)(rd16(m, obj + 0x50u) - rd16(m, obj + 0x14u));
        int16_t dy = (int16_t)(rd16(m, obj + 0x54u) - rd16(m, obj + 0x18u));
        uint32_t a4 = rd32(m, obj + 0x08u);
        wr16(m, a4 + 0x50u, (uint16_t)(rd16(m, a4 + 0x50u) + (uint16_t)dx));
        wr16(m, a4 + 0x54u, (uint16_t)(rd16(m, a4 + 0x54u) + (uint16_t)dy));
    }

    /* 0x17939A: cmpi.l #-0x1,(0x44,A0) ; bne.b 0x17934C */
    if (rd32(m, obj + 0x44u) != 0xFFFFFFFFu) {
        sub_17934C(m, obj);
        return 0;
    }

    /* 0x1793A4: pea A0 ; jsr 0x0105EC */
    Obj_Free(m, obj);
    return 0;
}
