#include "obj_123FCA.h"
#include "../objects/obj_pool.h"
#include "../video/palette.h"
#include "../video/sprite.h"

/* ROM 0x118154: clear stage-scroll state, reset 0x41B50E table and 0x627600
 * line RAM, then clear PF0b. */
static void sub_118154(const f3mem_t *m)
{
    wr16(m, F3_A5 + 0x30C0u, 0u);
    for (uint32_t i = 0; i < 0x20u; i++)
        wr32(m, 0x0041B50Eu + i * 4u, 0x80008000u);

    uint32_t a0 = 0x00627600u + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
    for (uint32_t i = 0; i < 0xE8u; i++)
        wr16(m, a0 + i * 2u, 0x0DDDu);

    Gfx_ClearPf0b(m);
}

/* ROM 0x123FCA: boss/scene controller init, upload palettes, spawn parts */
int Obj_123FCA_Tick(const f3mem_t *m, uint32_t obj)
{
    static const struct {
        uint32_t fn;
        uint16_t group;
        uint16_t ngroups;
        uint32_t arg;
    } spawns[] = {
        { 0x00118CF6u, 3u, 2u, 0x001257DCu },
        { 0x00118D12u, 3u, 2u, 0x001257EAu },
        { 0x00124D64u, 3u, 2u, 0u },
        { 0x00124E00u, 3u, 2u, 0x001258C8u },
        { 0x00125016u, 3u, 2u, 0x001258D8u },
        { 0x00125D22u, 3u, 2u, 0u },
        { 0x0011B14Cu, 5u, 2u, 0x0012589Au },
        { 0x0011B14Cu, 5u, 2u, 0x00125832u },
        { 0x0011B14Cu, 5u, 2u, 0x00125860u },
        { 0x001250BCu, 5u, 2u, 0x001258E8u },
        { 0x001253B0u, 5u, 2u, 0x00125918u },
        { 0x0012552Eu, 5u, 2u, 0x00125984u },
        { 0x0011B14Cu, 5u, 2u, 0x001257F8u },
        { 0x0011AFA6u, 5u, 2u, 0x001259FCu },
        { 0x0011AEBEu, 5u, 2u, 0u },
        { 0x0011C1DEu, 5u, 2u, 0u },
        { 0x0011C39Au, 5u, 2u, 0u },
    };

    wr32(m, F3_A5 + 0x30E6u, 0x00125A0Cu);
    wr32(m, F3_A5 + 0x30EAu, 0x00125A8Cu);
    uint32_t d0 = Obj_Spawn(m, obj, 0x00124404u, 3u, 2u, 0u);

    /* 0x124076 */
    wr32(m, F3_A5 + 0x30DEu, d0);
    wr32(m, F3_A5 + 0x30EEu, 0u);
    wr8(m, F3_A5 + 0x30F3u, 0xFFu);
    wr32(m, F3_A5 + 0x30DAu, 0x00FA0000u);
    Pal_QueueUpload(m, rd32(m, F3_A5 + 0x30E6u), 0x5280u, 0x20u);
    Pal_QueueUpload(m, rd32(m, F3_A5 + 0x30EAu), 0x5200u, 0x20u);
    wr8(m, F3_A5 + 0x30F4u, 0x00u);
    wr16(m, F3_A5 + 0x30C2u, (uint16_t)-0x5A);
    wr16(m, F3_A5 + 0x30C4u, 0x0008u);
    wr16(m, F3_A5 + 0x30C6u, (uint16_t)-0x24);
    wr16(m, F3_A5 + 0x30C8u, 0x0020u);
    sub_118154(m);

    wr16(m, obj + 0x34u, 0u);
    wr8(m, obj + 0x1Du, 0x00u);
    wr8(m, obj + 0x4Fu, 0x00u);
    wr8(m, obj + 0x59u, 0x00u);

    for (uint32_t i = 0; i < sizeof(spawns) / sizeof(spawns[0]); i++)
        Obj_Spawn(m, obj, spawns[i].fn, spawns[i].group, spawns[i].ngroups, spawns[i].arg);

    wr32(m, obj + 0x00u, 0x001242ECu);
    return 0;
}
