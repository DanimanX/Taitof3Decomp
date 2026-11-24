#include "obj_17D6C8.h"
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

/* ROM 0x17D6C8: spawn-wait tail. The +0x6 timer counts down; when it runs out
 * the object plays sound 0x11, seeds its stats from the 0x17D856 record,
 * starts the 0x17D86C animation through the 0x179A5C engine, installs the
 * 0x17D8CE behaviour record and parks on 0x17D72C. */
int Obj_17D6C8_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t n = (int16_t)(uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)n);
    if (n > 0) {
        /* 0x17D6CC bgt.b -> 0x17D6C0, back into the wait body */
        wr32(m, obj + 0x00u, 0x0017D6C8u);
        return 0;
    }

    Sound_PlayCmd(m, 0x11);                    /* 0x17D6CE */
    sub_184888(m, obj, 0x0017D856u);     /* 0x17D6E8 jsr 0x184888 */

    /* 0x17D6EE: animation table 0x17D86C, first entry is the index into
     * the 0x179A5C engine */
    {
        uint32_t a1 = 0x0017D86Cu;
        uint16_t idx = rd16(m, a1);
        a1 += 2u;
        wr16(m, obj + 0x40u, idx);
        wr32(m, obj + 0x3Au, a1);
        if (Obj_AnimInit(m, obj, idx) < 0)
            return -1;
    }

    /* 0x17D70A: behaviour record 0x17D8CE */
    wr32(m, obj + 0x44u, 0x0017D8CEu);
    wr16(m, obj + 0x48u, rd16(m, 0x0017D8D2u));
    wr16(m, obj + 0x4Au, 0x0000u);
    wr16(m, obj + 0x0Cu, rd16(m, obj + 0x20u));
    wr32(m, obj + 0x00u, 0x0017D72Cu);         /* 0x17D724 */
    return 0;
}
