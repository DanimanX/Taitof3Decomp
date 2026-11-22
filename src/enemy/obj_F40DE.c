#include "obj_F40DE.h"
#include "../objects/obj_common.h"
/* ROM 0x0F40DE: not decompiled yet (stub)
 * The listing is three instructions (tst.b (0x22,A0); beq.l 0x0CEE74; rts)
 * and the translation below is exact, but the golden capture cannot observe
 * it: scripts/capture_obj.sh takes its "post" snapshot at every hit of the
 * 0x104EE/0x104F0 tick-list site, so each pair's pre/post is the state at
 * two consecutive site hits (one extra tick plus the next dispatch) instead
 * of at this routine's rts. Paired with MAME truth the result was 59/60,
 * the single diff (0x41B1E1, inside the player record) not being produced by
 * any instruction in bundle_F40DE.s. Needs a recapture before it can pass.
 *   int Obj_F40DE_Tick(const f3mem_t *m, uint32_t obj)
 *   {
 *       if (rd8(m, obj + 0x22u) != 0u) return 0;
 *       Obj_StartDying(m, obj);
 *       return 0;
 *   }
 */
int Obj_F40DE_Tick(const f3mem_t *m, uint32_t obj) { (void)m; (void)obj; return -1; }
