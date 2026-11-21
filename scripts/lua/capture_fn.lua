-- Exact pre/post Work RAM capture around a code region using the MAME
-- debugger (run with: -debug -debugger none). Generic "golden vector" tool.
-- Env:
--   F3_ENTRY   hex PC where "pre" is taken   (e.g. C63F2)
--   F3_EXIT    hex PC(s) where "post" is taken, comma-separated (e.g. C63F0),
--              or "ret": post at the return address of the entry call
--              (entry must be the first instruction of a jsr/bsr target)
--   F3_SPR     "1" also dumps sprite RAM 0x600000-0x60FFFF to <prefix>_spr.bin
--   F3_DPRAM   "1" also dumps the 2 KiB sound DPRAM (0xC00000) to <prefix>_dpram.bin
--   F3_SAMPLES number of pre/post pairs       (default 300)
--   F3_OUT     output prefix (default work/ram/fn) -> <prefix>_pairs.bin, <prefix>_regs.tsv
--   F3_SWEEP   "N" overwrites the word arg at SP+4 on entry with pair_index % N
--              (forces every command id through a routine like Sound_PlayCmd);
--              F3_SWEEP_BASE adds a starting id
--   F3_LOG     hex PC(s), comma-separated: every hit between pre and post is
--              written to <prefix>_regs.tsv as which="log" (no RAM dump)
--   F3_POKE32  "addr=val,addr=val" (hex) 32-bit writes applied at each entry
--              before the pre dump (force rare branches)
--   F3_COND    MAME debugger condition on the entry breakpoint, e.g.
--              "a1==102fe6" (capture one object routine at the tick call site)
--   F3_POKEA0B "off=val,..." (hex) byte writes at A0+off at each entry
--   F3_POKEFN  hex fn: apply F3_POKEA0B only when A1==fn, and do not capture
--              those calls (force a parent object, e.g. HP, while F3_COND also
--              matches the children to capture)
--   F3_MAXFRAMES stop after this many frames even if short of samples
--              (default 6000; a stopped -debug MAME ignores SIGTERM)
--   F3_INPUT   "fire" = P1 autofire + sweeping joystick (default), "none"
local ROOT = "/PROCESOS/Taitof3Decomp"
local entry = tonumber(os.getenv("F3_ENTRY"), 16)
local exits = {}
local exit_ret = os.getenv("F3_EXIT") == "ret"
if not exit_ret then
  for h in string.gmatch(os.getenv("F3_EXIT"), "[^,]+") do exits[tonumber(h, 16)] = true end
end
local ret_pc, ret_sp, ret_bp = nil, nil, nil
local want_dpram = os.getenv("F3_DPRAM") == "1"
local logs = {}
for h in string.gmatch(os.getenv("F3_LOG") or "", "[^,]+") do logs[tonumber(h, 16)] = true end
local pokes = {}
for a, v in string.gmatch(os.getenv("F3_POKE32") or "", "(%x+)=(%x+)") do pokes[#pokes + 1] = {tonumber(a, 16), tonumber(v, 16)} end
local pokesa0 = {}
for o, v in string.gmatch(os.getenv("F3_POKEA0B") or "", "(%x+)=(%x+)") do pokesa0[#pokesa0 + 1] = {tonumber(o, 16), tonumber(v, 16)} end
local pokefn = os.getenv("F3_POKEFN") and tonumber(os.getenv("F3_POKEFN"), 16)
local sweep = tonumber(os.getenv("F3_SWEEP") or "0")
local sweep_base = tonumber(os.getenv("F3_SWEEP_BASE") or "0")
local want = tonumber(os.getenv("F3_SAMPLES") or "300")
local prefix = os.getenv("F3_OUT") or (ROOT .. "/work/ram/fn")
local inmode = os.getenv("F3_INPUT") or "fire"
local dbg = manager.machine.debugger
local cpu = manager.machine.devices[":maincpu"]
local sp = cpu.spaces["program"]
local p = manager.machine.ioport.ports
local out = assert(io.open(prefix .. "_pairs.bin", "wb"))
local regs = assert(io.open(prefix .. "_regs.tsv", "w"))
regs:write("pair\twhich\tframe\tD0\tD1\tD2\tD3\tD4\tD5\tD6\tD7\tA0\tA1\tA2\tA3\tA4\tA5\tA6\tSP\n")
cpu.debug:bpset(entry, os.getenv("F3_COND") or "", "")
for pc, _ in pairs(exits) do cpu.debug:bpset(pc, "", "") end
for pc, _ in pairs(logs) do cpu.debug:bpset(pc, "", "") end
local dp = want_dpram and assert(io.open(prefix .. "_dpram.bin", "wb")) or nil
local spf = os.getenv("F3_SPR") == "1" and assert(io.open(prefix .. "_spr.bin", "wb")) or nil
-- snapshot() returns the dumps as strings; pairs are written only when the
-- post is reached, so an abandoned pre (routine left without returning)
-- never desynchronises the output files.
local function snapshot()
  local t = {}
  for a = 0x400000, 0x41FFFF, 4 do t[#t + 1] = string.pack(">I4", sp:read_u32(a)) end
  local snap = { w = table.concat(t) }
  if dp then
    local d = {}
    for a = 0xC00000, 0xC007FF do d[#d + 1] = string.char(sp:read_u8(a)) end
    snap.d = table.concat(d)
  end
  if spf then
    local t2 = {}
    for a = 0x600000, 0x60FFFF, 4 do t2[#t2 + 1] = string.pack(">I4", sp:read_u32(a)) end
    snap.s = table.concat(t2)
  end
  return snap
end
local function emit(snap)
  out:write(snap.w)
  if dp then dp:write(snap.d) end
  if spf then spf:write(snap.s) end
end
local function regline_str(i, which)
  local r = {}
  for k = 0, 7 do r[#r + 1] = string.format("%08X", cpu.state["D" .. k].value) end
  for k = 0, 6 do r[#r + 1] = string.format("%08X", cpu.state["A" .. k].value) end
  r[#r + 1] = string.format("%08X", cpu.state["SP"].value)
  return string.format("%d\t%s\t%04X\t%s\n", i, which, sp:read_u16(0x4022AC), table.concat(r, "\t"))
end
local pairs_done, have_pre, n = 0, false, 0
local pre_snap, pre_regs, pending_logs, abandoned = nil, nil, {}, 0
local maxframes = tonumber(os.getenv("F3_MAXFRAMES") or "6000")
local function finish(tag)
  out:close(); regs:close(); if dp then dp:close() end; if spf then spf:close() end
  print(string.format("CAPTURE %s pairs=%d abandoned=%d", tag, pairs_done, abandoned)); manager.machine:exit()
end
emu.register_periodic(function()
  if dbg.execution_state ~= "stop" then return end
  local pc = cpu.state["PC"].value
  local spv = cpu.state["SP"].value
  if pokefn and pc == entry and not have_pre and cpu.state["A1"].value == pokefn then
    for _, pv in ipairs(pokesa0) do sp:write_u8(cpu.state["A0"].value + pv[1], pv[2]) end
    dbg.execution_state = "run"; return
  end
  if pc == entry and have_pre and exit_ret and spv ~= ret_sp then
    -- previous call never returned (e.g. a script opcode popped the return
    -- address): drop it and start over with this call
    if ret_bp then cpu.debug:bpclear(ret_bp); ret_bp = nil; ret_pc = nil end
    have_pre = false; abandoned = abandoned + 1
  end
  if pc == entry and not have_pre then
    for _, pv in ipairs(pokes) do sp:write_u32(pv[1], pv[2]) end
    if not pokefn then for _, pv in ipairs(pokesa0) do sp:write_u8(cpu.state["A0"].value + pv[1], pv[2]) end end
    if sweep > 0 then sp:write_u16(spv + 4, sweep_base + pairs_done % sweep) end
    pre_snap = snapshot(); pre_regs = regline_str(pairs_done, "pre"); pending_logs = {}; have_pre = true
    if exit_ret then
      ret_sp = spv
      ret_pc = sp:read_u32(ret_sp) & 0xFFFFFF
      ret_bp = cpu.debug:bpset(ret_pc, "", "")
    end
  elseif logs[pc] and have_pre then
    pending_logs[#pending_logs + 1] = regline_str(pairs_done, "log")
  elseif have_pre and (exits[pc] or (exit_ret and pc == ret_pc and spv == ret_sp + 4)) then
    if ret_bp then cpu.debug:bpclear(ret_bp); ret_bp = nil; ret_pc = nil end
    emit(pre_snap); regs:write(pre_regs); regs:write(table.concat(pending_logs))
    emit(snapshot()); regs:write(regline_str(pairs_done, "post")); have_pre = false
    out:flush(); regs:flush(); if dp then dp:flush() end; if spf then spf:flush() end
    pairs_done = pairs_done + 1
    if pairs_done >= want then finish("DONE"); return end
  end
  dbg.execution_state = "run"
end)
emu.register_frame_done(function()
  n = n + 1
  if n >= maxframes then finish("PARTIAL"); return end
  if inmode == "fire" then
    p[":IN.0"].fields["P1 Button 1"]:set_value((n // 3) % 2)
    local k = (n // 45) % 4
    p[":IN.1"].fields["P1 Up"]:set_value(k == 0 and 1 or 0)
    p[":IN.1"].fields["P1 Right"]:set_value(k == 1 and 1 or 0)
    p[":IN.1"].fields["P1 Down"]:set_value(k == 2 and 1 or 0)
    p[":IN.1"].fields["P1 Left"]:set_value(k == 3 and 1 or 0)
  end
end)
