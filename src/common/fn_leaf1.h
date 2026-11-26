#ifndef FN_LEAF1_H
#define FN_LEAF1_H

#include "f3mem.h"

#ifndef F3REGS_T_DEFINED
#define F3REGS_T_DEFINED
typedef struct {
    uint32_t d[8];
    uint32_t a[8];
} f3regs_t;
#endif

void Leaf_16AB58(const f3mem_t *m, f3regs_t *r);
void Leaf_16AB5C(const f3mem_t *m, f3regs_t *r);
void Leaf_16AB6A(const f3mem_t *m, f3regs_t *r);
void Leaf_16AB62(const f3mem_t *m, f3regs_t *r);
void Leaf_1766B6(const f3mem_t *m, f3regs_t *r);
void Leaf_0CEC40(const f3mem_t *m, f3regs_t *r);
void Leaf_1766DA(const f3mem_t *m, f3regs_t *r);
void Leaf_1766EE(const f3mem_t *m, f3regs_t *r);
void Leaf_13E9B0(const f3mem_t *m, f3regs_t *r);
void Leaf_0C660A(const f3mem_t *m, f3regs_t *r);
void Leaf_16A2F6(const f3mem_t *m, f3regs_t *r);
void Leaf_13E994(const f3mem_t *m, f3regs_t *r);
void Leaf_1766BE(const f3mem_t *m, f3regs_t *r);
void Leaf_13CAAE(const f3mem_t *m, f3regs_t *r);

#endif /* FN_LEAF1_H */
