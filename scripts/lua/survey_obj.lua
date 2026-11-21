-- Survey which object tick routines run: at each frame end, read the fn
-- field (+0) of every live pool slot (12 groups x 32 slots, free bitmap (1 = free) at
-- 0x410000, 68020 bit order) and record first frame / count per fn.
-- Env: F3_FRAMES (default 30000), F3_OUT (tsv path), F3_INPUT fire|none,
--      F3_POKE32 "addr=val,..." written every frame (e.g. invincibility)
local cpu = manager.machine.devices[":maincpu"]
local sp = cpu.spaces["program"]
local p = manager.machine.ioport.ports
local frames = tonumber(os.getenv("F3_FRAMES") or "30000")
local outp = os.getenv("F3_OUT") or "work/survey.tsv"
local inmode = os.getenv("F3_INPUT") or "fire"
local pokes = {}
for a, v in string.gmatch(os.getenv("F3_POKE32") or "", "(%x+)=(%x+)") do pokes[#pokes + 1] = {tonumber(a, 16), tonumber(v, 16)} end
local seen, n = {}, 0
emu.register_frame_done(function()
  n = n + 1
  for _, pv in ipairs(pokes) do sp:write_u32(pv[1], pv[2]) end
  for g = 0, 11 do
    local mask = sp:read_u32(0x410000 + 4 * g)
    for s = 0, 31 do
      if (mask >> (31 - s)) & 1 == 0 then
        local fn = sp:read_u32(0x410060 + g * 0xD80 + s * 0x6C) & 0xFFFFFF
        local e = seen[fn]
        if e then e[2] = e[2] + 1 else seen[fn] = {n, 1, g} end
      end
    end
  end
  if inmode == "fire" then
    p[":IN.0"].fields["P1 Button 1"]:set_value((n // 3) % 2)
    local k = (n // 45) % 4
    p[":IN.1"].fields["P1 Up"]:set_value(k == 0 and 1 or 0)
    p[":IN.1"].fields["P1 Right"]:set_value(k == 1 and 1 or 0)
    p[":IN.1"].fields["P1 Down"]:set_value(k == 2 and 1 or 0)
    p[":IN.1"].fields["P1 Left"]:set_value(k == 3 and 1 or 0)
  end
  if n >= frames then
    local f = assert(io.open(outp, "w"))
    for fn, e in pairs(seen) do f:write(string.format("%06X\t%d\t%d\t%d\n", fn, e[1], e[2], e[3])) end
    f:close(); print("SURVEY done fns=" .. (function() local c = 0 for _ in pairs(seen) do c = c + 1 end return c end)())
    manager.machine:exit()
  end
end)
