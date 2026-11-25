#include "obj_159D68.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x176198: fill 0x1D0 bytes (232 words) of line RAM at base + (A5-0x5D56) with val */
static void sub_176198(const f3mem_t *m, uint32_t base, uint16_t val)
{
    uint32_t a0 = (uint32_t)(base + (int16_t)rd16(m, F3_A5 - 0x5D56u));
    for (uint32_t i = 0u; i < 0xE8u; i++)
        wr16(m, a0 + i * 2u, val);
}

/* ROM 0x176940: perspective line-RAM scroll & zoom table fill */
static void sub_176940(const f3mem_t *m, uint32_t a0, uint32_t a1,
                       int32_t d4, int32_t d5, int16_t d6w, int16_t d7w)
{
    int16_t step = (int16_t)((uint32_t)(d4 * 2 + 0x8000) >> 16);
    int16_t scroll_y = (int16_t)(0x24 - (int16_t)rd16(m, F3_A5 + 0x42A0u));

    while (d7w <= d6w) {
        int16_t r = (int16_t)(Rng_NextMem(m) & 0xFFFFu);
        int32_t acc = (int32_t)r * (int32_t)step + d5;
        step = (int16_t)((uint32_t)acc >> 16);
        int16_t d0w = (int16_t)(-0xA000 / (int16_t)(0x00A0 + step));
        d0w = (int16_t)(d0w + 0x0100);
        int32_t d2 = ((int32_t)scroll_y * (int32_t)d0w) << 6;
        int16_t d2w = (int16_t)((d2 / 50) ^ 0x003F);
        uint32_t off = (uint32_t)(int32_t)d7w * 2u;
        wr16(m, a0 + off, (uint16_t)d2w);
        wr8(m, a1 + off, (uint8_t)d0w);
        d7w++;
    }
}

/* ROM 0x159D68: transition into boss/stage tilemap & line-RAM setup */
int Obj_159D68_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)t);
    if (t > 0) {
        wr32(m, obj + 0x00u, 0x00159D68u);
        return 0;
    }

    sub_176198(m, 0x0062B200u, 0xB00Eu);

    for (uint32_t off = 0u; off <= 0x60u; off += 0x20u) {
        Gfx_DrawCompressedTilemap(m, 0x00066F5Cu, 0x00611000u + off, 0u);
        Gfx_DrawCompressedTilemap(m, 0x00066F88u, 0x00611400u + off, 0u);
    }
    for (uint32_t off = 0u; off <= 0x60u; off += 0x20u)
        Gfx_DrawCompressedTilemap(m, 0x00066D20u, 0x00611800u + off, 0u);
    for (uint32_t off = 0u; off <= 0x60u; off += 0x20u)
        Gfx_DrawCompressedTilemap(m, 0x00066D20u, 0x00611C00u + off, 0u);

    Mem_WritePair64(m, F3_A5 + 0x420Cu, F3_A5 - 0x5DA0u, 0u, (uint32_t)(-0x00D38000));
    wr32(m, F3_A5 + 0x422Cu, 0x000CCCCCu);
    wr32(m, F3_A5 + 0x4230u, 0x0000E000u);

    int16_t line_off = (int16_t)rd16(m, F3_A5 - 0x5D56u);
    sub_176940(m, (uint32_t)(0x0062A200 + line_off),
               (uint32_t)(0x00628200 + line_off),
               0x00060000, 0x00100000, 0x00E7, 0);

    int16_t pal_idx = (int16_t)(rd16(m, F3_A5 + 0x32DCu) - 0x0Fu);
    uint32_t pal_ptr = rd32(m, (uint32_t)(0x0015A27A + (int32_t)pal_idx * 4));
    Pal_QueueRamp(m, 0x0015A492u, pal_ptr, 0x0800u, 0x00B4u, 0x0020u);

    wr32(m, F3_A5 + 0x4234u, 0u);
    wr32(m, F3_A5 + 0x4238u, 0u);
    wr32(m, F3_A5 + 0x423Cu, 0u);
    wr32(m, F3_A5 + 0x4240u, 0u);

    wr16(m, obj + 0x44u, 0x00FFu);
    wr32(m, obj + 0x00u, 0x0015A088u);
    return 0;
}
