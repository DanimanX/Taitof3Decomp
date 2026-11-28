/* ROM 0x0F4060: small enemy/effect init (A0 = object).
 * Seeds HP/hitboxes from template 0x0F8214 (0x0CEDE4 = 0x0CEDE4 body), loads
 * frame record 0x0F407A through 0x0CFA86 (D0 = its frame word, A1 = record+6),
 * then tail-calls 0x0CEF0E: on bomb-clear (A5-0x144B bit 5) it starts dying
 * (0x0CEE74), on debug kill (A5+0x1D58 bit 1, X in [-0xA..0x11C]) it copies
 * +0x4F to +0x1D and enters 0x0CBB2C, otherwise it parks the tick at
 * 0x0CEF44. */
#include "fn_auto.h"
#include "../objects/obj_shared_ext.h"
#include "../enemy/obj_E8B72.h"

int Fn_0F4060(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    Obj_0CEDE4(m, obj, 0x000F8214u);                    /* 0x0F4060..0x0F4064 */
    Obj_SetFrame(m, obj, 0x000F407Au);                  /* 0x0F406A..0x0F406E -> 0x0CFA86 */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | rd16(m, 0x000F407Au);
    r->a[1] = 0x000F4080u;

    Obj_0CEF0E(m, obj);                                 /* 0x0F4074 bra.l 0x0CEF0E */
    return 0;
}
