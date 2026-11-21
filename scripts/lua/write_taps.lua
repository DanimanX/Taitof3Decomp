-- Log unique (PC, A0..A6) of writers to watched Work RAM addresses.
-- Env-free config: edit WATCH below. Starts from a savestate (-state ...).
-- Ranges must be 32-bit aligned on the 68EC020 bus; `mask` selects the bytes
-- of interest within the long (big-endian: 0xFFFF0000 = word at +0).
local WATCH = {
  {name = "player_x",  lo = 0x40566C, hi = 0x40566F, mask = 0x0000FFFF},
  {name = "player_y",  lo = 0x405674, hi = 0x405677, mask = 0x0000FFFF},
  {name = "score_p1",  lo = 0x41B1D8, hi = 0x41B1DB, mask = 0xFFFFFFFF},
}
local FRAMES = 300
local cpu = manager.machine.devices[":maincpu"]
local sp = cpu.spaces["program"]
local p = manager.machine.ioport.ports
local seen, taps = {}, {}
for _, w in ipairs(WATCH) do
  taps[#taps + 1] = sp:install_write_tap(w.lo, w.hi, w.name, function(off, data, mask)
    if (mask & w.mask) == 0 then return end
    local pc = cpu.state["PC"].value
    local regs = {}
    for i = 0, 6 do regs[#regs + 1] = string.format("%06X", cpu.state["A" .. i].value & 0xFFFFFF) end
    local key = string.format("%s pc=%06X A=%s", w.name, pc, table.concat(regs, ","))
    seen[key] = (seen[key] or 0) + 1
  end)
end
local n = 0
emu.register_frame_done(function()
  n = n + 1
  p[":IN.0"].fields["P1 Button 1"]:set_value((n // 4) % 2)
  p[":IN.1"].fields["P1 Right"]:set_value(((n // 25) % 2 == 0) and 1 or 0)
  p[":IN.1"].fields["P1 Up"]:set_value(((n // 40) % 2 == 0) and 1 or 0)
  if n == FRAMES then
    local keys = {}
    for k, _ in pairs(seen) do keys[#keys + 1] = k end
    table.sort(keys)
    for _, k in ipairs(keys) do print(string.format("TAP %s hits=%d", k, seen[k])) end
    manager.machine:exit()
  end
end)
