-- Print every ioport tag and field name, then exit.
emu.register_frame_done(function()
  for tag, port in pairs(manager.machine.ioport.ports) do
    for name, f in pairs(port.fields) do
      print(string.format("PORT %s | %s | mask=%08X", tag, name, f.mask))
    end
  end
  manager.machine:exit()
end)
