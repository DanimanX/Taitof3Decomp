#include "fn_auto.h"
/* ROM 0x162C4E: set up the line effects and tilemaps, then arm 0x162D98. */
#include "fn_call_frames.h"
#include "../video/sprite.h"

/* 0x162CB6–0x162D86 makes eight calls to 0x0053B2 with an unflipped map. */
static void draw_maps(const f3mem_t *m, f3regs_t *r)
{
    static const uint32_t source[] = { 0x6B02Cu, 0x6B03Eu, 0x6B04Eu, 0x6B064u };
    static const uint32_t dest[] = {
        0x611C00u, 0x611000u, 0x611C20u, 0x611020u,
        0x611C40u, 0x611040u, 0x611C60u, 0x611060u
    };
    uint32_t sp = r->a[7];
    for (unsigned i = 0; i < 8u; ++i) {
        push_word(m, r, 0u);
        push_long(m, r, dest[i]);
        push_long(m, r, source[i & 3u]);
        push_long(m, r, 0x162CCCu + 0x1Au * i);
        for (unsigned j = 3u; j > 0u;)
            push_long(m, r, r->a[--j]);
        for (unsigned j = 8u; j > 0u;)
            push_long(m, r, r->d[--j]);
        push_long(m, r, 0x0053DEu);
        Gfx_DrawCompressedTilemap(m, source[i & 3u], dest[i], 0u);
        r->a[7] = sp;
    }
}

int Fn_162C4E(const f3mem_t *m, f3regs_t *r)
{
    push_long(m, r, r->a[0]);
    r->d[0] = set_word(r->d[0], 0u);
    r->d[1] = set_word(r->d[1], 4u);
    uint32_t sp = r->a[7];
    push_long(m, r, r->a[0]);
    for (unsigned i = 8u; i > 0u;)
        push_long(m, r, r->d[--i]);

    /* 0x162C68–0x162CA0 clamps 4 and 0 to eight, packs them, and inverts
     * the word. The DBF at 0x162CAE writes 0xE8 words in descending order. */
    push_word(m, r, 8u);
    wr16(m, r->a[7], 4u);
    r->a[7] += 2u;
    push_word(m, r, 8u);
    wr16(m, r->a[7], 8u);
    r->a[7] += 2u;
    uint32_t dst = 0x626200u + (int16_t)rd16(m, r->a[5] - 0x5D56u);
    for (unsigned i = 0xE8u; i > 0u;)
        wr16(m, dst + 2u * --i, 0xBFB7u);
    r->a[7] = sp;

    draw_maps(m, r);
    uint32_t obj = rd32(m, r->a[7]);
    r->a[0] = obj;
    r->a[7] += 4u;
    wr16(m, obj + 0x44u, 0x80u);
    wr32(m, obj, 0x162D98u);
    r->a[7] += 4u;
    return 0;
}
