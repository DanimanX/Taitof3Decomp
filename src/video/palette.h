#ifndef TAITOF3_VIDEO_PALETTE_H
#define TAITOF3_VIDEO_PALETTE_H

#include "../common/f3mem.h"

void Pal_QueueUpload(const f3mem_t *m, uint32_t src, uint16_t dst, uint16_t count); /* ROM 0x001660 */
void Pal_ClearRamps(const f3mem_t *m);                                              /* ROM 0x001774 */
void Pal_Blend64(const f3mem_t *m, uint32_t dst, uint32_t src1, uint32_t src2, uint16_t weight); /* ROM 0x16A258 */

void Pal_UploadFade150156(const f3mem_t *m); /* ROM 0x150156 */
void Pal_UploadFade150578(const f3mem_t *m); /* ROM 0x150578 */
void Pal_UploadFade15099A(const f3mem_t *m); /* ROM 0x15099A */
void Pal_UploadFade15458C(const f3mem_t *m); /* ROM 0x15458C */
void Pal_UploadFade1549AE(const f3mem_t *m); /* ROM 0x1549AE */
void Pal_UploadFade154DD0(const f3mem_t *m); /* ROM 0x154DD0 */
void Pal_UploadFade1551F2(const f3mem_t *m); /* ROM 0x1551F2 */
void Pal_UploadFade163D1A(const f3mem_t *m); /* ROM 0x163D1A */
void Pal_UploadFade16413C(const f3mem_t *m); /* ROM 0x16413C */

#endif
