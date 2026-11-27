#!/usr/bin/env bash
# Poke-driven recaptures of the 7 weak routines (entry-only pokes force the
# branch no zone reaches; the pre dump already holds the poked values).
cd /PROCESOS/Taitof3Decomp
run() { a=$1; shift; env "$@" python3 scripts/zcap_weak.py $a 60 > work/zcap/recap/poke/$a.out 2>&1; cat work/zcap/recap/poke/$a.out; }
run 1659B0 F3_POKE8=402268=01,402269=C1 &
run 12C2EA F3_POKE8=40B29B=01 &
run 13AD8A F3_POKE8=40B2D6=00,40B2D7=02 &
run 192954 F3_POKE8=40B30C=16,40B30D=51 &
run 198CEC F3_POKE8=40CB3A=01 &
run 0C8BCE F3_POKEA0W=22=0000 F3_POKE32=61CD80=11223344,61CE00=11223344,61CD84=11223344,61CE04=11223344,61CD88=11223344,61CE08=11223344,61CD8C=11223344,61CE0C=11223344,61CD90=11223344,61CE10=11223344 &
run 12AB7C F3_POKEA0W=36=0041,38=00CC &
wait
