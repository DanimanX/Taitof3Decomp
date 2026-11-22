CC ?= cc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Werror
SRC = src/kernel/rng.c src/io/input.c src/common/f3mem.c src/player/player_move.c src/collision/collision.c src/audio/sound_cmd.c src/objects/obj_pool.c src/objects/obj_common.c src/objects/objscript_CF1E4.c src/objects/objscript_CF4B8.c src/stage/stage_timeline.c src/video/sprite.c src/enemy/enemy_bullet.c src/player/player_shot.c src/enemy/obj_D9116.c src/enemy/obj_CABF4.c src/enemy/obj_EAD28.c src/enemy/obj_CC11C.c src/enemy/obj_DBF70.c src/enemy/obj_DBF16.c src/enemy/obj_D9E06.c src/enemy/obj_D9D9E.c src/enemy/obj_F4E1E.c src/enemy/obj_F4DDE.c src/enemy/obj_F4D52.c src/enemy/obj_108DFC.c src/enemy/obj_108D8E.c src/enemy/obj_E7DEE.c src/enemy/obj_E7E1C.c src/enemy/obj_E6E54.c src/enemy/obj_E6E0E.c src/enemy/obj_E6C64.c src/enemy/obj_E6C36.c src/enemy/obj_E6906.c src/enemy/obj_E68C2.c src/enemy/obj_E7024.c src/enemy/obj_E7036.c src/enemy/obj_E6508.c src/enemy/obj_E64DA.c src/enemy/obj_F131C.c src/enemy/obj_F0DB8.c src/enemy/obj_F0D8A.c src/enemy/obj_F0D7E.c src/enemy/obj_113644.c src/enemy/obj_114814.c src/enemy/obj_13EE50.c src/enemy/obj_13EE08.c src/enemy/obj_1131A0.c src/enemy/obj_1134E6.c src/enemy/obj_114984.c src/enemy/obj_114800.c src/enemy/obj_114926.c src/enemy/obj_114A10.c src/enemy/obj_114A76.c src/enemy/obj_11464E.c src/enemy/obj_10AFCC.c src/enemy/obj_10AFBA.c src/enemy/obj_10AC10.c src/enemy/obj_10AB30.c src/enemy/obj_10AAC2.c src/enemy/obj_108E8E.c src/enemy/obj_109514.c src/enemy/obj_1094A6.c src/enemy/obj_E81A0.c src/enemy/obj_E8240.c src/enemy/obj_DA6C0.c src/enemy/obj_E8494.c src/enemy/obj_DA64C.c src/enemy/obj_E7A7A.c src/enemy/obj_E7ABE.c src/enemy/obj_E70CE.c src/enemy/obj_F5CF6.c src/enemy/obj_F5D58.c src/enemy/obj_E6082.c src/enemy/obj_F5AFE.c src/enemy/obj_E5C2A.c src/enemy/obj_E60EA.c src/enemy/obj_DC358.c src/enemy/obj_E5CBC.c src/enemy/obj_E61AA.c src/enemy/obj_E6166.c src/enemy/obj_1088F0.c src/enemy/obj_F0814.c src/enemy/obj_E598C.c src/enemy/obj_DC486.c src/enemy/obj_1087E8.c src/enemy/obj_108936.c src/enemy/obj_F0846.c src/enemy/obj_F0788.c src/enemy/obj_118D7C.c src/enemy/obj_118DDC.c src/enemy/obj_1134BA.c src/enemy/obj_114108.c src/enemy/obj_112D88.c src/enemy/obj_118CF6.c src/enemy/obj_112C3C.c src/enemy/obj_1140BE.c src/enemy/obj_112C1A.c src/enemy/obj_113ED0.c src/enemy/obj_1134B2.c src/enemy/obj_E101A.c src/enemy/obj_112D1A.c src/enemy/obj_E1036.c src/enemy/obj_E077A.c src/enemy/obj_1180F8.c src/enemy/obj_CD556.c src/enemy/obj_CD5EA.c src/enemy/obj_CD504.c src/enemy/obj_CD646.c src/enemy/obj_D747E.c src/enemy/obj_DC5B6.c src/enemy/obj_D740E.c src/enemy/obj_ECB3A.c src/enemy/obj_1049C4.c src/enemy/obj_EA45E.c src/enemy/obj_E1328.c src/enemy/obj_DC6D0.c src/enemy/obj_F35B4.c src/enemy/obj_10967C.c src/enemy/obj_DB074.c src/enemy/obj_EA848.c src/enemy/obj_E1886.c src/enemy/obj_E96C8.c src/enemy/obj_E9DE0.c src/enemy/obj_BDE92.c src/enemy/obj_D7306.c src/enemy/obj_12EB90.c src/enemy/obj_DA138.c src/enemy/obj_12EE24.c src/enemy/obj_D9FC8.c src/enemy/obj_BC70A.c src/enemy/obj_BD9BE.c src/enemy/obj_104F26.c src/enemy/obj_13EFA0.c src/enemy/obj_BE16C.c src/enemy/obj_BD7E4.c src/enemy/obj_BD550.c src/enemy/obj_BD3A4.c src/enemy/obj_BC08C.c src/enemy/obj_BDC5C.c src/enemy/obj_BDDE2.c src/enemy/obj_11A8A8.c src/enemy/obj_BC1E8.c src/enemy/obj_BC16C.c src/enemy/obj_12EF40.c src/enemy/obj_BD0EC.c src/enemy/obj_12F29E.c src/enemy/obj_12CFD4.c src/enemy/obj_CAFB8.c src/enemy/obj_C9DEC.c src/enemy/obj_E1B36.c src/enemy/obj_CB97E.c src/enemy/obj_BD700.c src/enemy/obj_11C3A8.c src/enemy/obj_CB9EE.c src/enemy/obj_ECAC8.c src/enemy/obj_11C1FC.c src/enemy/obj_BDDB8.c src/enemy/obj_BD2C2.c src/enemy/obj_BCC96.c src/enemy/obj_EACEA.c src/enemy/obj_10963E.c src/enemy/obj_BC2A0.c src/enemy/obj_F3582.c src/enemy/obj_CAF90.c src/enemy/obj_CB422.c src/enemy/obj_CB054.c src/enemy/obj_BD308.c src/enemy/obj_ECFDA.c src/enemy/obj_CBA34.c src/enemy/obj_C9B3E.c src/enemy/obj_104EFA.c src/enemy/obj_EA29E.c src/enemy/obj_BDE42.c src/enemy/obj_BC576.c src/enemy/obj_BBBF4.c src/enemy/obj_ECFAE.c src/enemy/obj_ECF0C.c src/enemy/obj_EA418.c src/enemy/obj_ECA04.c src/enemy/obj_DA0D6.c src/enemy/obj_CBC78.c src/enemy/obj_BBAE4.c src/enemy/obj_102A70.c src/enemy/obj_102844.c src/enemy/obj_102798.c src/enemy/obj_E186A.c src/enemy/obj_10290C.c src/enemy/obj_CB2EC.c src/enemy/obj_12CE5A.c src/enemy/obj_12EAD4.c src/enemy/obj_BD990.c src/enemy/obj_12EA24.c src/enemy/obj_1116E2.c src/enemy/obj_F34F6.c src/enemy/obj_1095B0.c src/enemy/obj_10524A.c src/enemy/obj_EA836.c src/enemy/obj_EA28C.c src/enemy/obj_CAEBA.c src/enemy/obj_BBB18.c src/enemy/obj_105268.c src/enemy/obj_DC674.c src/enemy/obj_DBDE8.c src/enemy/obj_BBEC0.c src/enemy/obj_BBD94.c src/enemy/obj_BB952.c src/enemy/obj_ECF3E.c src/enemy/obj_DBDA2.c src/enemy/obj_BCE52.c src/enemy/obj_BBE50.c src/enemy/obj_BB5C2.c src/enemy/obj_12EB0E.c src/enemy/obj_BCFF4.c src/enemy/obj_BC680.c src/enemy/obj_12E19A.c src/enemy/obj_12EDA8.c src/enemy/obj_CAF5E.c src/enemy/obj_BCDC0.c src/enemy/obj_BB874.c src/enemy/obj_BB86A.c src/enemy/obj_BB860.c src/enemy/obj_BB856.c src/enemy/obj_BB84C.c src/enemy/obj_BE262.c src/enemy/obj_BCF30.c src/enemy/obj_BBD00.c src/enemy/obj_12E1BE.c src/enemy/obj_BCEB0.c src/enemy/obj_BB692.c src/enemy/obj_CAF1A.c src/enemy/obj_BC24A.c src/enemy/obj_BC422.c src/enemy/obj_BC418.c src/enemy/obj_BC40E.c src/enemy/obj_BC404.c src/enemy/obj_BC3FA.c src/enemy/obj_CAF3E.c src/enemy/obj_CA28C.c src/enemy/obj_12E21A.c src/enemy/obj_11A87E.c src/enemy/obj_BC540.c src/enemy/obj_12EA9E.c src/enemy/obj_11C1DE.c src/enemy/obj_BBE2A.c src/enemy/obj_BBD6E.c src/enemy/obj_BBCDA.c src/enemy/obj_BCD90.c src/enemy/obj_CB2C6.c src/enemy/obj_BC03C.c src/enemy/obj_BB746.c src/enemy/obj_CABAC.c src/enemy/obj_BBE9A.c src/enemy/obj_BBDE0.c src/enemy/obj_BBD24.c src/enemy/obj_12F27A.c src/enemy/obj_12F26A.c src/enemy/obj_BCE38.c src/enemy/obj_BCF08.c src/enemy/obj_12E1FA.c src/enemy/obj_BD354.c src/enemy/obj_BCE8E.c src/enemy/obj_BBFE8.c src/enemy/obj_D7570.c src/enemy/obj_BD340.c src/enemy/obj_BC008.c src/enemy/obj_BBBE0.c src/enemy/obj_BD256.c src/enemy/obj_BC4A8.c src/enemy/obj_BBE06.c src/enemy/obj_BBD4A.c src/enemy/obj_12EF24.c src/enemy/obj_CB412.c src/enemy/obj_BD298.c src/enemy/obj_BC638.c src/enemy/obj_BC064.c src/enemy/obj_BC3DE.c src/enemy/obj_BC1CE.c src/enemy/obj_BC152.c src/enemy/obj_CB0DA.c src/enemy/obj_CB406.c src/enemy/obj_C685A.c src/enemy/obj_BD544.c src/enemy/obj_BC07E.c src/enemy/obj_11C39A.c src/enemy/obj_CBC98.c src/enemy/obj_BCD82.c src/enemy/obj_BCA00.c src/enemy/obj_BC3EC.c src/enemy/obj_BB83E.c src/enemy/obj_CBC90.c src/enemy/obj_CBC88.c src/enemy/obj_DF844.c src/enemy/obj_DF7EA.c src/enemy/obj_DC0F8.c src/enemy/obj_DC328.c src/enemy/obj_13E4EE.c src/enemy/obj_117C28.c src/enemy/obj_117B44.c src/enemy/obj_102F52.c src/enemy/obj_F30B4.c src/enemy/obj_E0266.c src/enemy/obj_E029E.c src/enemy/obj_DEC9E.c src/enemy/obj_DC586.c src/enemy/obj_DC456.c src/enemy/obj_D9092.c src/enemy/obj_CC0F4.c src/enemy/obj_D901A.c src/enemy/obj_D540A.c src/enemy/obj_CBF6C.c src/enemy/obj_E02C4.c src/enemy/obj_DC542.c src/enemy/obj_C9D7E.c src/enemy/obj_C9978.c src/enemy/obj_C6866.c src/enemy/obj_DC3F6.c src/enemy/obj_F302E.c src/enemy/obj_117BDE.c src/enemy/obj_E0278.c src/enemy/obj_EACDE.c src/enemy/obj_DECF8.c src/enemy/obj_E04CC.c src/enemy/obj_F30F6.c src/enemy/obj_DC8BE.c src/enemy/obj_DCDF2.c src/enemy/obj_CBF8A.c src/enemy/obj_119714.c src/enemy/obj_CAB36.c src/enemy/obj_13E5B2.c src/enemy/obj_D5464.c src/enemy/obj_117D06.c src/enemy/obj_119CBA.c src/enemy/obj_CBF5E.c src/enemy/obj_119990.c src/enemy/obj_1027B4.c src/enemy/obj_102860.c src/enemy/obj_102928.c src/enemy/obj_C9F2E.c src/enemy/obj_10252E.c src/enemy/obj_102A9A.c

.PHONY: test clean
test: build/test_kernel build/test_player build/test_collision build/test_sound build/test_objpool build/test_stage build/test_sprite build/test_objects
	./build/test_kernel
	./build/test_player
	./build/test_collision
	./build/test_sound
	./build/test_objpool
	./build/test_stage
	./build/test_sprite
	./build/test_objects

build/test_kernel: tests/test_kernel.c $(SRC) src/kernel/rng.h src/io/input.h
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ tests/test_kernel.c src/kernel/rng.c src/io/input.c

build/test_player: tests/test_player.c $(SRC) src/common/f3mem.h src/player/player_move.h
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ tests/test_player.c $(SRC)

build/test_collision: tests/test_collision.c $(SRC) src/collision/collision.h
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ tests/test_collision.c $(SRC)

build/test_sound: tests/test_sound.c $(SRC) src/audio/sound_cmd.h
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ tests/test_sound.c $(SRC)

build/test_objpool: tests/test_objpool.c $(SRC) src/objects/obj_pool.h
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ tests/test_objpool.c $(SRC)

build/test_stage: tests/test_stage.c $(SRC) src/stage/stage_timeline.h
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ tests/test_stage.c $(SRC)

build/test_sprite: tests/test_sprite.c tests/capture.h $(SRC) src/video/sprite.h
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ tests/test_sprite.c $(SRC)

build/test_objects: tests/test_objects.c tests/capture.h $(SRC)
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ tests/test_objects.c $(SRC)

clean:
	rm -rf build

.PHONY: coverage
coverage:
	python3 scripts/coverage.py
