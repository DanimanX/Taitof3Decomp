-- Whole-run instruction trace for entry-point discovery in a new game.
-- Sends the maincpu trace (noloop) to F3_TRACE (a FIFO read by
-- scripts/trace_pcs.py), plays: coin at F3_COIN, start 60 frames later,
-- then random stick/buttons; exits at F3_FRAMES.
-- Env: F3_TRACE (path), F3_FRAMES (default 6000), F3_COIN (default 600, 0 = attract only),
--      F3_SEED (random seed).
local frames = tonumber(os.getenv("F3_FRAMES") or "6000")
local coin = tonumber(os.getenv("F3_COIN") or "600")
math.randomseed(tonumber(os.getenv("F3_SEED") or "1"))
local fields = {}
for _, port in pairs(manager.machine.ioport.ports) do
  for name, f in pairs(port.fields) do fields[name] = f end
end
local play = { "P1 Up", "P1 Down", "P1 Left", "P1 Right", "P1 Button 1", "P1 Button 2", "P1 Button 3" }
local started, n = false, 0
local held = {}
emu.register_frame_done(function()
  n = n + 1
  if not started then
    manager.machine.debugger:command("trace " .. os.getenv("F3_TRACE") .. ",maincpu,noloop")
    started = true
  end
  for k, f in pairs(held) do f:set_value(0); held[k] = nil end
  local function press(name) local f = fields[name]; if f then f:set_value(1); held[name] = f end end
  if coin > 0 then
    if n >= coin and n < coin + 6 then press("Coin 1") end
    if n >= coin + 60 and n < coin + 66 then press("1 Player Start") end
    if n >= coin + 900 and n % 1800 < 6 then press("Coin 1") end     -- continue
    if n >= coin + 960 and n % 1800 >= 60 and n % 1800 < 66 then press("1 Player Start") end
    if n > coin + 120 then
      for _, b in ipairs(play) do if math.random() < 0.25 then press(b) end end
    end
  end
  if n >= frames then
    manager.machine.debugger:command("trace off,maincpu")
    manager.machine:exit()
  end
end)
