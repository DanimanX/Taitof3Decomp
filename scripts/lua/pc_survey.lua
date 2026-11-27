-- PC survey: which of a list of routine entries run, and from which frame.
-- A breakpoint per entry (F3_BPLIST: file with one hex address per line);
-- the first hit records the frame and the breakpoint is cleared.
-- Env: F3_FRAMES (default 20000), F3_OUT (tsv: addr, first frame, hits=1),
--      F3_SERVICE=1: pulse the Service Mode switch (EEPROMIN, active low)
--        at frames 10..20 and then walk the test menu: P1 Down every 90
--        frames, P1 Button 1 every 270, P1 Up every 630 (5 frames each)
--      F3_SAVEEVERY N / F3_SAVEPFX: save state <pfx>_<frame> every N frames
local cpu = manager.machine.devices[":maincpu"]
local dbg = cpu.debug
local p = manager.machine.ioport.ports
local frames = tonumber(os.getenv("F3_FRAMES") or "20000")
local outp = os.getenv("F3_OUT") or "work/pc_survey.tsv"
local service = os.getenv("F3_SERVICE") == "1"
-- menu-walk periods (frames), F3_SVC="down,button,up" (default 90,270,630)
local svc_dn, svc_bt, svc_up = (os.getenv("F3_SVC") or "90,270,630"):match("(%d+),(%d+),(%d+)")
svc_dn, svc_bt, svc_up = tonumber(svc_dn), tonumber(svc_bt), tonumber(svc_up)
local saveevery = tonumber(os.getenv("F3_SAVEEVERY") or "0")
local savepfx = os.getenv("F3_SAVEPFX") or "PS"
local want, first, n, scr0 = {}, {}, 0, nil
for l in io.lines(os.getenv("F3_BPLIST")) do
  local a = tonumber(l:match("%x+"), 16)
  if a then want[a] = dbg:bpset(a, "", "") end
end
local function dump()
  local f = assert(io.open(outp, "w"))
  for a, fr in pairs(first) do f:write(string.format("%06X\t%d\n", a, fr)) end
  f:close()
end
emu.register_periodic(function()
  if manager.machine.debugger.execution_state ~= "stop" then return end
  local pc = cpu.state["PC"].value
  if want[pc] then
    first[pc] = n; dbg:bpclear(want[pc]); want[pc] = nil
  end
  manager.machine.debugger.execution_state = "run"
end)
emu.register_frame_done(function()
  local fnum = manager.machine.screens[":screen"]:frame_number()
  if not scr0 then scr0 = fnum - 1 end
  if fnum - scr0 == n then return end
  n = fnum - scr0
  if service then
    p[":EEPROMIN"].fields["Service Mode"]:set_value((n >= 10 and n < 20) and 1 or 0)
    p[":IN.1"].fields["P1 Down"]:set_value((n > 40 and n % svc_dn < 5) and 1 or 0)
    p[":IN.0"].fields["P1 Button 1"]:set_value((n > 40 and n % svc_bt >= 45 and n % svc_bt < 50) and 1 or 0)
    p[":IN.1"].fields["P1 Up"]:set_value((n > 40 and n % svc_up >= 300 and n % svc_up < 305) and 1 or 0)
  end
  if saveevery > 0 and n % saveevery == 0 then
    manager.machine:save(string.format("%s_%06d", savepfx, n)); dump()
  end
  if n >= frames then
    dump(); print("PCSURVEY done hit=" .. (function() local c = 0 for _ in pairs(first) do c = c + 1 end return c end)())
    manager.machine:exit()
  end
end)
