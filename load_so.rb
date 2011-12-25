# encoding: utf-8
#__END__
Win32API.new("ext_rgss", "Init_ext_rgss", "ii", "i").call(method(:method).object_id * 2, Object.object_id * 2)
Graphics.init

# generate stub and show TODO
[:Graphics].each do |name|
  old_name = "Old#{name}".intern
  method_list = Object.const_get(old_name).methods(false) - Object.const_get(name).methods(false)
  method_list.each do |method_name|
    eval "def #{name}.#{method_name}(*args); #{old_name}.send(:#{method_name}, *args); end"
  end
  puts "* #{name} TODO:\n  methods - #{method_list.join(', ')}"
end
