/* Cross-unit entry points of the register-level drafts: routines that started
 * as file-static helpers inside one translated routine but have a capture of
 * their own, so src/auto/fn_<ADDR>.c calls them instead of restating them. */
#ifndef FN_HELPERS_H
#define FN_HELPERS_H
#include "fn_auto.h"

/* src/auto/fn_0B760C.c -- blank the w x h box at {x.w, y.w} = *(rect). */
void sub_0B73E2(const f3mem_t *m, uint16_t w, uint16_t h, uint32_t rect);

/* src/enemy/obj_105EDE.c -- apply frame record A1 + table 0x0CFD80[D0.w]. */
void sub_0CFD66(const f3mem_t *m, uint32_t obj, uint32_t a1, uint16_t d0);

/* src/enemy/obj_CC11C.c -- difficulty table 0x0D0CC2[(A5+0x87F) >> 3]. */
uint16_t sub_0D0C7A(const f3mem_t *m);

/* src/auto/fn_144378.c -- close the row at the cursor; returns the new cursor. */
uint32_t sub_14477C(const f3mem_t *m);

#endif
