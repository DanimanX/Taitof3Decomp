# Lotes de decompilación de objetos — cómo se publica el progreso

La pila vive en `scripts/decomp_planner.py`. El estado de cada rutina intentada
queda en `work/batches/state.tsv` con una de tres marcas:

- `accepted` — verificada bit-exacta con MAME y ya en `src/enemy/`
- `rejected` — intentada y descartada; el stub sigue en su sitio (la razón está
  en el mensaje del commit que la marcó)
- `tried` — intentada, sin veredicto todavía

## Ciclo de un lote

```bash
python3 scripts/decomp_planner.py status          # qué está listo y por qué no
python3 scripts/decomp_planner.py next --n 6      # arma el siguiente lote
# ... traducir (workers Codewhale / hermes) ...
python3 scripts/decomp_planner.py verify          # veredicto por rutina
python3 scripts/decomp_planner.py mark A1 A2 --state accepted
python3 scripts/decomp_planner.py mark B1     --state rejected
```

## Publicar (obligatorio al cerrar cada lote)

Ningún lote se da por terminado hasta que `origin/main` lo tiene. Después de
verificar y marcar:

```bash
bash scripts/object_health.sh                     # ACCEPTED=n, failing=0
for t in kernel player collision sound objpool stage sprite; do ./build/test_$t; done
make coverage                                     # regenera docs/COVERAGE.md + README
git add -A src/enemy docs/COVERAGE.md README.md work/batches
git commit -m "feat: N more object routines verified bit-exact (wave NN)"
git push origin main
git status -sb                                    # debe decir: ## main...origin/main
```

Reglas que no se saltan:

- solo se commitea lo que pasó `F3_ONLY=ADDR ./build/test_objects` con
  `N/N ticks bit-exact` y `0 reach undecompiled`;
- lo rechazado se restaura al stub (`git checkout HEAD -- src/enemy/obj_ADDR.c`)
  antes de commitear, no se deja a medias;
- `git status -sb` sin `[ahead N]` es la prueba de que el push llegó.

`scripts/harvest.sh --push` hace commit+push por su cuenta para el flujo de
worktrees; este documento es el equivalente para el flujo del planner en el
checkout principal.
