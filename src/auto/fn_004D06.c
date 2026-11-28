/* ROM 0x004D06: UNVERIFIED (weak capture): read the 0x80-byte settings block
 * from the serial EEPROM (0x00466E) and publish it into the RAM globals
 * 0x402237-0x402240, 0x4022A8, 0x409D3A/3C, then rescale the derived tables
 * through 0x004C78 and back them up through 0x004C12.
 *
 * The device read itself is not modelled, exactly as in the sibling helper
 * eeprom_block.h:settings_apply_4D06 that the test-mode screens use. It clocks
 * port 0x4A0013 and samples 0x4A0001, which the harness does not implement; a
 * modelled read would return zeros and wipe the block the captures hold. The
 * device returns the bytes it already holds, so skipping the read leaves the
 * block unchanged. D1 is reloaded by 0x004C78 and the read's D0/A0 are dead
 * before the next write, so the skipped call is invisible to the ALLREGS
 * comparison.
 *
 * Verification: the capture has 3 pairs and an empty body fails all of them,
 * but this body cannot be 3/3 either - the skipped read is the call whose
 * 0x0047C0 prologue stores the CPU status register into 0x406C50, and the
 * captures hold 0x2700 / 0x2710 / 0x2700 there (supervisor, IPL 7, X flag
 * varying) while f3regs_t carries no SR. Pair 0 (whose pre image already holds
 * 0x2700) is bit-exact, pairs 1 and 2 differ in that one device-side word and
 * in nothing else, so no capture line is registered for this address. */
#include "fn_auto.h"
#include "eeprom_block.h"

int Fn_004D06(const f3mem_t *m, f3regs_t *r)
{
    settings_apply_4D06(m, r);      /* 0x004D06..0x004DEC, see eeprom_block.h */
    return 0;
}
