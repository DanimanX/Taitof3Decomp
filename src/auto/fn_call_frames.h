#ifndef FN_CALL_FRAMES_H
#define FN_CALL_FRAMES_H

#include "fn_auto.h"
#include "../objects/obj_pool.h"

static inline uint32_t set_word(uint32_t reg, uint16_t word)
{
    return (reg & 0xFFFF0000u) | word;
}

static inline void push_word(const f3mem_t *m, f3regs_t *r, uint16_t word)
{
    r->a[7] -= 2u;
    wr16(m, r->a[7], word);
}

static inline void push_long(const f3mem_t *m, f3regs_t *r, uint32_t value)
{
    r->a[7] -= 4u;
    wr32(m, r->a[7], value);
}

/* 0x01057C saves D1-D4/A0-A2 and returns the allocation in D0. */
static inline void call_spawn(const f3mem_t *m, f3regs_t *r, uint32_t ret,
                              uint32_t fn, uint16_t group, uint16_t count,
                              uint32_t arg)
{
    uint32_t sp = r->a[7];
    push_long(m, r, arg);
    push_word(m, r, count);
    push_word(m, r, group);
    push_long(m, r, fn);
    push_long(m, r, ret);
    for (unsigned i = 3; i > 0;)
        push_long(m, r, r->a[--i]);
    for (unsigned i = 5; i > 1;)
        push_long(m, r, r->d[--i]);
    r->d[0] = Obj_Spawn(m, r->a[0], fn, group, count, arg);
    r->a[7] = sp;
}

/* 0x0105EC saves D0/A0-A1. */
static inline void call_free(const f3mem_t *m, f3regs_t *r, uint32_t ret)
{
    uint32_t sp = r->a[7];
    push_long(m, r, r->a[0]);
    push_long(m, r, ret);
    push_long(m, r, r->a[1]);
    push_long(m, r, r->a[0]);
    push_long(m, r, r->d[0]);
    Obj_Free(m, r->a[0]);
    r->a[7] = sp;
}

#endif
