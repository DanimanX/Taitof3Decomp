-- Deterministic dariusg session: attract -> coin -> start -> scripted input
-- phases. Dumps Work RAM (0x400000-0x41FFFF) at checkpoints to work/ram/,
-- saves canonical savestates to states/, logs kernel vars every frame.
local ROOT = "/PROCESOS/Taitof3Decomp"
local cpu = manager.machine.devices[":maincpu"]
local sp = cpu.spaces["program"]
local ports = manager.machine.ioport.ports
local function field(port, name) return ports[port].fields[name] end
local F = {
  coin = field(":EEPROMIN", "Coin 1"), start = field(":IN.0", "1 Player Start"),
  fire = field(":IN.0", "P1 Button 1"), bomb = field(":IN.0", "P1 Button 2"),
  right = field(":IN.1", "P1 Right"), left = field(":IN.1", "P1 Left"),
  up = field(":IN.1", "P1 Up"), down = field(":IN.1", "P1 Down"),
}
local function set(name, v) F[name]:set_value(v) end
local function release_all() for k, _ in pairs(F) do set(k, 0) end end

local function dump_ram(name)
  local f = assert(io.open(ROOT .. "/work/ram/" .. name .. ".bin", "wb"))
  local t = {}
  for a = 0x400000, 0x41FFFF, 4 do
    local v = sp:read_u32(a)
    t[#t + 1] = string.char((v >> 24) & 0xFF, (v >> 16) & 0xFF, (v >> 8) & 0xFF, v & 0xFF)
    if #t >= 4096 then f:write(table.concat(t)); t = {} end
  end
  f:write(table.concat(t)); f:close()
  print("DUMP " .. name)
end

local log = assert(io.open(ROOT .. "/work/ram/session_log.tsv", "w"))
log:write("frame\tphase\tcredit\tgame_flags\ttask_mask\tframe_ctr\n")

-- frame -> action list. Phases chosen so each movement window is 60 frames.
local plan = {
  [900]  = function() dump_ram("01_attract"); manager.machine:save("01_attract") end,
  [1000] = function() set("coin", 1) end,  [1006] = function() set("coin", 0) end,
  [1100] = function() set("start", 1) end, [1106] = function() set("start", 0) end,
  [1700] = function() dump_ram("02_game_t0"); manager.machine:save("02_stage1_start") end,
  [1760] = function() dump_ram("10_idle"); set("right", 1) end,
  [1820] = function() set("right", 0); dump_ram("11_after_right"); set("left", 1) end,
  [1880] = function() set("left", 0); dump_ram("12_after_left"); set("up", 1) end,
  [1940] = function() set("up", 0); dump_ram("13_after_up"); set("down", 1) end,
  [2000] = function() set("down", 0); dump_ram("14_after_down") end,
  [2600] = function() release_all(); dump_ram("15_after_fire"); manager.machine:save("03_active_combat") end,
  [2601] = function() log:close(); print("SESSION DONE"); manager.machine:exit() end,
}
local phase = "boot"
local n = 0
emu.register_frame_done(function()
  n = n + 1
  if n >= 2000 and n < 2600 then set("fire", (n // 4) % 2) end  -- autofire
  if plan[n] then plan[n]() end
  if n == 1000 then phase = "coin" elseif n == 1100 then phase = "start"
  elseif n == 1760 then phase = "right" elseif n == 1820 then phase = "left"
  elseif n == 1880 then phase = "up" elseif n == 1940 then phase = "down"
  elseif n == 2000 then phase = "fire" elseif n == 1700 then phase = "game" end
  if n <= 2600 then
    log:write(string.format("%d\t%s\t%04X\t%02X\t%08X\t%04X\n", n, phase,
      sp:read_u16(0x400090), sp:read_u8(0x406BB5), sp:read_u32(0x4066FC), sp:read_u16(0x4022AC)))
  end
end)
