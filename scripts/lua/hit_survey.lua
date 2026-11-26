-- Which of a list of ROM entry points actually execute, and when.
-- One breakpoint per address; the first hit records the frame (screen frame
-- number since the state loaded) and clears that breakpoint, so the run
-- slows down only until the hot ones are found.
-- Env: F3_LIST   file with one hex address per line (first column)
--      F3_OUT    tsv: addr, first frame, A0, caller (return address at SP)
--      F3_FRAMES frames to run (default 6000)
--      F3_INPUT  fire|none|up|down (same patterns as survey_obj.lua)
--      F3_NOFFSET input phase offset (frames elapsed when the state was saved)
--      F3_POKE16 "addr=val,..." 16-bit writes every frame (P1 invuln: 41B1E0=00D2)
local cpu = manager.machine.devices[":maincpu"]
local sp = cpu.spaces["program"]
local p = manager.machine.ioport.ports
local dbg = manager.machine.debugger
assert(dbg, "hit_survey: need -debug -debugger none")
local frames = tonumber(os.getenv("F3_FRAMES") or "6000")
local outp = os.getenv("F3_OUT") or "work/hits.tsv"
local inmode = os.getenv("F3_INPUT") or "fire"
local noffset = tonumber(os.getenv("F3_NOFFSET") or "0")
local pokes16 = {}
for a, v in string.gmatch(os.getenv("F3_POKE16") or "", "(%x+)=(%x+)") do pokes16[#pokes16 + 1] = {tonumber(a, 16), tonumber(v, 16)} end

local bps, nbp = {}, 0
for line in io.lines(os.getenv("F3_LIST")) do
  local a = line:match("^(%x+)")
  if a then
    local addr = tonumber(a, 16)
    if not bps[addr] then bps[addr] = cpu.debug:bpset(addr, "", ""); nbp = nbp + 1 end
  end
end
local hits, n, scr0, done = {}, 0, nil, false

local function finish()
  if done then return end
  done = true
  local f = assert(io.open(outp, "w"))
  for a, h in pairs(hits) do f:write(string.format("%06X\t%d\t%08X\t%06X\n", a, h[1], h[2], h[3])) end
  f:close()
  print(string.format("HITS %d of %d in %d frames", (function() local c = 0 for _ in pairs(hits) do c = c + 1 end return c end)(), nbp, n))
  manager.machine:exit()
end

emu.register_periodic(function()
  if done or dbg.execution_state ~= "stop" then return end
  local pc = cpu.state["PC"].value & 0xFFFFFF
  if bps[pc] then
    local spv = cpu.state["SP"].value
    hits[pc] = {n, cpu.state["A0"].value, sp:read_u32(spv) & 0xFFFFFF}
    cpu.debug:bpclear(bps[pc]); bps[pc] = nil
  end
  dbg.execution_state = "run"
end)

emu.register_frame_done(function()
  if done then return end
  local fnum = manager.machine.screens[":screen"]:frame_number()
  if not scr0 then scr0 = fnum - 1 end
  if fnum - scr0 == n then return end
  n = fnum - scr0
  for _, pv in ipairs(pokes16) do sp:write_u16(pv[1], pv[2]) end
  local t = n + noffset
  if inmode == "fire" then
    p[":IN.0"].fields["P1 Button 1"]:set_value((t // 3) % 2)
    local k = (t // 45) % 4
    p[":IN.1"].fields["P1 Up"]:set_value(k == 0 and 1 or 0)
    p[":IN.1"].fields["P1 Right"]:set_value(k == 1 and 1 or 0)
    p[":IN.1"].fields["P1 Down"]:set_value(k == 2 and 1 or 0)
    p[":IN.1"].fields["P1 Left"]:set_value(k == 3 and 1 or 0)
  elseif inmode == "up" or inmode == "down" then
    p[":IN.0"].fields["P1 Button 1"]:set_value((t // 3) % 2)
    p[":IN.1"].fields["P1 Up"]:set_value(inmode == "up" and 1 or 0)
    p[":IN.1"].fields["P1 Down"]:set_value(inmode == "down" and 1 or 0)
  end
  if n >= frames then finish() end
end)
