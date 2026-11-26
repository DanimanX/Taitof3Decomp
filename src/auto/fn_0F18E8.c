/* ROM 0x0F18E8: enemy/projectile init (A0 = object).
 * Seeds from 0x0EABE2 (0x0CEDE4), installs script 0x0F1AD0 (0x0CF1D8),
 * loads frame 0x0F1AFA (0x0CFA86), sets +0x1D = 0xDE, +0x1E = 0x8080,
 * doubles +0x4E.b, and arms tick 0x0F1922. */
#include "fn_auto.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../enemy/obj_E8B72.h"

int Fn_0F18E8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    Obj_0CEDE4(m, obj, 0x000EABE2u);                         /* 0x0F18E8..0x0F18EC */
    Obj_StartScript(m, obj, 0x000F1AD0u);                    /* 0x0F18F2..0x0F18F6 */
    Obj_SetFrame(m, obj, 0x000F1AFAu);                       /* 0x0F18FC..0x0F1900 -> 0x0CFA86 */
    r->a[1] = 0x000F1B00u;                                   /* 0x0F1AFA + 6 */

    wr8(m, obj + 0x1Du, 0xDEu);                              /* 0x0F1906 */
    wr16(m, obj + 0x1Eu, 0x8080u);                           /* 0x0F190C */

    uint8_t d0b = rd8(m, obj + 0x4Eu);                       /* 0x0F1912 */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | (rd16(m, 0x000F1AFAu) & 0xFF00u) | d0b;
    wr8(m, obj + 0x4Eu, (uint8_t)(d0b + d0b));               /* 0x0F1916 */
    wr32(m, obj, 0x000F1922u);                               /* 0x0F191A */
    return 0;
}
