#include "obj_17D186.h"
#include "../objects/obj_anim.h"
#include "../audio/sound_cmd.h"

/* ROM 0x184888: initialise the object's stats from the record at A1 */
static void sub_184888(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr32(m, obj + 0x24u, 0u);
    wr32(m, obj + 0x28u, 0u);
    wr8(m, obj + 0x34u, 0x08u);
    wr8(m, obj + 0x35u, 0x00u);
    wr16(m, obj + 0x64u, 0u);
    wr16(m, obj + 0x1Cu, rd16(m, a1 + 0x00u));
    wr16(m, obj + 0x1Eu, rd16(m, a1 + 0x02u));
    wr16(m, obj + 0x24u, rd16(m, a1 + 0x04u));
    wr16(m, obj + 0x28u, rd16(m, a1 + 0x06u));
    wr32(m, obj + 0x2Cu, rd32(m, a1 + 0x08u));
    wr32(m, obj + 0x30u, rd32(m, a1 + 0x0Cu));
    wr16(m, obj + 0x62u, rd16(m, a1 + 0x10u));
    wr32(m, obj + 0x66u, rd32(m, a1 + 0x12u));
    wr8(m, obj + 0x22u, rd8(m, obj + 0x1Du));
    wr32(m, obj + 0x24u, rd32(m, obj + 0x24u) / 10u);
}

/* ROM 0x17D186: spawn-wait tail. When the +0x6 timer runs out it plays sound
 * 0x2A, seeds stats from 0x17D1E0, starts animation index 10, installs the
 * 0x17D2B8 behaviour record and hands over to 0x17BE14. */
int Obj_17D186_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t n = (int16_t)(uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)n);
    if (n > 0) {
        /* 0x17D18A bgt.b -> 0x17D17E, back into the wait body */
        wr32(m, obj + 0x00u, 0x0017D186u);
        return 0;
    }

    Sound_PlayCmd(m, 0x2A);                    /* 0x17D18C */
    sub_184888(m, obj, 0x0017D1E0u);           /* 0x17D1A6 */

    {
        uint32_t a1 = 0x0017D1F6u;
        uint16_t idx = rd16(m, a1);
        a1 += 2u;
        wr16(m, obj + 0x40u, idx);
        wr32(m, obj + 0x3Au, a1);
        if (Obj_AnimInit(m, obj, idx) < 0)
            return -1;
    }

    wr32(m, obj + 0x44u, 0x0017D2B8u);         /* 0x17D1C8 */
    wr16(m, obj + 0x48u, rd16(m, 0x0017D2BCu));
    wr16(m, obj + 0x4Au, 0x0000u);
    wr32(m, obj + 0x00u, 0x0017BE1Cu);         /* 0x17D1DC bra.w 0x17BE14 */
    return 0;
}
