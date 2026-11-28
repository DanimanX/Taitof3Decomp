-- Play a game (coin, start, random stick/buttons) and save MAME states.
-- Env: F3_SAVEAT "frame:name,frame:name,..." (states/<game>/<name>.sta),
--      F3_COIN (default 600, 0 = attract only), F3_SEED, F3_FRAMES (exit; default last save).
local coin = tonumber(os.getenv("F3_COIN") or "600")
math.randomseed(tonumber(os.getenv("F3_SEED") or "1"))
local saves, last = {}, 0
for fr, name in (os.getenv("F3_SAVEAT") or ""):gmatch("(%d+):([%w_]+)") do
  saves[tonumber(fr)] = name; last = math.max(last, tonumber(fr))
end
local frames = tonumber(os.getenv("F3_FRAMES") or tostring(last + 1))
local fields = {}
for _, port in pairs(manager.machine.ioport.ports) do
  for name, f in pairs(port.fields) do fields[name] = f end
end
local play = { "P1 Up", "P1 Down", "P1 Left", "P1 Right", "P1 Button 1", "P1 Button 2", "P1 Button 3" }
local n, held = 0, {}
emu.register_frame_done(function()
  n = n + 1
  for k, f in pairs(held) do f:set_value(0); held[k] = nil end
  local function press(name) local f = fields[name]; if f then f:set_value(1); held[name] = f end end
  if coin > 0 then
    if n >= coin and n < coin + 6 then press("Coin 1") end
    if n >= coin + 60 and n < coin + 66 then press("1 Player Start") end
    if n > coin + 120 then
      for _, b in ipairs(play) do if math.random() < 0.25 then press(b) end end
    end
  end
  if saves[n] then manager.machine:save(saves[n]); print("saved " .. saves[n] .. " at " .. n) end
  if n >= frames then manager.machine:exit() end
end)
