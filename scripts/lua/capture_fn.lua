-- Exact pre/post Work RAM capture around a code region using the MAME
-- debugger (run with: -debug -debugger none). Generic "golden vector" tool.
-- Env:
--   F3_ENTRY   hex PC where "pre" is taken   (e.g. C63F2)
--   F3_EXIT    hex PC where "post" is taken  (e.g. C63F0)
--   F3_SAMPLES number of pre/post pairs       (default 300)
--   F3_OUT     output prefix (default work/ram/fn) -> <prefix>_pairs.bin, <prefix>_regs.tsv
--   F3_INPUT   "fire" = P1 autofire + sweeping joystick (default), "none"
local ROOT = "/PROCESOS/Taitof3Decomp"
local entry = tonumber(os.getenv("F3_ENTRY"), 16)
local exitpc = tonumber(os.getenv("F3_EXIT"), 16)
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
cpu.debug:bpset(entry, "", "")
cpu.debug:bpset(exitpc, "", "")
local function dump()
  local t = {}
  for a = 0x400000, 0x41FFFF, 4 do t[#t + 1] = string.pack(">I4", sp:read_u32(a)) end
  out:write(table.concat(t))
end
local function regline(i, which)
  local r = {}
  for k = 0, 7 do r[#r + 1] = string.format("%08X", cpu.state["D" .. k].value) end
  for k = 0, 6 do r[#r + 1] = string.format("%08X", cpu.state["A" .. k].value) end
  r[#r + 1] = string.format("%08X", cpu.state["SP"].value)
  regs:write(string.format("%d\t%s\t%04X\t%s\n", i, which, sp:read_u16(0x4022AC), table.concat(r, "\t")))
end
local pairs_done, have_pre, n = 0, false, 0
emu.register_periodic(function()
  if dbg.execution_state ~= "stop" then return end
  local pc = cpu.state["PC"].value
  if pc == entry and not have_pre then
    dump(); regline(pairs_done, "pre"); have_pre = true
  elseif pc == exitpc and have_pre then
    dump(); regline(pairs_done, "post"); have_pre = false
    pairs_done = pairs_done + 1
    if pairs_done >= want then
      out:close(); regs:close(); print("CAPTURE DONE pairs=" .. pairs_done); manager.machine:exit(); return
    end
  end
  dbg.execution_state = "run"
end)
emu.register_frame_done(function()
  n = n + 1
  if inmode == "fire" then
    p[":IN.0"].fields["P1 Button 1"]:set_value((n // 3) % 2)
    local k = (n // 45) % 4
    p[":IN.1"].fields["P1 Up"]:set_value(k == 0 and 1 or 0)
    p[":IN.1"].fields["P1 Right"]:set_value(k == 1 and 1 or 0)
    p[":IN.1"].fields["P1 Down"]:set_value(k == 2 and 1 or 0)
    p[":IN.1"].fields["P1 Left"]:set_value(k == 3 and 1 or 0)
  end
end)
