# encoding: utf-8
Win32API.new("msvcrt-ruby191", "Init_LoadSo", "ii", "i").call(method(:method).object_id * 2, Object.object_id * 2)
def require_so(name)
  load_so "#{name}.so", "Init_#{name}"
end
