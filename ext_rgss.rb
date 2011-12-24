# encoding: utf-8
#__END__
if Win32API.new("ext_rgss", "Init_ext_rgss", "", "i").call == 0
  raise "ExtRgss initialize error"
end
class << Graphics
  def update
    sleep 1.0 / frame_rate
  end
  def transition(duration = 10, *)
    sleep duration.to_f / frame_rate
  end
  def fadein(duration)
    sleep duration.to_f / frame_rate
   end
  def fadeout(duration)
    sleep duration.to_f / frame_rate
   end
end
