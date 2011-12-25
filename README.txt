* LoadSo について
LoadSo は、RPGツクールのゲームエンジンで Ruby の拡張ライブラリを読み込むことを目的としたプロジェクトです。
現在、RPGツクールVX Aceのみを対象に開発しています。
現状、ExtRgss から分離したばかりで、まだ動かしてもいません。多分動きません。

「RPGツクール」は、株式会社アスキー、および株式会社エンターブレインの登録商標です。

* 使用方法
まず msvcrt-ruby191.dll を Game.exe と同じディレクトリに置いてください。
次にツクールのスクリプトエディタ先頭に load_so.rb を張り付けてください。
または require や load で読み込むようにしても構いません。
必ず他のスクリプトより先に読み込むようにしてください。
最後に、dll と同じ場所に読み込みたい拡張ライブラリを置き、スクリプトから
  require_so "（ライブラリ名）"
としてください。

* ビルド方法
msvcrt-ruby191.dll のビルドから始める場合は、

 + Makefile が読める make（GNU Make で動作確認しています）
 + Windows DLL が出力可能な C コンパイラ（MinGW-w64 で動作確認しています）

が必要になります。用意できたらソースコードのあるディレクトリで
make コマンドを実行するだけで dll が出来上がると思います。

* ライセンス
COPYING.txt または COPYING.ja.txt いずれかのファイルに従ってください。
単純に ruby の COPYING, COPYING.ja の二つのファイルを Windows で読みやすいように
改行コードと文字コードを変えたものです。

本ライブラリには ruby 1.9.2 からの引用を多く含みます。
http://svn.ruby-lang.org/cgi-bin/viewvc.cgi/branches/ruby_1_9_2/
Ruby : Copyright (C) 1993-2010 Yukihiro Matsumoto. All rights reserved.
