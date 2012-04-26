module RbConfig
  rubydir = ENV["RUBY_DIR"]
  devdir = ENV["DEVKIT_DIR"]

  ENV["PATH"] += ";#{devdir.gsub(/\//, '\\')}\\mingw\\bin" if devdir

  CONFIG = {
    "game_dir" => File.expand_path("#{__FILE__}/../../.."),
    "ruby_version" => "1.9.2",
    "rubyhdrdir" => "#{rubydir}/include/ruby-1.9.1",
    "arch" => "i386-mingw32",
    "LDSHARED" => "#{devdir}/mingw/bin/gcc -shared",
    "libdir" => "#{rubydir}/lib",
    "RUBY_SO_NAME" => "msvcrt-ruby191" ,
    "DLEXT" => "so",
  }
end
