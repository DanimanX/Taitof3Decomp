-- Start a game in a chosen zone (0..27, Darius Gaiden A..Z branches) by
-- overriding A5+0x32DC each time ROM 0x13CAAE (zone -> stage number via the
-- byte table 0x13D600) runs. Needs -debug -debugger none.
-- Env: F3_ZONE      zone to force (hex or dec, e.g. 0x0A); unset = only log
--      F3_ZONEMAX   force only the first N hits (default 1: later stage
--                   clears advance normally from the forced zone)
--      F3_FRAMES    frames to run (default 6000)
--      F3_INPUT     fire|up|down|none (as survey_obj.lua)
--      F3_POKE16    "addr=val,..." 16-bit writes every frame
--      F3_SAVEEVERY N, F3_SAVEPFX: save <pfx>_<frame> every N frames
--      F3_LOG       log path (default work/zone_start.log)
--      F3_COIN      frame at which to insert a coin (start follows 60 frames
--                   later); use from 01_attract to begin a real game
local dbg = manager.machine.debugger
assert(dbg, "zone_start: need -debug -debugger none")
local cpu = manager.machine.devices[":maincpu"]
local sp = cpu.spaces["program"]
local p = manager.machine.ioport.ports
local ZONE_PC, ZONE_ADDR = 0x13CAAE, 0x408000 + 0x32DC
local zone = os.getenv("F3_ZONE") and tonumber(os.getenv("F3_ZONE"))
local zmax = tonumber(os.getenv("F3_ZONEMAX") or "1")
local frames = tonumber(os.getenv("F3_FRAMES") or "6000")
local inmode = os.getenv("F3_INPUT") or "fire"
local saveevery = tonumber(os.getenv("F3_SAVEEVERY") or "0")
local savepfx = os.getenv("F3_SAVEPFX") or "Z"
local pokes16 = {}
for a, v in string.gmatch(os.getenv("F3_POKE16") or "", "(%x+)=(%x+)") do pokes16[#pokes16 + 1] = {tonumber(a, 16), tonumber(v, 16)} end
local log = assert(io.open(os.getenv("F3_LOG") or "work/zone_start.log", "a"))
local coinf = tonumber(os.getenv("F3_COIN") or "-1")
local function setfield(name, v)
  for _, port in pairs(p) do
    local f = port.fields[name]
    if f then f:set_value(v); return true end
  end
  return false
end
local n, hits = 0, 0
cpu.debug:bpset(ZONE_PC, "", "")
emu.register_periodic(function()
  if dbg.execution_state ~= "stop" then return end
  if cpu.state["PC"].value == ZONE_PC then
    hits = hits + 1
    local old = sp:read_u16(ZONE_ADDR)
    if zone and hits <= zmax then sp:write_u16(ZONE_ADDR, zone) end
    log:write(string.format("frame %d hit %d zone %d -> %d\n", n, hits, old, sp:read_u16(ZONE_ADDR)))
    log:flush()
  end
  dbg.execution_state = "run"
end)
emu.register_frame_done(function()
  n = n + 1
  for _, pv in ipairs(pokes16) do sp:write_u16(pv[1], pv[2]) end
  if coinf >= 0 then
    assert(setfield("Coin 1", (n >= coinf and n < coinf + 8) and 1 or 0), "no Coin 1 field")
    assert(setfield("1 Player Start", (n >= coinf + 60 and n < coinf + 68) and 1 or 0), "no 1 Player Start field")
  end
  if inmode == "fire" then
    p[":IN.0"].fields["P1 Button 1"]:set_value((n // 3) % 2)
    local k = (n // 45) % 4
    p[":IN.1"].fields["P1 Up"]:set_value(k == 0 and 1 or 0)
    p[":IN.1"].fields["P1 Right"]:set_value(k == 1 and 1 or 0)
    p[":IN.1"].fields["P1 Down"]:set_value(k == 2 and 1 or 0)
    p[":IN.1"].fields["P1 Left"]:set_value(k == 3 and 1 or 0)
  elseif inmode == "up" or inmode == "down" then
    p[":IN.0"].fields["P1 Button 1"]:set_value((n // 3) % 2)
    p[":IN.1"].fields["P1 Up"]:set_value(inmode == "up" and 1 or 0)
    p[":IN.1"].fields["P1 Down"]:set_value(inmode == "down" and 1 or 0)
  end
  if n % 500 == 0 then
    log:write(string.format("frame %d zone %d stage %d timeline %06X\n", n,
      sp:read_u16(ZONE_ADDR), sp:read_u16(0x408000 + 0x32DE), sp:read_u32(0x40B308) & 0xFFFFFF))
    log:flush()
  end
  if saveevery > 0 and n % saveevery == 0 then
    manager.machine:save(string.format("%s_%06d", savepfx, n))
  end
  if n >= frames then log:close(); manager.machine:exit() end
end)
