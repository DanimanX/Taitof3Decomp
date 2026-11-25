#include "obj_17ACA2.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_script10.h"

/* ROM 0x17ACA2: scripted carrier. Starts at the parent's position, runs the
 * 0x178982 script and pushes the parent by the distance the script moved it;
 * when the script pointer (+0x44) reaches -1 it releases the parent's hold
 * bit and frees itself. */
int Obj_17ACA2_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a4 = rd32(m, obj + 0x08u);
    wr16(m, obj + 0x50u, rd16(m, a4 + 0x50u));
    wr16(m, obj + 0x54u, rd16(m, a4 + 0x54u));
    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));

    if (Obj_Script10(m, obj) < 0)                   /* 0x17ACBE */
        return -1;

    /* 0x17ACC4: parent += (+0x50,+0x54) - (+0x14,+0x18) */
    uint16_t d0 = (uint16_t)(rd16(m, obj + 0x50u) - rd16(m, obj + 0x14u));
    uint16_t d1 = (uint16_t)(rd16(m, obj + 0x54u) - rd16(m, obj + 0x18u));
    wr16(m, a4 + 0x50u, (uint16_t)(rd16(m, a4 + 0x50u) + d0));
    wr16(m, a4 + 0x54u, (uint16_t)(rd16(m, a4 + 0x54u) + d1));

    /* 0x17ACDC: cmpi.l #-1,(0x44,A0) ; bne 0x17AC9A (re-arm) */
    if (rd32(m, obj + 0x44u) != 0xFFFFFFFFu) {
        wr32(m, obj + 0x00u, 0x0017ACA2u);
        return 0;
    }
    /* 0x17ACE6: bfclr (0x23,A4){1:1} = bit 6 of that byte ; free self */
    wr8(m, a4 + 0x23u, (uint8_t)(rd8(m, a4 + 0x23u) & (uint8_t)~0x40u));
    Obj_Free(m, obj);
    return 0;
}
