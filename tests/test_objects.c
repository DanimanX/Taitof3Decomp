#include <string.h>
#include <stdlib.h>
/* Differential tests for individual object tick routines, captured at the
 * pool call site with a conditional breakpoint, e.g.
 *   F3_SPR=1 F3_COND="a1==102fe6" scripts/capture_fn.sh 104EE 104F0 400 work/ram/obj_102FE6
 * Compares all Work RAM (except the stack window) and sprite RAM. Pairs where
 * the C reaches a not-yet-decompiled path (returns -1) are counted apart. */
#include "capture.h"
#include "../src/enemy/enemy_bullet.h"
#include "../src/player/player_shot.h"
#include "../src/enemy/obj_D9116.h"
#include "../src/enemy/obj_CABF4.h"
#include "../src/enemy/obj_EAD28.h"
#include "../src/enemy/obj_1027B4.h"
#include "../src/enemy/obj_102860.h"
#include "../src/enemy/obj_102928.h"
#include "../src/enemy/obj_C9F2E.h"
#include "../src/enemy/obj_10252E.h"
#include "../src/enemy/obj_102A9A.h"
#include "../src/enemy/obj_119990.h"
#include "../src/enemy/obj_CBF5E.h"
#include "../src/enemy/obj_119CBA.h"
#include "../src/enemy/obj_117D06.h"
#include "../src/enemy/obj_D5464.h"
#include "../src/enemy/obj_13E5B2.h"
#include "../src/enemy/obj_CAB36.h"
#include "../src/objects/obj_common.h"
#include "../src/enemy/obj_119714.h"
#include "../src/enemy/obj_CBF8A.h"
#include "../src/enemy/obj_DCDF2.h"
#include "../src/enemy/obj_DC8BE.h"
#include "../src/enemy/obj_F30F6.h"
#include "../src/enemy/obj_E04CC.h"
#include "../src/enemy/obj_DECF8.h"
#include "../src/enemy/obj_EACDE.h"
#include "../src/enemy/obj_E0278.h"
#include "../src/enemy/obj_117BDE.h"
#include "../src/enemy/obj_F302E.h"
#include "../src/enemy/obj_DC3F6.h"
#include "../src/enemy/obj_C6866.h"
#include "../src/enemy/obj_C9978.h"
#include "../src/enemy/obj_C9D7E.h"
#include "../src/enemy/obj_DC542.h"
#include "../src/enemy/obj_E02C4.h"
#include "../src/enemy/obj_CBF6C.h"
#include "../src/enemy/obj_D540A.h"
#include "../src/enemy/obj_D901A.h"
#include "../src/enemy/obj_CC0F4.h"
#include "../src/enemy/obj_D9092.h"
#include "../src/enemy/obj_DC456.h"
#include "../src/enemy/obj_DC586.h"
#include "../src/enemy/obj_DEC9E.h"
#include "../src/enemy/obj_E029E.h"
#include "../src/enemy/obj_E0266.h"
#include "../src/enemy/obj_F30B4.h"
#include "../src/enemy/obj_102F52.h"
#include "../src/enemy/obj_117B44.h"
#include "../src/enemy/obj_117C28.h"
#include "../src/enemy/obj_13E4EE.h"
#include "../src/enemy/obj_DC328.h"
#include "../src/enemy/obj_DC0F8.h"
#include "../src/enemy/obj_DF7EA.h"
#include "../src/enemy/obj_DF844.h"
#include "../src/enemy/obj_CBC88.h"
#include "../src/enemy/obj_CBC90.h"
#include "../src/enemy/obj_BB83E.h"
#include "../src/enemy/obj_BC3EC.h"
#include "../src/enemy/obj_BCA00.h"
#include "../src/enemy/obj_BCD82.h"
#include "../src/enemy/obj_CBC98.h"
#include "../src/enemy/obj_11C39A.h"
#include "../src/enemy/obj_BC07E.h"
#include "../src/enemy/obj_BD544.h"
#include "../src/enemy/obj_C685A.h"
#include "../src/enemy/obj_CB406.h"
#include "../src/enemy/obj_CB0DA.h"
#include "../src/enemy/obj_BC152.h"
#include "../src/enemy/obj_BC1CE.h"
#include "../src/enemy/obj_BC3DE.h"
#include "../src/enemy/obj_BC064.h"
#include "../src/enemy/obj_BC638.h"
#include "../src/enemy/obj_BD298.h"
#include "../src/enemy/obj_CB412.h"
#include "../src/enemy/obj_12EF24.h"
#include "../src/enemy/obj_BBD4A.h"
#include "../src/enemy/obj_BBE06.h"
#include "../src/enemy/obj_BC4A8.h"
#include "../src/enemy/obj_BD256.h"
#include "../src/enemy/obj_BBBE0.h"
#include "../src/enemy/obj_BC008.h"
#include "../src/enemy/obj_BD340.h"
#include "../src/enemy/obj_D7570.h"
#include "../src/enemy/obj_BBFE8.h"
#include "../src/enemy/obj_BCE8E.h"
#include "../src/enemy/obj_BD354.h"
#include "../src/enemy/obj_12E1FA.h"
#include "../src/enemy/obj_BCF08.h"
#include "../src/enemy/obj_BCE38.h"
#include "../src/enemy/obj_12F26A.h"
#include "../src/enemy/obj_12F27A.h"
#include "../src/enemy/obj_BBD24.h"
#include "../src/enemy/obj_BBDE0.h"
#include "../src/enemy/obj_BBE9A.h"
#include "../src/enemy/obj_CABAC.h"
#include "../src/enemy/obj_BB746.h"
#include "../src/enemy/obj_BC03C.h"
#include "../src/enemy/obj_CB2C6.h"
#include "../src/enemy/obj_BCD90.h"
#include "../src/enemy/obj_BBCDA.h"
#include "../src/enemy/obj_BBD6E.h"
#include "../src/enemy/obj_BBE2A.h"
#include "../src/enemy/obj_11C1DE.h"
#include "../src/enemy/obj_12EA9E.h"
#include "../src/enemy/obj_BC540.h"
#include "../src/enemy/obj_11A87E.h"
#include "../src/enemy/obj_12E21A.h"
#include "../src/enemy/obj_CA28C.h"
#include "../src/enemy/obj_CAF3E.h"
#include "../src/enemy/obj_BC3FA.h"
#include "../src/enemy/obj_BC404.h"
#include "../src/enemy/obj_BC40E.h"
#include "../src/enemy/obj_BC418.h"
#include "../src/enemy/obj_BC422.h"
#include "../src/enemy/obj_BC24A.h"
#include "../src/enemy/obj_CAF1A.h"
#include "../src/enemy/obj_BB692.h"
#include "../src/enemy/obj_BCEB0.h"
#include "../src/enemy/obj_12E1BE.h"
#include "../src/enemy/obj_BBD00.h"
#include "../src/enemy/obj_BCF30.h"
#include "../src/enemy/obj_BE262.h"
#include "../src/enemy/obj_BB84C.h"
#include "../src/enemy/obj_BB856.h"
#include "../src/enemy/obj_BB860.h"
#include "../src/enemy/obj_BB86A.h"
#include "../src/enemy/obj_BB874.h"
#include "../src/enemy/obj_BCDC0.h"
#include "../src/enemy/obj_CAF5E.h"
#include "../src/enemy/obj_12EDA8.h"
#include "../src/enemy/obj_12E19A.h"
#include "../src/enemy/obj_BC680.h"
#include "../src/enemy/obj_BCFF4.h"
#include "../src/enemy/obj_12EB0E.h"
#include "../src/enemy/obj_BB5C2.h"
#include "../src/enemy/obj_BBE50.h"
#include "../src/enemy/obj_BCE52.h"
#include "../src/enemy/obj_DBDA2.h"
#include "../src/enemy/obj_ECF3E.h"
#include "../src/enemy/obj_BB952.h"
#include "../src/enemy/obj_BBD94.h"
#include "../src/enemy/obj_BBEC0.h"
#include "../src/enemy/obj_DBDE8.h"
#include "../src/enemy/obj_DC674.h"
#include "../src/enemy/obj_105268.h"
#include "../src/enemy/obj_BBB18.h"
#include "../src/enemy/obj_CAEBA.h"
#include "../src/enemy/obj_EA28C.h"
#include "../src/enemy/obj_EA836.h"
#include "../src/enemy/obj_10524A.h"
#include "../src/enemy/obj_1095B0.h"
#include "../src/enemy/obj_F34F6.h"
#include "../src/enemy/obj_1116E2.h"
#include "../src/enemy/obj_12EA24.h"
#include "../src/enemy/obj_BD990.h"
#include "../src/enemy/obj_12EAD4.h"
#include "../src/enemy/obj_12CE5A.h"
#include "../src/enemy/obj_CB2EC.h"
#include "../src/enemy/obj_10290C.h"
#include "../src/enemy/obj_E186A.h"
#include "../src/enemy/obj_102798.h"
#include "../src/enemy/obj_102844.h"
#include "../src/enemy/obj_102A70.h"
#include "../src/enemy/obj_BBAE4.h"
#include "../src/enemy/obj_CBC78.h"
#include "../src/enemy/obj_DA0D6.h"
#include "../src/enemy/obj_ECA04.h"
#include "../src/enemy/obj_EA418.h"
#include "../src/enemy/obj_ECF0C.h"
#include "../src/enemy/obj_ECFAE.h"
#include "../src/enemy/obj_BBBF4.h"
#include "../src/enemy/obj_BC576.h"
#include "../src/enemy/obj_BDE42.h"
#include "../src/enemy/obj_EA29E.h"
#include "../src/enemy/obj_104EFA.h"
#include "../src/enemy/obj_C9B3E.h"
#include "../src/enemy/obj_CBA34.h"
#include "../src/enemy/obj_ECFDA.h"
#include "../src/enemy/obj_BD308.h"
#include "../src/enemy/obj_CB054.h"
#include "../src/enemy/obj_CB422.h"
#include "../src/enemy/obj_CAF90.h"
#include "../src/enemy/obj_F3582.h"
#include "../src/enemy/obj_BC2A0.h"
#include "../src/enemy/obj_10963E.h"
#include "../src/enemy/obj_EACEA.h"
#include "../src/enemy/obj_BCC96.h"
#include "../src/enemy/obj_BD2C2.h"
#include "../src/enemy/obj_BDDB8.h"
#include "../src/enemy/obj_11C1FC.h"
#include "../src/enemy/obj_ECAC8.h"
#include "../src/enemy/obj_CB9EE.h"
#include "../src/enemy/obj_11C3A8.h"
#include "../src/enemy/obj_BD700.h"
#include "../src/enemy/obj_CB97E.h"
#include "../src/enemy/obj_E1B36.h"
#include "../src/enemy/obj_C9DEC.h"
#include "../src/enemy/obj_CAFB8.h"
#include "../src/enemy/obj_12CFD4.h"
#include "../src/enemy/obj_12F29E.h"
#include "../src/enemy/obj_BD0EC.h"
#include "../src/enemy/obj_12EF40.h"
#include "../src/enemy/obj_BC16C.h"
#include "../src/enemy/obj_BC1E8.h"
#include "../src/enemy/obj_11A8A8.h"
#include "../src/enemy/obj_BDDE2.h"
#include "../src/enemy/obj_BDC5C.h"
#include "../src/enemy/obj_BC08C.h"
#include "../src/enemy/obj_BD3A4.h"
#include "../src/enemy/obj_BD550.h"
#include "../src/enemy/obj_BD7E4.h"
#include "../src/enemy/obj_BE16C.h"
#include "../src/enemy/obj_13EFA0.h"
#include "../src/enemy/obj_104F26.h"
#include "../src/enemy/obj_BD9BE.h"
#include "../src/enemy/obj_BC70A.h"
#include "../src/enemy/obj_D9FC8.h"
#include "../src/enemy/obj_12EE24.h"
#include "../src/enemy/obj_DA138.h"
#include "../src/enemy/obj_12EB90.h"
#include "../src/enemy/obj_D7306.h"
#include "../src/enemy/obj_BDE92.h"
#include "../src/enemy/obj_E9DE0.h"
#include "../src/enemy/obj_E96C8.h"
#include "../src/enemy/obj_E1886.h"
#include "../src/enemy/obj_EA848.h"
#include "../src/enemy/obj_DB074.h"
#include "../src/enemy/obj_10967C.h"
#include "../src/enemy/obj_F35B4.h"
#include "../src/enemy/obj_DC6D0.h"
#include "../src/enemy/obj_E1328.h"
#include "../src/enemy/obj_EA45E.h"
#include "../src/enemy/obj_1049C4.h"
#include "../src/enemy/obj_ECB3A.h"
#include "../src/enemy/obj_D740E.h"
#include "../src/enemy/obj_DC5B6.h"
#include "../src/enemy/obj_D747E.h"
#include "../src/enemy/obj_CD646.h"
#include "../src/enemy/obj_CD504.h"
#include "../src/enemy/obj_CD5EA.h"
#include "../src/enemy/obj_CD556.h"
#include "../src/enemy/obj_1180F8.h"
#include "../src/enemy/obj_E077A.h"
#include "../src/enemy/obj_E1036.h"
#include "../src/enemy/obj_112D1A.h"
#include "../src/enemy/obj_E101A.h"
#include "../src/enemy/obj_1134B2.h"
#include "../src/enemy/obj_113ED0.h"
#include "../src/enemy/obj_112C1A.h"
#include "../src/enemy/obj_1140BE.h"
#include "../src/enemy/obj_112C3C.h"
#include "../src/enemy/obj_118CF6.h"
#include "../src/enemy/obj_112D88.h"
#include "../src/enemy/obj_114108.h"
#include "../src/enemy/obj_1134BA.h"
#include "../src/enemy/obj_118DDC.h"
#include "../src/enemy/obj_118D7C.h"
#include "../src/enemy/obj_F0788.h"
#include "../src/enemy/obj_F0846.h"
#include "../src/enemy/obj_108936.h"
#include "../src/enemy/obj_1087E8.h"
#include "../src/enemy/obj_DC486.h"
#include "../src/enemy/obj_E598C.h"
#include "../src/enemy/obj_F0814.h"
#include "../src/enemy/obj_1088F0.h"
#include "../src/enemy/obj_E6166.h"
#include "../src/enemy/obj_E61AA.h"
#include "../src/enemy/obj_E5CBC.h"
#include "../src/enemy/obj_DC358.h"
#include "../src/enemy/obj_E60EA.h"
#include "../src/enemy/obj_E5C2A.h"
#include "../src/enemy/obj_F5AFE.h"
#include "../src/enemy/obj_E6082.h"
#include "../src/enemy/obj_F5D58.h"
#include "../src/enemy/obj_F5CF6.h"
#include "../src/enemy/obj_E70CE.h"
#include "../src/enemy/obj_E7ABE.h"
#include "../src/enemy/obj_E7A7A.h"
#include "../src/enemy/obj_DA64C.h"
#include "../src/enemy/obj_E8494.h"
#include "../src/enemy/obj_DA6C0.h"
#include "../src/enemy/obj_E8240.h"
#include "../src/enemy/obj_E81A0.h"
#include "../src/enemy/obj_1094A6.h"
#include "../src/enemy/obj_109514.h"
#include "../src/enemy/obj_108E8E.h"
#include "../src/enemy/obj_10AAC2.h"
#include "../src/enemy/obj_10AB30.h"
#include "../src/enemy/obj_10AC10.h"
#include "../src/enemy/obj_10AFBA.h"
#include "../src/enemy/obj_10AFCC.h"
#include "../src/enemy/obj_11464E.h"
#include "../src/enemy/obj_114A76.h"
#include "../src/enemy/obj_114A10.h"
#include "../src/enemy/obj_114926.h"
#include "../src/enemy/obj_114800.h"
#include "../src/enemy/obj_114984.h"
#include "../src/enemy/obj_1134E6.h"
#include "../src/enemy/obj_1131A0.h"
#include "../src/enemy/obj_13EE08.h"
#include "../src/enemy/obj_13EE50.h"
#include "../src/enemy/obj_114814.h"
#include "../src/enemy/obj_113644.h"
#include "../src/enemy/obj_F0D7E.h"
#include "../src/enemy/obj_F0D8A.h"
#include "../src/enemy/obj_F0DB8.h"
#include "../src/enemy/obj_F131C.h"
#include "../src/enemy/obj_E64DA.h"
#include "../src/enemy/obj_E6508.h"
#include "../src/enemy/obj_E7036.h"
#include "../src/enemy/obj_E7024.h"
#include "../src/enemy/obj_E68C2.h"
#include "../src/enemy/obj_E6906.h"
#include "../src/enemy/obj_E6C36.h"
#include "../src/enemy/obj_E6C64.h"
#include "../src/enemy/obj_E6E0E.h"
#include "../src/enemy/obj_E6E54.h"
#include "../src/enemy/obj_E7E1C.h"
#include "../src/enemy/obj_E7DEE.h"
#include "../src/enemy/obj_108D8E.h"
#include "../src/enemy/obj_108DFC.h"
#include "../src/enemy/obj_F4D52.h"
#include "../src/enemy/obj_F4DDE.h"
#include "../src/enemy/obj_F4E1E.h"
#include "../src/enemy/obj_D9D9E.h"
#include "../src/enemy/obj_D9E06.h"
#include "../src/enemy/obj_DBF16.h"
#include "../src/enemy/obj_DBF70.h"
#include "../src/enemy/obj_CC11C.h"
#include "../src/enemy/obj_EB2B0.h"
#include "../src/enemy/obj_EB2EA.h"
#include "../src/enemy/obj_EEB76.h"
#include "../src/enemy/obj_EB2BC.h"
#include "../src/enemy/obj_EEBB0.h"
#include "../src/enemy/obj_EBA0E.h"
#include "../src/enemy/obj_114092.h"
#include "../src/enemy/obj_EEB82.h"
#include "../src/enemy/obj_17A8E0.h"
#include "../src/enemy/obj_18764E.h"
#include "../src/enemy/obj_17AAFE.h"
#include "../src/enemy/obj_187938.h"
#include "../src/enemy/obj_18765E.h"
#include "../src/enemy/obj_17AAEE.h"
#include "../src/enemy/obj_17B05E.h"
#include "../src/enemy/obj_17A8D0.h"
#include "../src/enemy/obj_17B056.h"
#include "../src/enemy/obj_17C05E.h"
#include "../src/enemy/obj_17E05A.h"
#include "../src/enemy/obj_184184.h"
#include "../src/enemy/obj_17E108.h"
#include "../src/enemy/obj_17E000.h"
#include "../src/enemy/obj_17E6B6.h"
#include "../src/enemy/obj_D76F4.h"
#include "../src/enemy/obj_D77C4.h"
#include "../src/enemy/obj_CD9F6.h"
#include "../src/enemy/obj_CD956.h"
#include "../src/enemy/obj_CDAC8.h"
#include "../src/enemy/obj_D78C0.h"
#include "../src/enemy/obj_CDC1E.h"
#include "../src/enemy/obj_126B10.h"
#include "../src/enemy/obj_126B3E.h"
#include "../src/enemy/obj_CEAA4.h"
#include "../src/enemy/obj_CEB94.h"
#include "../src/enemy/obj_CEB3E.h"
#include "../src/enemy/obj_CE9FC.h"
#include "../src/enemy/obj_E4B02.h"
#include "../src/enemy/obj_DE214.h"
#include "../src/enemy/obj_DE8B0.h"
#include "../src/enemy/obj_DE794.h"
#include "../src/enemy/obj_DE7E0.h"
#include "../src/enemy/obj_DDAE0.h"
#include "../src/enemy/obj_DE86A.h"
#include "../src/enemy/obj_F28EA.h"
#include "../src/enemy/obj_DE0B6.h"
#include "../src/enemy/obj_F299C.h"
#include "../src/enemy/obj_D7A86.h"
#include "../src/enemy/obj_DC77A.h"
#include "../src/enemy/obj_E1BAE.h"
#include "../src/enemy/obj_CEF44.h"
#include "../src/enemy/obj_DE18C.h"
#include "../src/enemy/obj_E2288.h"
#include "../src/enemy/obj_CEF4C.h"
#include "../src/enemy/obj_DE140.h"
#include "../src/enemy/obj_CEF54.h"
#include "../src/enemy/obj_E2376.h"
#include "../src/enemy/obj_DE064.h"
#include "../src/enemy/obj_EBA96.h"
#include "../src/enemy/obj_EBB4A.h"
#include "../src/enemy/obj_E2170.h"
#include "../src/enemy/obj_F2964.h"
#include "../src/enemy/obj_1138BC.h"
#include "../src/enemy/obj_E226C.h"
#include "../src/enemy/obj_E2318.h"
#include "../src/enemy/obj_113BC2.h"
#include "../src/enemy/obj_FF66C.h"
#include "../src/enemy/obj_FF990.h"
#include "../src/enemy/obj_E2124.h"
#include "../src/enemy/obj_EBB12.h"
#include "../src/enemy/obj_119062.h"
#include "../src/enemy/obj_119090.h"
#include "../src/enemy/obj_DA8B4.h"
#include "../src/enemy/obj_119D06.h"
#include "../src/enemy/obj_113B74.h"
#include "../src/enemy/obj_119176.h"
#include "../src/enemy/obj_EC65E.h"
#include "../src/enemy/obj_1191A6.h"
#include "../src/enemy/obj_1191FA.h"
#include "../src/enemy/obj_EC734.h"
#include "../src/enemy/obj_119270.h"
#include "../src/enemy/obj_11922A.h"
#include "../src/enemy/obj_1192A0.h"
#include "../src/enemy/obj_113A4E.h"
#include "../src/enemy/obj_DA82E.h"
#include "../src/enemy/obj_1192AE.h"
#include "../src/enemy/obj_1199A8.h"
#include "../src/enemy/obj_119D36.h"
#include "../src/enemy/obj_1199CC.h"
#include "../src/enemy/obj_EC6FC.h"
#include "../src/enemy/obj_130054.h"
#include "../src/enemy/obj_131414.h"
#include "../src/enemy/obj_CC05A.h"
#include "../src/enemy/obj_12CB72.h"
#include "../src/enemy/obj_12CC44.h"
#include "../src/enemy/obj_119BBE.h"
#include "../src/enemy/obj_130044.h"
#include "../src/enemy/obj_119B40.h"
#include "../src/enemy/obj_12D0E6.h"
#include "../src/enemy/obj_131408.h"
#include "../src/enemy/obj_12D10C.h"
#include "../src/enemy/obj_119ACA.h"
#include "../src/enemy/obj_12D140.h"
#include "../src/enemy/obj_12D17E.h"
#include "../src/enemy/obj_118EDE.h"
#include "../src/enemy/obj_12D1E0.h"
#include "../src/enemy/obj_118E88.h"
#include "../src/enemy/obj_1302F8.h"
#include "../src/enemy/obj_130312.h"
#include "../src/enemy/obj_12D1C6.h"
#include "../src/enemy/obj_12D2B4.h"
#include "../src/enemy/obj_CCE6E.h"
#include "../src/enemy/obj_CD256.h"
#include "../src/enemy/obj_12D2D4.h"
#include "../src/enemy/obj_12D244.h"
#include "../src/enemy/obj_CD412.h"
#include "../src/enemy/obj_12D22C.h"
#include "../src/enemy/obj_12D284.h"
#include "../src/enemy/obj_12D36C.h"
#include "../src/enemy/obj_1303AC.h"
#include "../src/enemy/obj_1307E0.h"
#include "../src/enemy/obj_130794.h"
#include "../src/enemy/obj_12D2FC.h"
#include "../src/enemy/obj_12D31A.h"
#include "../src/enemy/obj_12D350.h"
#include "../src/enemy/obj_12D382.h"
#include "../src/enemy/obj_1304E2.h"
#include "../src/enemy/obj_130360.h"
#include "../src/enemy/obj_130482.h"
#include "../src/enemy/obj_12D3FA.h"
#include "../src/enemy/obj_130546.h"
#include "../src/enemy/obj_1302B8.h"
#include "../src/enemy/obj_12D414.h"
#include "../src/enemy/obj_12D446.h"
#include "../src/enemy/obj_12D39C.h"
#include "../src/enemy/obj_13157A.h"
#include "../src/enemy/obj_13029E.h"
#include "../src/enemy/obj_13151E.h"
#include "../src/enemy/obj_1314C2.h"
#include "../src/enemy/obj_12D466.h"
#include "../src/enemy/obj_12D4BA.h"
#include "../src/enemy/obj_12D49E.h"
#include "../src/enemy/obj_12D482.h"
#include "../src/enemy/obj_CCD04.h"
#include "../src/enemy/obj_12D0C4.h"
#include "../src/enemy/obj_118358.h"
#include "../src/enemy/obj_131616.h"
#include "../src/enemy/obj_13DEF0.h"
#include "../src/enemy/obj_12D0AE.h"
#include "../src/enemy/obj_1186C4.h"
#include "../src/enemy/obj_13DF4C.h"
#include "../src/enemy/obj_118602.h"
#include "../src/enemy/obj_118486.h"
#include "../src/enemy/obj_13DE92.h"
#include "../src/enemy/obj_118494.h"
#include "../src/enemy/obj_119360.h"
#include "../src/enemy/obj_1187DE.h"
#include "../src/enemy/obj_1193E2.h"
#include "../src/enemy/obj_13DEAE.h"
#include "../src/enemy/obj_119422.h"
#include "../src/enemy/obj_11946C.h"
#include "../src/enemy/obj_1194C2.h"
#include "../src/enemy/obj_11949A.h"
#include "../src/enemy/obj_13E12C.h"
#include "../src/enemy/obj_ED156.h"
#include "../src/enemy/obj_13DF5A.h"
#include "../src/enemy/obj_13E34C.h"
#include "../src/enemy/obj_13E150.h"
#include "../src/enemy/obj_186388.h"
#include "../src/enemy/obj_1867A4.h"
#include "../src/enemy/obj_ED124.h"
#include "../src/enemy/obj_11418E.h"
#include "../src/enemy/obj_184A20.h"
#include "../src/enemy/obj_184A10.h"
#include "../src/enemy/obj_11423E.h"
#include "../src/enemy/obj_114212.h"
#include "../src/enemy/obj_1143C8.h"
#include "../src/enemy/obj_186326.h"
#include "../src/enemy/obj_186782.h"
#include "../src/enemy/obj_184D6C.h"
#include "../src/enemy/obj_186482.h"
#include "../src/enemy/obj_184BAE.h"
#include "../src/enemy/obj_186406.h"
#include "../src/enemy/obj_17A688.h"
#include "../src/enemy/obj_18724E.h"
#include "../src/enemy/obj_18702C.h"
#include "../src/enemy/obj_1865A0.h"
#include "../src/enemy/obj_187370.h"
#include "../src/enemy/obj_1873D6.h"
#include "../src/enemy/obj_17DAA0.h"
#include "../src/enemy/obj_18747C.h"
#include "../src/enemy/obj_187508.h"
#include "../src/enemy/obj_118D12.h"
#include "../src/enemy/obj_17A070.h"
#include "../src/enemy/obj_17A664.h"
#include "../src/enemy/obj_17C14C.h"
#include "../src/enemy/obj_17C8CC.h"
#include "../src/enemy/obj_17C660.h"
#include "../src/enemy/obj_179C6E.h"
#include "../src/enemy/obj_18657E.h"
#include "../src/enemy/obj_17A3F4.h"
#include "../src/enemy/obj_17A420.h"
#include "../src/enemy/obj_187246.h"
#include "../src/enemy/obj_17C9D0.h"
#include "../src/enemy/obj_187024.h"
#include "../src/enemy/obj_17BE1C.h"
#include "../src/enemy/obj_17A016.h"
#include "../src/enemy/obj_17D136.h"
#include "../src/enemy/obj_17D912.h"
#include "../src/enemy/obj_17D186.h"
#include "../src/enemy/obj_17D586.h"
#include "../src/enemy/obj_17A122.h"
#include "../src/enemy/obj_18415A.h"
#include "../src/enemy/obj_17A52A.h"
#include "../src/enemy/obj_17D4F4.h"
#include "../src/enemy/obj_17DDD0.h"
#include "../src/enemy/obj_17D640.h"
#include "../src/enemy/obj_179E7A.h"
#include "../src/enemy/obj_17DE20.h"
#include "../src/enemy/obj_17A50E.h"
#include "../src/enemy/obj_185C18.h"
#include "../src/enemy/obj_1860AA.h"
#include "../src/enemy/obj_17D66E.h"
#include "../src/enemy/obj_17DB68.h"
#include "../src/enemy/obj_17D72C.h"
#include "../src/enemy/obj_17D6C8.h"
#include "../src/enemy/obj_17DC1E.h"
#include "../src/enemy/obj_17CDA6.h"
#include "../src/enemy/obj_179DEE.h"
#include "../src/enemy/obj_17CE04.h"
#include "../src/enemy/obj_184EDE.h"
#include "../src/enemy/obj_17DBB8.h"
#include "../src/enemy/obj_186064.h"
#include "../src/enemy/obj_179C26.h"
#include "../src/enemy/obj_17CE88.h"
#include "../src/enemy/obj_17A42A.h"
#include "../src/enemy/obj_18675E.h"
#include "../src/enemy/obj_17F038.h"
#include "../src/enemy/obj_17D3E6.h"
#include "../src/enemy/obj_17C240.h"
#include "../src/enemy/obj_17C508.h"
#include "../src/enemy/obj_17C360.h"
#include "../src/enemy/obj_11BCB2.h"
#include "../src/enemy/obj_11BDAA.h"
#include "../src/enemy/obj_11BD48.h"
#include "../src/enemy/obj_17F006.h"
#include "../src/enemy/obj_17C4D4.h"
#include "../src/enemy/obj_11BE14.h"
#include "../src/enemy/obj_17D394.h"
#include "../src/enemy/obj_17A5C4.h"
#include "../src/enemy/obj_13EA38.h"
#include "../src/enemy/obj_13E9E8.h"
#include "../src/enemy/obj_17DD46.h"
#include "../src/enemy/obj_11BD96.h"
#include "../src/enemy/obj_17A5A8.h"
#include "../src/enemy/obj_13EB4C.h"
#include "../src/enemy/obj_11BEF6.h"
#include "../src/enemy/obj_13EA54.h"
#include "../src/enemy/obj_179DCC.h"
#include "../src/enemy/obj_17AE0A.h"
#include "../src/enemy/obj_184DA8.h"
#include "../src/enemy/obj_E66F8.h"
#include "../src/enemy/obj_E739E.h"
#include "../src/enemy/obj_E7FC6.h"
#include "../src/enemy/obj_E800C.h"
#include "../src/enemy/obj_E66B2.h"
#include "../src/enemy/obj_F13F4.h"
#include "../src/enemy/obj_E735A.h"
#include "../src/enemy/obj_F1400.h"
#include "../src/enemy/obj_F1432.h"
#include "../src/enemy/obj_F1814.h"
#include "../src/enemy/obj_F40DE.h"
#include "../src/enemy/obj_F40EA.h"
#include "../src/enemy/obj_F4112.h"
#include "../src/enemy/obj_EDBFE.h"
#include "../src/enemy/obj_EA8EE.h"
#include "../src/enemy/obj_EDC38.h"
#include "../src/enemy/obj_EDC0A.h"
#include "../src/enemy/obj_EE0A4.h"
#include "../src/enemy/obj_EE076.h"
#include "../src/enemy/obj_E8DFE.h"
#include "../src/enemy/obj_E9426.h"
#include "../src/enemy/obj_E940A.h"
#include "../src/enemy/obj_E949E.h"
#include "../src/enemy/obj_E94BC.h"
#include "../src/enemy/obj_D5EE8.h"
#include "../src/enemy/obj_ED76E.h"
#include "../src/enemy/obj_ED7AC.h"
#include "../src/enemy/obj_ED77A.h"
#include "../src/enemy/obj_DD314.h"
#include "../src/enemy/obj_E9556.h"
#include "../src/enemy/obj_E953A.h"
#include "../src/enemy/obj_D6BD8.h"
#include "../src/enemy/obj_D5FFE.h"
#include "../src/enemy/obj_11AD46.h"
#include "../src/enemy/obj_D6CEC.h"
#include "../src/enemy/obj_D6DCC.h"
#include "../src/enemy/obj_EF8F8.h"
#include "../src/enemy/obj_D6E98.h"
#include "../src/enemy/obj_EF938.h"
#include "../src/enemy/obj_184170.h"
#include "../src/enemy/obj_DD6FC.h"
#include "../src/enemy/obj_EF904.h"
#include "../src/enemy/obj_17AD56.h"
#include "../src/enemy/obj_17AD18.h"
#include "../src/enemy/obj_179D26.h"
#include "../src/enemy/obj_17E1DA.h"
#include "../src/enemy/obj_DC43E.h"
#include "../src/enemy/obj_17E188.h"
#include "../src/enemy/obj_C99B8.h"
#include "../src/enemy/obj_D70A0.h"
#include "../src/enemy/obj_F7BD2.h"
#include "../src/enemy/obj_F7CDC.h"
#include "../src/enemy/obj_11AA5E.h"
#include "../src/enemy/obj_F7C48.h"
#include "../src/enemy/obj_14C9FE.h"
#include "../src/enemy/obj_14CAFA.h"
#include "../src/enemy/obj_17ABC8.h"
#include "../src/enemy/obj_14CC4C.h"
#include "../src/enemy/obj_101A6C.h"
#include "../src/enemy/obj_187986.h"
#include "../src/enemy/obj_14C8F6.h"
#include "../src/enemy/obj_101BB6.h"
#include "../src/enemy/obj_14CC12.h"
#include "../src/enemy/obj_14CC8C.h"
#include "../src/enemy/obj_109AEC.h"
#include "../src/enemy/obj_14CD0C.h"
#include "../src/enemy/obj_14CCCC.h"
#include "../src/enemy/obj_101B20.h"
#include "../src/enemy/obj_109BB6.h"
#include "../src/enemy/obj_14CD4C.h"
#include "../src/enemy/obj_14CD8C.h"
#include "../src/enemy/obj_14CB12.h"
#include "../src/enemy/obj_109B7E.h"
#include "../src/enemy/obj_11AB3E.h"
#include "../src/enemy/obj_14CDCC.h"
#include "../src/enemy/obj_FE0AA.h"
#include "../src/enemy/obj_FE724.h"
#include "../src/enemy/obj_11AAB8.h"
#include "../src/enemy/obj_14CB96.h"
#include "../src/enemy/obj_FE5BA.h"
#include "../src/enemy/obj_11AAF0.h"
#include "../src/enemy/obj_FF3E4.h"
#include "../src/enemy/obj_11AAD4.h"
#include "../src/enemy/obj_FE53C.h"
#include "../src/enemy/obj_11AB96.h"
#include "../src/enemy/obj_FF4E0.h"
#include "../src/enemy/obj_EFE92.h"
#include "../src/enemy/obj_FF4A8.h"
#include "../src/enemy/obj_EFF34.h"
#include "../src/enemy/obj_DBEE6.h"
#include "../src/enemy/obj_FEC64.h"
#include "../src/enemy/obj_FEEA0.h"
#include "../src/enemy/obj_DBE98.h"
#include "../src/enemy/obj_D9CE8.h"
#include "../src/enemy/obj_F44B4.h"
#include "../src/enemy/obj_114E0E.h"
#include "../src/enemy/obj_EFEDE.h"
#include "../src/enemy/obj_FEE74.h"
#include "../src/enemy/obj_1899B0.h"
#include "../src/enemy/obj_D9C88.h"
#include "../src/enemy/obj_18A4EC.h"
#include "../src/enemy/obj_1897BA.h"
#include "../src/enemy/obj_1897AA.h"
#include "../src/enemy/obj_18A4D4.h"
#include "../src/enemy/obj_18B41C.h"
#include "../src/enemy/obj_189BE2.h"
#include "../src/enemy/obj_114F38.h"
#include "../src/enemy/obj_18B2CA.h"
#include "../src/enemy/obj_18B3CC.h"
#include "../src/enemy/obj_18B4E8.h"
#include "../src/enemy/obj_18B4B8.h"
#include "../src/enemy/obj_189C1E.h"
#include "../src/enemy/obj_18B230.h"
#include "../src/enemy/obj_18A582.h"
#include "../src/enemy/obj_189C56.h"
#include "../src/enemy/obj_18A572.h"
#include "../src/enemy/obj_189C7C.h"
#include "../src/enemy/obj_18B3AE.h"
#include "../src/enemy/obj_18A550.h"
#include "../src/enemy/obj_179354.h"
#include "../src/enemy/obj_189CCC.h"
#include "../src/enemy/obj_17933A.h"
#include "../src/enemy/obj_18A540.h"
#include "../src/enemy/obj_189D8A.h"
#include "../src/enemy/obj_189CE8.h"
#include "../src/enemy/obj_189D2A.h"
#include "../src/enemy/obj_189D20.h"
#include "../src/enemy/obj_189E2E.h"
#include "../src/enemy/obj_189E6A.h"
#include "../src/enemy/obj_17F48E.h"
#include "../src/enemy/obj_189FE2.h"
#include "../src/enemy/obj_189EDC.h"
#include "../src/enemy/obj_189EBA.h"
#include "../src/enemy/obj_17F43C.h"
#include "../src/enemy/obj_18A01E.h"
#include "../src/enemy/obj_17EDA4.h"
#include "../src/enemy/obj_17ED40.h"
#include "../src/enemy/obj_17F260.h"
#include "../src/enemy/obj_17EFC8.h"
#include "../src/enemy/obj_18A12C.h"
#include "../src/enemy/obj_17E99A.h"
#include "../src/enemy/obj_17E9F6.h"
#include "../src/enemy/obj_17EA7E.h"
#include "../src/enemy/obj_17EA0A.h"
#include "../src/enemy/obj_18A06E.h"
#include "../src/enemy/obj_18A08A.h"
#include "../src/enemy/obj_17E84E.h"
#include "../src/enemy/obj_18B9DE.h"
#include "../src/enemy/obj_17E89E.h"
#include "../src/enemy/obj_18A0C2.h"
#include "../src/enemy/obj_17AADA.h"
#include "../src/enemy/obj_18A0CC.h"
#include "../src/enemy/obj_14D86E.h"
#include "../src/enemy/obj_18A20C.h"
#include "../src/enemy/obj_18A1D0.h"
#include "../src/enemy/obj_18A25C.h"
#include "../src/enemy/obj_18A27E.h"
#include "../src/enemy/obj_183F18.h"
#include "../src/enemy/obj_183CF6.h"
#include "../src/enemy/obj_18A474.h"
#include "../src/enemy/obj_14D81A.h"
#include "../src/enemy/obj_18B9CE.h"
#include "../src/enemy/obj_18A4CA.h"
#include "../src/enemy/obj_17F812.h"
#include "../src/enemy/obj_183CBA.h"
#include "../src/enemy/obj_17F7CE.h"
#include "../src/enemy/obj_17F5DA.h"
#include "../src/enemy/obj_17F6C8.h"
#include "../src/enemy/obj_183DA6.h"
#include "../src/enemy/obj_17F8FC.h"
#include "../src/enemy/obj_183EDA.h"
#include "../src/enemy/obj_17F74E.h"
#include "../src/enemy/obj_183FB0.h"
#include "../src/enemy/obj_17AA36.h"
#include "../src/enemy/obj_F6700.h"
#include "../src/enemy/obj_179ED0.h"
#include "../src/enemy/obj_17F8B0.h"
#include "../src/enemy/obj_17F594.h"
#include "../src/enemy/obj_F752E.h"
#include "../src/enemy/obj_F7722.h"
#include "../src/enemy/obj_F778A.h"
#include "../src/enemy/obj_179F6C.h"
#include "../src/enemy/obj_F75DE.h"
#include "../src/enemy/obj_F754C.h"
#include "../src/enemy/obj_F7618.h"
#include "../src/enemy/obj_10B93C.h"
#include "../src/enemy/obj_F773E.h"
#include "../src/enemy/obj_F7800.h"
#include "../src/enemy/obj_10B83E.h"
#include "../src/enemy/obj_10BD20.h"
#include "../src/enemy/obj_FB526.h"
#include "../src/enemy/obj_10BD88.h"
#include "../src/enemy/obj_CDD12.h"
#include "../src/enemy/obj_CDCA0.h"
#include "../src/enemy/obj_D9056.h"
#include "../src/enemy/obj_D92B4.h"
#include "../src/enemy/obj_C99F8.h"
#include "../src/enemy/obj_1118BC.h"
#include "../src/enemy/obj_FB584.h"
#include "../src/enemy/obj_111A7A.h"
#include "../src/enemy/obj_FCC4C.h"
#include "../src/enemy/obj_1899EC.h"
#include "../src/enemy/obj_111B56.h"
#include "../src/enemy/obj_111DF0.h"
#include "../src/enemy/obj_D93D4.h"
#include "../src/enemy/obj_D936A.h"
#include "../src/enemy/obj_1153B8.h"
#include "../src/enemy/obj_1150E0.h"
#include "../src/enemy/obj_111D18.h"
#include "../src/enemy/obj_115092.h"
#include "../src/enemy/obj_111D9C.h"
#include "../src/enemy/obj_111D8E.h"
#include "../src/enemy/obj_115334.h"
#include "../src/enemy/obj_11509A.h"
#include "../src/enemy/obj_118D4A.h"
#include "../src/enemy/obj_D7EAE.h"
#include "../src/enemy/obj_C9A38.h"
#include "../src/enemy/obj_115326.h"
#include "../src/enemy/obj_D8030.h"
#include "../src/enemy/obj_D7DD8.h"
#include "../src/enemy/obj_D808C.h"
#include "../src/enemy/obj_D8574.h"
#include "../src/enemy/obj_D7F52.h"
#include "../src/enemy/obj_D8164.h"
#include "../src/enemy/obj_D7F0E.h"
#include "../src/enemy/obj_D7F8E.h"
#include "../src/enemy/obj_D853C.h"
#include "../src/enemy/obj_D823E.h"
#include "../src/enemy/obj_15929C.h"
#include "../src/enemy/obj_111DA6.h"
#include "../src/enemy/obj_1242EC.h"
#include "../src/enemy/obj_1592EC.h"
#include "../src/enemy/obj_11AEBE.h"
#include "../src/enemy/obj_123FCA.h"
#include "../src/enemy/obj_11B14C.h"
#include "../src/enemy/obj_11AFA6.h"
#include "../src/enemy/obj_124404.h"
#include "../src/enemy/obj_124D64.h"
#include "../src/enemy/obj_125016.h"
#include "../src/enemy/obj_11AECE.h"
#include "../src/enemy/obj_124E00.h"
#include "../src/enemy/obj_1250BC.h"
#include "../src/enemy/obj_11B17C.h"
#include "../src/enemy/obj_1253B0.h"
#include "../src/enemy/obj_12442A.h"
#include "../src/enemy/obj_12552E.h"
#include "../src/enemy/obj_124D78.h"
#include "../src/enemy/obj_125D22.h"
#include "../src/enemy/obj_11AFE4.h"
#include "../src/enemy/obj_11B3D2.h"
#include "../src/enemy/obj_1250E4.h"
#include "../src/enemy/obj_125046.h"
#include "../src/enemy/obj_124E30.h"
#include "../src/enemy/obj_1253D0.h"
#include "../src/enemy/obj_12554E.h"
#include "../src/enemy/obj_125D44.h"
#include "../src/enemy/obj_11B022.h"
#include "../src/enemy/obj_11B472.h"
#include "../src/enemy/obj_124438.h"
#include "../src/enemy/obj_125DAA.h"
#include "../src/enemy/obj_124466.h"
#include "../src/enemy/obj_1244A2.h"
#include "../src/enemy/obj_1244DA.h"
#include "../src/enemy/obj_125B88.h"
#include "../src/enemy/obj_CD7B8.h"
#include "../src/enemy/obj_CD74E.h"
#include "../src/enemy/obj_125B0C.h"
#include "../src/enemy/obj_CCD5C.h"
#include "../src/enemy/obj_124524.h"
#include "../src/enemy/obj_124550.h"
#include "../src/enemy/obj_124532.h"
#include "../src/enemy/obj_125CA8.h"
#include "../src/enemy/obj_11B668.h"
#include "../src/enemy/obj_11B12C.h"
#include "../src/enemy/obj_11B35E.h"
#include "../src/enemy/obj_11B5E6.h"
#include "../src/enemy/obj_124358.h"
#include "../src/enemy/obj_12431E.h"
#include "../src/enemy/obj_11B656.h"
#include "../src/enemy/obj_11B23A.h"
#include "../src/enemy/obj_159C8C.h"
#include "../src/enemy/obj_11B1BE.h"
#include "../src/enemy/obj_11C054.h"
#include "../src/enemy/obj_125CFA.h"
#include "../src/enemy/obj_C7DBA.h"
#include "../src/enemy/obj_11BFAC.h"
#include "../src/enemy/obj_C8218.h"
#include "../src/enemy/obj_15A264.h"
#include "../src/enemy/obj_C7DCA.h"
#include "../src/enemy/obj_15A1CA.h"
#include "../src/enemy/obj_C823E.h"
#include "../src/enemy/obj_C7E5E.h"
#include "../src/enemy/obj_11BF5C.h"
#include "../src/enemy/obj_C7F3E.h"
#include "../src/enemy/obj_C80DC.h"
#include "../src/enemy/obj_11B1FA.h"
#include "../src/enemy/obj_C7E10.h"
#include "../src/enemy/obj_C8168.h"
#include "../src/enemy/obj_C7DDE.h"
#include "../src/enemy/obj_C7F14.h"
#include "../src/enemy/obj_15A088.h"
#include "../src/enemy/obj_159D68.h"
#include "../src/enemy/obj_159D22.h"
#include "../src/enemy/obj_15A154.h"
#include "../src/enemy/obj_FFCF4.h"
#include "../src/enemy/obj_111432.h"
#include "../src/enemy/obj_111424.h"
#include "../src/enemy/obj_111492.h"
#include "../src/enemy/obj_1114F6.h"
#include "../src/enemy/obj_10010C.h"
#include "../src/enemy/obj_1002CC.h"
#include "../src/enemy/obj_111450.h"
#include "../src/enemy/obj_15A0C8.h"
#include "../src/enemy/obj_C7EE6.h"
#include "../src/enemy/obj_C7EF4.h"
#include "../src/enemy/obj_1114B4.h"
#include "../src/enemy/obj_11151A.h"
#include "../src/enemy/obj_1000DE.h"
#include "../src/enemy/obj_1002B0.h"
#include "../src/enemy/obj_1008A6.h"
#include "../src/enemy/obj_1008E2.h"
#include "../src/enemy/obj_115F96.h"
#include "../src/enemy/obj_1117CC.h"
#include "../src/enemy/obj_115F36.h"
#include "../src/enemy/obj_115B3E.h"
#include "../src/enemy/obj_116078.h"
#include "../src/enemy/obj_1161C4.h"
#include "../src/enemy/obj_116118.h"
#include "../src/enemy/obj_1160C4.h"
#include "../src/enemy/obj_116030.h"
#include "../src/enemy/obj_D6242.h"
#include "../src/enemy/obj_D6192.h"
#include "../src/enemy/obj_E8516.h"
#include "../src/enemy/obj_118F84.h"
#include "../src/enemy/obj_E8B72.h"
#include "../src/enemy/obj_E8C36.h"
#include "../src/enemy/obj_118F4E.h"
#include "../src/enemy/obj_109FAA.h"
#include "../src/enemy/obj_E8BEA.h"
#include "../src/enemy/obj_D9224.h"
#include "../src/enemy/obj_E8B26.h"
#include "../src/enemy/obj_10A054.h"
#include "../src/enemy/obj_D8DC4.h"
#include "../src/enemy/obj_10A024.h"
#include "../src/enemy/obj_D8D22.h"
#include "../src/enemy/obj_E8D12.h"
#include "../src/enemy/obj_D8D30.h"
#include "../src/enemy/obj_D572C.h"
#include "../src/enemy/obj_E8CAE.h"
#include "../src/enemy/obj_ED214.h"
#include "../src/enemy/obj_DC56E.h"
#include "../src/enemy/obj_D56C6.h"
#include "../src/enemy/obj_115D14.h"
#include "../src/enemy/obj_ED2A0.h"
#include "../src/enemy/obj_D64BA.h"
#include "../src/enemy/obj_ED2D8.h"
#include "../src/enemy/obj_D6636.h"
#include "../src/enemy/obj_D653C.h"
#include "../src/enemy/obj_10A42C.h"
#include "../src/enemy/obj_D6846.h"
#include "../src/enemy/obj_10A484.h"
#include "../src/enemy/obj_D67F6.h"
#include "../src/enemy/obj_D6ACA.h"
#include "../src/enemy/obj_DB2E0.h"
#include "../src/enemy/obj_D6958.h"
#include "../src/enemy/obj_10A438.h"
#include "../src/enemy/obj_118B3C.h"
#include "../src/enemy/obj_D8E42.h"
#include "../src/enemy/obj_DB26C.h"
#include "../src/enemy/obj_118B74.h"
#include "../src/enemy/obj_F238A.h"
#include "../src/enemy/obj_D66BA.h"
#include "../src/enemy/obj_F2454.h"
#include "../src/enemy/obj_D59F0.h"
#include "../src/enemy/obj_F241C.h"
#include "../src/enemy/obj_D5CD0.h"
#include "../src/enemy/obj_D5974.h"
#include "../src/enemy/obj_D5C64.h"
#include "../src/enemy/obj_1003E2.h"
#include "../src/enemy/obj_18F1C2.h"
#include "../src/enemy/obj_10048C.h"
#include "../src/enemy/obj_100B94.h"
#include "../src/enemy/obj_18F002.h"
#include "../src/enemy/obj_190B1A.h"
#include "../src/enemy/obj_1909B2.h"
#include "../src/enemy/obj_18F012.h"
#include "../src/enemy/obj_19090C.h"
#include "../src/enemy/obj_190AA6.h"
#include "../src/enemy/obj_190FD0.h"
#include "../src/enemy/obj_191014.h"
#include "../src/enemy/obj_191374.h"
#include "../src/enemy/obj_191356.h"
#include "../src/enemy/obj_1914E6.h"
#include "../src/enemy/obj_191486.h"
#include "../src/enemy/obj_1915E0.h"
#include "../src/enemy/obj_191700.h"
#include "../src/enemy/obj_1915AE.h"
#include "../src/enemy/obj_191742.h"
#include "../src/enemy/obj_1916BE.h"
#include "../src/enemy/obj_19163C.h"
#include "../src/enemy/obj_181AEC.h"
#include "../src/enemy/obj_1802F4.h"
#include "../src/enemy/obj_1802A4.h"
#include "../src/enemy/obj_1804DA.h"
#include "../src/enemy/obj_18048A.h"
#include "../src/enemy/obj_191688.h"
#include "../src/enemy/obj_18F216.h"
#include "../src/enemy/obj_EE42E.h"
#include "../src/enemy/obj_10C966.h"
#include "../src/enemy/obj_EE468.h"
#include "../src/enemy/obj_EE43A.h"
#include "../src/enemy/obj_EE7D8.h"
#include "../src/enemy/obj_EE7AA.h"
#include "../src/enemy/obj_EF366.h"
#include "../src/enemy/obj_EF44E.h"
#include "../src/enemy/obj_DA28C.h"
#include "../src/enemy/obj_EF400.h"
#include "../src/enemy/obj_EF434.h"
#include "../src/enemy/obj_DA20E.h"
#include "../src/enemy/obj_105ED6.h"
#include "../src/enemy/obj_1070DA.h"
#include "../src/enemy/obj_10747E.h"
#include "../src/enemy/obj_107B6E.h"
#include "../src/enemy/obj_107786.h"
#include "../src/enemy/obj_107DB2.h"
#include "../src/enemy/obj_1080A4.h"
#include "../src/enemy/obj_107112.h"
#include "../src/enemy/obj_105EDE.h"
#include "../src/enemy/obj_108294.h"
#include "../src/enemy/obj_1084C0.h"
#include "../src/enemy/obj_1074B6.h"
#include "../src/enemy/obj_1077DA.h"
#include "../src/enemy/obj_107BC2.h"
#include "../src/enemy/obj_107E00.h"
#include "../src/enemy/obj_1080F2.h"
#include "../src/enemy/obj_1082E6.h"
#include "../src/enemy/obj_108512.h"
#include "../src/enemy/obj_DBC1A.h"
#include "../src/enemy/obj_C9AB8.h"
#include "../src/enemy/obj_DBCAE.h"
#include "../src/enemy/obj_10DBE0.h"
#include "../src/enemy/obj_10E1BC.h"
#include "../src/enemy/obj_10DBD8.h"
#include "../src/enemy/obj_10E178.h"
#include "../src/enemy/obj_DC03A.h"
#include "../src/enemy/obj_116B86.h"
#include "../src/enemy/obj_DC008.h"
#include "../src/enemy/obj_DC46E.h"
#include "../src/enemy/obj_116B0C.h"
#include "../src/enemy/obj_1165CC.h"
#include "../src/enemy/obj_116C48.h"
#include "../src/enemy/obj_116C9A.h"
#include "../src/enemy/obj_1166EA.h"
#include "../src/enemy/obj_116DD2.h"
#include "../src/enemy/obj_116E22.h"
#include "../src/enemy/obj_E5372.h"
#include "../src/enemy/obj_116802.h"
#include "../src/enemy/obj_E54EE.h"
#include "../src/enemy/obj_10E316.h"
#include "../src/enemy/obj_FEF76.h"
#include "../src/enemy/obj_E262A.h"
#include "../src/enemy/obj_116F82.h"
#include "../src/enemy/obj_116EE4.h"
#include "../src/enemy/obj_E54A0.h"
#include "../src/enemy/obj_E2804.h"
#include "../src/enemy/obj_117826.h"
#include "../src/enemy/obj_DACDC.h"
#include "../src/enemy/obj_1177AC.h"
#include "../src/enemy/obj_116EDC.h"
#include "../src/enemy/obj_116F7A.h"
#include "../src/enemy/obj_12601C.h"
#include "../src/enemy/obj_117980.h"
#include "../src/enemy/obj_E27C0.h"
#include "../src/enemy/obj_DAC54.h"
#include "../src/enemy/obj_11792A.h"
#include "../src/enemy/obj_126158.h"
#include "../src/enemy/obj_11699A.h"
#include "../src/enemy/obj_125DC2.h"
#include "../src/enemy/obj_126DEC.h"
#include "../src/enemy/obj_126D92.h"
#include "../src/enemy/obj_126EB4.h"
#include "../src/enemy/obj_126E84.h"
#include "../src/enemy/obj_126132.h"
#include "../src/enemy/obj_126FF6.h"
#include "../src/enemy/obj_126FA4.h"
#include "../src/enemy/obj_1270BE.h"
#include "../src/enemy/obj_12739E.h"
#include "../src/enemy/obj_12706C.h"
#include "../src/enemy/obj_1169CA.h"
#include "../src/enemy/obj_127584.h"
#include "../src/enemy/obj_12737E.h"
#include "../src/enemy/obj_1261FE.h"
#include "../src/enemy/obj_1289A6.h"
#include "../src/enemy/obj_127560.h"
#include "../src/enemy/obj_126282.h"
#include "../src/enemy/obj_128B9E.h"
#include "../src/enemy/obj_128A84.h"
#include "../src/enemy/obj_128BB2.h"
#include "../src/enemy/obj_126310.h"
#include "../src/enemy/obj_CD1AE.h"
#include "../src/enemy/obj_CCCA2.h"
#include "../src/enemy/obj_CCFD4.h"
#include "../src/enemy/obj_128924.h"
#include "../src/enemy/obj_126254.h"
#include "../src/enemy/obj_1261AA.h"
#include "../src/enemy/obj_126364.h"
#include "../src/enemy/obj_1289EA.h"
#include "../src/enemy/obj_128B44.h"
#include "../src/enemy/obj_1263BE.h"
#include "../src/enemy/obj_128476.h"
#include "../src/enemy/obj_1262B0.h"
#include "../src/enemy/obj_1284D4.h"
#include "../src/enemy/obj_126430.h"
#include "../src/enemy/obj_CCF72.h"
#include "../src/enemy/obj_12645E.h"
#include "../src/enemy/obj_128D4A.h"
#include "../src/enemy/obj_128660.h"
#include "../src/enemy/obj_126390.h"
#include "../src/enemy/obj_12886A.h"
#include "../src/enemy/obj_1264CA.h"
#include "../src/enemy/obj_12649C.h"
#include "../src/enemy/obj_128226.h"
#include "../src/enemy/obj_1264F8.h"
#include "../src/enemy/obj_126402.h"
#include "../src/enemy/obj_1286E2.h"
#include "../src/enemy/obj_1282B4.h"
#include "../src/enemy/obj_1285FC.h"
#include "../src/enemy/obj_12652C.h"
#include "../src/enemy/obj_1287A4.h"
#include "../src/enemy/obj_1287C4.h"
#include "../src/enemy/obj_126562.h"
#include "../src/enemy/obj_1281C4.h"
#include "../src/enemy/obj_1265BE.h"
#include "../src/enemy/obj_CD124.h"
#include "../src/enemy/obj_126608.h"
#include "../src/enemy/obj_126670.h"
#include "../src/enemy/obj_CC7C0.h"
#include "../src/enemy/obj_CC860.h"
#include "../src/enemy/obj_1266C2.h"
#include "../src/enemy/obj_126590.h"
#include "../src/enemy/obj_126710.h"
#include "../src/enemy/obj_126744.h"
#include "../src/enemy/obj_CDDF8.h"
#include "../src/enemy/obj_CDF68.h"
#include "../src/enemy/obj_CDE50.h"
#include "../src/enemy/obj_CDF84.h"
#include "../src/enemy/obj_CC756.h"
#include "../src/enemy/obj_126772.h"
#include "../src/enemy/obj_1267BC.h"
#include "../src/enemy/obj_1267F0.h"
#include "../src/enemy/obj_CDDB0.h"
#include "../src/enemy/obj_126844.h"
#include "../src/enemy/obj_1268D4.h"
#include "../src/enemy/obj_126902.h"
#include "../src/enemy/obj_12681E.h"
#include "../src/enemy/obj_126936.h"
#include "../src/enemy/obj_CCDB6.h"
#include "../src/enemy/obj_126B7E.h"
#include "../src/enemy/obj_126976.h"
#include "../src/enemy/obj_12687E.h"
#include "../src/enemy/obj_1269C4.h"
#include "../src/enemy/obj_126BAA.h"
#include "../src/enemy/obj_11939E.h"
#include "../src/enemy/obj_1269F2.h"
#include "../src/enemy/obj_126A2A.h"
#include "../src/enemy/obj_126086.h"
#include "../src/enemy/obj_128D6C.h"
#include "../src/enemy/obj_CDDA6.h"
#include "../src/enemy/obj_128E44.h"
#include "../src/enemy/obj_12604C.h"
#include "../src/enemy/obj_C2BBE.h"
#include "../src/enemy/obj_128E8C.h"
#include "../src/enemy/obj_C2C02.h"
#include "../src/enemy/obj_126A7A.h"
#include "../src/enemy/obj_C2C22.h"
#include "../src/enemy/obj_126AB4.h"
#include "../src/enemy/obj_126AE2.h"
#include "../src/enemy/obj_11B262.h"
#include "../src/enemy/obj_11B288.h"
#include "../src/enemy/obj_11B2CA.h"
#include "../src/enemy/obj_11B09A.h"
#include "../src/enemy/obj_11B06E.h"
#include "../src/enemy/obj_179550.h"
#include "../src/enemy/obj_1795F6.h"
#include "../src/enemy/obj_17960A.h"
#include "../src/enemy/obj_179572.h"
#include "../src/enemy/obj_17961E.h"
#include "../src/enemy/obj_1795A4.h"
#include "../src/enemy/obj_179632.h"
#include "../src/enemy/obj_179646.h"
#include "../src/enemy/obj_17955E.h"
#include "../src/enemy/obj_179600.h"
#include "../src/enemy/obj_179614.h"
#include "../src/enemy/obj_179588.h"
#include "../src/enemy/obj_179628.h"
#include "../src/enemy/obj_1795CA.h"
#include "../src/enemy/obj_17963C.h"
#include "../src/enemy/obj_179650.h"
#include "../src/objects/objscript_CF1E4.h"
#include "../src/objects/objscript_CF4B8.h"
#include "../src/objects/obj_shared.h"
#include "../src/objects/obj_anim.h"

typedef int (*tick_fn)(const f3mem_t *, uint32_t);
typedef int (*tick_a1_fn)(const f3mem_t *, uint32_t, uint32_t);

static int run_any(const char *name, const char *pfx, tick_fn fn, tick_a1_fn fn_a1, int reg, const uint8_t *rom)
{
    capture_t c;
    if (cap_load(&c, pfx) || !c.spr) { printf("objects: %-22s SKIP (no %s)\n", name, pfx); return 0; }
    uint8_t *w = malloc(F3_WRAM_SIZE), *s = malloc(F3_SPR_SIZE), *g = c.gfx ? malloc(F3_GFX_SIZE) : NULL;
    int ok = 0, unimpl = 0, budget = 8, died = 0, freed = 0, hidden = 0;
    for (int i = 0; i < c.pairs; i++) {
        memcpy(w, cap_wram(&c, i, 0), F3_WRAM_SIZE);
        memcpy(s, cap_spr(&c, i, 0), F3_SPR_SIZE);
        if (g) memcpy(g, cap_gfx(&c, i, 0), F3_GFX_SIZE);
        f3mem_t m = { w, (uint8_t *)rom, NULL, s, g };
        uint32_t obj = c.pre[i][CR_A0];
        uint32_t fn0 = rd32(&m, obj);   /* tick fn before the call */
        int r = fn ? fn(&m, obj) : fn_a1(&m, obj, c.pre[i][reg]);
        if (r < 0) {
            /* the tick's effect is unobservable, so this pair cannot be
             * compared; strict mode (harvest gates) refuses to count it */
            unimpl++; hidden++; continue;
        }
        if (rd32(&m, obj) != fn0) died++;   /* switched state (dying) */
        if (memcmp(w + 0x10000 - 0, cap_wram(&c, i, 0) + 0x10000, 0x30)) freed++; /* free masks changed */
        uint32_t sp = c.pre[i][CR_SP];
        int d = cap_diff(name, i, F3_WRAM_BASE, w, cap_wram(&c, i, 1), F3_WRAM_SIZE, sp - 0x100, sp, &budget)
              + cap_diff(name, i, F3_SPR_BASE, s, cap_spr(&c, i, 1), F3_SPR_SIZE, 0, 0, &budget)
              + (g ? cap_diff(name, i, F3_GFX_BASE, g, cap_gfx(&c, i, 1), F3_GFX_SIZE, 0, 0, &budget) : 0);
        ok += d == 0;
    }
    int tested = c.pairs - unimpl;
    printf("objects: %-22s %3d/%3d ticks bit-exact (%d state changes, %d alloc/free; %d reach undecompiled paths)\n",
           name, ok, tested, died, freed, unimpl);
    free(w); free(s); free(g); cap_free(&c);
    if (hidden && tested == 0) {
        /* every captured tick took a not-yet-decompiled path: nothing was
         * observable, so this routine must not pass silently (the harvest
         * gate has to see a failure) */
        printf("objects: %-22s %3d/%3d ticks bit-exact REJECT %d unobservable tick(s) (undecompiled path)\n", name, ok, tested, hidden);
        return hidden;
    }
    return tested - ok;
}

/* F3_ONLY=<substring>: run only the tests whose name contains it */
static int skip(const char *name)
{ const char *o = getenv("F3_ONLY"); return o && *o && !strstr(name, o); }
static int run(const char *name, const char *pfx, tick_fn fn, const uint8_t *rom)
{ return skip(name) ? 0 : run_any(name, pfx, fn, NULL, 0, rom); }
static int run_a1(const char *name, const char *pfx, tick_a1_fn fn, const uint8_t *rom)
{ return skip(name) ? 0 : run_any(name, pfx, NULL, fn, CR_A1, rom); }
/* the pool runner (0x104D0..0x104EE) does not set D3: a tick that reads it
 * sees whatever the previous object's tick left there */
static int run_d3(const char *name, const char *pfx, tick_a1_fn fn, const uint8_t *rom)
{ return skip(name) ? 0 : run_any(name, pfx, NULL, fn, CR_D0 + 3, rom); }

int main(void)
{
    long rl;
    uint8_t *rom = cap_slurp("work/maincpu.bin", &rl);
    (void)rl;
    if (!rom) { printf("objects: SKIP (need work/maincpu.bin)\n"); return 0; }
    int bad = 0;
    bad += run("EnemyBullet 0x102FE6", "work/ram/obj_102FE6", Obj_EnemyBullet_Tick, rom);
    bad += run("PlayerShot 0x0C9C6A", "work/ram/obj_C9C6A", Obj_PlayerShot_Tick, rom);
    bad += run("Obj 0x0D9116", "work/ram/obj_D9116", Obj_D9116_Tick, rom);
    bad += run("Obj 0x0CABF4", "work/ram/obj_CABF4", Obj_CABF4_Tick, rom);
    bad += run("Obj 0x0EAD28", "work/ram/obj_EAD28", Obj_EAD28_Tick, rom);
    bad += run("Obj 0x1027B4", "work/ram/obj_1027B4", Obj_1027B4_Tick, rom);
    bad += run("Obj 0x102860", "work/ram/obj_102860", Obj_102860_Tick, rom);
    bad += run("Obj 0x102928", "work/ram/obj_102928", Obj_102928_Tick, rom);
    bad += run("Obj 0x0C9F2E", "work/ram/obj_C9F2E", Obj_C9F2E_Tick, rom);
    bad += run("Obj 0x10252E", "work/ram/obj_10252E", Obj_10252E_Tick, rom);
    bad += run("Obj 0x102A9A", "work/ram/obj_102A9A", Obj_102A9A_Tick, rom);
    bad += run("Obj 0x119990", "work/ram/obj_119990", Obj_119990_Tick, rom);
    bad += run("Obj 0x0CBF5E", "work/ram/obj_CBF5E", Obj_CBF5E_Tick, rom);
    bad += run("Obj 0x119CBA", "work/ram/obj_119CBA", Obj_119CBA_Tick, rom);
    bad += run("Obj 0x117D06", "work/ram/obj_117D06", Obj_117D06_Tick, rom);
    bad += run("Obj 0x0D5464", "work/ram/obj_D5464", Obj_D5464_Tick, rom);
    bad += run("Obj 0x13E5B2", "work/ram/obj_13E5B2", Obj_13E5B2_Tick, rom);
    bad += run("Obj 0x0CAB36", "work/ram/obj_CAB36", Obj_CAB36_Tick, rom);
    bad += run("Obj_Dying1 0x0CEE7C", "work/ram/obj_CEE7C", Obj_Dying1_Tick, rom);
    bad += run("Obj_Dying2 0x0CEE84", "work/ram/obj_CEE84", Obj_Dying2_Tick, rom);
    bad += run("Obj_Dying3 0x0CEE8C", "work/ram/obj_CEE8C", Obj_Dying3_Tick, rom);
    bad += run("Obj 0x119714", "work/ram/obj_119714", Obj_119714_Tick, rom);
    bad += run("Obj 0x0CBF8A", "work/ram/obj_CBF8A", Obj_CBF8A_Tick, rom);
    bad += run("Obj 0x0DCDF2", "work/ram/obj_DCDF2", Obj_DCDF2_Tick, rom);
    bad += run("Obj 0x0DC8BE", "work/ram/obj_DC8BE", Obj_DC8BE_Tick, rom);
    bad += run("Obj 0x0F30F6", "work/ram/obj_F30F6", Obj_F30F6_Tick, rom);
    bad += run("Obj 0x0E04CC", "work/ram/obj_E04CC", Obj_E04CC_Tick, rom);
    bad += run("Obj 0x0DECF8", "work/ram/obj_DECF8", Obj_DECF8_Tick, rom);
    bad += run("Obj 0x0EACDE", "work/ram/obj_EACDE", Obj_EACDE_Tick, rom);
    bad += run("Obj 0x0E0278", "work/ram/obj_E0278", Obj_E0278_Tick, rom);
    bad += run("Obj 0x117BDE", "work/ram/obj_117BDE", Obj_117BDE_Tick, rom);
    bad += run("Obj 0x0F302E", "work/ram/obj_F302E", Obj_F302E_Tick, rom);
    bad += run("Obj 0x0DC3F6", "work/ram/obj_DC3F6", Obj_DC3F6_Tick, rom);
    bad += run("Obj 0x0C6866", "work/ram/obj_C6866", Obj_C6866_Tick, rom);
    bad += run("Obj 0x0C9978", "work/ram/obj_C9978", Obj_C9978_Tick, rom);
    bad += run("Obj 0x0C9D7E", "work/ram/obj_C9D7E", Obj_C9D7E_Tick, rom);
    bad += run("Obj 0x0DC542", "work/ram/obj_DC542", Obj_DC542_Tick, rom);
    bad += run("Obj 0x0E02C4", "work/ram/obj_E02C4", Obj_E02C4_Tick, rom);
    bad += run("Obj 0x0CBF6C", "work/ram/obj_CBF6C", Obj_CBF6C_Tick, rom);
    bad += run("Obj 0x0D540A", "work/ram/obj_D540A", Obj_D540A_Tick, rom);
    bad += run("Obj 0x0D901A", "work/ram/obj_D901A", Obj_D901A_Tick, rom);
    bad += run("Obj 0x0CC0F4", "work/ram/obj_CC0F4", Obj_CC0F4_Tick, rom);
    bad += run("Obj 0x0D9092", "work/ram/obj_D9092", Obj_D9092_Tick, rom);
    bad += run("Obj 0x0DC456", "work/ram/obj_DC456", Obj_DC456_Tick, rom);
    bad += run("Obj 0x0DC586", "work/ram/obj_DC586", Obj_DC586_Tick, rom);
    bad += run("Obj 0x0DEC9E", "work/ram/obj_DEC9E", Obj_DEC9E_Tick, rom);
    bad += run("Obj 0x0E029E", "work/ram/obj_E029E", Obj_E029E_Tick, rom);
    bad += run("Obj 0x0E0266", "work/ram/obj_E0266", Obj_E0266_Tick, rom);
    bad += run("Obj 0x0F30B4", "work/ram/obj_F30B4", Obj_F30B4_Tick, rom);
    bad += run("Obj 0x102F52", "work/ram/obj_102F52", Obj_102F52_Tick, rom);
    bad += run("Obj 0x117B44", "work/ram/obj_117B44", Obj_117B44_Tick, rom);
    bad += run("Obj 0x117C28", "work/ram/obj_117C28", Obj_117C28_Tick, rom);
    bad += run("Obj 0x13E4EE", "work/ram/obj_13E4EE", Obj_13E4EE_Tick, rom);
    bad += run("Obj 0x0DC328", "work/ram/obj_DC328", Obj_DC328_Tick, rom);
    bad += run("Obj 0x0DC0F8", "work/ram/obj_DC0F8", Obj_DC0F8_Tick, rom);
    bad += run("Obj 0x0DF7EA", "work/ram/obj_DF7EA", Obj_DF7EA_Tick, rom);
    bad += run("Obj 0x0DF844", "work/ram/obj_DF844", Obj_DF844_Tick, rom);
    bad += run("Obj 0x0CBC88", "work/ram/obj_CBC88", Obj_CBC88_Tick, rom);
    bad += run("Obj 0x0CBC90", "work/ram/obj_CBC90", Obj_CBC90_Tick, rom);
    bad += run("Obj 0x0BB83E", "work/ram/obj_BB83E", Obj_BB83E_Tick, rom);
    bad += run("Obj 0x0BC3EC", "work/ram/obj_BC3EC", Obj_BC3EC_Tick, rom);
    bad += run("Obj 0x0BCA00", "work/ram/obj_BCA00", Obj_BCA00_Tick, rom);
    bad += run("Obj 0x0BCD82", "work/ram/obj_BCD82", Obj_BCD82_Tick, rom);
    bad += run("Obj 0x0CBC98", "work/ram/obj_CBC98", Obj_CBC98_Tick, rom);
    bad += run("Obj 0x11C39A", "work/ram/obj_11C39A", Obj_11C39A_Tick, rom);
    bad += run("Obj 0x0BC07E", "work/ram/obj_BC07E", Obj_BC07E_Tick, rom);
    bad += run("Obj 0x0BD544", "work/ram/obj_BD544", Obj_BD544_Tick, rom);
    bad += run("Obj 0x0C685A", "work/ram/obj_C685A", Obj_C685A_Tick, rom);
    bad += run("Obj 0x0CB406", "work/ram/obj_CB406", Obj_CB406_Tick, rom);
    bad += run("Obj 0x0CB0DA", "work/ram/obj_CB0DA", Obj_CB0DA_Tick, rom);
    bad += run("Obj 0x0BC152", "work/ram/obj_BC152", Obj_BC152_Tick, rom);
    bad += run("Obj 0x0BC1CE", "work/ram/obj_BC1CE", Obj_BC1CE_Tick, rom);
    bad += run("Obj 0x0BC3DE", "work/ram/obj_BC3DE", Obj_BC3DE_Tick, rom);
    bad += run("Obj 0x0BC064", "work/ram/obj_BC064", Obj_BC064_Tick, rom);
    bad += run("Obj 0x0BC638", "work/ram/obj_BC638", Obj_BC638_Tick, rom);
    bad += run("Obj 0x0BD298", "work/ram/obj_BD298", Obj_BD298_Tick, rom);
    bad += run("Obj 0x0CB412", "work/ram/obj_CB412", Obj_CB412_Tick, rom);
    bad += run("Obj 0x12EF24", "work/ram/obj_12EF24", Obj_12EF24_Tick, rom);
    bad += run("Obj 0x0BBD4A", "work/ram/obj_BBD4A", Obj_BBD4A_Tick, rom);
    bad += run("Obj 0x0BBE06", "work/ram/obj_BBE06", Obj_BBE06_Tick, rom);
    bad += run("Obj 0x0BC4A8", "work/ram/obj_BC4A8", Obj_BC4A8_Tick, rom);
    bad += run("Obj 0x0BD256", "work/ram/obj_BD256", Obj_BD256_Tick, rom);
    bad += run("Obj 0x0BBBE0", "work/ram/obj_BBBE0", Obj_BBBE0_Tick, rom);
    bad += run("Obj 0x0BC008", "work/ram/obj_BC008", Obj_BC008_Tick, rom);
    bad += run("Obj 0x0BD340", "work/ram/obj_BD340", Obj_BD340_Tick, rom);
    bad += run("Obj 0x0D7570", "work/ram/obj_D7570", Obj_D7570_Tick, rom);
    bad += run("Obj 0x0BBFE8", "work/ram/obj_BBFE8", Obj_BBFE8_Tick, rom);
    bad += run("Obj 0x0BCE8E", "work/ram/obj_BCE8E", Obj_BCE8E_Tick, rom);
    bad += run("Obj 0x0BD354", "work/ram/obj_BD354", Obj_BD354_Tick, rom);
    bad += run("Obj 0x12E1FA", "work/ram/obj_12E1FA", Obj_12E1FA_Tick, rom);
    bad += run("Obj 0x0BCF08", "work/ram/obj_BCF08", Obj_BCF08_Tick, rom);
    bad += run("Obj 0x0BCE38", "work/ram/obj_BCE38", Obj_BCE38_Tick, rom);
    bad += run("Obj 0x12F26A", "work/ram/obj_12F26A", Obj_12F26A_Tick, rom);
    bad += run("Obj 0x12F27A", "work/ram/obj_12F27A", Obj_12F27A_Tick, rom);
    bad += run("Obj 0x0BBD24", "work/ram/obj_BBD24", Obj_BBD24_Tick, rom);
    bad += run("Obj 0x0BBDE0", "work/ram/obj_BBDE0", Obj_BBDE0_Tick, rom);
    bad += run("Obj 0x0BBE9A", "work/ram/obj_BBE9A", Obj_BBE9A_Tick, rom);
    bad += run("Obj 0x0CABAC", "work/ram/obj_CABAC", Obj_CABAC_Tick, rom);
    bad += run("Obj 0x0BB746", "work/ram/obj_BB746", Obj_BB746_Tick, rom);
    bad += run("Obj 0x0BC03C", "work/ram/obj_BC03C", Obj_BC03C_Tick, rom);
    bad += run("Obj 0x0CB2C6", "work/ram/obj_CB2C6", Obj_CB2C6_Tick, rom);
    bad += run("Obj 0x0BCD90", "work/ram/obj_BCD90", Obj_BCD90_Tick, rom);
    bad += run("Obj 0x0BBCDA", "work/ram/obj_BBCDA", Obj_BBCDA_Tick, rom);
    bad += run("Obj 0x0BBD6E", "work/ram/obj_BBD6E", Obj_BBD6E_Tick, rom);
    bad += run("Obj 0x0BBE2A", "work/ram/obj_BBE2A", Obj_BBE2A_Tick, rom);
    bad += run("Obj 0x11C1DE", "work/ram/obj_11C1DE", Obj_11C1DE_Tick, rom);
    bad += run("Obj 0x12EA9E", "work/ram/obj_12EA9E", Obj_12EA9E_Tick, rom);
    bad += run("Obj 0x0BC540", "work/ram/obj_BC540", Obj_BC540_Tick, rom);
    bad += run("Obj 0x11A87E", "work/ram/obj_11A87E", Obj_11A87E_Tick, rom);
    bad += run("Obj 0x12E21A", "work/ram/obj_12E21A", Obj_12E21A_Tick, rom);
    bad += run("Obj 0x0CA28C", "work/ram/obj_CA28C", Obj_CA28C_Tick, rom);
    bad += run("Obj 0x0CAF3E", "work/ram/obj_CAF3E", Obj_CAF3E_Tick, rom);
    bad += run("Obj 0x0BC3FA", "work/ram/obj_BC3FA", Obj_BC3FA_Tick, rom);
    bad += run("Obj 0x0BC404", "work/ram/obj_BC404", Obj_BC404_Tick, rom);
    bad += run("Obj 0x0BC40E", "work/ram/obj_BC40E", Obj_BC40E_Tick, rom);
    bad += run("Obj 0x0BC418", "work/ram/obj_BC418", Obj_BC418_Tick, rom);
    bad += run("Obj 0x0BC422", "work/ram/obj_BC422", Obj_BC422_Tick, rom);
    bad += run("Obj 0x0BC24A", "work/ram/obj_BC24A", Obj_BC24A_Tick, rom);
    bad += run("Obj 0x0CAF1A", "work/ram/obj_CAF1A", Obj_CAF1A_Tick, rom);
    bad += run("Obj 0x0BB692", "work/ram/obj_BB692", Obj_BB692_Tick, rom);
    bad += run("Obj 0x0BCEB0", "work/ram/obj_BCEB0", Obj_BCEB0_Tick, rom);
    bad += run("Obj 0x12E1BE", "work/ram/obj_12E1BE", Obj_12E1BE_Tick, rom);
    bad += run("Obj 0x0BBD00", "work/ram/obj_BBD00", Obj_BBD00_Tick, rom);
    bad += run("Obj 0x0BCF30", "work/ram/obj_BCF30", Obj_BCF30_Tick, rom);
    bad += run("Obj 0x0BE262", "work/ram/obj_BE262", Obj_BE262_Tick, rom);
    bad += run("Obj 0x0BB84C", "work/ram/obj_BB84C", Obj_BB84C_Tick, rom);
    bad += run("Obj 0x0BB856", "work/ram/obj_BB856", Obj_BB856_Tick, rom);
    bad += run("Obj 0x0BB860", "work/ram/obj_BB860", Obj_BB860_Tick, rom);
    bad += run("Obj 0x0BB86A", "work/ram/obj_BB86A", Obj_BB86A_Tick, rom);
    bad += run("Obj 0x0BB874", "work/ram/obj_BB874", Obj_BB874_Tick, rom);
    bad += run("Obj 0x0BCDC0", "work/ram/obj_BCDC0", Obj_BCDC0_Tick, rom);
    bad += run("Obj 0x0CAF5E", "work/ram/obj_CAF5E", Obj_CAF5E_Tick, rom);
    bad += run("Obj 0x12EDA8", "work/ram/obj_12EDA8", Obj_12EDA8_Tick, rom);
    bad += run("Obj 0x12E19A", "work/ram/obj_12E19A", Obj_12E19A_Tick, rom);
    bad += run("Obj 0x0BC680", "work/ram/obj_BC680", Obj_BC680_Tick, rom);
    bad += run("Obj 0x0BCFF4", "work/ram/obj_BCFF4", Obj_BCFF4_Tick, rom);
    bad += run("Obj 0x12EB0E", "work/ram/obj_12EB0E", Obj_12EB0E_Tick, rom);
    bad += run("Obj 0x0BB5C2", "work/ram/obj_BB5C2", Obj_BB5C2_Tick, rom);
    bad += run("Obj 0x0BBE50", "work/ram/obj_BBE50", Obj_BBE50_Tick, rom);
    bad += run("Obj 0x0BCE52", "work/ram/obj_BCE52", Obj_BCE52_Tick, rom);
    bad += run("Obj 0x0DBDA2", "work/ram/obj_DBDA2", Obj_DBDA2_Tick, rom);
    bad += run("Obj 0x0ECF3E", "work/ram/obj_ECF3E", Obj_ECF3E_Tick, rom);
    bad += run("Obj 0x0BB952", "work/ram/obj_BB952", Obj_BB952_Tick, rom);
    bad += run("Obj 0x0BBD94", "work/ram/obj_BBD94", Obj_BBD94_Tick, rom);
    bad += run("Obj 0x0BBEC0", "work/ram/obj_BBEC0", Obj_BBEC0_Tick, rom);
    bad += run("Obj 0x0DBDE8", "work/ram/obj_DBDE8", Obj_DBDE8_Tick, rom);
    bad += run("Obj 0x0DC674", "work/ram/obj_DC674", Obj_DC674_Tick, rom);
    bad += run("Obj 0x105268", "work/ram/obj_105268", Obj_105268_Tick, rom);
    bad += run("Obj 0x0BBB18", "work/ram/obj_BBB18", Obj_BBB18_Tick, rom);
    bad += run("Obj 0x0CAEBA", "work/ram/obj_CAEBA", Obj_CAEBA_Tick, rom);
    bad += run("Obj 0x0EA28C", "work/ram/obj_EA28C", Obj_EA28C_Tick, rom);
    bad += run("Obj 0x0EA836", "work/ram/obj_EA836", Obj_EA836_Tick, rom);
    bad += run("Obj 0x10524A", "work/ram/obj_10524A", Obj_10524A_Tick, rom);
    bad += run("Obj 0x1095B0", "work/ram/obj_1095B0", Obj_1095B0_Tick, rom);
    bad += run("Obj 0x0F34F6", "work/ram/obj_F34F6", Obj_F34F6_Tick, rom);
    bad += run("Obj 0x1116E2", "work/ram/obj_1116E2", Obj_1116E2_Tick, rom);
    bad += run("Obj 0x12EA24", "work/ram/obj_12EA24", Obj_12EA24_Tick, rom);
    bad += run("Obj 0x0BD990", "work/ram/obj_BD990", Obj_BD990_Tick, rom);
    bad += run("Obj 0x12EAD4", "work/ram/obj_12EAD4", Obj_12EAD4_Tick, rom);
    bad += run("Obj 0x12CE5A", "work/ram/obj_12CE5A", Obj_12CE5A_Tick, rom);
    bad += run("Obj 0x0CB2EC", "work/ram/obj_CB2EC", Obj_CB2EC_Tick, rom);
    bad += run("Obj 0x10290C", "work/ram/obj_10290C", Obj_10290C_Tick, rom);
    bad += run("Obj 0x0E186A", "work/ram/obj_E186A", Obj_E186A_Tick, rom);
    bad += run("Obj 0x102798", "work/ram/obj_102798", Obj_102798_Tick, rom);
    bad += run("Obj 0x102844", "work/ram/obj_102844", Obj_102844_Tick, rom);
    bad += run("Obj 0x102A70", "work/ram/obj_102A70", Obj_102A70_Tick, rom);
    bad += run("Obj 0x0BBAE4", "work/ram/obj_BBAE4", Obj_BBAE4_Tick, rom);
    bad += run("Obj 0x0CBC78", "work/ram/obj_CBC78", Obj_CBC78_Tick, rom);
    bad += run("Obj 0x0DA0D6", "work/ram/obj_DA0D6", Obj_DA0D6_Tick, rom);
    bad += run("Obj 0x0ECA04", "work/ram/obj_ECA04", Obj_ECA04_Tick, rom);
    bad += run("Obj 0x0EA418", "work/ram/obj_EA418", Obj_EA418_Tick, rom);
    bad += run("Obj 0x0ECF0C", "work/ram/obj_ECF0C", Obj_ECF0C_Tick, rom);
    bad += run("Obj 0x0ECFAE", "work/ram/obj_ECFAE", Obj_ECFAE_Tick, rom);
    bad += run("Obj 0x0BBBF4", "work/ram/obj_BBBF4", Obj_BBBF4_Tick, rom);
    bad += run("Obj 0x0BC576", "work/ram/obj_BC576", Obj_BC576_Tick, rom);
    bad += run("Obj 0x0BDE42", "work/ram/obj_BDE42", Obj_BDE42_Tick, rom);
    bad += run("Obj 0x0EA29E", "work/ram/obj_EA29E", Obj_EA29E_Tick, rom);
    bad += run("Obj 0x104EFA", "work/ram/obj_104EFA", Obj_104EFA_Tick, rom);
    bad += run("Obj 0x0C9B3E", "work/ram/obj_C9B3E", Obj_C9B3E_Tick, rom);
    bad += run("Obj 0x0CBA34", "work/ram/obj_CBA34", Obj_CBA34_Tick, rom);
    bad += run("Obj 0x0ECFDA", "work/ram/obj_ECFDA", Obj_ECFDA_Tick, rom);
    bad += run("Obj 0x0BD308", "work/ram/obj_BD308", Obj_BD308_Tick, rom);
    bad += run("Obj 0x0CB054", "work/ram/obj_CB054", Obj_CB054_Tick, rom);
    bad += run("Obj 0x0CB422", "work/ram/obj_CB422", Obj_CB422_Tick, rom);
    bad += run("Obj 0x0CAF90", "work/ram/obj_CAF90", Obj_CAF90_Tick, rom);
    bad += run("Obj 0x0F3582", "work/ram/obj_F3582", Obj_F3582_Tick, rom);
    bad += run("Obj 0x0BC2A0", "work/ram/obj_BC2A0", Obj_BC2A0_Tick, rom);
    bad += run("Obj 0x10963E", "work/ram/obj_10963E", Obj_10963E_Tick, rom);
    bad += run("Obj 0x0EACEA", "work/ram/obj_EACEA", Obj_EACEA_Tick, rom);
    bad += run("Obj 0x0BCC96", "work/ram/obj_BCC96", Obj_BCC96_Tick, rom);
    bad += run("Obj 0x0BD2C2", "work/ram/obj_BD2C2", Obj_BD2C2_Tick, rom);
    bad += run("Obj 0x0BDDB8", "work/ram/obj_BDDB8", Obj_BDDB8_Tick, rom);
    bad += run("Obj 0x11C1FC", "work/ram/obj_11C1FC", Obj_11C1FC_Tick, rom);
    bad += run("Obj 0x0ECAC8", "work/ram/obj_ECAC8", Obj_ECAC8_Tick, rom);
    bad += run("Obj 0x0CB9EE", "work/ram/obj_CB9EE", Obj_CB9EE_Tick, rom);
    bad += run("Obj 0x11C3A8", "work/ram/obj_11C3A8", Obj_11C3A8_Tick, rom);
    bad += run("Obj 0x0BD700", "work/ram/obj_BD700", Obj_BD700_Tick, rom);
    bad += run("Obj 0x0CB97E", "work/ram/obj_CB97E", Obj_CB97E_Tick, rom);
    bad += run("Obj 0x0E1B36", "work/ram/obj_E1B36", Obj_E1B36_Tick, rom);
    bad += run("Obj 0x0C9DEC", "work/ram/obj_C9DEC", Obj_C9DEC_Tick, rom);
    bad += run("Obj 0x0CAFB8", "work/ram/obj_CAFB8", Obj_CAFB8_Tick, rom);
    bad += run("Obj 0x12CFD4", "work/ram/obj_12CFD4", Obj_12CFD4_Tick, rom);
    bad += run("Obj 0x12F29E", "work/ram/obj_12F29E", Obj_12F29E_Tick, rom);
    bad += run("Obj 0x0BD0EC", "work/ram/obj_BD0EC", Obj_BD0EC_Tick, rom);
    bad += run("Obj 0x12EF40", "work/ram/obj_12EF40", Obj_12EF40_Tick, rom);
    bad += run("Obj 0x0BC16C", "work/ram/obj_BC16C", Obj_BC16C_Tick, rom);
    bad += run("Obj 0x0BC1E8", "work/ram/obj_BC1E8", Obj_BC1E8_Tick, rom);
    bad += run("Obj 0x11A8A8", "work/ram/obj_11A8A8", Obj_11A8A8_Tick, rom);
    bad += run("Obj 0x0BDDE2", "work/ram/obj_BDDE2", Obj_BDDE2_Tick, rom);
    bad += run("Obj 0x0BDC5C", "work/ram/obj_BDC5C", Obj_BDC5C_Tick, rom);
    bad += run("Obj 0x0BC08C", "work/ram/obj_BC08C", Obj_BC08C_Tick, rom);
    bad += run("Obj 0x0BD3A4", "work/ram/obj_BD3A4", Obj_BD3A4_Tick, rom);
    bad += run("Obj 0x0BD550", "work/ram/obj_BD550", Obj_BD550_Tick, rom);
    bad += run("Obj 0x0BD7E4", "work/ram/obj_BD7E4", Obj_BD7E4_Tick, rom);
    bad += run("Obj 0x0BE16C", "work/ram/obj_BE16C", Obj_BE16C_Tick, rom);
    bad += run("Obj 0x13EFA0", "work/ram/obj_13EFA0", Obj_13EFA0_Tick, rom);
    bad += run("Obj 0x104F26", "work/ram/obj_104F26", Obj_104F26_Tick, rom);
    bad += run("Obj 0x0BD9BE", "work/ram/obj_BD9BE", Obj_BD9BE_Tick, rom);
    bad += run("Obj 0x0BC70A", "work/ram/obj_BC70A", Obj_BC70A_Tick, rom);
    bad += run("Obj 0x0D9FC8", "work/ram/obj_D9FC8", Obj_D9FC8_Tick, rom);
    bad += run("Obj 0x12EE24", "work/ram/obj_12EE24", Obj_12EE24_Tick, rom);
    bad += run("Obj 0x0DA138", "work/ram/obj_DA138", Obj_DA138_Tick, rom);
    bad += run("Obj 0x12EB90", "work/ram/obj_12EB90", Obj_12EB90_Tick, rom);
    bad += run("Obj 0x0D7306", "work/ram/obj_D7306", Obj_D7306_Tick, rom);
    bad += run("Obj 0x0BDE92", "work/ram/obj_BDE92", Obj_BDE92_Tick, rom);
    bad += run("Obj 0x0E9DE0", "work/ram/obj_E9DE0", Obj_E9DE0_Tick, rom);
    bad += run("Obj 0x0E96C8", "work/ram/obj_E96C8", Obj_E96C8_Tick, rom);
    bad += run("Obj 0x0E1886", "work/ram/obj_E1886", Obj_E1886_Tick, rom);
    bad += run("Obj 0x0EA848", "work/ram/obj_EA848", Obj_EA848_Tick, rom);
    bad += run("Obj 0x0DB074", "work/ram/obj_DB074", Obj_DB074_Tick, rom);
    bad += run("Obj 0x10967C", "work/ram/obj_10967C", Obj_10967C_Tick, rom);
    bad += run("Obj 0x0F35B4", "work/ram/obj_F35B4", Obj_F35B4_Tick, rom);
    bad += run("Obj 0x0DC6D0", "work/ram/obj_DC6D0", Obj_DC6D0_Tick, rom);
    bad += run("Obj 0x0E1328", "work/ram/obj_E1328", Obj_E1328_Tick, rom);
    bad += run("Obj 0x0EA45E", "work/ram/obj_EA45E", Obj_EA45E_Tick, rom);
    bad += run("Obj 0x1049C4", "work/ram/obj_1049C4", Obj_1049C4_Tick, rom);
    bad += run("Obj 0x0ECB3A", "work/ram/obj_ECB3A", Obj_ECB3A_Tick, rom);
    bad += run("Obj 0x0D740E", "work/ram/obj_D740E", Obj_D740E_Tick, rom);
    bad += run("Obj 0x0DC5B6", "work/ram/obj_DC5B6", Obj_DC5B6_Tick, rom);
    bad += run("Obj 0x0D747E", "work/ram/obj_D747E", Obj_D747E_Tick, rom);
    bad += run("Obj 0x0CD646", "work/ram/obj_CD646", Obj_CD646_Tick, rom);
    bad += run("Obj 0x0CD504", "work/ram/obj_CD504", Obj_CD504_Tick, rom);
    bad += run("Obj 0x0CD5EA", "work/ram/obj_CD5EA", Obj_CD5EA_Tick, rom);
    bad += run("Obj 0x0CD556", "work/ram/obj_CD556", Obj_CD556_Tick, rom);
    bad += run("Obj 0x1180F8", "work/ram/obj_1180F8", Obj_1180F8_Tick, rom);
    bad += run("Obj 0x0E077A", "work/ram/obj_E077A", Obj_E077A_Tick, rom);
    bad += run("Obj 0x0E1036", "work/ram/obj_E1036", Obj_E1036_Tick, rom);
    bad += run("Obj 0x112D1A", "work/ram/obj_112D1A", Obj_112D1A_Tick, rom);
    bad += run("Obj 0x0E101A", "work/ram/obj_E101A", Obj_E101A_Tick, rom);
    bad += run("Obj 0x1134B2", "work/ram/obj_1134B2", Obj_1134B2_Tick, rom);
    bad += run("Obj 0x113ED0", "work/ram/obj_113ED0", Obj_113ED0_Tick, rom);
    bad += run("Obj 0x112C1A", "work/ram/obj_112C1A", Obj_112C1A_Tick, rom);
    bad += run("Obj 0x1140BE", "work/ram/obj_1140BE", Obj_1140BE_Tick, rom);
    bad += run("Obj 0x112C3C", "work/ram/obj_112C3C", Obj_112C3C_Tick, rom);
    bad += run("Obj 0x118CF6", "work/ram/obj_118CF6", Obj_118CF6_Tick, rom);
    bad += run("Obj 0x112D88", "work/ram/obj_112D88", Obj_112D88_Tick, rom);
    bad += run("Obj 0x114108", "work/ram/obj_114108", Obj_114108_Tick, rom);
    bad += run("Obj 0x1134BA", "work/ram/obj_1134BA", Obj_1134BA_Tick, rom);
    bad += run("Obj 0x118DDC", "work/ram/obj_118DDC", Obj_118DDC_Tick, rom);
    bad += run("Obj 0x118D7C", "work/ram/obj_118D7C", Obj_118D7C_Tick, rom);
    bad += run("Obj 0x0F0788", "work/ram/obj_F0788", Obj_F0788_Tick, rom);
    bad += run("Obj 0x0F0846", "work/ram/obj_F0846", Obj_F0846_Tick, rom);
    bad += run("Obj 0x108936", "work/ram/obj_108936", Obj_108936_Tick, rom);
    bad += run("Obj 0x1087E8", "work/ram/obj_1087E8", Obj_1087E8_Tick, rom);
    bad += run("Obj 0x0DC486", "work/ram/obj_DC486", Obj_DC486_Tick, rom);
    bad += run("Obj 0x0E598C", "work/ram/obj_E598C", Obj_E598C_Tick, rom);
    bad += run("Obj 0x0F0814", "work/ram/obj_F0814", Obj_F0814_Tick, rom);
    bad += run("Obj 0x1088F0", "work/ram/obj_1088F0", Obj_1088F0_Tick, rom);
    bad += run("Obj 0x0E6166", "work/ram/obj_E6166", Obj_E6166_Tick, rom);
    bad += run("Obj 0x0E61AA", "work/ram/obj_E61AA", Obj_E61AA_Tick, rom);
    bad += run("Obj 0x0E5CBC", "work/ram/obj_E5CBC", Obj_E5CBC_Tick, rom);
    bad += run("Obj 0x0DC358", "work/ram/obj_DC358", Obj_DC358_Tick, rom);
    bad += run("Obj 0x0E60EA", "work/ram/obj_E60EA", Obj_E60EA_Tick, rom);
    bad += run("Obj 0x0E5C2A", "work/ram/obj_E5C2A", Obj_E5C2A_Tick, rom);
    bad += run("Obj 0x0F5AFE", "work/ram/obj_F5AFE", Obj_F5AFE_Tick, rom);
    bad += run("Obj 0x0E6082", "work/ram/obj_E6082", Obj_E6082_Tick, rom);
    bad += run("Obj 0x0F5D58", "work/ram/obj_F5D58", Obj_F5D58_Tick, rom);
    bad += run("Obj 0x0F5CF6", "work/ram/obj_F5CF6", Obj_F5CF6_Tick, rom);
    bad += run("Obj 0x0E70CE", "work/ram/obj_E70CE", Obj_E70CE_Tick, rom);
    bad += run("Obj 0x0E7ABE", "work/ram/obj_E7ABE", Obj_E7ABE_Tick, rom);
    bad += run("Obj 0x0E7A7A", "work/ram/obj_E7A7A", Obj_E7A7A_Tick, rom);
    bad += run("Obj 0x0DA64C", "work/ram/obj_DA64C", Obj_DA64C_Tick, rom);
    bad += run("Obj 0x0E8494", "work/ram/obj_E8494", Obj_E8494_Tick, rom);
    bad += run("Obj 0x0DA6C0", "work/ram/obj_DA6C0", Obj_DA6C0_Tick, rom);
    bad += run("Obj 0x0E8240", "work/ram/obj_E8240", Obj_E8240_Tick, rom);
    bad += run("Obj 0x0E81A0", "work/ram/obj_E81A0", Obj_E81A0_Tick, rom);
    bad += run("Obj 0x1094A6", "work/ram/obj_1094A6", Obj_1094A6_Tick, rom);
    bad += run("Obj 0x109514", "work/ram/obj_109514", Obj_109514_Tick, rom);
    bad += run("Obj 0x108E8E", "work/ram/obj_108E8E", Obj_108E8E_Tick, rom);
    bad += run("Obj 0x10AAC2", "work/ram/obj_10AAC2", Obj_10AAC2_Tick, rom);
    bad += run("Obj 0x10AB30", "work/ram/obj_10AB30", Obj_10AB30_Tick, rom);
    bad += run("Obj 0x10AC10", "work/ram/obj_10AC10", Obj_10AC10_Tick, rom);
    bad += run("Obj 0x10AFBA", "work/ram/obj_10AFBA", Obj_10AFBA_Tick, rom);
    bad += run("Obj 0x10AFCC", "work/ram/obj_10AFCC", Obj_10AFCC_Tick, rom);
    bad += run("Obj 0x11464E", "work/ram/obj_11464E", Obj_11464E_Tick, rom);
    bad += run("Obj 0x114A76", "work/ram/obj_114A76", Obj_114A76_Tick, rom);
    bad += run("Obj 0x114A10", "work/ram/obj_114A10", Obj_114A10_Tick, rom);
    bad += run("Obj 0x114926", "work/ram/obj_114926", Obj_114926_Tick, rom);
    bad += run("Obj 0x114800", "work/ram/obj_114800", Obj_114800_Tick, rom);
    bad += run("Obj 0x114984", "work/ram/obj_114984", Obj_114984_Tick, rom);
    bad += run("Obj 0x1134E6", "work/ram/obj_1134E6", Obj_1134E6_Tick, rom);
    bad += run("Obj 0x1131A0", "work/ram/obj_1131A0", Obj_1131A0_Tick, rom);
    bad += run("Obj 0x13EE08", "work/ram/obj_13EE08", Obj_13EE08_Tick, rom);
    bad += run("Obj 0x13EE50", "work/ram/obj_13EE50", Obj_13EE50_Tick, rom);
    bad += run("Obj 0x114814", "work/ram/obj_114814", Obj_114814_Tick, rom);
    bad += run("Obj 0x113644", "work/ram/obj_113644", Obj_113644_Tick, rom);
    bad += run("Obj 0x0F0D7E", "work/ram/obj_F0D7E", Obj_F0D7E_Tick, rom);
    bad += run("Obj 0x0F0D8A", "work/ram/obj_F0D8A", Obj_F0D8A_Tick, rom);
    bad += run("Obj 0x0F0DB8", "work/ram/obj_F0DB8", Obj_F0DB8_Tick, rom);
    bad += run("Obj 0x0F131C", "work/ram/obj_F131C", Obj_F131C_Tick, rom);
    bad += run("Obj 0x0E64DA", "work/ram/obj_E64DA", Obj_E64DA_Tick, rom);
    bad += run("Obj 0x0E6508", "work/ram/obj_E6508", Obj_E6508_Tick, rom);
    bad += run("Obj 0x0E7036", "work/ram/obj_E7036", Obj_E7036_Tick, rom);
    bad += run("Obj 0x0E7024", "work/ram/obj_E7024", Obj_E7024_Tick, rom);
    bad += run("Obj 0x0E68C2", "work/ram/obj_E68C2", Obj_E68C2_Tick, rom);
    bad += run("Obj 0x0E6906", "work/ram/obj_E6906", Obj_E6906_Tick, rom);
    bad += run("Obj 0x0E6C36", "work/ram/obj_E6C36", Obj_E6C36_Tick, rom);
    bad += run("Obj 0x0E6C64", "work/ram/obj_E6C64", Obj_E6C64_Tick, rom);
    bad += run("Obj 0x0E6E0E", "work/ram/obj_E6E0E", Obj_E6E0E_Tick, rom);
    bad += run("Obj 0x0E6E54", "work/ram/obj_E6E54", Obj_E6E54_Tick, rom);
    bad += run("Obj 0x0E7E1C", "work/ram/obj_E7E1C", Obj_E7E1C_Tick, rom);
    bad += run("Obj 0x0E7DEE", "work/ram/obj_E7DEE", Obj_E7DEE_Tick, rom);
    bad += run("Obj 0x108D8E", "work/ram/obj_108D8E", Obj_108D8E_Tick, rom);
    bad += run("Obj 0x108DFC", "work/ram/obj_108DFC", Obj_108DFC_Tick, rom);
    bad += run("Obj 0x0F4D52", "work/ram/obj_F4D52", Obj_F4D52_Tick, rom);
    bad += run("Obj 0x0F4DDE", "work/ram/obj_F4DDE", Obj_F4DDE_Tick, rom);
    bad += run("Obj 0x0F4E1E", "work/ram/obj_F4E1E", Obj_F4E1E_Tick, rom);
    bad += run("Obj 0x0D9D9E", "work/ram/obj_D9D9E", Obj_D9D9E_Tick, rom);
    bad += run("Obj 0x0D9E06", "work/ram/obj_D9E06", Obj_D9E06_Tick, rom);
    bad += run("Obj 0x0DBF16", "work/ram/obj_DBF16", Obj_DBF16_Tick, rom);
    bad += run("Obj 0x0DBF70", "work/ram/obj_DBF70", Obj_DBF70_Tick, rom);
    bad += run("Obj 0x0CC11C", "work/ram/obj_CC11C", Obj_CC11C_Tick, rom);
    bad += run("Obj 0x0EB2B0", "work/ram/obj_EB2B0", Obj_EB2B0_Tick, rom);
    bad += run("Obj 0x0EB2EA", "work/ram/obj_EB2EA", Obj_EB2EA_Tick, rom);
    bad += run("Obj 0x0EEB76", "work/ram/obj_EEB76", Obj_EEB76_Tick, rom);
    bad += run("Obj 0x0EB2BC", "work/ram/obj_EB2BC", Obj_EB2BC_Tick, rom);
    bad += run("Obj 0x0EEBB0", "work/ram/obj_EEBB0", Obj_EEBB0_Tick, rom);
    bad += run("Obj 0x0EBA0E", "work/ram/obj_EBA0E", Obj_EBA0E_Tick, rom);
    bad += run("Obj 0x114092", "work/ram/obj_114092", Obj_114092_Tick, rom);
    bad += run("Obj 0x0EEB82", "work/ram/obj_EEB82", Obj_EEB82_Tick, rom);
    bad += run("Obj 0x17A8E0", "work/ram/obj_17A8E0", Obj_17A8E0_Tick, rom);
    bad += run("Obj 0x18764E", "work/ram/obj_18764E", Obj_18764E_Tick, rom);
    bad += run("Obj 0x17AAFE", "work/ram/obj_17AAFE", Obj_17AAFE_Tick, rom);
    bad += run("Obj 0x187938", "work/ram/obj_187938", Obj_187938_Tick, rom);
    bad += run("Obj 0x18765E", "work/ram/obj_18765E", Obj_18765E_Tick, rom);
    bad += run("Obj 0x17AAEE", "work/ram/obj_17AAEE", Obj_17AAEE_Tick, rom);
    bad += run("Obj 0x17B05E", "work/ram/obj_17B05E", Obj_17B05E_Tick, rom);
    bad += run("Obj 0x17A8D0", "work/ram/obj_17A8D0", Obj_17A8D0_Tick, rom);
    bad += run("Obj 0x17B056", "work/ram/obj_17B056", Obj_17B056_Tick, rom);
    bad += run("Obj 0x17C05E", "work/ram/obj_17C05E", Obj_17C05E_Tick, rom);
    bad += run("Obj 0x17E05A", "work/ram/obj_17E05A", Obj_17E05A_Tick, rom);
    bad += run("Obj 0x184184", "work/ram/obj_184184", Obj_184184_Tick, rom);
    bad += run("Obj 0x17E108", "work/ram/obj_17E108", Obj_17E108_Tick, rom);
    bad += run("Obj 0x17E000", "work/ram/obj_17E000", Obj_17E000_Tick, rom);
    bad += run("Obj 0x17E6B6", "work/ram/obj_17E6B6", Obj_17E6B6_Tick, rom);
    bad += run("Obj 0x0D76F4", "work/ram/obj_D76F4", Obj_D76F4_Tick, rom);
    bad += run("Obj 0x0D77C4", "work/ram/obj_D77C4", Obj_D77C4_Tick, rom);
    bad += run("Obj 0x0CD9F6", "work/ram/obj_CD9F6", Obj_CD9F6_Tick, rom);
    bad += run("Obj 0x0CD956", "work/ram/obj_CD956", Obj_CD956_Tick, rom);
    bad += run("Obj 0x0CDAC8", "work/ram/obj_CDAC8", Obj_CDAC8_Tick, rom);
    bad += run("Obj 0x0D78C0", "work/ram/obj_D78C0", Obj_D78C0_Tick, rom);
    bad += run("Obj 0x0CDC1E", "work/ram/obj_CDC1E", Obj_CDC1E_Tick, rom);
    bad += run("Obj 0x126B10", "work/ram/obj_126B10", Obj_126B10_Tick, rom);
    bad += run("Obj 0x126B3E", "work/ram/obj_126B3E", Obj_126B3E_Tick, rom);
    bad += run("Obj 0x0CEAA4", "work/ram/obj_CEAA4", Obj_CEAA4_Tick, rom);
    bad += run("Obj 0x0CEB94", "work/ram/obj_CEB94", Obj_CEB94_Tick, rom);
    bad += run("Obj 0x0CEB3E", "work/ram/obj_CEB3E", Obj_CEB3E_Tick, rom);
    bad += run("Obj 0x0CE9FC", "work/ram/obj_CE9FC", Obj_CE9FC_Tick, rom);
    bad += run("Obj 0x0E4B02", "work/ram/obj_E4B02", Obj_E4B02_Tick, rom);
    bad += run("Obj 0x0DE214", "work/ram/obj_DE214", Obj_DE214_Tick, rom);
    bad += run("Obj 0x0DE8B0", "work/ram/obj_DE8B0", Obj_DE8B0_Tick, rom);
    bad += run("Obj 0x0DE794", "work/ram/obj_DE794", Obj_DE794_Tick, rom);
    bad += run("Obj 0x0DE7E0", "work/ram/obj_DE7E0", Obj_DE7E0_Tick, rom);
    bad += run("Obj 0x0DDAE0", "work/ram/obj_DDAE0", Obj_DDAE0_Tick, rom);
    bad += run("Obj 0x0DE86A", "work/ram/obj_DE86A", Obj_DE86A_Tick, rom);
    bad += run("Obj 0x0F28EA", "work/ram/obj_F28EA", Obj_F28EA_Tick, rom);
    bad += run("Obj 0x0DE0B6", "work/ram/obj_DE0B6", Obj_DE0B6_Tick, rom);
    bad += run("Obj 0x0F299C", "work/ram/obj_F299C", Obj_F299C_Tick, rom);
    bad += run("Obj 0x0D7A86", "work/ram/obj_D7A86", Obj_D7A86_Tick, rom);
    bad += run("Obj 0x0DC77A", "work/ram/obj_DC77A", Obj_DC77A_Tick, rom);
    bad += run("Obj 0x0E1BAE", "work/ram/obj_E1BAE", Obj_E1BAE_Tick, rom);
    bad += run("Obj 0x0CEF44", "work/ram/obj_CEF44", Obj_CEF44_Tick, rom);
    bad += run("Obj 0x0DE18C", "work/ram/obj_DE18C", Obj_DE18C_Tick, rom);
    bad += run("Obj 0x0E2288", "work/ram/obj_E2288", Obj_E2288_Tick, rom);
    bad += run("Obj 0x0CEF4C", "work/ram/obj_CEF4C", Obj_CEF4C_Tick, rom);
    bad += run("Obj 0x0DE140", "work/ram/obj_DE140", Obj_DE140_Tick, rom);
    bad += run("Obj 0x0CEF54", "work/ram/obj_CEF54", Obj_CEF54_Tick, rom);
    bad += run("Obj 0x0E2376", "work/ram/obj_E2376", Obj_E2376_Tick, rom);
    bad += run("Obj 0x0DE064", "work/ram/obj_DE064", Obj_DE064_Tick, rom);
    bad += run("Obj 0x0EBA96", "work/ram/obj_EBA96", Obj_EBA96_Tick, rom);
    bad += run("Obj 0x0EBB4A", "work/ram/obj_EBB4A", Obj_EBB4A_Tick, rom);
    bad += run("Obj 0x0E2170", "work/ram/obj_E2170", Obj_E2170_Tick, rom);
    bad += run("Obj 0x0F2964", "work/ram/obj_F2964", Obj_F2964_Tick, rom);
    bad += run("Obj 0x1138BC", "work/ram/obj_1138BC", Obj_1138BC_Tick, rom);
    bad += run("Obj 0x0E226C", "work/ram/obj_E226C", Obj_E226C_Tick, rom);
    bad += run("Obj 0x0E2318", "work/ram/obj_E2318", Obj_E2318_Tick, rom);
    bad += run("Obj 0x113BC2", "work/ram/obj_113BC2", Obj_113BC2_Tick, rom);
    bad += run("Obj 0x0FF66C", "work/ram/obj_FF66C", Obj_FF66C_Tick, rom);
    bad += run("Obj 0x0FF990", "work/ram/obj_FF990", Obj_FF990_Tick, rom);
    bad += run("Obj 0x0E2124", "work/ram/obj_E2124", Obj_E2124_Tick, rom);
    bad += run("Obj 0x0EBB12", "work/ram/obj_EBB12", Obj_EBB12_Tick, rom);
    bad += run("Obj 0x119062", "work/ram/obj_119062", Obj_119062_Tick, rom);
    bad += run("Obj 0x119090", "work/ram/obj_119090", Obj_119090_Tick, rom);
    bad += run("Obj 0x0DA8B4", "work/ram/obj_DA8B4", Obj_DA8B4_Tick, rom);
    bad += run("Obj 0x119D06", "work/ram/obj_119D06", Obj_119D06_Tick, rom);
    bad += run("Obj 0x113B74", "work/ram/obj_113B74", Obj_113B74_Tick, rom);
    bad += run("Obj 0x119176", "work/ram/obj_119176", Obj_119176_Tick, rom);
    bad += run("Obj 0x0EC65E", "work/ram/obj_EC65E", Obj_EC65E_Tick, rom);
    bad += run("Obj 0x1191A6", "work/ram/obj_1191A6", Obj_1191A6_Tick, rom);
    bad += run("Obj 0x1191FA", "work/ram/obj_1191FA", Obj_1191FA_Tick, rom);
    bad += run("Obj 0x0EC734", "work/ram/obj_EC734", Obj_EC734_Tick, rom);
    bad += run("Obj 0x119270", "work/ram/obj_119270", Obj_119270_Tick, rom);
    bad += run("Obj 0x11922A", "work/ram/obj_11922A", Obj_11922A_Tick, rom);
    bad += run("Obj 0x1192A0", "work/ram/obj_1192A0", Obj_1192A0_Tick, rom);
    bad += run("Obj 0x113A4E", "work/ram/obj_113A4E", Obj_113A4E_Tick, rom);
    bad += run("Obj 0x0DA82E", "work/ram/obj_DA82E", Obj_DA82E_Tick, rom);
    bad += run("Obj 0x1192AE", "work/ram/obj_1192AE", Obj_1192AE_Tick, rom);
    bad += run("Obj 0x1199A8", "work/ram/obj_1199A8", Obj_1199A8_Tick, rom);
    bad += run("Obj 0x119D36", "work/ram/obj_119D36", Obj_119D36_Tick, rom);
    bad += run("Obj 0x1199CC", "work/ram/obj_1199CC", Obj_1199CC_Tick, rom);
    bad += run("Obj 0x0EC6FC", "work/ram/obj_EC6FC", Obj_EC6FC_Tick, rom);
    bad += run("Obj 0x130054", "work/ram/obj_130054", Obj_130054_Tick, rom);
    bad += run("Obj 0x131414", "work/ram/obj_131414", Obj_131414_Tick, rom);
    bad += run("Obj 0x0CC05A", "work/ram/obj_CC05A", Obj_CC05A_Tick, rom);
    bad += run("Obj 0x12CB72", "work/ram/obj_12CB72", Obj_12CB72_Tick, rom);
    bad += run("Obj 0x12CC44", "work/ram/obj_12CC44", Obj_12CC44_Tick, rom);
    bad += run("Obj 0x119BBE", "work/ram/obj_119BBE", Obj_119BBE_Tick, rom);
    bad += run("Obj 0x130044", "work/ram/obj_130044", Obj_130044_Tick, rom);
    bad += run("Obj 0x119B40", "work/ram/obj_119B40", Obj_119B40_Tick, rom);
    bad += run("Obj 0x12D0E6", "work/ram/obj_12D0E6", Obj_12D0E6_Tick, rom);
    bad += run("Obj 0x131408", "work/ram/obj_131408", Obj_131408_Tick, rom);
    bad += run("Obj 0x12D10C", "work/ram/obj_12D10C", Obj_12D10C_Tick, rom);
    bad += run("Obj 0x119ACA", "work/ram/obj_119ACA", Obj_119ACA_Tick, rom);
    bad += run("Obj 0x12D140", "work/ram/obj_12D140", Obj_12D140_Tick, rom);
    bad += run("Obj 0x12D17E", "work/ram/obj_12D17E", Obj_12D17E_Tick, rom);
    bad += run("Obj 0x118EDE", "work/ram/obj_118EDE", Obj_118EDE_Tick, rom);
    bad += run("Obj 0x12D1E0", "work/ram/obj_12D1E0", Obj_12D1E0_Tick, rom);
    bad += run("Obj 0x118E88", "work/ram/obj_118E88", Obj_118E88_Tick, rom);
    bad += run("Obj 0x1302F8", "work/ram/obj_1302F8", Obj_1302F8_Tick, rom);
    bad += run_d3("Obj 0x130312", "work/ram/obj_130312", Obj_130312_TickD3, rom);
    bad += run("Obj 0x12D1C6", "work/ram/obj_12D1C6", Obj_12D1C6_Tick, rom);
    bad += run("Obj 0x12D2B4", "work/ram/obj_12D2B4", Obj_12D2B4_Tick, rom);
    bad += run("Obj 0x0CCE6E", "work/ram/obj_CCE6E", Obj_CCE6E_Tick, rom);
    bad += run("Obj 0x0CD256", "work/ram/obj_CD256", Obj_CD256_Tick, rom);
    bad += run("Obj 0x12D2D4", "work/ram/obj_12D2D4", Obj_12D2D4_Tick, rom);
    bad += run("Obj 0x12D244", "work/ram/obj_12D244", Obj_12D244_Tick, rom);
    bad += run("Obj 0x0CD412", "work/ram/obj_CD412", Obj_CD412_Tick, rom);
    bad += run("Obj 0x12D22C", "work/ram/obj_12D22C", Obj_12D22C_Tick, rom);
    bad += run("Obj 0x12D284", "work/ram/obj_12D284", Obj_12D284_Tick, rom);
    bad += run("Obj 0x12D36C", "work/ram/obj_12D36C", Obj_12D36C_Tick, rom);
    bad += run("Obj 0x1303AC", "work/ram/obj_1303AC", Obj_1303AC_Tick, rom);
    bad += run("Obj 0x1307E0", "work/ram/obj_1307E0", Obj_1307E0_Tick, rom);
    bad += run("Obj 0x130794", "work/ram/obj_130794", Obj_130794_Tick, rom);
    bad += run("Obj 0x12D2FC", "work/ram/obj_12D2FC", Obj_12D2FC_Tick, rom);
    bad += run("Obj 0x12D31A", "work/ram/obj_12D31A", Obj_12D31A_Tick, rom);
    bad += run("Obj 0x12D350", "work/ram/obj_12D350", Obj_12D350_Tick, rom);
    bad += run("Obj 0x12D382", "work/ram/obj_12D382", Obj_12D382_Tick, rom);
    bad += run("Obj 0x1304E2", "work/ram/obj_1304E2", Obj_1304E2_Tick, rom);
    bad += run("Obj 0x130360", "work/ram/obj_130360", Obj_130360_Tick, rom);
    bad += run("Obj 0x130482", "work/ram/obj_130482", Obj_130482_Tick, rom);
    bad += run("Obj 0x12D3FA", "work/ram/obj_12D3FA", Obj_12D3FA_Tick, rom);
    bad += run("Obj 0x130546", "work/ram/obj_130546", Obj_130546_Tick, rom);
    bad += run("Obj 0x1302B8", "work/ram/obj_1302B8", Obj_1302B8_Tick, rom);
    bad += run("Obj 0x12D414", "work/ram/obj_12D414", Obj_12D414_Tick, rom);
    bad += run("Obj 0x12D446", "work/ram/obj_12D446", Obj_12D446_Tick, rom);
    bad += run("Obj 0x12D39C", "work/ram/obj_12D39C", Obj_12D39C_Tick, rom);
    bad += run("Obj 0x13157A", "work/ram/obj_13157A", Obj_13157A_Tick, rom);
    bad += run("Obj 0x13029E", "work/ram/obj_13029E", Obj_13029E_Tick, rom);
    bad += run("Obj 0x13151E", "work/ram/obj_13151E", Obj_13151E_Tick, rom);
    bad += run("Obj 0x1314C2", "work/ram/obj_1314C2", Obj_1314C2_Tick, rom);
    bad += run("Obj 0x12D466", "work/ram/obj_12D466", Obj_12D466_Tick, rom);
    bad += run("Obj 0x12D4BA", "work/ram/obj_12D4BA", Obj_12D4BA_Tick, rom);
    bad += run("Obj 0x12D49E", "work/ram/obj_12D49E", Obj_12D49E_Tick, rom);
    bad += run("Obj 0x12D482", "work/ram/obj_12D482", Obj_12D482_Tick, rom);
    bad += run("Obj 0x0CCD04", "work/ram/obj_CCD04", Obj_CCD04_Tick, rom);
    bad += run("Obj 0x12D0C4", "work/ram/obj_12D0C4", Obj_12D0C4_Tick, rom);
    bad += run("Obj 0x118358", "work/ram/obj_118358", Obj_118358_Tick, rom);
    bad += run("Obj 0x131616", "work/ram/obj_131616", Obj_131616_Tick, rom);
    bad += run("Obj 0x13DEF0", "work/ram/obj_13DEF0", Obj_13DEF0_Tick, rom);
    bad += run("Obj 0x12D0AE", "work/ram/obj_12D0AE", Obj_12D0AE_Tick, rom);
    bad += run("Obj 0x1186C4", "work/ram/obj_1186C4", Obj_1186C4_Tick, rom);
    bad += run("Obj 0x13DF4C", "work/ram/obj_13DF4C", Obj_13DF4C_Tick, rom);
    bad += run("Obj 0x118602", "work/ram/obj_118602", Obj_118602_Tick, rom);
    bad += run("Obj 0x118486", "work/ram/obj_118486", Obj_118486_Tick, rom);
    bad += run("Obj 0x13DE92", "work/ram/obj_13DE92", Obj_13DE92_Tick, rom);
    bad += run("Obj 0x118494", "work/ram/obj_118494", Obj_118494_Tick, rom);
    bad += run("Obj 0x119360", "work/ram/obj_119360", Obj_119360_Tick, rom);
    bad += run("Obj 0x1187DE", "work/ram/obj_1187DE", Obj_1187DE_Tick, rom);
    bad += run("Obj 0x1193E2", "work/ram/obj_1193E2", Obj_1193E2_Tick, rom);
    bad += run("Obj 0x13DEAE", "work/ram/obj_13DEAE", Obj_13DEAE_Tick, rom);
    bad += run("Obj 0x119422", "work/ram/obj_119422", Obj_119422_Tick, rom);
    bad += run("Obj 0x11946C", "work/ram/obj_11946C", Obj_11946C_Tick, rom);
    bad += run("Obj 0x1194C2", "work/ram/obj_1194C2", Obj_1194C2_Tick, rom);
    bad += run("Obj 0x11949A", "work/ram/obj_11949A", Obj_11949A_Tick, rom);
    bad += run("Obj 0x13E12C", "work/ram/obj_13E12C", Obj_13E12C_Tick, rom);
    bad += run("Obj 0x0ED156", "work/ram/obj_ED156", Obj_ED156_Tick, rom);
    bad += run("Obj 0x13DF5A", "work/ram/obj_13DF5A", Obj_13DF5A_Tick, rom);
    bad += run("Obj 0x13E34C", "work/ram/obj_13E34C", Obj_13E34C_Tick, rom);
    bad += run("Obj 0x13E150", "work/ram/obj_13E150", Obj_13E150_Tick, rom);
    bad += run("Obj 0x186388", "work/ram/obj_186388", Obj_186388_Tick, rom);
    bad += run("Obj 0x1867A4", "work/ram/obj_1867A4", Obj_1867A4_Tick, rom);
    bad += run("Obj 0x0ED124", "work/ram/obj_ED124", Obj_ED124_Tick, rom);
    bad += run("Obj 0x11418E", "work/ram/obj_11418E", Obj_11418E_Tick, rom);
    bad += run("Obj 0x184A20", "work/ram/obj_184A20", Obj_184A20_Tick, rom);
    bad += run("Obj 0x184A10", "work/ram/obj_184A10", Obj_184A10_Tick, rom);
    bad += run("Obj 0x11423E", "work/ram/obj_11423E", Obj_11423E_Tick, rom);
    bad += run("Obj 0x114212", "work/ram/obj_114212", Obj_114212_Tick, rom);
    bad += run("Obj 0x1143C8", "work/ram/obj_1143C8", Obj_1143C8_Tick, rom);
    bad += run("Obj 0x186326", "work/ram/obj_186326", Obj_186326_Tick, rom);
    bad += run("Obj 0x186782", "work/ram/obj_186782", Obj_186782_Tick, rom);
    bad += run("Obj 0x184D6C", "work/ram/obj_184D6C", Obj_184D6C_Tick, rom);
    bad += run("Obj 0x186482", "work/ram/obj_186482", Obj_186482_Tick, rom);
    bad += run("Obj 0x184BAE", "work/ram/obj_184BAE", Obj_184BAE_Tick, rom);
    bad += run("Obj 0x186406", "work/ram/obj_186406", Obj_186406_Tick, rom);
    bad += run("Obj 0x17A688", "work/ram/obj_17A688", Obj_17A688_Tick, rom);
    bad += run("Obj 0x18724E", "work/ram/obj_18724E", Obj_18724E_Tick, rom);
    bad += run("Obj 0x18702C", "work/ram/obj_18702C", Obj_18702C_Tick, rom);
    bad += run("Obj 0x1865A0", "work/ram/obj_1865A0", Obj_1865A0_Tick, rom);
    bad += run("Obj 0x187370", "work/ram/obj_187370", Obj_187370_Tick, rom);
    bad += run("Obj 0x1873D6", "work/ram/obj_1873D6", Obj_1873D6_Tick, rom);
    bad += run("Obj 0x17DAA0", "work/ram/obj_17DAA0", Obj_17DAA0_Tick, rom);
    bad += run("Obj 0x18747C", "work/ram/obj_18747C", Obj_18747C_Tick, rom);
    bad += run("Obj 0x187508", "work/ram/obj_187508", Obj_187508_Tick, rom);
    bad += run("Obj 0x118D12", "work/ram/obj_118D12", Obj_118D12_Tick, rom);
    bad += run("Obj 0x17A070", "work/ram/obj_17A070", Obj_17A070_Tick, rom);
    bad += run("Obj 0x17A664", "work/ram/obj_17A664", Obj_17A664_Tick, rom);
    bad += run("Obj 0x17C14C", "work/ram/obj_17C14C", Obj_17C14C_Tick, rom);
    bad += run("Obj 0x17C8CC", "work/ram/obj_17C8CC", Obj_17C8CC_Tick, rom);
    bad += run("Obj 0x17C660", "work/ram/obj_17C660", Obj_17C660_Tick, rom);
    bad += run("Obj 0x179C6E", "work/ram/obj_179C6E", Obj_179C6E_Tick, rom);
    bad += run("Obj 0x18657E", "work/ram/obj_18657E", Obj_18657E_Tick, rom);
    bad += run("Obj 0x17A3F4", "work/ram/obj_17A3F4", Obj_17A3F4_Tick, rom);
    bad += run("Obj 0x17A420", "work/ram/obj_17A420", Obj_17A420_Tick, rom);
    bad += run("Obj 0x187246", "work/ram/obj_187246", Obj_187246_Tick, rom);
    bad += run("Obj 0x17C9D0", "work/ram/obj_17C9D0", Obj_17C9D0_Tick, rom);
    bad += run("Obj 0x187024", "work/ram/obj_187024", Obj_187024_Tick, rom);
    bad += run("Obj 0x17BE1C", "work/ram/obj_17BE1C", Obj_17BE1C_Tick, rom);
    bad += run("Obj 0x17A016", "work/ram/obj_17A016", Obj_17A016_Tick, rom);
    bad += run("Obj 0x17D136", "work/ram/obj_17D136", Obj_17D136_Tick, rom);
    bad += run("Obj 0x17D912", "work/ram/obj_17D912", Obj_17D912_Tick, rom);
    bad += run("Obj 0x17D186", "work/ram/obj_17D186", Obj_17D186_Tick, rom);
    bad += run("Obj 0x17D586", "work/ram/obj_17D586", Obj_17D586_Tick, rom);
    bad += run("Obj 0x17A122", "work/ram/obj_17A122", Obj_17A122_Tick, rom);
    bad += run("Obj 0x18415A", "work/ram/obj_18415A", Obj_18415A_Tick, rom);
    bad += run("Obj 0x17A52A", "work/ram/obj_17A52A", Obj_17A52A_Tick, rom);
    bad += run("Obj 0x17D4F4", "work/ram/obj_17D4F4", Obj_17D4F4_Tick, rom);
    bad += run("Obj 0x17DDD0", "work/ram/obj_17DDD0", Obj_17DDD0_Tick, rom);
    bad += run("Obj 0x17D640", "work/ram/obj_17D640", Obj_17D640_Tick, rom);
    bad += run("Obj 0x179E7A", "work/ram/obj_179E7A", Obj_179E7A_Tick, rom);
    bad += run("Obj 0x17DE20", "work/ram/obj_17DE20", Obj_17DE20_Tick, rom);
    bad += run("Obj 0x17A50E", "work/ram/obj_17A50E", Obj_17A50E_Tick, rom);
    bad += run("Obj 0x185C18", "work/ram/obj_185C18", Obj_185C18_Tick, rom);
    bad += run("Obj 0x1860AA", "work/ram/obj_1860AA", Obj_1860AA_Tick, rom);
    bad += run("Obj 0x17D66E", "work/ram/obj_17D66E", Obj_17D66E_Tick, rom);
    bad += run("Obj 0x17DB68", "work/ram/obj_17DB68", Obj_17DB68_Tick, rom);
    bad += run("Obj 0x17D72C", "work/ram/obj_17D72C", Obj_17D72C_Tick, rom);
    bad += run("Obj 0x17D6C8", "work/ram/obj_17D6C8", Obj_17D6C8_Tick, rom);
    bad += run("Obj 0x17DC1E", "work/ram/obj_17DC1E", Obj_17DC1E_Tick, rom);
    bad += run("Obj 0x17CDA6", "work/ram/obj_17CDA6", Obj_17CDA6_Tick, rom);
    bad += run("Obj 0x179DEE", "work/ram/obj_179DEE", Obj_179DEE_Tick, rom);
    bad += run("Obj 0x17CE04", "work/ram/obj_17CE04", Obj_17CE04_Tick, rom);
    bad += run("Obj 0x184EDE", "work/ram/obj_184EDE", Obj_184EDE_Tick, rom);
    bad += run("Obj 0x17DBB8", "work/ram/obj_17DBB8", Obj_17DBB8_Tick, rom);
    bad += run("Obj 0x186064", "work/ram/obj_186064", Obj_186064_Tick, rom);
    bad += run("Obj 0x179C26", "work/ram/obj_179C26", Obj_179C26_Tick, rom);
    bad += run("Obj 0x17CE88", "work/ram/obj_17CE88", Obj_17CE88_Tick, rom);
    bad += run("Obj 0x17A42A", "work/ram/obj_17A42A", Obj_17A42A_Tick, rom);
    bad += run("Obj 0x18675E", "work/ram/obj_18675E", Obj_18675E_Tick, rom);
    bad += run("Obj 0x17F038", "work/ram/obj_17F038", Obj_17F038_Tick, rom);
    bad += run("Obj 0x17D3E6", "work/ram/obj_17D3E6", Obj_17D3E6_Tick, rom);
    bad += run("Obj 0x17C240", "work/ram/obj_17C240", Obj_17C240_Tick, rom);
    bad += run("Obj 0x17C508", "work/ram/obj_17C508", Obj_17C508_Tick, rom);
    bad += run("Obj 0x17C360", "work/ram/obj_17C360", Obj_17C360_Tick, rom);
    bad += run("Obj 0x11BCB2", "work/ram/obj_11BCB2", Obj_11BCB2_Tick, rom);
    bad += run("Obj 0x11BDAA", "work/ram/obj_11BDAA", Obj_11BDAA_Tick, rom);
    bad += run("Obj 0x11BD48", "work/ram/obj_11BD48", Obj_11BD48_Tick, rom);
    bad += run("Obj 0x17F006", "work/ram/obj_17F006", Obj_17F006_Tick, rom);
    bad += run("Obj 0x17C4D4", "work/ram/obj_17C4D4", Obj_17C4D4_Tick, rom);
    bad += run("Obj 0x11BE14", "work/ram/obj_11BE14", Obj_11BE14_Tick, rom);
    bad += run("Obj 0x17D394", "work/ram/obj_17D394", Obj_17D394_Tick, rom);
    bad += run("Obj 0x17A5C4", "work/ram/obj_17A5C4", Obj_17A5C4_Tick, rom);
    bad += run("Obj 0x13EA38", "work/ram/obj_13EA38", Obj_13EA38_Tick, rom);
    bad += run("Obj 0x13E9E8", "work/ram/obj_13E9E8", Obj_13E9E8_Tick, rom);
    bad += run("Obj 0x17DD46", "work/ram/obj_17DD46", Obj_17DD46_Tick, rom);
    bad += run("Obj 0x11BD96", "work/ram/obj_11BD96", Obj_11BD96_Tick, rom);
    bad += run("Obj 0x17A5A8", "work/ram/obj_17A5A8", Obj_17A5A8_Tick, rom);
    bad += run("Obj 0x13EB4C", "work/ram/obj_13EB4C", Obj_13EB4C_Tick, rom);
    bad += run("Obj 0x11BEF6", "work/ram/obj_11BEF6", Obj_11BEF6_Tick, rom);
    bad += run("Obj 0x13EA54", "work/ram/obj_13EA54", Obj_13EA54_Tick, rom);
    bad += run("Obj 0x179DCC", "work/ram/obj_179DCC", Obj_179DCC_Tick, rom);
    bad += run("Obj 0x17AE0A", "work/ram/obj_17AE0A", Obj_17AE0A_Tick, rom);
    bad += run("Obj 0x184DA8", "work/ram/obj_184DA8", Obj_184DA8_Tick, rom);
    bad += run("Obj 0x0E66F8", "work/ram/obj_E66F8", Obj_E66F8_Tick, rom);
    bad += run("Obj 0x0E739E", "work/ram/obj_E739E", Obj_E739E_Tick, rom);
    bad += run("Obj 0x0E7FC6", "work/ram/obj_E7FC6", Obj_E7FC6_Tick, rom);
    bad += run("Obj 0x0E800C", "work/ram/obj_E800C", Obj_E800C_Tick, rom);
    bad += run("Obj 0x0E66B2", "work/ram/obj_E66B2", Obj_E66B2_Tick, rom);
    bad += run("Obj 0x0F13F4", "work/ram/obj_F13F4", Obj_F13F4_Tick, rom);
    bad += run("Obj 0x0E735A", "work/ram/obj_E735A", Obj_E735A_Tick, rom);
    bad += run("Obj 0x0F1400", "work/ram/obj_F1400", Obj_F1400_Tick, rom);
    bad += run("Obj 0x0F1432", "work/ram/obj_F1432", Obj_F1432_Tick, rom);
    bad += run("Obj 0x0F1814", "work/ram/obj_F1814", Obj_F1814_Tick, rom);
    bad += run("Obj 0x0F40DE", "work/ram/obj_F40DE", Obj_F40DE_Tick, rom);
    bad += run("Obj 0x0F40EA", "work/ram/obj_F40EA", Obj_F40EA_Tick, rom);
    bad += run("Obj 0x0F4112", "work/ram/obj_F4112", Obj_F4112_Tick, rom);
    bad += run("Obj 0x0EDBFE", "work/ram/obj_EDBFE", Obj_EDBFE_Tick, rom);
    bad += run("Obj 0x0EA8EE", "work/ram/obj_EA8EE", Obj_EA8EE_Tick, rom);
    bad += run("Obj 0x0EDC38", "work/ram/obj_EDC38", Obj_EDC38_Tick, rom);
    bad += run("Obj 0x0EDC0A", "work/ram/obj_EDC0A", Obj_EDC0A_Tick, rom);
    bad += run("Obj 0x0EE0A4", "work/ram/obj_EE0A4", Obj_EE0A4_Tick, rom);
    bad += run("Obj 0x0EE076", "work/ram/obj_EE076", Obj_EE076_Tick, rom);
    bad += run("Obj 0x0E8DFE", "work/ram/obj_E8DFE", Obj_E8DFE_Tick, rom);
    bad += run("Obj 0x0E9426", "work/ram/obj_E9426", Obj_E9426_Tick, rom);
    bad += run("Obj 0x0E940A", "work/ram/obj_E940A", Obj_E940A_Tick, rom);
    bad += run("Obj 0x0E949E", "work/ram/obj_E949E", Obj_E949E_Tick, rom);
    bad += run("Obj 0x0E94BC", "work/ram/obj_E94BC", Obj_E94BC_Tick, rom);
    bad += run("Obj 0x0D5EE8", "work/ram/obj_D5EE8", Obj_D5EE8_Tick, rom);
    bad += run("Obj 0x0ED76E", "work/ram/obj_ED76E", Obj_ED76E_Tick, rom);
    bad += run("Obj 0x0ED7AC", "work/ram/obj_ED7AC", Obj_ED7AC_Tick, rom);
    bad += run("Obj 0x0ED77A", "work/ram/obj_ED77A", Obj_ED77A_Tick, rom);
    bad += run("Obj 0x0DD314", "work/ram/obj_DD314", Obj_DD314_Tick, rom);
    bad += run("Obj 0x0E9556", "work/ram/obj_E9556", Obj_E9556_Tick, rom);
    bad += run("Obj 0x0E953A", "work/ram/obj_E953A", Obj_E953A_Tick, rom);
    bad += run("Obj 0x0D6BD8", "work/ram/obj_D6BD8", Obj_D6BD8_Tick, rom);
    bad += run("Obj 0x0D5FFE", "work/ram/obj_D5FFE", Obj_D5FFE_Tick, rom);
    bad += run("Obj 0x11AD46", "work/ram/obj_11AD46", Obj_11AD46_Tick, rom);
    bad += run("Obj 0x0D6CEC", "work/ram/obj_D6CEC", Obj_D6CEC_Tick, rom);
    bad += run("Obj 0x0D6DCC", "work/ram/obj_D6DCC", Obj_D6DCC_Tick, rom);
    bad += run("Obj 0x0EF8F8", "work/ram/obj_EF8F8", Obj_EF8F8_Tick, rom);
    bad += run("Obj 0x0D6E98", "work/ram/obj_D6E98", Obj_D6E98_Tick, rom);
    bad += run("Obj 0x0EF938", "work/ram/obj_EF938", Obj_EF938_Tick, rom);
    bad += run("Obj 0x184170", "work/ram/obj_184170", Obj_184170_Tick, rom);
    bad += run("Obj 0x0DD6FC", "work/ram/obj_DD6FC", Obj_DD6FC_Tick, rom);
    bad += run("Obj 0x0EF904", "work/ram/obj_EF904", Obj_EF904_Tick, rom);
    bad += run("Obj 0x17AD56", "work/ram/obj_17AD56", Obj_17AD56_Tick, rom);
    bad += run("Obj 0x17AD18", "work/ram/obj_17AD18", Obj_17AD18_Tick, rom);
    bad += run("Obj 0x179D26", "work/ram/obj_179D26", Obj_179D26_Tick, rom);
    bad += run("Obj 0x17E1DA", "work/ram/obj_17E1DA", Obj_17E1DA_Tick, rom);
    bad += run("Obj 0x0DC43E", "work/ram/obj_DC43E", Obj_DC43E_Tick, rom);
    bad += run("Obj 0x17E188", "work/ram/obj_17E188", Obj_17E188_Tick, rom);
    bad += run("Obj 0x0C99B8", "work/ram/obj_C99B8", Obj_C99B8_Tick, rom);
    bad += run("Obj 0x0D70A0", "work/ram/obj_D70A0", Obj_D70A0_Tick, rom);
    bad += run("Obj 0x0F7BD2", "work/ram/obj_F7BD2", Obj_F7BD2_Tick, rom);
    bad += run("Obj 0x0F7CDC", "work/ram/obj_F7CDC", Obj_F7CDC_Tick, rom);
    bad += run("Obj 0x11AA5E", "work/ram/obj_11AA5E", Obj_11AA5E_Tick, rom);
    bad += run("Obj 0x0F7C48", "work/ram/obj_F7C48", Obj_F7C48_Tick, rom);
    bad += run("Obj 0x14C9FE", "work/ram/obj_14C9FE", Obj_14C9FE_Tick, rom);
    bad += run("Obj 0x14CAFA", "work/ram/obj_14CAFA", Obj_14CAFA_Tick, rom);
    bad += run("Obj 0x17ABC8", "work/ram/obj_17ABC8", Obj_17ABC8_Tick, rom);
    bad += run("Obj 0x14CC4C", "work/ram/obj_14CC4C", Obj_14CC4C_Tick, rom);
    bad += run("Obj 0x101A6C", "work/ram/obj_101A6C", Obj_101A6C_Tick, rom);
    bad += run("Obj 0x187986", "work/ram/obj_187986", Obj_187986_Tick, rom);
    bad += run("Obj 0x14C8F6", "work/ram/obj_14C8F6", Obj_14C8F6_Tick, rom);
    bad += run("Obj 0x101BB6", "work/ram/obj_101BB6", Obj_101BB6_Tick, rom);
    bad += run("Obj 0x14CC12", "work/ram/obj_14CC12", Obj_14CC12_Tick, rom);
    bad += run("Obj 0x14CC8C", "work/ram/obj_14CC8C", Obj_14CC8C_Tick, rom);
    bad += run("Obj 0x109AEC", "work/ram/obj_109AEC", Obj_109AEC_Tick, rom);
    bad += run("Obj 0x14CD0C", "work/ram/obj_14CD0C", Obj_14CD0C_Tick, rom);
    bad += run("Obj 0x14CCCC", "work/ram/obj_14CCCC", Obj_14CCCC_Tick, rom);
    bad += run("Obj 0x101B20", "work/ram/obj_101B20", Obj_101B20_Tick, rom);
    bad += run("Obj 0x109BB6", "work/ram/obj_109BB6", Obj_109BB6_Tick, rom);
    bad += run("Obj 0x14CD4C", "work/ram/obj_14CD4C", Obj_14CD4C_Tick, rom);
    bad += run("Obj 0x14CD8C", "work/ram/obj_14CD8C", Obj_14CD8C_Tick, rom);
    bad += run("Obj 0x14CB12", "work/ram/obj_14CB12", Obj_14CB12_Tick, rom);
    bad += run("Obj 0x109B7E", "work/ram/obj_109B7E", Obj_109B7E_Tick, rom);
    bad += run("Obj 0x11AB3E", "work/ram/obj_11AB3E", Obj_11AB3E_Tick, rom);
    bad += run("Obj 0x14CDCC", "work/ram/obj_14CDCC", Obj_14CDCC_Tick, rom);
    bad += run("Obj 0x0FE0AA", "work/ram/obj_FE0AA", Obj_FE0AA_Tick, rom);
    bad += run("Obj 0x0FE724", "work/ram/obj_FE724", Obj_FE724_Tick, rom);
    bad += run("Obj 0x11AAB8", "work/ram/obj_11AAB8", Obj_11AAB8_Tick, rom);
    bad += run("Obj 0x14CB96", "work/ram/obj_14CB96", Obj_14CB96_Tick, rom);
    bad += run("Obj 0x0FE5BA", "work/ram/obj_FE5BA", Obj_FE5BA_Tick, rom);
    bad += run("Obj 0x11AAF0", "work/ram/obj_11AAF0", Obj_11AAF0_Tick, rom);
    bad += run("Obj 0x0FF3E4", "work/ram/obj_FF3E4", Obj_FF3E4_Tick, rom);
    bad += run("Obj 0x11AAD4", "work/ram/obj_11AAD4", Obj_11AAD4_Tick, rom);
    bad += run("Obj 0x0FE53C", "work/ram/obj_FE53C", Obj_FE53C_Tick, rom);
    bad += run("Obj 0x11AB96", "work/ram/obj_11AB96", Obj_11AB96_Tick, rom);
    bad += run("Obj 0x0FF4E0", "work/ram/obj_FF4E0", Obj_FF4E0_Tick, rom);
    bad += run("Obj 0x0EFE92", "work/ram/obj_EFE92", Obj_EFE92_Tick, rom);
    bad += run("Obj 0x0FF4A8", "work/ram/obj_FF4A8", Obj_FF4A8_Tick, rom);
    bad += run("Obj 0x0EFF34", "work/ram/obj_EFF34", Obj_EFF34_Tick, rom);
    bad += run("Obj 0x0DBEE6", "work/ram/obj_DBEE6", Obj_DBEE6_Tick, rom);
    bad += run("Obj 0x0FEC64", "work/ram/obj_FEC64", Obj_FEC64_Tick, rom);
    bad += run("Obj 0x0FEEA0", "work/ram/obj_FEEA0", Obj_FEEA0_Tick, rom);
    bad += run("Obj 0x0DBE98", "work/ram/obj_DBE98", Obj_DBE98_Tick, rom);
    bad += run("Obj 0x0D9CE8", "work/ram/obj_D9CE8", Obj_D9CE8_Tick, rom);
    bad += run("Obj 0x0F44B4", "work/ram/obj_F44B4", Obj_F44B4_Tick, rom);
    bad += run("Obj 0x114E0E", "work/ram/obj_114E0E", Obj_114E0E_Tick, rom);
    bad += run("Obj 0x0EFEDE", "work/ram/obj_EFEDE", Obj_EFEDE_Tick, rom);
    bad += run("Obj 0x0FEE74", "work/ram/obj_FEE74", Obj_FEE74_Tick, rom);
    bad += run("Obj 0x1899B0", "work/ram/obj_1899B0", Obj_1899B0_Tick, rom);
    bad += run("Obj 0x0D9C88", "work/ram/obj_D9C88", Obj_D9C88_Tick, rom);
    bad += run("Obj 0x18A4EC", "work/ram/obj_18A4EC", Obj_18A4EC_Tick, rom);
    bad += run("Obj 0x1897BA", "work/ram/obj_1897BA", Obj_1897BA_Tick, rom);
    bad += run("Obj 0x1897AA", "work/ram/obj_1897AA", Obj_1897AA_Tick, rom);
    bad += run("Obj 0x18A4D4", "work/ram/obj_18A4D4", Obj_18A4D4_Tick, rom);
    bad += run("Obj 0x18B41C", "work/ram/obj_18B41C", Obj_18B41C_Tick, rom);
    bad += run("Obj 0x189BE2", "work/ram/obj_189BE2", Obj_189BE2_Tick, rom);
    bad += run("Obj 0x114F38", "work/ram/obj_114F38", Obj_114F38_Tick, rom);
    bad += run("Obj 0x18B2CA", "work/ram/obj_18B2CA", Obj_18B2CA_Tick, rom);
    bad += run("Obj 0x18B3CC", "work/ram/obj_18B3CC", Obj_18B3CC_Tick, rom);
    bad += run("Obj 0x18B4E8", "work/ram/obj_18B4E8", Obj_18B4E8_Tick, rom);
    bad += run("Obj 0x18B4B8", "work/ram/obj_18B4B8", Obj_18B4B8_Tick, rom);
    bad += run("Obj 0x189C1E", "work/ram/obj_189C1E", Obj_189C1E_Tick, rom);
    bad += run("Obj 0x18B230", "work/ram/obj_18B230", Obj_18B230_Tick, rom);
    bad += run("Obj 0x18A582", "work/ram/obj_18A582", Obj_18A582_Tick, rom);
    bad += run("Obj 0x189C56", "work/ram/obj_189C56", Obj_189C56_Tick, rom);
    bad += run("Obj 0x18A572", "work/ram/obj_18A572", Obj_18A572_Tick, rom);
    bad += run("Obj 0x189C7C", "work/ram/obj_189C7C", Obj_189C7C_Tick, rom);
    bad += run("Obj 0x18B3AE", "work/ram/obj_18B3AE", Obj_18B3AE_Tick, rom);
    bad += run("Obj 0x18A550", "work/ram/obj_18A550", Obj_18A550_Tick, rom);
    bad += run("Obj 0x179354", "work/ram/obj_179354", Obj_179354_Tick, rom);
    bad += run("Obj 0x189CCC", "work/ram/obj_189CCC", Obj_189CCC_Tick, rom);
    bad += run("Obj 0x17933A", "work/ram/obj_17933A", Obj_17933A_Tick, rom);
    bad += run("Obj 0x18A540", "work/ram/obj_18A540", Obj_18A540_Tick, rom);
    bad += run("Obj 0x189D8A", "work/ram/obj_189D8A", Obj_189D8A_Tick, rom);
    bad += run("Obj 0x189CE8", "work/ram/obj_189CE8", Obj_189CE8_Tick, rom);
    bad += run("Obj 0x189D2A", "work/ram/obj_189D2A", Obj_189D2A_Tick, rom);
    bad += run("Obj 0x189D20", "work/ram/obj_189D20", Obj_189D20_Tick, rom);
    bad += run("Obj 0x189E2E", "work/ram/obj_189E2E", Obj_189E2E_Tick, rom);
    bad += run("Obj 0x189E6A", "work/ram/obj_189E6A", Obj_189E6A_Tick, rom);
    bad += run("Obj 0x17F48E", "work/ram/obj_17F48E", Obj_17F48E_Tick, rom);
    bad += run("Obj 0x189FE2", "work/ram/obj_189FE2", Obj_189FE2_Tick, rom);
    bad += run("Obj 0x189EDC", "work/ram/obj_189EDC", Obj_189EDC_Tick, rom);
    bad += run("Obj 0x189EBA", "work/ram/obj_189EBA", Obj_189EBA_Tick, rom);
    bad += run("Obj 0x17F43C", "work/ram/obj_17F43C", Obj_17F43C_Tick, rom);
    bad += run("Obj 0x18A01E", "work/ram/obj_18A01E", Obj_18A01E_Tick, rom);
    bad += run("Obj 0x17EDA4", "work/ram/obj_17EDA4", Obj_17EDA4_Tick, rom);
    bad += run("Obj 0x17ED40", "work/ram/obj_17ED40", Obj_17ED40_Tick, rom);
    bad += run("Obj 0x17F260", "work/ram/obj_17F260", Obj_17F260_Tick, rom);
    bad += run("Obj 0x17EFC8", "work/ram/obj_17EFC8", Obj_17EFC8_Tick, rom);
    bad += run("Obj 0x18A12C", "work/ram/obj_18A12C", Obj_18A12C_Tick, rom);
    bad += run("Obj 0x17E99A", "work/ram/obj_17E99A", Obj_17E99A_Tick, rom);
    bad += run("Obj 0x17E9F6", "work/ram/obj_17E9F6", Obj_17E9F6_Tick, rom);
    bad += run("Obj 0x17EA7E", "work/ram/obj_17EA7E", Obj_17EA7E_Tick, rom);
    bad += run("Obj 0x17EA0A", "work/ram/obj_17EA0A", Obj_17EA0A_Tick, rom);
    bad += run("Obj 0x18A06E", "work/ram/obj_18A06E", Obj_18A06E_Tick, rom);
    bad += run("Obj 0x18A08A", "work/ram/obj_18A08A", Obj_18A08A_Tick, rom);
    bad += run("Obj 0x17E84E", "work/ram/obj_17E84E", Obj_17E84E_Tick, rom);
    bad += run("Obj 0x18B9DE", "work/ram/obj_18B9DE", Obj_18B9DE_Tick, rom);
    bad += run("Obj 0x17E89E", "work/ram/obj_17E89E", Obj_17E89E_Tick, rom);
    bad += run("Obj 0x18A0C2", "work/ram/obj_18A0C2", Obj_18A0C2_Tick, rom);
    bad += run("Obj 0x17AADA", "work/ram/obj_17AADA", Obj_17AADA_Tick, rom);
    bad += run("Obj 0x18A0CC", "work/ram/obj_18A0CC", Obj_18A0CC_Tick, rom);
    bad += run("Obj 0x14D86E", "work/ram/obj_14D86E", Obj_14D86E_Tick, rom);
    bad += run("Obj 0x18A20C", "work/ram/obj_18A20C", Obj_18A20C_Tick, rom);
    bad += run("Obj 0x18A1D0", "work/ram/obj_18A1D0", Obj_18A1D0_Tick, rom);
    bad += run("Obj 0x18A25C", "work/ram/obj_18A25C", Obj_18A25C_Tick, rom);
    bad += run("Obj 0x18A27E", "work/ram/obj_18A27E", Obj_18A27E_Tick, rom);
    bad += run("Obj 0x183F18", "work/ram/obj_183F18", Obj_183F18_Tick, rom);
    bad += run("Obj 0x183CF6", "work/ram/obj_183CF6", Obj_183CF6_Tick, rom);
    bad += run("Obj 0x18A474", "work/ram/obj_18A474", Obj_18A474_Tick, rom);
    bad += run("Obj 0x14D81A", "work/ram/obj_14D81A", Obj_14D81A_Tick, rom);
    bad += run("Obj 0x18B9CE", "work/ram/obj_18B9CE", Obj_18B9CE_Tick, rom);
    bad += run("Obj 0x18A4CA", "work/ram/obj_18A4CA", Obj_18A4CA_Tick, rom);
    bad += run("Obj 0x17F812", "work/ram/obj_17F812", Obj_17F812_Tick, rom);
    bad += run("Obj 0x183CBA", "work/ram/obj_183CBA", Obj_183CBA_Tick, rom);
    bad += run("Obj 0x17F7CE", "work/ram/obj_17F7CE", Obj_17F7CE_Tick, rom);
    bad += run("Obj 0x17F5DA", "work/ram/obj_17F5DA", Obj_17F5DA_Tick, rom);
    bad += run("Obj 0x17F6C8", "work/ram/obj_17F6C8", Obj_17F6C8_Tick, rom);
    bad += run("Obj 0x183DA6", "work/ram/obj_183DA6", Obj_183DA6_Tick, rom);
    bad += run("Obj 0x17F8FC", "work/ram/obj_17F8FC", Obj_17F8FC_Tick, rom);
    bad += run("Obj 0x183EDA", "work/ram/obj_183EDA", Obj_183EDA_Tick, rom);
    bad += run("Obj 0x17F74E", "work/ram/obj_17F74E", Obj_17F74E_Tick, rom);
    bad += run("Obj 0x183FB0", "work/ram/obj_183FB0", Obj_183FB0_Tick, rom);
    bad += run("Obj 0x17AA36", "work/ram/obj_17AA36", Obj_17AA36_Tick, rom);
    bad += run("Obj 0x0F6700", "work/ram/obj_F6700", Obj_F6700_Tick, rom);
    bad += run("Obj 0x179ED0", "work/ram/obj_179ED0", Obj_179ED0_Tick, rom);
    bad += run("Obj 0x17F8B0", "work/ram/obj_17F8B0", Obj_17F8B0_Tick, rom);
    bad += run("Obj 0x17F594", "work/ram/obj_17F594", Obj_17F594_Tick, rom);
    bad += run("Obj 0x0F752E", "work/ram/obj_F752E", Obj_F752E_Tick, rom);
    bad += run("Obj 0x0F7722", "work/ram/obj_F7722", Obj_F7722_Tick, rom);
    bad += run("Obj 0x0F778A", "work/ram/obj_F778A", Obj_F778A_Tick, rom);
    bad += run("Obj 0x179F6C", "work/ram/obj_179F6C", Obj_179F6C_Tick, rom);
    bad += run("Obj 0x0F75DE", "work/ram/obj_F75DE", Obj_F75DE_Tick, rom);
    bad += run("Obj 0x0F754C", "work/ram/obj_F754C", Obj_F754C_Tick, rom);
    bad += run("Obj 0x0F7618", "work/ram/obj_F7618", Obj_F7618_Tick, rom);
    bad += run("Obj 0x10B93C", "work/ram/obj_10B93C", Obj_10B93C_Tick, rom);
    bad += run("Obj 0x0F773E", "work/ram/obj_F773E", Obj_F773E_Tick, rom);
    bad += run("Obj 0x0F7800", "work/ram/obj_F7800", Obj_F7800_Tick, rom);
    bad += run("Obj 0x10B83E", "work/ram/obj_10B83E", Obj_10B83E_Tick, rom);
    bad += run("Obj 0x10BD20", "work/ram/obj_10BD20", Obj_10BD20_Tick, rom);
    bad += run("Obj 0x0FB526", "work/ram/obj_FB526", Obj_FB526_Tick, rom);
    bad += run("Obj 0x10BD88", "work/ram/obj_10BD88", Obj_10BD88_Tick, rom);
    bad += run("Obj 0x0CDD12", "work/ram/obj_CDD12", Obj_CDD12_Tick, rom);
    bad += run("Obj 0x0CDCA0", "work/ram/obj_CDCA0", Obj_CDCA0_Tick, rom);
    bad += run("Obj 0x0D9056", "work/ram/obj_D9056", Obj_D9056_Tick, rom);
    bad += run("Obj 0x0D92B4", "work/ram/obj_D92B4", Obj_D92B4_Tick, rom);
    bad += run("Obj 0x0C99F8", "work/ram/obj_C99F8", Obj_C99F8_Tick, rom);
    bad += run("Obj 0x1118BC", "work/ram/obj_1118BC", Obj_1118BC_Tick, rom);
    bad += run("Obj 0x0FB584", "work/ram/obj_FB584", Obj_FB584_Tick, rom);
    bad += run("Obj 0x111A7A", "work/ram/obj_111A7A", Obj_111A7A_Tick, rom);
    bad += run("Obj 0x0FCC4C", "work/ram/obj_FCC4C", Obj_FCC4C_Tick, rom);
    bad += run("Obj 0x1899EC", "work/ram/obj_1899EC", Obj_1899EC_Tick, rom);
    bad += run("Obj 0x111B56", "work/ram/obj_111B56", Obj_111B56_Tick, rom);
    bad += run("Obj 0x111DF0", "work/ram/obj_111DF0", Obj_111DF0_Tick, rom);
    bad += run("Obj 0x0D93D4", "work/ram/obj_D93D4", Obj_D93D4_Tick, rom);
    bad += run("Obj 0x0D936A", "work/ram/obj_D936A", Obj_D936A_Tick, rom);
    bad += run("Obj 0x1153B8", "work/ram/obj_1153B8", Obj_1153B8_Tick, rom);
    bad += run("Obj 0x1150E0", "work/ram/obj_1150E0", Obj_1150E0_Tick, rom);
    bad += run("Obj 0x111D18", "work/ram/obj_111D18", Obj_111D18_Tick, rom);
    bad += run("Obj 0x115092", "work/ram/obj_115092", Obj_115092_Tick, rom);
    bad += run("Obj 0x111D9C", "work/ram/obj_111D9C", Obj_111D9C_Tick, rom);
    bad += run("Obj 0x111D8E", "work/ram/obj_111D8E", Obj_111D8E_Tick, rom);
    bad += run("Obj 0x115334", "work/ram/obj_115334", Obj_115334_Tick, rom);
    bad += run("Obj 0x11509A", "work/ram/obj_11509A", Obj_11509A_Tick, rom);
    bad += run("Obj 0x118D4A", "work/ram/obj_118D4A", Obj_118D4A_Tick, rom);
    bad += run("Obj 0x0D7EAE", "work/ram/obj_D7EAE", Obj_D7EAE_Tick, rom);
    bad += run("Obj 0x0C9A38", "work/ram/obj_C9A38", Obj_C9A38_Tick, rom);
    bad += run("Obj 0x115326", "work/ram/obj_115326", Obj_115326_Tick, rom);
    bad += run("Obj 0x0D8030", "work/ram/obj_D8030", Obj_D8030_Tick, rom);
    bad += run("Obj 0x0D7DD8", "work/ram/obj_D7DD8", Obj_D7DD8_Tick, rom);
    bad += run("Obj 0x0D808C", "work/ram/obj_D808C", Obj_D808C_Tick, rom);
    bad += run("Obj 0x0D8574", "work/ram/obj_D8574", Obj_D8574_Tick, rom);
    bad += run("Obj 0x0D7F52", "work/ram/obj_D7F52", Obj_D7F52_Tick, rom);
    bad += run("Obj 0x0D8164", "work/ram/obj_D8164", Obj_D8164_Tick, rom);
    bad += run("Obj 0x0D7F0E", "work/ram/obj_D7F0E", Obj_D7F0E_Tick, rom);
    bad += run("Obj 0x0D7F8E", "work/ram/obj_D7F8E", Obj_D7F8E_Tick, rom);
    bad += run("Obj 0x0D853C", "work/ram/obj_D853C", Obj_D853C_Tick, rom);
    bad += run("Obj 0x0D823E", "work/ram/obj_D823E", Obj_D823E_Tick, rom);
    bad += run("Obj 0x15929C", "work/ram/obj_15929C", Obj_15929C_Tick, rom);
    bad += run("Obj 0x111DA6", "work/ram/obj_111DA6", Obj_111DA6_Tick, rom);
    bad += run("Obj 0x1242EC", "work/ram/obj_1242EC", Obj_1242EC_Tick, rom);
    bad += run("Obj 0x1592EC", "work/ram/obj_1592EC", Obj_1592EC_Tick, rom);
    bad += run("Obj 0x11AEBE", "work/ram/obj_11AEBE", Obj_11AEBE_Tick, rom);
    bad += run("Obj 0x123FCA", "work/ram/obj_123FCA", Obj_123FCA_Tick, rom);
    bad += run("Obj 0x11B14C", "work/ram/obj_11B14C", Obj_11B14C_Tick, rom);
    bad += run("Obj 0x11AFA6", "work/ram/obj_11AFA6", Obj_11AFA6_Tick, rom);
    bad += run("Obj 0x124404", "work/ram/obj_124404", Obj_124404_Tick, rom);
    bad += run("Obj 0x124D64", "work/ram/obj_124D64", Obj_124D64_Tick, rom);
    bad += run("Obj 0x125016", "work/ram/obj_125016", Obj_125016_Tick, rom);
    bad += run("Obj 0x11AECE", "work/ram/obj_11AECE", Obj_11AECE_Tick, rom);
    bad += run("Obj 0x124E00", "work/ram/obj_124E00", Obj_124E00_Tick, rom);
    bad += run("Obj 0x1250BC", "work/ram/obj_1250BC", Obj_1250BC_Tick, rom);
    bad += run("Obj 0x11B17C", "work/ram/obj_11B17C", Obj_11B17C_Tick, rom);
    bad += run("Obj 0x1253B0", "work/ram/obj_1253B0", Obj_1253B0_Tick, rom);
    bad += run("Obj 0x12442A", "work/ram/obj_12442A", Obj_12442A_Tick, rom);
    bad += run("Obj 0x12552E", "work/ram/obj_12552E", Obj_12552E_Tick, rom);
    bad += run("Obj 0x124D78", "work/ram/obj_124D78", Obj_124D78_Tick, rom);
    bad += run("Obj 0x125D22", "work/ram/obj_125D22", Obj_125D22_Tick, rom);
    bad += run("Obj 0x11AFE4", "work/ram/obj_11AFE4", Obj_11AFE4_Tick, rom);
    bad += run("Obj 0x11B3D2", "work/ram/obj_11B3D2", Obj_11B3D2_Tick, rom);
    bad += run("Obj 0x1250E4", "work/ram/obj_1250E4", Obj_1250E4_Tick, rom);
    bad += run("Obj 0x125046", "work/ram/obj_125046", Obj_125046_Tick, rom);
    bad += run("Obj 0x124E30", "work/ram/obj_124E30", Obj_124E30_Tick, rom);
    bad += run("Obj 0x1253D0", "work/ram/obj_1253D0", Obj_1253D0_Tick, rom);
    bad += run("Obj 0x12554E", "work/ram/obj_12554E", Obj_12554E_Tick, rom);
    bad += run("Obj 0x125D44", "work/ram/obj_125D44", Obj_125D44_Tick, rom);
    bad += run("Obj 0x11B022", "work/ram/obj_11B022", Obj_11B022_Tick, rom);
    bad += run("Obj 0x11B472", "work/ram/obj_11B472", Obj_11B472_Tick, rom);
    bad += run("Obj 0x124438", "work/ram/obj_124438", Obj_124438_Tick, rom);
    bad += run("Obj 0x125DAA", "work/ram/obj_125DAA", Obj_125DAA_Tick, rom);
    bad += run("Obj 0x124466", "work/ram/obj_124466", Obj_124466_Tick, rom);
    bad += run("Obj 0x1244A2", "work/ram/obj_1244A2", Obj_1244A2_Tick, rom);
    bad += run("Obj 0x1244DA", "work/ram/obj_1244DA", Obj_1244DA_Tick, rom);
    bad += run("Obj 0x125B88", "work/ram/obj_125B88", Obj_125B88_Tick, rom);
    bad += run("Obj 0x0CD7B8", "work/ram/obj_CD7B8", Obj_CD7B8_Tick, rom);
    bad += run("Obj 0x0CD74E", "work/ram/obj_CD74E", Obj_CD74E_Tick, rom);
    bad += run("Obj 0x125B0C", "work/ram/obj_125B0C", Obj_125B0C_Tick, rom);
    bad += run("Obj 0x0CCD5C", "work/ram/obj_CCD5C", Obj_CCD5C_Tick, rom);
    bad += run("Obj 0x124524", "work/ram/obj_124524", Obj_124524_Tick, rom);
    bad += run("Obj 0x124550", "work/ram/obj_124550", Obj_124550_Tick, rom);
    bad += run("Obj 0x124532", "work/ram/obj_124532", Obj_124532_Tick, rom);
    bad += run("Obj 0x125CA8", "work/ram/obj_125CA8", Obj_125CA8_Tick, rom);
    bad += run("Obj 0x11B668", "work/ram/obj_11B668", Obj_11B668_Tick, rom);
    bad += run("Obj 0x11B12C", "work/ram/obj_11B12C", Obj_11B12C_Tick, rom);
    bad += run("Obj 0x11B35E", "work/ram/obj_11B35E", Obj_11B35E_Tick, rom);
    bad += run("Obj 0x11B5E6", "work/ram/obj_11B5E6", Obj_11B5E6_Tick, rom);
    bad += run("Obj 0x124358", "work/ram/obj_124358", Obj_124358_Tick, rom);
    bad += run("Obj 0x12431E", "work/ram/obj_12431E", Obj_12431E_Tick, rom);
    bad += run("Obj 0x11B656", "work/ram/obj_11B656", Obj_11B656_Tick, rom);
    bad += run("Obj 0x11B23A", "work/ram/obj_11B23A", Obj_11B23A_Tick, rom);
    bad += run("Obj 0x159C8C", "work/ram/obj_159C8C", Obj_159C8C_Tick, rom);
    bad += run("Obj 0x11B1BE", "work/ram/obj_11B1BE", Obj_11B1BE_Tick, rom);
    bad += run("Obj 0x11C054", "work/ram/obj_11C054", Obj_11C054_Tick, rom);
    bad += run("Obj 0x125CFA", "work/ram/obj_125CFA", Obj_125CFA_Tick, rom);
    bad += run("Obj 0x0C7DBA", "work/ram/obj_C7DBA", Obj_C7DBA_Tick, rom);
    bad += run("Obj 0x11BFAC", "work/ram/obj_11BFAC", Obj_11BFAC_Tick, rom);
    bad += run("Obj 0x0C8218", "work/ram/obj_C8218", Obj_C8218_Tick, rom);
    bad += run("Obj 0x15A264", "work/ram/obj_15A264", Obj_15A264_Tick, rom);
    bad += run("Obj 0x0C7DCA", "work/ram/obj_C7DCA", Obj_C7DCA_Tick, rom);
    bad += run("Obj 0x15A1CA", "work/ram/obj_15A1CA", Obj_15A1CA_Tick, rom);
    bad += run("Obj 0x0C823E", "work/ram/obj_C823E", Obj_C823E_Tick, rom);
    bad += run("Obj 0x0C7E5E", "work/ram/obj_C7E5E", Obj_C7E5E_Tick, rom);
    bad += run("Obj 0x11BF5C", "work/ram/obj_11BF5C", Obj_11BF5C_Tick, rom);
    bad += run("Obj 0x0C7F3E", "work/ram/obj_C7F3E", Obj_C7F3E_Tick, rom);
    bad += run("Obj 0x0C80DC", "work/ram/obj_C80DC", Obj_C80DC_Tick, rom);
    bad += run("Obj 0x11B1FA", "work/ram/obj_11B1FA", Obj_11B1FA_Tick, rom);
    bad += run("Obj 0x0C7E10", "work/ram/obj_C7E10", Obj_C7E10_Tick, rom);
    bad += run("Obj 0x0C8168", "work/ram/obj_C8168", Obj_C8168_Tick, rom);
    bad += run("Obj 0x0C7DDE", "work/ram/obj_C7DDE", Obj_C7DDE_Tick, rom);
    bad += run("Obj 0x0C7F14", "work/ram/obj_C7F14", Obj_C7F14_Tick, rom);
    bad += run("Obj 0x15A088", "work/ram/obj_15A088", Obj_15A088_Tick, rom);
    bad += run("Obj 0x159D68", "work/ram/obj_159D68", Obj_159D68_Tick, rom);
    bad += run("Obj 0x159D22", "work/ram/obj_159D22", Obj_159D22_Tick, rom);
    bad += run("Obj 0x15A154", "work/ram/obj_15A154", Obj_15A154_Tick, rom);
    bad += run("Obj 0x0FFCF4", "work/ram/obj_FFCF4", Obj_FFCF4_Tick, rom);
    bad += run("Obj 0x111432", "work/ram/obj_111432", Obj_111432_Tick, rom);
    bad += run("Obj 0x111424", "work/ram/obj_111424", Obj_111424_Tick, rom);
    bad += run("Obj 0x111492", "work/ram/obj_111492", Obj_111492_Tick, rom);
    bad += run("Obj 0x1114F6", "work/ram/obj_1114F6", Obj_1114F6_Tick, rom);
    bad += run("Obj 0x10010C", "work/ram/obj_10010C", Obj_10010C_Tick, rom);
    bad += run("Obj 0x1002CC", "work/ram/obj_1002CC", Obj_1002CC_Tick, rom);
    bad += run("Obj 0x111450", "work/ram/obj_111450", Obj_111450_Tick, rom);
    bad += run("Obj 0x15A0C8", "work/ram/obj_15A0C8", Obj_15A0C8_Tick, rom);
    bad += run("Obj 0x0C7EE6", "work/ram/obj_C7EE6", Obj_C7EE6_Tick, rom);
    bad += run("Obj 0x0C7EF4", "work/ram/obj_C7EF4", Obj_C7EF4_Tick, rom);
    bad += run("Obj 0x1114B4", "work/ram/obj_1114B4", Obj_1114B4_Tick, rom);
    bad += run("Obj 0x11151A", "work/ram/obj_11151A", Obj_11151A_Tick, rom);
    bad += run("Obj 0x1000DE", "work/ram/obj_1000DE", Obj_1000DE_Tick, rom);
    bad += run("Obj 0x1002B0", "work/ram/obj_1002B0", Obj_1002B0_Tick, rom);
    bad += run("Obj 0x1008A6", "work/ram/obj_1008A6", Obj_1008A6_Tick, rom);
    bad += run("Obj 0x1008E2", "work/ram/obj_1008E2", Obj_1008E2_Tick, rom);
    bad += run("Obj 0x115F96", "work/ram/obj_115F96", Obj_115F96_Tick, rom);
    bad += run("Obj 0x1117CC", "work/ram/obj_1117CC", Obj_1117CC_Tick, rom);
    bad += run("Obj 0x115F36", "work/ram/obj_115F36", Obj_115F36_Tick, rom);
    bad += run("Obj 0x115B3E", "work/ram/obj_115B3E", Obj_115B3E_Tick, rom);
    bad += run("Obj 0x116078", "work/ram/obj_116078", Obj_116078_Tick, rom);
    bad += run("Obj 0x1161C4", "work/ram/obj_1161C4", Obj_1161C4_Tick, rom);
    bad += run("Obj 0x116118", "work/ram/obj_116118", Obj_116118_Tick, rom);
    bad += run("Obj 0x1160C4", "work/ram/obj_1160C4", Obj_1160C4_Tick, rom);
    bad += run("Obj 0x116030", "work/ram/obj_116030", Obj_116030_Tick, rom);
    bad += run("Obj 0x0D6242", "work/ram/obj_D6242", Obj_D6242_Tick, rom);
    bad += run("Obj 0x0D6192", "work/ram/obj_D6192", Obj_D6192_Tick, rom);
    bad += run("Obj 0x0E8516", "work/ram/obj_E8516", Obj_E8516_Tick, rom);
    bad += run("Obj 0x118F84", "work/ram/obj_118F84", Obj_118F84_Tick, rom);
    bad += run("Obj 0x0E8B72", "work/ram/obj_E8B72", Obj_E8B72_Tick, rom);
    bad += run("Obj 0x0E8C36", "work/ram/obj_E8C36", Obj_E8C36_Tick, rom);
    bad += run("Obj 0x118F4E", "work/ram/obj_118F4E", Obj_118F4E_Tick, rom);
    bad += run("Obj 0x109FAA", "work/ram/obj_109FAA", Obj_109FAA_Tick, rom);
    bad += run("Obj 0x0E8BEA", "work/ram/obj_E8BEA", Obj_E8BEA_Tick, rom);
    bad += run("Obj 0x0D9224", "work/ram/obj_D9224", Obj_D9224_Tick, rom);
    bad += run("Obj 0x0E8B26", "work/ram/obj_E8B26", Obj_E8B26_Tick, rom);
    bad += run("Obj 0x10A054", "work/ram/obj_10A054", Obj_10A054_Tick, rom);
    bad += run("Obj 0x0D8DC4", "work/ram/obj_D8DC4", Obj_D8DC4_Tick, rom);
    bad += run("Obj 0x10A024", "work/ram/obj_10A024", Obj_10A024_Tick, rom);
    bad += run("Obj 0x0D8D22", "work/ram/obj_D8D22", Obj_D8D22_Tick, rom);
    bad += run("Obj 0x0E8D12", "work/ram/obj_E8D12", Obj_E8D12_Tick, rom);
    bad += run("Obj 0x0D8D30", "work/ram/obj_D8D30", Obj_D8D30_Tick, rom);
    bad += run("Obj 0x0D572C", "work/ram/obj_D572C", Obj_D572C_Tick, rom);
    bad += run("Obj 0x0E8CAE", "work/ram/obj_E8CAE", Obj_E8CAE_Tick, rom);
    bad += run("Obj 0x0ED214", "work/ram/obj_ED214", Obj_ED214_Tick, rom);
    bad += run("Obj 0x0DC56E", "work/ram/obj_DC56E", Obj_DC56E_Tick, rom);
    bad += run("Obj 0x0D56C6", "work/ram/obj_D56C6", Obj_D56C6_Tick, rom);
    bad += run("Obj 0x115D14", "work/ram/obj_115D14", Obj_115D14_Tick, rom);
    bad += run("Obj 0x0ED2A0", "work/ram/obj_ED2A0", Obj_ED2A0_Tick, rom);
    bad += run("Obj 0x0D64BA", "work/ram/obj_D64BA", Obj_D64BA_Tick, rom);
    bad += run("Obj 0x0ED2D8", "work/ram/obj_ED2D8", Obj_ED2D8_Tick, rom);
    bad += run("Obj 0x0D6636", "work/ram/obj_D6636", Obj_D6636_Tick, rom);
    bad += run("Obj 0x0D653C", "work/ram/obj_D653C", Obj_D653C_Tick, rom);
    bad += run("Obj 0x10A42C", "work/ram/obj_10A42C", Obj_10A42C_Tick, rom);
    bad += run("Obj 0x0D6846", "work/ram/obj_D6846", Obj_D6846_Tick, rom);
    bad += run("Obj 0x10A484", "work/ram/obj_10A484", Obj_10A484_Tick, rom);
    bad += run("Obj 0x0D67F6", "work/ram/obj_D67F6", Obj_D67F6_Tick, rom);
    bad += run("Obj 0x0D6ACA", "work/ram/obj_D6ACA", Obj_D6ACA_Tick, rom);
    bad += run("Obj 0x0DB2E0", "work/ram/obj_DB2E0", Obj_DB2E0_Tick, rom);
    bad += run("Obj 0x0D6958", "work/ram/obj_D6958", Obj_D6958_Tick, rom);
    bad += run("Obj 0x10A438", "work/ram/obj_10A438", Obj_10A438_Tick, rom);
    bad += run("Obj 0x118B3C", "work/ram/obj_118B3C", Obj_118B3C_Tick, rom);
    bad += run("Obj 0x0D8E42", "work/ram/obj_D8E42", Obj_D8E42_Tick, rom);
    bad += run("Obj 0x0DB26C", "work/ram/obj_DB26C", Obj_DB26C_Tick, rom);
    bad += run("Obj 0x118B74", "work/ram/obj_118B74", Obj_118B74_Tick, rom);
    bad += run("Obj 0x0F238A", "work/ram/obj_F238A", Obj_F238A_Tick, rom);
    bad += run("Obj 0x0D66BA", "work/ram/obj_D66BA", Obj_D66BA_Tick, rom);
    bad += run("Obj 0x0F2454", "work/ram/obj_F2454", Obj_F2454_Tick, rom);
    bad += run("Obj 0x0D59F0", "work/ram/obj_D59F0", Obj_D59F0_Tick, rom);
    bad += run("Obj 0x0F241C", "work/ram/obj_F241C", Obj_F241C_Tick, rom);
    bad += run("Obj 0x0D5CD0", "work/ram/obj_D5CD0", Obj_D5CD0_Tick, rom);
    bad += run("Obj 0x0D5974", "work/ram/obj_D5974", Obj_D5974_Tick, rom);
    bad += run("Obj 0x0D5C64", "work/ram/obj_D5C64", Obj_D5C64_Tick, rom);
    bad += run("Obj 0x1003E2", "work/ram/obj_1003E2", Obj_1003E2_Tick, rom);
    bad += run("Obj 0x18F1C2", "work/ram/obj_18F1C2", Obj_18F1C2_Tick, rom);
    bad += run("Obj 0x10048C", "work/ram/obj_10048C", Obj_10048C_Tick, rom);
    bad += run("Obj 0x100B94", "work/ram/obj_100B94", Obj_100B94_Tick, rom);
    bad += run("Obj 0x18F002", "work/ram/obj_18F002", Obj_18F002_Tick, rom);
    bad += run("Obj 0x190B1A", "work/ram/obj_190B1A", Obj_190B1A_Tick, rom);
    bad += run("Obj 0x1909B2", "work/ram/obj_1909B2", Obj_1909B2_Tick, rom);
    bad += run("Obj 0x18F012", "work/ram/obj_18F012", Obj_18F012_Tick, rom);
    bad += run("Obj 0x19090C", "work/ram/obj_19090C", Obj_19090C_Tick, rom);
    bad += run("Obj 0x190AA6", "work/ram/obj_190AA6", Obj_190AA6_Tick, rom);
    bad += run("Obj 0x190FD0", "work/ram/obj_190FD0", Obj_190FD0_Tick, rom);
    bad += run("Obj 0x191014", "work/ram/obj_191014", Obj_191014_Tick, rom);
    bad += run("Obj 0x191374", "work/ram/obj_191374", Obj_191374_Tick, rom);
    bad += run("Obj 0x191356", "work/ram/obj_191356", Obj_191356_Tick, rom);
    bad += run("Obj 0x1914E6", "work/ram/obj_1914E6", Obj_1914E6_Tick, rom);
    bad += run("Obj 0x191486", "work/ram/obj_191486", Obj_191486_Tick, rom);
    bad += run("Obj 0x1915E0", "work/ram/obj_1915E0", Obj_1915E0_Tick, rom);
    bad += run("Obj 0x191700", "work/ram/obj_191700", Obj_191700_Tick, rom);
    bad += run("Obj 0x1915AE", "work/ram/obj_1915AE", Obj_1915AE_Tick, rom);
    bad += run("Obj 0x191742", "work/ram/obj_191742", Obj_191742_Tick, rom);
    bad += run("Obj 0x1916BE", "work/ram/obj_1916BE", Obj_1916BE_Tick, rom);
    bad += run("Obj 0x19163C", "work/ram/obj_19163C", Obj_19163C_Tick, rom);
    bad += run("Obj 0x181AEC", "work/ram/obj_181AEC", Obj_181AEC_Tick, rom);
    bad += run("Obj 0x1802F4", "work/ram/obj_1802F4", Obj_1802F4_Tick, rom);
    bad += run("Obj 0x1802A4", "work/ram/obj_1802A4", Obj_1802A4_Tick, rom);
    bad += run("Obj 0x1804DA", "work/ram/obj_1804DA", Obj_1804DA_Tick, rom);
    bad += run("Obj 0x18048A", "work/ram/obj_18048A", Obj_18048A_Tick, rom);
    bad += run("Obj 0x191688", "work/ram/obj_191688", Obj_191688_Tick, rom);
    bad += run("Obj 0x18F216", "work/ram/obj_18F216", Obj_18F216_Tick, rom);
    bad += run("Obj 0x0EE42E", "work/ram/obj_EE42E", Obj_EE42E_Tick, rom);
    bad += run("Obj 0x10C966", "work/ram/obj_10C966", Obj_10C966_Tick, rom);
    bad += run("Obj 0x0EE468", "work/ram/obj_EE468", Obj_EE468_Tick, rom);
    bad += run("Obj 0x0EE43A", "work/ram/obj_EE43A", Obj_EE43A_Tick, rom);
    bad += run("Obj 0x0EE7D8", "work/ram/obj_EE7D8", Obj_EE7D8_Tick, rom);
    bad += run("Obj 0x0EE7AA", "work/ram/obj_EE7AA", Obj_EE7AA_Tick, rom);
    bad += run("Obj 0x0EF366", "work/ram/obj_EF366", Obj_EF366_Tick, rom);
    bad += run("Obj 0x0EF44E", "work/ram/obj_EF44E", Obj_EF44E_Tick, rom);
    bad += run("Obj 0x0DA28C", "work/ram/obj_DA28C", Obj_DA28C_Tick, rom);
    bad += run("Obj 0x0EF400", "work/ram/obj_EF400", Obj_EF400_Tick, rom);
    bad += run("Obj 0x0EF434", "work/ram/obj_EF434", Obj_EF434_Tick, rom);
    bad += run("Obj 0x0DA20E", "work/ram/obj_DA20E", Obj_DA20E_Tick, rom);
    bad += run("Obj 0x105ED6", "work/ram/obj_105ED6", Obj_105ED6_Tick, rom);
    bad += run("Obj 0x1070DA", "work/ram/obj_1070DA", Obj_1070DA_Tick, rom);
    bad += run("Obj 0x10747E", "work/ram/obj_10747E", Obj_10747E_Tick, rom);
    bad += run("Obj 0x107B6E", "work/ram/obj_107B6E", Obj_107B6E_Tick, rom);
    bad += run("Obj 0x107786", "work/ram/obj_107786", Obj_107786_Tick, rom);
    bad += run("Obj 0x107DB2", "work/ram/obj_107DB2", Obj_107DB2_Tick, rom);
    bad += run("Obj 0x1080A4", "work/ram/obj_1080A4", Obj_1080A4_Tick, rom);
    bad += run("Obj 0x107112", "work/ram/obj_107112", Obj_107112_Tick, rom);
    bad += run("Obj 0x105EDE", "work/ram/obj_105EDE", Obj_105EDE_Tick, rom);
    bad += run("Obj 0x108294", "work/ram/obj_108294", Obj_108294_Tick, rom);
    bad += run("Obj 0x1084C0", "work/ram/obj_1084C0", Obj_1084C0_Tick, rom);
    bad += run("Obj 0x1074B6", "work/ram/obj_1074B6", Obj_1074B6_Tick, rom);
    bad += run("Obj 0x1077DA", "work/ram/obj_1077DA", Obj_1077DA_Tick, rom);
    bad += run("Obj 0x107BC2", "work/ram/obj_107BC2", Obj_107BC2_Tick, rom);
    bad += run("Obj 0x107E00", "work/ram/obj_107E00", Obj_107E00_Tick, rom);
    bad += run("Obj 0x1080F2", "work/ram/obj_1080F2", Obj_1080F2_Tick, rom);
    bad += run("Obj 0x1082E6", "work/ram/obj_1082E6", Obj_1082E6_Tick, rom);
    bad += run("Obj 0x108512", "work/ram/obj_108512", Obj_108512_Tick, rom);
    bad += run("Obj 0x0DBC1A", "work/ram/obj_DBC1A", Obj_DBC1A_Tick, rom);
    bad += run("Obj 0x0C9AB8", "work/ram/obj_C9AB8", Obj_C9AB8_Tick, rom);
    bad += run("Obj 0x0DBCAE", "work/ram/obj_DBCAE", Obj_DBCAE_Tick, rom);
    bad += run("Obj 0x10DBE0", "work/ram/obj_10DBE0", Obj_10DBE0_Tick, rom);
    bad += run("Obj 0x10E1BC", "work/ram/obj_10E1BC", Obj_10E1BC_Tick, rom);
    bad += run("Obj 0x10DBD8", "work/ram/obj_10DBD8", Obj_10DBD8_Tick, rom);
    bad += run("Obj 0x10E178", "work/ram/obj_10E178", Obj_10E178_Tick, rom);
    bad += run("Obj 0x0DC03A", "work/ram/obj_DC03A", Obj_DC03A_Tick, rom);
    bad += run("Obj 0x116B86", "work/ram/obj_116B86", Obj_116B86_Tick, rom);
    bad += run("Obj 0x0DC008", "work/ram/obj_DC008", Obj_DC008_Tick, rom);
    bad += run("Obj 0x0DC46E", "work/ram/obj_DC46E", Obj_DC46E_Tick, rom);
    bad += run("Obj 0x116B0C", "work/ram/obj_116B0C", Obj_116B0C_Tick, rom);
    bad += run("Obj 0x1165CC", "work/ram/obj_1165CC", Obj_1165CC_Tick, rom);
    bad += run("Obj 0x116C48", "work/ram/obj_116C48", Obj_116C48_Tick, rom);
    bad += run("Obj 0x116C9A", "work/ram/obj_116C9A", Obj_116C9A_Tick, rom);
    bad += run("Obj 0x1166EA", "work/ram/obj_1166EA", Obj_1166EA_Tick, rom);
    bad += run("Obj 0x116DD2", "work/ram/obj_116DD2", Obj_116DD2_Tick, rom);
    bad += run("Obj 0x116E22", "work/ram/obj_116E22", Obj_116E22_Tick, rom);
    bad += run("Obj 0x0E5372", "work/ram/obj_E5372", Obj_E5372_Tick, rom);
    bad += run("Obj 0x116802", "work/ram/obj_116802", Obj_116802_Tick, rom);
    bad += run("Obj 0x0E54EE", "work/ram/obj_E54EE", Obj_E54EE_Tick, rom);
    bad += run("Obj 0x10E316", "work/ram/obj_10E316", Obj_10E316_Tick, rom);
    bad += run("Obj 0x0FEF76", "work/ram/obj_FEF76", Obj_FEF76_Tick, rom);
    bad += run("Obj 0x0E262A", "work/ram/obj_E262A", Obj_E262A_Tick, rom);
    bad += run("Obj 0x116F82", "work/ram/obj_116F82", Obj_116F82_Tick, rom);
    bad += run("Obj 0x116EE4", "work/ram/obj_116EE4", Obj_116EE4_Tick, rom);
    bad += run("Obj 0x0E54A0", "work/ram/obj_E54A0", Obj_E54A0_Tick, rom);
    bad += run("Obj 0x0E2804", "work/ram/obj_E2804", Obj_E2804_Tick, rom);
    bad += run("Obj 0x117826", "work/ram/obj_117826", Obj_117826_Tick, rom);
    bad += run("Obj 0x0DACDC", "work/ram/obj_DACDC", Obj_DACDC_Tick, rom);
    bad += run("Obj 0x1177AC", "work/ram/obj_1177AC", Obj_1177AC_Tick, rom);
    bad += run("Obj 0x116EDC", "work/ram/obj_116EDC", Obj_116EDC_Tick, rom);
    bad += run("Obj 0x116F7A", "work/ram/obj_116F7A", Obj_116F7A_Tick, rom);
    bad += run("Obj 0x12601C", "work/ram/obj_12601C", Obj_12601C_Tick, rom);
    bad += run("Obj 0x117980", "work/ram/obj_117980", Obj_117980_Tick, rom);
    bad += run("Obj 0x0E27C0", "work/ram/obj_E27C0", Obj_E27C0_Tick, rom);
    bad += run("Obj 0x0DAC54", "work/ram/obj_DAC54", Obj_DAC54_Tick, rom);
    bad += run("Obj 0x11792A", "work/ram/obj_11792A", Obj_11792A_Tick, rom);
    bad += run("Obj 0x126158", "work/ram/obj_126158", Obj_126158_Tick, rom);
    bad += run("Obj 0x11699A", "work/ram/obj_11699A", Obj_11699A_Tick, rom);
    bad += run("Obj 0x125DC2", "work/ram/obj_125DC2", Obj_125DC2_Tick, rom);
    bad += run("Obj 0x126DEC", "work/ram/obj_126DEC", Obj_126DEC_Tick, rom);
    bad += run("Obj 0x126D92", "work/ram/obj_126D92", Obj_126D92_Tick, rom);
    bad += run("Obj 0x126EB4", "work/ram/obj_126EB4", Obj_126EB4_Tick, rom);
    bad += run("Obj 0x126E84", "work/ram/obj_126E84", Obj_126E84_Tick, rom);
    bad += run("Obj 0x126132", "work/ram/obj_126132", Obj_126132_Tick, rom);
    bad += run("Obj 0x126FF6", "work/ram/obj_126FF6", Obj_126FF6_Tick, rom);
    bad += run("Obj 0x126FA4", "work/ram/obj_126FA4", Obj_126FA4_Tick, rom);
    bad += run("Obj 0x1270BE", "work/ram/obj_1270BE", Obj_1270BE_Tick, rom);
    bad += run("Obj 0x12739E", "work/ram/obj_12739E", Obj_12739E_Tick, rom);
    bad += run("Obj 0x12706C", "work/ram/obj_12706C", Obj_12706C_Tick, rom);
    bad += run("Obj 0x1169CA", "work/ram/obj_1169CA", Obj_1169CA_Tick, rom);
    bad += run("Obj 0x127584", "work/ram/obj_127584", Obj_127584_Tick, rom);
    bad += run("Obj 0x12737E", "work/ram/obj_12737E", Obj_12737E_Tick, rom);
    bad += run("Obj 0x1261FE", "work/ram/obj_1261FE", Obj_1261FE_Tick, rom);
    bad += run("Obj 0x1289A6", "work/ram/obj_1289A6", Obj_1289A6_Tick, rom);
    bad += run("Obj 0x127560", "work/ram/obj_127560", Obj_127560_Tick, rom);
    bad += run("Obj 0x126282", "work/ram/obj_126282", Obj_126282_Tick, rom);
    bad += run("Obj 0x128B9E", "work/ram/obj_128B9E", Obj_128B9E_Tick, rom);
    bad += run("Obj 0x128A84", "work/ram/obj_128A84", Obj_128A84_Tick, rom);
    bad += run("Obj 0x128BB2", "work/ram/obj_128BB2", Obj_128BB2_Tick, rom);
    bad += run("Obj 0x126310", "work/ram/obj_126310", Obj_126310_Tick, rom);
    bad += run("Obj 0x0CD1AE", "work/ram/obj_CD1AE", Obj_CD1AE_Tick, rom);
    bad += run("Obj 0x0CCCA2", "work/ram/obj_CCCA2", Obj_CCCA2_Tick, rom);
    bad += run("Obj 0x0CCFD4", "work/ram/obj_CCFD4", Obj_CCFD4_Tick, rom);
    bad += run("Obj 0x128924", "work/ram/obj_128924", Obj_128924_Tick, rom);
    bad += run("Obj 0x126254", "work/ram/obj_126254", Obj_126254_Tick, rom);
    bad += run("Obj 0x1261AA", "work/ram/obj_1261AA", Obj_1261AA_Tick, rom);
    bad += run("Obj 0x126364", "work/ram/obj_126364", Obj_126364_Tick, rom);
    bad += run("Obj 0x1289EA", "work/ram/obj_1289EA", Obj_1289EA_Tick, rom);
    bad += run("Obj 0x128B44", "work/ram/obj_128B44", Obj_128B44_Tick, rom);
    bad += run("Obj 0x1263BE", "work/ram/obj_1263BE", Obj_1263BE_Tick, rom);
    bad += run("Obj 0x128476", "work/ram/obj_128476", Obj_128476_Tick, rom);
    bad += run("Obj 0x1262B0", "work/ram/obj_1262B0", Obj_1262B0_Tick, rom);
    bad += run("Obj 0x1284D4", "work/ram/obj_1284D4", Obj_1284D4_Tick, rom);
    bad += run("Obj 0x126430", "work/ram/obj_126430", Obj_126430_Tick, rom);
    bad += run("Obj 0x0CCF72", "work/ram/obj_CCF72", Obj_CCF72_Tick, rom);
    bad += run("Obj 0x12645E", "work/ram/obj_12645E", Obj_12645E_Tick, rom);
    bad += run("Obj 0x128D4A", "work/ram/obj_128D4A", Obj_128D4A_Tick, rom);
    bad += run("Obj 0x128660", "work/ram/obj_128660", Obj_128660_Tick, rom);
    bad += run("Obj 0x126390", "work/ram/obj_126390", Obj_126390_Tick, rom);
    bad += run("Obj 0x12886A", "work/ram/obj_12886A", Obj_12886A_Tick, rom);
    bad += run("Obj 0x1264CA", "work/ram/obj_1264CA", Obj_1264CA_Tick, rom);
    bad += run("Obj 0x12649C", "work/ram/obj_12649C", Obj_12649C_Tick, rom);
    bad += run("Obj 0x128226", "work/ram/obj_128226", Obj_128226_Tick, rom);
    bad += run("Obj 0x1264F8", "work/ram/obj_1264F8", Obj_1264F8_Tick, rom);
    bad += run("Obj 0x126402", "work/ram/obj_126402", Obj_126402_Tick, rom);
    bad += run("Obj 0x1286E2", "work/ram/obj_1286E2", Obj_1286E2_Tick, rom);
    bad += run("Obj 0x1282B4", "work/ram/obj_1282B4", Obj_1282B4_Tick, rom);
    bad += run("Obj 0x1285FC", "work/ram/obj_1285FC", Obj_1285FC_Tick, rom);
    bad += run("Obj 0x12652C", "work/ram/obj_12652C", Obj_12652C_Tick, rom);
    bad += run("Obj 0x1287A4", "work/ram/obj_1287A4", Obj_1287A4_Tick, rom);
    bad += run("Obj 0x1287C4", "work/ram/obj_1287C4", Obj_1287C4_Tick, rom);
    bad += run("Obj 0x126562", "work/ram/obj_126562", Obj_126562_Tick, rom);
    bad += run("Obj 0x1281C4", "work/ram/obj_1281C4", Obj_1281C4_Tick, rom);
    bad += run("Obj 0x1265BE", "work/ram/obj_1265BE", Obj_1265BE_Tick, rom);
    bad += run("Obj 0x0CD124", "work/ram/obj_CD124", Obj_CD124_Tick, rom);
    bad += run("Obj 0x126608", "work/ram/obj_126608", Obj_126608_Tick, rom);
    bad += run("Obj 0x126670", "work/ram/obj_126670", Obj_126670_Tick, rom);
    bad += run("Obj 0x0CC7C0", "work/ram/obj_CC7C0", Obj_CC7C0_Tick, rom);
    bad += run("Obj 0x0CC860", "work/ram/obj_CC860", Obj_CC860_Tick, rom);
    bad += run("Obj 0x1266C2", "work/ram/obj_1266C2", Obj_1266C2_Tick, rom);
    bad += run("Obj 0x126590", "work/ram/obj_126590", Obj_126590_Tick, rom);
    bad += run("Obj 0x126710", "work/ram/obj_126710", Obj_126710_Tick, rom);
    bad += run("Obj 0x126744", "work/ram/obj_126744", Obj_126744_Tick, rom);
    bad += run("Obj 0x0CDDF8", "work/ram/obj_CDDF8", Obj_CDDF8_Tick, rom);
    bad += run("Obj 0x0CDF68", "work/ram/obj_CDF68", Obj_CDF68_Tick, rom);
    bad += run("Obj 0x0CDE50", "work/ram/obj_CDE50", Obj_CDE50_Tick, rom);
    bad += run("Obj 0x0CDF84", "work/ram/obj_CDF84", Obj_CDF84_Tick, rom);
    bad += run("Obj 0x0CC756", "work/ram/obj_CC756", Obj_CC756_Tick, rom);
    bad += run("Obj 0x126772", "work/ram/obj_126772", Obj_126772_Tick, rom);
    bad += run("Obj 0x1267BC", "work/ram/obj_1267BC", Obj_1267BC_Tick, rom);
    bad += run("Obj 0x1267F0", "work/ram/obj_1267F0", Obj_1267F0_Tick, rom);
    bad += run("Obj 0x0CDDB0", "work/ram/obj_CDDB0", Obj_CDDB0_Tick, rom);
    bad += run("Obj 0x126844", "work/ram/obj_126844", Obj_126844_Tick, rom);
    bad += run("Obj 0x1268D4", "work/ram/obj_1268D4", Obj_1268D4_Tick, rom);
    bad += run("Obj 0x126902", "work/ram/obj_126902", Obj_126902_Tick, rom);
    bad += run("Obj 0x12681E", "work/ram/obj_12681E", Obj_12681E_Tick, rom);
    bad += run("Obj 0x126936", "work/ram/obj_126936", Obj_126936_Tick, rom);
    bad += run("Obj 0x0CCDB6", "work/ram/obj_CCDB6", Obj_CCDB6_Tick, rom);
    bad += run("Obj 0x126B7E", "work/ram/obj_126B7E", Obj_126B7E_Tick, rom);
    bad += run("Obj 0x126976", "work/ram/obj_126976", Obj_126976_Tick, rom);
    bad += run("Obj 0x12687E", "work/ram/obj_12687E", Obj_12687E_Tick, rom);
    bad += run("Obj 0x1269C4", "work/ram/obj_1269C4", Obj_1269C4_Tick, rom);
    bad += run("Obj 0x126BAA", "work/ram/obj_126BAA", Obj_126BAA_Tick, rom);
    bad += run("Obj 0x11939E", "work/ram/obj_11939E", Obj_11939E_Tick, rom);
    bad += run("Obj 0x1269F2", "work/ram/obj_1269F2", Obj_1269F2_Tick, rom);
    bad += run("Obj 0x126A2A", "work/ram/obj_126A2A", Obj_126A2A_Tick, rom);
    bad += run("Obj 0x126086", "work/ram/obj_126086", Obj_126086_Tick, rom);
    bad += run("Obj 0x128D6C", "work/ram/obj_128D6C", Obj_128D6C_Tick, rom);
    bad += run("Obj 0x0CDDA6", "work/ram/obj_CDDA6", Obj_CDDA6_Tick, rom);
    bad += run("Obj 0x128E44", "work/ram/obj_128E44", Obj_128E44_Tick, rom);
    bad += run("Obj 0x12604C", "work/ram/obj_12604C", Obj_12604C_Tick, rom);
    bad += run("Obj 0x0C2BBE", "work/ram/obj_C2BBE", Obj_C2BBE_Tick, rom);
    bad += run("Obj 0x128E8C", "work/ram/obj_128E8C", Obj_128E8C_Tick, rom);
    bad += run("Obj 0x0C2C02", "work/ram/obj_C2C02", Obj_C2C02_Tick, rom);
    bad += run("Obj 0x126A7A", "work/ram/obj_126A7A", Obj_126A7A_Tick, rom);
    bad += run("Obj 0x0C2C22", "work/ram/obj_C2C22", Obj_C2C22_Tick, rom);
    bad += run("Obj 0x126AB4", "work/ram/obj_126AB4", Obj_126AB4_Tick, rom);
    bad += run("Obj 0x126AE2", "work/ram/obj_126AE2", Obj_126AE2_Tick, rom);
    bad += run("Obj 0x011B262", "work/ram/obj_11B262", Obj_11B262_Tick, rom);
    bad += run("Obj 0x011B288", "work/ram/obj_11B288", Obj_11B288_Tick, rom);
    bad += run("Obj 0x011B2CA", "work/ram/obj_11B2CA", Obj_11B2CA_Tick, rom);
    bad += run("Obj 0x011B09A", "work/ram/obj_11B09A", Obj_11B09A_Tick, rom);
    bad += run("Obj 0x011B06E", "work/ram/obj_11B06E", Obj_11B06E_Tick, rom);
    bad += run("Obj 0x0179550", "work/ram/obj_179550", Obj_179550_Tick, rom);
    bad += run("Obj 0x01795F6", "work/ram/obj_1795F6", Obj_1795F6_Tick, rom);
    bad += run("Obj 0x017960A", "work/ram/obj_17960A", Obj_17960A_Tick, rom);
    bad += run("Obj 0x0179572", "work/ram/obj_179572", Obj_179572_Tick, rom);
    bad += run("Obj 0x017961E", "work/ram/obj_17961E", Obj_17961E_Tick, rom);
    bad += run("Obj 0x01795A4", "work/ram/obj_1795A4", Obj_1795A4_Tick, rom);
    bad += run("Obj 0x0179632", "work/ram/obj_179632", Obj_179632_Tick, rom);
    bad += run("Obj 0x0179646", "work/ram/obj_179646", Obj_179646_Tick, rom);
    bad += run("Obj 0x017955E", "work/ram/obj_17955E", Obj_17955E_Tick, rom);
    bad += run("Obj 0x0179600", "work/ram/obj_179600", Obj_179600_Tick, rom);
    bad += run("Obj 0x0179614", "work/ram/obj_179614", Obj_179614_Tick, rom);
    bad += run("Obj 0x0179588", "work/ram/obj_179588", Obj_179588_Tick, rom);
    bad += run("Obj 0x0179628", "work/ram/obj_179628", Obj_179628_Tick, rom);
    bad += run("Obj 0x01795CA", "work/ram/obj_1795CA", Obj_1795CA_Tick, rom);
    bad += run("Obj 0x017963C", "work/ram/obj_17963C", Obj_17963C_Tick, rom);
    bad += run("Obj 0x0179650", "work/ram/obj_179650", Obj_179650_Tick, rom);
    /* captured with F3_EXIT=ret at the routine entry */
    bad += run("ObjScript 0x0CF1E4", "work/ram/vm_CF1E4", ObjScript_CF1E4, rom);
    bad += run_a1("ObjMove 0x0CF4B8", "work/ram/vm_CF4B8", ObjScript_CF4B8, rom);
    /* every script opcode forced through a synthetic script at 0x41E000:
       F3_POKE32=41E000=<op>0010,41E004=00100000,41E008=0 F3_POKEA0B=44=00,45=41,46=E0,47=00
       (ops 17/24/25 jump back by their parameter: captured with parameter 0) */
    for (int op = 0; op < 29; op++) {
        char pfx[64], name[32];
        snprintf(pfx, sizeof pfx, "work/ram/vmop_CF1E4_%02d", op);
        snprintf(name, sizeof name, "ObjScript_CF1E4 op %2d", op);
        bad += run(name, pfx, ObjScript_CF1E4, rom);
    }
    /* every movement mode forced: F3_POKEA0B=20=<mode> (CF4B8 dispatches on obj+0x20) */
    for (int op = 0; op < 29; op++) {
        char pfx[64], name[32];
        snprintf(pfx, sizeof pfx, "work/ram/vmop_CF4B8_%02d", op);
        snprintf(name, sizeof name, "ObjMove_CF4B8 mode %2d", op);
        bad += run_a1(name, pfx, ObjScript_CF4B8, rom);
    }
    /* Shared jsr primitives: one representative call shape each. These have no
       per-routine golden capture, so they are checked against the ROM formula
       directly (Obj_Atan2/Obj_Distance go through the 0x026E40 / 0x028340
       tables and are covered by every object routine that calls them). */
    {
        uint8_t w[F3_WRAM_SIZE] = {0}, g2[F3_GFX_SIZE] = {0};
        f3mem_t sm = { w, (const uint8_t *)rom, NULL, NULL, g2 };
        int bad_s = 0;
        wr16(&sm, F3_A5 + 0x1D60u, 0x0102u);   /* x' = x + 4x + 1; return x' >> 8 */
        bad_s += Obj_Next4Way(&sm) != (uint16_t)((0x0102u + (uint16_t)(0x0102u << 2) + 1u) >> 8);
        wr32(&sm, 0x410060u + 0x48u, 0u);
        wr32(&sm, F3_A5 + 0x1D50u, 0x410060u);
        Obj_RelayTimer(&sm, 0x12Cu);
        bad_s += rd32(&sm, 0x410060u + 0x48u) != 0x12Cu;
        Obj_StartScript(&sm, 0x410100u, 0x00105160u);
        bad_s += rd32(&sm, 0x410100u + 0x44u) != 0x00105160u;
        bad_s += (rd8(&sm, 0x410100u + 0x07u) & 0x02u) == 0u;
        bad_s += Obj_Distance(&sm, 0u, 0u) != 0u;
        printf("objects: %-22s %3d/%3d ticks bit-exact (shared primitives)\n",
               "Shared primitives", bad_s ? 0 : 6, 6);
        bad += bad_s;
    }
    /* 0x179A5C animation engine: index 0 walks a frame list at +0x3A. */
    {
        uint8_t w[F3_WRAM_SIZE] = {0}, g2[F3_GFX_SIZE] = {0};
        f3mem_t am = { w, (const uint8_t *)rom, NULL, NULL, g2 };
        int bad_a = 0;
        /* index 0 with a positive duration only counts down */
        wr32(&am, 0x410100u + 0x3Au, 0x00020000u);
        wr16(&am, 0x410100u + 0x3Eu, 3u);
        bad_a += Obj_AnimTick(&am, 0x410100u, 0u) != 0;
        bad_a += rd16(&am, 0x410100u + 0x3Eu) != 2u;
        bad_a += rd16(&am, 0x410100u + 0x40u) != 0u;
        /* unimplemented indices must refuse before touching the object */
        uint16_t keep = rd16(&am, 0x410100u + 0x3Eu);
        bad_a += Obj_AnimTick(&am, 0x410100u, 3u) != -1;
        bad_a += Obj_AnimInit(&am, 0x410100u, 17u) != -1;
        bad_a += rd16(&am, 0x410100u + 0x3Eu) != keep;
        printf("objects: %-22s %3d/%3d ticks bit-exact (0x179A5C index 0)\n",
               "Anim engine 0x179A5C", bad_a ? 0 : 4, 4);
        bad += bad_a;
    }
    free(rom);
    return bad ? 1 : 0;
}
