#ifndef TAITOF3_AUDIO_SOUND_CMD_H
#define TAITOF3_AUDIO_SOUND_CMD_H
/* Main-CPU side of the sound interface: command byte-strings from ROM tables
 * are copied into a 1 KiB ring in the MB8421 DPRAM (0xC00000); DPRAM word
 * 0xC00480 holds the ring write index * 2. */
#include "../common/f3mem.h"

void Sound_ResetDpram(const f3mem_t *m);                    /* ROM 0x3960 */
void Sound_WriteScratch(const f3mem_t *m, uint16_t *idx);   /* ROM 0x3998 */
void Sound_PlayCmd(const f3mem_t *m, int16_t cmd);          /* ROM 0x39D8 */
void Sound_PlayCmdForced(const f3mem_t *m, int16_t cmd);    /* ROM 0x3B66 */
void Sound_StopAll(const f3mem_t *m);                       /* ROM 0x3C24 */
void Sound_StopAllPart2(const f3mem_t *m);                  /* ROM 0x3C12 */
void Sound_SetVolume(const f3mem_t *m, uint8_t a, uint8_t b, uint8_t c, uint8_t d); /* ROM 0x3CBA */

#endif
