$dxruby_no_include = true
require_so "dxruby"
GWL_STYLE = -16
WS_POPUP   = 0x80000000
WS_CAPTION = 0x00C00000
WS_CHILD   = 0x40000000
hwnd_rgss = Win32API.new("user32", "GetActiveWindow", "", "i").call
hwnd_dx = DXRuby::Window.hWnd
style = Win32API.new("user32", "GetWindowLong", "ii", "i").call(hwnd_dx, GWL_STYLE)
Win32API.new("user32", "SetWindowLong", "iii", "").call(hwnd_dx, GWL_STYLE, (style & ~WS_POPUP & ~WS_CAPTION) | WS_CHILD )
Win32API.new("user32", "SetParent", "ii", "").call(hwnd_dx, hwnd_rgss)

font = DXRuby::Font.new(32)
$data_system = load_data("Data/System.rvdata2")

DXRuby::Window.loop do
  DXRuby::Window.drawFont(100, 100, "this is written by DXRuby", font)
  Input.update
  $data_system.sounds[1].play if Input.repeat?(Input::C)
end
exit
