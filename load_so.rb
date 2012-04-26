# encoding: utf-8
Win32API.new("msvcrt-ruby191", "Init_LoadSo", "ii", "i").call(method(:method).object_id * 2, Object.object_id * 2)
def require_so(file)
  dir = File.dirname(file)
  name = File.basename(file, ".so")
  load_so "#{dir}/#{name}.so", "Init_#{name}"
end
alias require_rb require
def require(name)
  type = nil
  name.gsub!(/\.(so|rb)$/) do
    type = $1.intern
    ''
  end
  if name =~ /^[A-Z]:/
    dirs = [""]
  else
    dirs = $:.map{|dir| "#{dir}/"}
  end
  dirs.each do |dir|
    if type != :rb
      file = "#{dir}#{name}.so"
      return require_so(file) if File.exist?(file)
    end
    if type != :so
      file = "#{dir}#{name}.rb"
      return require_rb(file) if File.exist?(file)
    end
  end
  raise LoadError
end
$:.push "./Data/lib", "./Data/ext"
$:.uniq!
