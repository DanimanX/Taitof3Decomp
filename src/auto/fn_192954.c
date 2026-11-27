#include "fn_auto.h"
/* ROM 0x192954: verified on a poke-forced capture (scripts/zcap_poke_weak.sh): wait for the stage position, then upload a palette and spawn three objects. */
#include "../objects/obj_pool.h"
#include "../video/palette.h"

/* ROM 0x001660: call the palette queue with its LINK and MOVEM frame. */
void FnAuto_QueuePalette(const f3mem_t *m, f3regs_t *r, uint32_t src,
                         uint16_t dst, uint16_t count, uint32_t return_addr)
{
    uint32_t sp = r->a[7];
    wr32(m, sp - 4, count);
    wr32(m, sp - 8, dst);
    wr32(m, sp - 12, src);
    wr32(m, sp - 16, return_addr);
    wr32(m, sp - 20, r->a[6]);
    wr32(m, sp - 24, r->a[2]);
    wr32(m, sp - 28, r->a[0]);
    wr32(m, sp - 32, r->d[2]);
    wr32(m, sp - 36, r->d[1]);
    wr32(m, sp - 40, r->d[0]);

    uint32_t globals = r->a[5];
    uint32_t queue_count = globals - 0x5DF0;
    int16_t index = (int16_t)rd16(m, queue_count);
    if (index >= 0) {
        Pal_QueueUpload(m, src, dst, count);
    } else {
        /* 0x001682..0x001688 shift in a word and sign-extend at ADDA.W. */
        uint16_t offset = (uint16_t)((uint16_t)index << 3);
        uint32_t slot = globals - 0x6750 + (uint32_t)(int32_t)(int16_t)offset;
        wr16(m, slot, count);
        wr32(m, slot + 2, src);
        wr16(m, slot + 6, dst);
        wr16(m, globals - 0x5DEE, 2);
        wr16(m, queue_count, (uint16_t)(rd16(m, queue_count) + 1u));
    }
    r->d[0] = rd32(m, sp - 40);
    r->d[1] = rd32(m, sp - 36);
    r->d[2] = rd32(m, sp - 32);
    r->a[0] = rd32(m, sp - 28);
    r->a[2] = rd32(m, sp - 24);
    r->a[6] = rd32(m, sp - 20);
}

/* ROM 0x01057C: allocate one slot in group zero and return its address in D0. */
static void spawn(const f3mem_t *m, f3regs_t *r, uint32_t arg, uint32_t return_addr)
{
    uint32_t sp = r->a[7];
    wr32(m, sp - 4, arg);
    wr16(m, sp - 6, 1);
    wr16(m, sp - 8, 0);
    wr32(m, sp - 12, 0x192A1A);
    wr32(m, sp - 16, return_addr);
    uint32_t save = sp - 16;
    for (unsigned i = 3; i > 0;)
        wr32(m, save -= 4, r->a[--i]);
    for (unsigned i = 5; i > 1;)
        wr32(m, save -= 4, r->d[--i]);
    r->d[0] = Obj_Spawn(m, r->a[0], 0x192A1A, 0, 1, arg);
    for (unsigned i = 1; i <= 4; ++i, save += 4)
        r->d[i] = rd32(m, save);
    for (unsigned i = 0; i < 3; ++i, save += 4)
        r->a[i] = rd32(m, save);
}

int Fn_192954(const f3mem_t *m, f3regs_t *r)
{
    if ((int16_t)rd16(m, r->a[5] + 0x330C) <= 0x1650) {
        wr32(m, r->a[0], 0x192954); /* 0x19294C */
        return 0;
    }

    wr16(m, r->a[5] + 0x4B94, 0);
    FnAuto_QueuePalette(m, r, 0x1E4C0, 0x5E80, 0x10, 0x192978);
    spawn(m, r, 0x01680058, 0x192996);
    spawn(m, r, 0x01E00078, 0x1929B4);
    spawn(m, r, 0x02580068, 0x1929D2);
    wr16(m, r->a[0] + 0x06, 0x12C);
    /* 0x1929DC: the nonzero MOVE.W disables the branch to 0x1929EC. */
    wr32(m, r->a[0], 0x1929E6);
    return 0;
}
