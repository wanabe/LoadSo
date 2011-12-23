# encoding: utf-8
#__END__
if Win32API.new("ext_rgss", "Init_ext_rgss", "", "i").call == 0
  raise "ExtRgss initialize error"
end
