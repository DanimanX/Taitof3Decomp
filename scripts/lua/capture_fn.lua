-- Exact pre/post Work RAM capture around a code region using the MAME
-- debugger (run with: -debug -debugger none). Generic "golden vector" tool.
-- Env:
--   F3_ENTRY   hex PC where "pre" is taken   (e.g. C63F2)
--   F3_LINGER  N: delay the "post" dump by N further entries of F3_ENTRY, so
--              a successor state's writes land in the same pair
--   F3_EXIT    hex PC(s) where "post" is taken, comma-separated (e.g. C63F0),
--              or "ret": post at the return address of the entry call,
--              or "a6": post at the PC in A6 at entry (jmp (a6) returns)
--              (entry must be the first instruction of a jsr/bsr target)
--   F3_SPR     "1" also dumps sprite RAM 0x600000-0x60FFFF to <prefix>_spr.bin
--   F3_GFX     "1" also dumps playfield/text/char/line RAM 0x610000-0x62FFFF to <prefix>_gfx.bin
--   F3_DPRAM   "1" also dumps the 2 KiB sound DPRAM (0xC00000) to <prefix>_dpram.bin
--   F3_SAMPLES number of pre/post pairs       (default 300)
--   F3_OUT     output prefix (default work/ram/fn) -> <prefix>_pairs.bin, <prefix>_regs.tsv
--   F3_SWEEP   "N" overwrites the word arg at SP+4 on entry with pair_index % N
--              (forces every command id through a routine like Sound_PlayCmd);
--              F3_SWEEP_BASE adds a starting id
--   F3_LOGW    hex offset into A0: print that 16-bit field with each F3_LOG hit
--   F3_LOGA0   hex A0: only record F3_LOG visits whose A0 matches (and the log
--              column carries the PC so the chain can be walked)
--   F3_LOG     hex PC(s), comma-separated: every hit between pre and post is
--              written to <prefix>_regs.tsv as which="log" (no RAM dump)
--   F3_POKE32  "addr=val,addr=val" (hex) 32-bit writes applied at each entry
--              before the pre dump (force rare branches)
--   F3_POKE8   "addr=val,..." (hex) byte writes applied at each entry before
--              the pre dump (a flag byte without touching its neighbour)
--   F3_EXITPC  "1" appends the PC (entry for pre, the exit reached for post)
--              as a 17th regs.tsv column: segment captures of task code,
--              where the exit list holds every TRAP #4/#5 the code can reach
--   F3_NOIRQ   "1" raises the 68k interrupt mask to 7 from the entry to the
--              exit of each captured call and restores it after the post
--              dump: a long routine is otherwise always cut by VBlank, whose
--              handler writes land in the post state (test_fn skips those)
--   F3_REMASK  hex PC(s) of 4-byte "andi #$F0FF,SR" (e.g. 16A4, the end of
--              0x1660's critical section): with F3_NOIRQ, inside a captured
--              call that instruction is skipped so the mask stays raised (the
--              IRQ would be taken before the next instruction otherwise)
--   F3_COND    MAME debugger condition on the entry breakpoint, e.g.
--              "a1==102fe6" (capture one object routine at the tick call site)
--   F3_POKEA0B "off=val,..." (hex) byte writes at A0+off at each entry
--   F3_POKEA0W "off=val,..." (hex) 16-bit writes at A0+off at each entry
--   F3_POKECAPTURE "1" makes F3_POKEFN poke *and* capture the call, instead
--              of skipping it (needed to drive a tail through its exit)
--   F3_POKEA0P "ptr:off=val,..." (hex) byte writes at rd32(A0+ptr)+off,
--              applied with the A0 pokes (e.g. "8:6=05": parent's +6)
--   F3_POKEFN  hex fn: apply F3_POKEA0B only when A1==fn, and do not capture
--              those calls (force a parent object, e.g. HP, while F3_COND also
--              matches the children to capture)
--   F3_MAXFRAMES stop after this many frames even if short of samples
--              (default 6000; a stopped -debug MAME ignores SIGTERM)
--   F3_INPUT   "fire" = P1 autofire + sweeping joystick (default), "up"/"down" = autofire
--              holding one direction, "none"
local ROOT = "/PROCESOS/Taitof3Decomp"
local entry = tonumber(os.getenv("F3_ENTRY"), 16)
local exits = {}
local exit_ret = os.getenv("F3_EXIT") == "ret"
-- "a6": post at the PC held in A6 at entry (stackless routines that return
-- with jmp (a6), e.g. the boot RAM tests around 0x0047C0)
local exit_a6 = os.getenv("F3_EXIT") == "a6"
if not exit_ret and not exit_a6 then
  for h in string.gmatch(os.getenv("F3_EXIT"), "[^,]+") do exits[tonumber(h, 16)] = true end
end
-- F3_LINGER=N: take the "post" dump N entries of the tick-list site *after*
-- this routine's return, so effects applied by the successor state (e.g.
-- 0x17C240 adding the velocity to +0x14) are included in the same pair.
local linger = tonumber(os.getenv("F3_LINGER") or "0")
local linger_left = 0
local ret_pc, ret_sp, ret_bp = nil, nil, nil
local want_dpram = os.getenv("F3_DPRAM") == "1"
local logs = {}
for h in string.gmatch(os.getenv("F3_LOG") or "", "[^,]+") do logs[tonumber(h, 16)] = true end
local pokes = {}
for a, v in string.gmatch(os.getenv("F3_POKE32") or "", "(%x+)=(%x+)") do pokes[#pokes + 1] = {tonumber(a, 16), tonumber(v, 16)} end
local noirq = os.getenv("F3_NOIRQ") == "1"
local saved_sr = nil
local remask = {}
for h in string.gmatch(os.getenv("F3_REMASK") or "", "[^,]+") do remask[tonumber(h, 16)] = true end
local pokes8 = {}
for a, v in string.gmatch(os.getenv("F3_POKE8") or "", "(%x+)=(%x+)") do pokes8[#pokes8 + 1] = {tonumber(a, 16), tonumber(v, 16)} end
local pokesa0 = {}
for o, v in string.gmatch(os.getenv("F3_POKEA0B") or "", "(%x+)=(%x+)") do pokesa0[#pokesa0 + 1] = {tonumber(o, 16), tonumber(v, 16)} end
local pokesa0w = {}
for o, v in string.gmatch(os.getenv("F3_POKEA0W") or "", "(%x+)=(%x+)") do pokesa0w[#pokesa0w + 1] = {tonumber(o, 16), tonumber(v, 16)} end
local pokesa0p = {}
for q, o, v in string.gmatch(os.getenv("F3_POKEA0P") or "", "(%x+):(%x+)=(%x+)") do pokesa0p[#pokesa0p + 1] = {tonumber(q, 16), tonumber(o, 16), tonumber(v, 16)} end
-- F3_LOGA0: hex A0 value; F3_LOG only records visits whose A0 matches, so a
-- trace is not polluted by other objects entering the same helper in the frame
local loga0 = os.getenv("F3_LOGA0") and tonumber(os.getenv("F3_LOGA0"), 16)
-- F3_LOGW: hex offset into A0 to print alongside each F3_LOG hit, so a trace
-- shows the object field the code is about to touch
local logw = os.getenv("F3_LOGW") and tonumber(os.getenv("F3_LOGW"), 16)
local pokefn = os.getenv("F3_POKEFN") and tonumber(os.getenv("F3_POKEFN"), 16)
-- F3_POKEAT=<hex pc>: apply the A0-relative pokes (F3_POKEA0B/W/P) whenever
-- that PC is reached (e.g. a parent state's tick, to force the state change
-- into the captured routine) instead of at the captured routine's entry
local pokeat = os.getenv("F3_POKEAT") and tonumber(os.getenv("F3_POKEAT"), 16)
-- F3_INJECT=<hex pc>: synthetic calls of the captured routine (F3_ENTRY must be
-- the same PC, exit "ret"). When F3_INJECT_AT (default 0104B8, Obj_RunGroups'
-- entry, reached every frame) is reached, its address is pushed as a return
-- PC and execution jumps to the routine, at most once per frame: the pair is
-- the routine's entry to its rts, then the game resumes at F3_INJECT_AT. For
-- routines reached only in states a run cannot get to; their inputs are then
-- whatever RAM and registers the game has at that point.
local inject = os.getenv("F3_INJECT") and tonumber(os.getenv("F3_INJECT"), 16)
local inject_at = tonumber(os.getenv("F3_INJECT_AT") or "0104B8", 16)
local last_inject = -1
-- F3_INJECT_A0=1: each injected call gets A0 = the next live object slot
-- (0x410060 + k*0x6C, k = 0..N-1 with a nonzero long at +0), cycling, so
-- object ticks run on real object records. F3_INJECT_SLOTS (default 0x100).
local inject_a0 = os.getenv("F3_INJECT_A0") == "1"
local inject_slots = tonumber(os.getenv("F3_INJECT_SLOTS") or "100", 16)
local inject_k = 0
-- F3_INJECT_REGS="D1=3,A0=05F9E0" (hex) sets registers on each injected call
-- (after F3_INJECT_A0); F3_INJECT_STACK="v1,v2" (hex longs) pushes C-style
-- arguments, v1 ending at 4(SP) after the call. A value may be "a|b|c": the
-- n-th injected call uses item n mod count, so one run covers several cases.
local function inject_vals(s)
  local t = {}
  for k, v in string.gmatch(s or "", "([%w]+)=([%x|]+)") do
    local alt = {}
    for x in string.gmatch(v, "%x+") do alt[#alt + 1] = tonumber(x, 16) end
    t[#t + 1] = { k:upper(), alt }
  end
  return t
end
local inject_regs = inject_vals(os.getenv("F3_INJECT_REGS"))
local inject_stack = {}
for v in string.gmatch(os.getenv("F3_INJECT_STACK") or "", "[%x|]+") do
  local alt = {}
  for x in string.gmatch(v, "%x+") do alt[#alt + 1] = tonumber(x, 16) end
  inject_stack[#inject_stack + 1] = alt
end
local inject_n = 0
local inject_orig_sp = nil
local inject_base_sp = nil
local inject_abort = os.getenv("F3_INJECT_ABORT") == "1"
local sweep = tonumber(os.getenv("F3_SWEEP") or "0")
local sweep_base = tonumber(os.getenv("F3_SWEEP_BASE") or "0")
local want = tonumber(os.getenv("F3_SAMPLES") or "300")
local prefix = os.getenv("F3_OUT") or (ROOT .. "/work/ram/fn")
local inmode = os.getenv("F3_INPUT") or "fire"
-- F3_NOFFSET: frame count already elapsed in the run that saved -state (keeps
-- the input pattern in phase with survey_obj.lua runs); F3_POKE16 per frame
local noffset = tonumber(os.getenv("F3_NOFFSET") or "0")
local pokes16 = {}
for a, v in string.gmatch(os.getenv("F3_POKE16") or "", "(%x+)=(%x+)") do pokes16[#pokes16 + 1] = {tonumber(a, 16), tonumber(v, 16)} end
local dbg = manager.machine.debugger
assert(dbg, "capture_fn: MAME has no debugger (need -debug -debugger none)")
local cpu = manager.machine.devices[":maincpu"]
local sp = cpu.spaces["program"]
local function poke_a0()
  local a0 = cpu.state["A0"].value
  for _, pv in ipairs(pokesa0) do sp:write_u8(a0 + pv[1], pv[2]) end
  for _, pv in ipairs(pokesa0w) do sp:write_u16(a0 + pv[1], pv[2]) end
  for _, pv in ipairs(pokesa0p) do sp:write_u8((sp:read_u32(a0 + pv[1]) & 0xFFFFFF) + pv[2], pv[3]) end
end
local p = manager.machine.ioport.ports
-- A machine reset during the capture (a broken injected call can make the
-- game crash into the watchdog) re-runs this autoboot script. A second copy
-- would reopen the outputs with "w" while the first still writes at its
-- offset, leaving a hole of NUL bytes at the start of regs.tsv. So: run once.
if _G.__f3_capture_fn then return end
_G.__f3_capture_fn = true
local out = assert(io.open(prefix .. "_pairs.bin", "wb"))
local regs = assert(io.open(prefix .. "_regs.tsv", "w"))
local exitpc = os.getenv("F3_EXITPC") == "1"
local service = os.getenv("F3_SERVICE") == "1"
-- menu-walk periods (frames), F3_SVC="down,button,up" (default 90,270,630)
local svc_dn, svc_bt, svc_up = (os.getenv("F3_SVC") or "90,270,630"):match("(%d+),(%d+),(%d+)")
svc_dn, svc_bt, svc_up = tonumber(svc_dn), tonumber(svc_bt), tonumber(svc_up)
regs:write("pair\twhich\tframe\tD0\tD1\tD2\tD3\tD4\tD5\tD6\tD7\tA0\tA1\tA2\tA3\tA4\tA5\tA6\tSP" .. (exitpc and "\tPC" or "") .. "\n")
cpu.debug:bpset(entry, os.getenv("F3_COND") or "", "")
for a in pairs(remask) do cpu.debug:bpset(a, "", "") end
if pokeat then cpu.debug:bpset(pokeat, "", "") end
if inject then cpu.debug:bpset(inject_at, "", "") end
for pc, _ in pairs(exits) do cpu.debug:bpset(pc, "", "") end
for pc, _ in pairs(logs) do cpu.debug:bpset(pc, "", "") end
local dp = want_dpram and assert(io.open(prefix .. "_dpram.bin", "wb")) or nil
local spf = os.getenv("F3_SPR") == "1" and assert(io.open(prefix .. "_spr.bin", "wb")) or nil
local gff = os.getenv("F3_GFX") == "1" and assert(io.open(prefix .. "_gfx.bin", "wb")) or nil
-- F3_PORTS=1: <prefix>_ports.tsv gets, per pair, the 32 input-port bytes at
-- 0x4A0000..0x4A001F sampled at entry (IO_PollInputs 0x001A78 reads them;
-- they are MMIO, not part of the RAM dumps). Interrupts must be masked
-- (F3_NOIRQ) so the frame cannot change between entry and the poll.
local portf = os.getenv("F3_PORTS") == "1" and assert(io.open(prefix .. "_ports.tsv", "w")) or nil
local pre_ports = ""
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
  if gff then
    local t3 = {}
    for a = 0x610000, 0x62FFFF, 4 do t3[#t3 + 1] = string.pack(">I4", sp:read_u32(a)) end
    snap.g = table.concat(t3)
  end
  return snap
end
local function emit(snap)
  out:write(snap.w)
  if dp then dp:write(snap.d) end
  if spf then spf:write(snap.s) end
  if gff then gff:write(snap.g) end
end
local function regline_str(i, which)
  local r = {}
  for k = 0, 7 do r[#r + 1] = string.format("%08X", cpu.state["D" .. k].value) end
  for k = 0, 6 do r[#r + 1] = string.format("%08X", cpu.state["A" .. k].value) end
  r[#r + 1] = string.format("%08X", cpu.state["SP"].value)
  if exitpc and which ~= "log" then r[#r + 1] = string.format("%08X", cpu.state["PC"].value) end
  return string.format("%d\t%s\t%04X\t%s\n", i, which, sp:read_u16(0x4022AC), table.concat(r, "\t"))
end
local pairs_done, have_pre, n = 0, false, 0
local scr0 = nil
local pre_snap, pre_regs, pending_logs, abandoned = nil, nil, {}, 0
local maxframes = tonumber(os.getenv("F3_MAXFRAMES") or "6000")
local finished = false
local function finish(tag)
  if finished then return end
  finished = true
  out:close(); regs:close(); if dp then dp:close() end; if spf then spf:close() end; if gff then gff:close() end
  print(string.format("CAPTURE %s pairs=%d abandoned=%d frames=%d screen=%d", tag, pairs_done, abandoned, n, manager.machine.screens[":screen"]:frame_number() - (scr0 or 0))); manager.machine:exit()
end
-- the pairs after a reset are not from the state the capture asked for: stop
-- at the first reset with the pairs taken so far (an open pre is dropped)
local reset_sub = emu.add_machine_reset_notifier and emu.add_machine_reset_notifier(function()
  if scr0 then finish("RESET") end
end)
emu.register_periodic(function()
  if dbg.execution_state ~= "stop" then return end
  local pc = cpu.state["PC"].value
  local spv = cpu.state["SP"].value
  -- SP as the call left it (arguments still pushed): the post match below needs
  -- this value, not the restored one. Restoring first made `spv == ret_sp + 4`
  -- false for every injected call made with F3_INJECT_STACK (ret_sp + 4 is
  -- inject_orig_sp - 4*#inject_stack, while the restore sets spv back to
  -- inject_orig_sp), so those calls never closed a pair: fnk_003CE0, fnk_005794,
  -- fnk_0057DA and fnk_0B7332 all came back with 0 pairs, and the 30 pairs
  -- fnk_005576 did record are its natural calls (pre SP 0x404E8A, A0 = the HUD
  -- table 0x000C8B5E), none of them the injected call.
  local spv_ret = spv
  if inject_orig_sp and pc == inject_at and spv == inject_orig_sp - 4 * #inject_stack then
    spv = inject_orig_sp; cpu.state["SP"].value = spv; inject_orig_sp = nil
  end
  if inject and pc == inject_at and not have_pre then
    local fr = sp:read_u16(0x4022AC)
    if fr ~= last_inject then
      last_inject = fr
      -- C-style arguments: pushed last-to-first so v1 sits at 4(SP) in the
      -- callee; the return to inject_at leaves them on the stack, so the
      -- original SP is restored there (the inject_orig_sp check above)
      if #inject_stack > 0 then inject_orig_sp = spv end
      inject_base_sp = spv
      for i = #inject_stack, 1, -1 do
        local alt = inject_stack[i]
        spv = spv - 4; sp:write_u32(spv, alt[inject_n % #alt + 1])
      end
      spv = spv - 4; sp:write_u32(spv, inject_at); cpu.state["SP"].value = spv
      cpu.state["PC"].value = inject; pc = inject
      if inject_a0 then
        for _ = 1, inject_slots do
          inject_k = (inject_k + 1) % inject_slots
          local o = 0x410060 + inject_k * 0x6C
          if sp:read_u32(o) ~= 0 then cpu.state["A0"].value = o; break end
        end
      end
      for _, kv in ipairs(inject_regs) do
        local alt = kv[2]
        cpu.state[kv[1]].value = alt[inject_n % #alt + 1]
      end
      inject_n = inject_n + 1
    end
  end
  if pokeat and pc == pokeat and pc ~= entry then
    poke_a0(); dbg.execution_state = "run"; return
  end
  if pokefn and pc == entry and not have_pre and cpu.state["A1"].value == pokefn then
    -- F3_POKEFN has two uses. Without F3_COND naming this routine it marks a
    -- parent we only want to poke, so return without capturing. With F3_COND
    -- naming it we are capturing it, and the poke must be applied *and* the
    -- pair recorded; returning on this path is why every poke-driven capture
    -- came back with 0 pairs.
    poke_a0()
    if os.getenv("F3_POKECAPTURE") ~= "1" then
      dbg.execution_state = "run"; return
    end
  end
  if noirq and have_pre and remask[pc] then
    cpu.state["PC"].value = pc + 4
    dbg.execution_state = "run"; return
  end
  if pc == entry and have_pre and exit_ret and spv ~= ret_sp then
    -- previous call never returned (e.g. a script opcode popped the return
    -- address): drop it and start over with this call
    if ret_bp then cpu.debug:bpclear(ret_bp); ret_bp = nil; ret_pc = nil end
    have_pre = false; abandoned = abandoned + 1
    if noirq and saved_sr then cpu.state["SR"].value = (cpu.state["SR"].value & ~0x0700) | (saved_sr & 0x0700); saved_sr = nil end
  end
  if pc == entry and not have_pre then
    for _, pv in ipairs(pokes) do sp:write_u32(pv[1], pv[2]) end
    for _, pv in ipairs(pokes16) do sp:write_u16(pv[1], pv[2]) end
    for _, pv in ipairs(pokes8) do sp:write_u8(pv[1], pv[2]) end
    if not pokefn and not pokeat then poke_a0() end
    if sweep > 0 then sp:write_u16(spv + 4, sweep_base + pairs_done % sweep) end
    pre_snap = snapshot(); pre_regs = regline_str(pairs_done, "pre"); pending_logs = {}; have_pre = true
    if portf then
      local t = {}
      for k = 0, 31 do t[#t + 1] = string.format("%02X", sp:read_u8(0x4A0000 + k)) end
      pre_ports = table.concat(t)
    end
    if noirq then saved_sr = cpu.state["SR"].value; cpu.state["SR"].value = saved_sr | 0x0700 end
    linger_left = linger
    if exit_a6 then
      ret_pc = cpu.state["A6"].value & 0xFFFFFF
      ret_bp = cpu.debug:bpset(ret_pc, "", "")
    end
    if exit_ret then
      ret_sp = spv
      ret_pc = sp:read_u32(ret_sp) & 0xFFFFFF
      ret_bp = cpu.debug:bpset(ret_pc, "", "")
    end
  elseif logs[pc] and have_pre and (not loga0 or cpu.state["A0"].value == loga0) then
    pending_logs[#pending_logs + 1] = string.format("%s\t%06X\t%s\n",
      (regline_str(pairs_done, "log"):gsub("\n$", "")), pc,
      logw and string.format("%04X", sp:read_u16(cpu.state["A0"].value + logw)) or "----")
  elseif have_pre and linger_left > 0 and pc == entry then
    -- one more tick went by for this object: count down, then dump
    linger_left = linger_left - 1
  elseif have_pre and (exits[pc] or (exit_ret and pc == ret_pc and spv_ret == ret_sp + 4) or (exit_a6 and pc == ret_pc)) then
    if ret_bp then cpu.debug:bpclear(ret_bp); ret_bp = nil; ret_pc = nil end
    emit(pre_snap); regs:write(pre_regs); regs:write(table.concat(pending_logs))
    if portf then portf:write(string.format("%d\t%s\n", pairs_done, pre_ports)); portf:flush() end
    emit(snapshot()); regs:write(regline_str(pairs_done, "post")); have_pre = false
    if noirq and saved_sr then
      local cur = cpu.state["SR"].value
      cpu.state["SR"].value = (cur & ~0x0700) | (saved_sr & 0x0700); saved_sr = nil
    end
    out:flush(); regs:flush(); if dp then dp:flush() end; if spf then spf:flush() end; if gff then gff:flush() end
    pairs_done = pairs_done + 1
    if pairs_done >= want then finish("DONE"); return end
    -- F3_INJECT_ABORT=1 with an F3_EXIT PC (e.g. a task body's first TRAP #5):
    -- the injected call cannot return from there, so after the post it is
    -- abandoned: PC back to the injection point, SP as before the call.
    if inject_abort and inject_base_sp and exits[pc] then
      cpu.state["SP"].value = inject_base_sp; cpu.state["PC"].value = inject_at
      inject_base_sp = nil; inject_orig_sp = nil
    end
  end
  dbg.execution_state = "run"
end)
-- frame_done fires many times per emulated frame while breakpoints are set
-- (measured: ~25-40x), so frames are counted on the screen's frame number
emu.register_frame_done(function()
  local fnum = manager.machine.screens[":screen"]:frame_number()
  if not scr0 then scr0 = fnum - 1 end
  if fnum - scr0 == n then return end
  n = fnum - scr0
  if n >= maxframes then finish("PARTIAL"); return end
  -- never between a pre and a post snapshot (the tick would see a write
  -- the routine did not make); the entry breakpoint re-applies them
  if not have_pre then for _, pv in ipairs(pokes16) do sp:write_u16(pv[1], pv[2]) end end
  -- F3_SERVICE=1: the test-mode input schedule of pc_survey.lua (on t =
  -- frames since the survey's 01_attract start), so captures from its TM_*
  -- states replay the same menu walk.
  if service then
    local t = n + noffset
    p[":EEPROMIN"].fields["Service Mode"]:set_value((t >= 10 and t < 20) and 1 or 0)
    p[":IN.1"].fields["P1 Down"]:set_value((t > 40 and t % svc_dn < 5) and 1 or 0)
    p[":IN.0"].fields["P1 Button 1"]:set_value((t > 40 and t % svc_bt >= 45 and t % svc_bt < 50) and 1 or 0)
    p[":IN.1"].fields["P1 Up"]:set_value((t > 40 and t % svc_up >= 300 and t % svc_up < 305) and 1 or 0)
  elseif inmode == "fire" then
    local t = n + noffset
    p[":IN.0"].fields["P1 Button 1"]:set_value((t // 3) % 2)
    local k = (t // 45) % 4
    p[":IN.1"].fields["P1 Up"]:set_value(k == 0 and 1 or 0)
    p[":IN.1"].fields["P1 Right"]:set_value(k == 1 and 1 or 0)
    p[":IN.1"].fields["P1 Down"]:set_value(k == 2 and 1 or 0)
    p[":IN.1"].fields["P1 Left"]:set_value(k == 3 and 1 or 0)
  end
  -- "up" / "down": autofire while holding one direction, so the stage-end
  -- route choice (player height) takes the upper or lower branch.
  if inmode == "up" or inmode == "down" then
    local t = n + noffset
    p[":IN.0"].fields["P1 Button 1"]:set_value((t // 3) % 2)
    p[":IN.1"].fields["P1 Up"]:set_value(inmode == "up" and 1 or 0)
    p[":IN.1"].fields["P1 Down"]:set_value(inmode == "down" and 1 or 0)
  end
end)
