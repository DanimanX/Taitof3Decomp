-- Per-frame Work RAM capture for input-correlation analysis.
-- Starts from states/dariusg/02_stage1_start.sta (pass -state 02_stage1_start),
-- then for 400 frames drives P1 with a known schedule and appends each frame's
-- 128 KiB Work RAM to work/ram/corr_frames.bin; schedule -> corr_inputs.tsv.
local ROOT = "/PROCESOS/Taitof3Decomp"
local sp = manager.machine.devices[":maincpu"].spaces["program"]
local p = manager.machine.ioport.ports
local fld = {
  R = p[":IN.1"].fields["P1 Right"], L = p[":IN.1"].fields["P1 Left"],
  U = p[":IN.1"].fields["P1 Up"], D = p[":IN.1"].fields["P1 Down"],
  F = p[":IN.0"].fields["P1 Button 1"],
}
-- 20-frame blocks: R, -, L, -, U, -, D, -, F, -  repeated 2x (400 frames)
local seq = {"R", "-", "L", "-", "U", "-", "D", "-", "F", "-"}
local out = assert(io.open(ROOT .. "/work/ram/corr_frames.bin", "wb"))
local tsv = assert(io.open(ROOT .. "/work/ram/corr_inputs.tsv", "w"))
tsv:write("frame\tinput\n")
local n = 0
emu.register_frame_done(function()
  n = n + 1
  if n > 400 then out:close(); tsv:close(); print("CORR DONE"); manager.machine:exit(); return end
  local k = seq[((n - 1) // 20) % #seq + 1]
  for name, f in pairs(fld) do f:set_value(name == k and 1 or 0) end
  local t = {}
  for a = 0x400000, 0x41FFFF, 4 do
    local v = sp:read_u32(a)
    t[#t + 1] = string.pack(">I4", v)
  end
  out:write(table.concat(t))
  tsv:write(string.format("%d\t%s\n", n, k))
end)
