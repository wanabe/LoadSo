# encoding: utf-8
# 準備：
# 0. LoadSo を準備しておく。（README.txt を参考にしてください）
#
# 1. DevKit, ruby 1.9.2, RubyInline の inline.rb を用意する。それぞれ、
#    http://rubyinstaller.org/downloads/ の「Development Kit」セクション、
#    同上 URL の Ruby 1.9.2、
#    https://raw.github.com/seattlerb/rubyinline/master/lib/inline.rb
#    あたりから。
#
# 2. ゲームの Data フォルダ内に lib フォルダを作り、さらにその lib フォルダ内に
#    digest フォルダを作る。
#
# 3. 用意した ruby から md5.so を digest フォルダに、digest.so, digest.rb, fileutils.rb,
#    および sample_rubyinline フォルダ内のすべてのファイルを lib フォルダに。
#    さらに、ruby 1.9.2 の bin フォルダから libeay32-1.0.0-msvcrt.dll を Game.exe と同じフォルダに。
#    （RubyInstaller 以外の ruby を使っている場合、この DLL は不要だったり別名だったりするかもしれません）
#    図示: Game.exe
#          libeay32-1.0.0-msvcrt.dll
#          Data/
#           + lib/
#              + digest/
#              |  +- md5.so
#              |
#              +- digest.so
#              +- digest.rb
#              +- fileutils.rb
#              +- rbconfig.rb (stub)
#              +- rubygems.rb
#              +- zentest_mapping.rb
#
# 4. DevKit をインストールしたフォルダ名を環境変数 DEVKIT_DIR に、ruby 1.9.2 を
#    インストールしたフォルダ名を環境変数 RUBY_DIR に指定する。
#    （RubyInstaller ならインストールせずに .7z を解凍するだけでもよい）
#    環境変数が何か分からない人は調べるか、または rbconfig.rb を直接書き換える。
#    そのとき、円記号区切りでなくスラッシュ区切りで書くことに注意。
#    例: ENV["RUBY_DIR"] の部分を "C:/ruby-1.9.2" に、ENV["DEVKIT_DIR"] の部分を
#        "C:/DevKit" に書き換える。
#    注意: スペースを含むフォルダ名を使用しないでください。(C:/Program Files など)
#          inline.rb の build メソッドを書き換えれば対応できるかもしれませんが未知数です。
#
# 5. ゲーム配布時などは、rbconfig.rb の ENV["RUBY_DIR"] および ENV["DEVKIT_DIR"]
#    の部分は nil に書き換えるとコンパイルが行われなくなる。
#    （あまりないことですが）偶然環境変数がセットされていたりすると、予期せぬ動作
#    をする可能性があるので注意。
#
# 6. さらに言えば、ゲーム配布時は RubyInline で作られた .so ファイルを抜き出して
#    （Data/.ruby_inline/ruby-1.9.2/ にあると思います）直接 require してやる方が
#    不要なバグを生まないのでよい。工程 3. で用意したライブラリたちも含めなくてよくなる。

require "inline"
ENV["INLINEDIR"] = "#{RbConfig::CONFIG['game_dir']}/Data"
module Inline
  class C
    def rb_file
      "#{RbConfig::CONFIG['game_dir']}/Data/Scripts.rvdata2"
    end
    def crap_for_windoze
      c = RbConfig::CONFIG
      " -Wl,--enable-auto-import -L#{c['libdir']} -l#{c['RUBY_SO_NAME']}"
    end
  end
end

# sample

class Array
  inline do |builder|
    builder.c_raw "
      static VALUE average(int argc, VALUE *argv, VALUE self) {
        double result = 0;
        long  i, len;
        VALUE *arr = RARRAY_PTR(self);
        len = RARRAY_LEN(self);

        for(i=0; i<len; i++) {
          result += NUM2DBL(arr[i]);
        }

        return rb_float_new(result/(double)len);
      }
    "
  end
end

max_loop = (ARGV.shift || 5).to_i
max_size = (ARGV.shift || 100_000).to_i
a = (1..max_size).to_a

1.upto(max_loop) do
  avg = a.average
  $stderr.print "."
end
