-- Branch-coverage capture for Player_MoveAndBounds (ROM 0x0C691A).
-- From states/dariusg/02_stage1_start.sta (-state 02_stage1_start), forces
-- rarely-hit branches by poking RAM between frames, and records pairs
--   pre_t  = Work RAM after the pokes (input to frame t+1)
--   post_t = Work RAM at the end of frame t+1 (before the next pokes)
-- to work/ram/branch_pairs.bin (pre,post,pre,post,...) + branch_phases.tsv.
-- Phases (100 frames each): normal, shake (ctrl bit4), terrain+arm<10
-- (shield break), terrain+arm>=10 (recoil). HP is refilled and invuln cleared
-- every frame so the ship stays alive and the terrain test runs.
local ROOT = "/PROCESOS/Taitof3Decomp"
local sp = manager.machine.devices[":maincpu"].spaces["program"]
local p = manager.machine.ioport.ports
local P1 = 0x41B190
local dirs = {"P1 Right", "P1 Up", "P1 Left", "P1 Down"}
local out = assert(io.open(ROOT .. "/work/ram/branch_pairs.bin", "wb"))
local tsv = assert(io.open(ROOT .. "/work/ram/branch_phases.tsv", "w"))
tsv:write("pair\tphase\n")
local function dump()
  local t = {}
  for a = 0x400000, 0x41FFFF, 4 do t[#t + 1] = string.pack(">I4", sp:read_u32(a)) end
  out:write(table.concat(t))
end
local function phase_of(n)
  if n <= 100 then return "normal" elseif n <= 200 then return "shake"
  elseif n <= 300 then return "terrain_arm0" else return "terrain_arm10" end
end
local n, pairs_written = 0, 0
emu.register_frame_done(function()
  n = n + 1
  if n > 1 then dump(); pairs_written = pairs_written + 1 end      -- post of previous pair
  if n > 400 then out:close(); tsv:close(); print("BRANCH DONE pairs=" .. pairs_written); manager.machine:exit(); return end
  -- joystick: rotate direction every 10 frames (incl. diagonals via overlap)
  local k = (n // 10) % 4 + 1
  for i, d in ipairs(dirs) do p[":IN.1"].fields[d]:set_value((i == k or i == (k % 4) + 1 and (n % 20) < 10) and 1 or 0) end
  local ph = phase_of(n)
  sp:write_u32(P1 + 0x24, 0x00050000)        -- hp = 5.0 (stay alive)
  sp:write_u16(P1 + 0x50, 0)                 -- invuln = 0
  local ctrl = sp:read_u8(0x409D36) & ~0x10
  if ph == "shake" then ctrl = ctrl | 0x10 end
  sp:write_u8(0x409D36, ctrl)
  if ph:sub(1, 7) == "terrain" then
    for a = 0x40891C, 0x40891C + 0x1400 - 4, 4 do sp:write_u32(a, 0xFFFFFFFF) end
    sp:write_u16(P1 + 0x58, ph == "terrain_arm10" and 10 or 0)
    sp:write_u16(P1 + 0x4E, 3)               -- shield, to observe the clear
  end
  dump()                                     -- pre of this pair
  tsv:write(string.format("%d\t%s\n", n, ph))
end)
