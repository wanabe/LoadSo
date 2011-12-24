# encoding: utf-8
#__END__
if Win32API.new("ext_rgss", "Init_ext_rgss", "ii", "i").call(method(:method).object_id * 2, Object.object_id * 2) == 0
  raise "ExtRgss initialize error"
end
class << Graphics
  def freeze
  end
  def transition(duration = 10, *)
  end
  def fadein(duration)
  end
  def fadeout(duration)
  end
end
