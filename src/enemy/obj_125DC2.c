#include "obj_125DC2.h"
#include "../objects/obj_pool.h"
#include "../video/palette.h"

/* ROM 0x0CE1B2: queue 8 consecutive 0x10-entry palette blocks from table 0xCE1EC[idx]. */
static void sub_0CE1B2(const f3mem_t *m, uint16_t idx)
{
    uint32_t src = rd32(m, 0x000CE1ECu + (uint32_t)idx * 4u);
    uint16_t dst = 0x6C04u;
    for (uint32_t i = 0; i < 8u; i++) {
        Pal_QueueUpload(m, src, dst, 0x10u);
        src += 0x40u;
        dst = (uint16_t)(dst + 0x80u);
    }
}

/* ROM 0x125DC2: boss/scene controller init, upload palettes, spawn parts */
int Obj_125DC2_Tick(const f3mem_t *m, uint32_t obj)
{
    static const struct {
        uint32_t fn;
        uint16_t group;
        uint16_t ngroups;
        uint32_t arg;
    } spawns[] = {
        { 0x00118CF6u, 3u, 2u, 0x00127EAAu },
        { 0x0012706Cu, 4u, 1u, 0u },
        { 0x0011B14Cu, 4u, 1u, 0x00127FB0u },
        { 0x00126D92u, 4u, 1u, 0u },
        { 0x0011B14Cu, 4u, 1u, 0x00127EB8u },
        { 0x0011B14Cu, 4u, 1u, 0x00127FC4u },
        { 0x0011B14Cu, 4u, 1u, 0x00127F48u },
        { 0x0011B14Cu, 4u, 1u, 0x00127ECCu },
        { 0x00126FA4u, 4u, 1u, 0u },
        { 0x00126E84u, 4u, 1u, 0x00127FD8u },
        { 0x0011B14Cu, 4u, 1u, 0x00127F5Cu },
        { 0x0011B14Cu, 4u, 1u, 0x00127EF4u },
        { 0x0011B14Cu, 4u, 1u, 0x00127EE0u },
        { 0x0012737Eu, 4u, 1u, 0x00128008u },
        { 0x00127560u, 4u, 1u, 0x001280F8u },
        { 0x0011C1DEu, 4u, 1u, 0u },
        { 0x0011C39Au, 4u, 1u, 0u },
    };

    uint32_t d0 = Obj_Spawn(m, obj, 0x00126132u, 3u, 2u, 0u);

    /* 0x125DE2 */
    wr32(m, F3_A5 + 0x30DEu, d0);
    wr32(m, F3_A5 + 0x30E2u, rd32(m, F3_A5 + 0x1D28u));
    wr8(m, obj + 0x59u, 0x00u);
    wr8(m, F3_A5 + 0x30F2u, 0x00u);
    wr8(m, F3_A5 + 0x30F4u, 0x00u);
    wr32(m, F3_A5 + 0x30EEu, 0u);
    wr8(m, F3_A5 + 0x30F3u, 0xFFu);
    wr32(m, F3_A5 + 0x30DAu, 0x0BB80000u);
    sub_0CE1B2(m, 0u);
    wr32(m, F3_A5 + 0x3272u, 0u);

    for (uint32_t i = 0; i < sizeof(spawns) / sizeof(spawns[0]); i++)
        Obj_Spawn(m, obj, spawns[i].fn, spawns[i].group, spawns[i].ngroups, spawns[i].arg);

    wr8(m, obj + 0x35u, 0x00u);
    wr8(m, obj + 0x1Du, 0x00u);
    wr8(m, obj + 0x4Fu, 0x00u);
    wr32(m, obj + 0x00u, 0x0012601Cu);
    return 0;
}
