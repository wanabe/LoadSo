* ExtRgss について
ExtRgss は、RPGツクールのゲームエンジンを高速化することを目的としたプロジェクトです。
現在、RPGツクールVX Aceのごく一部の機能のみを対象に開発しています。
現状何も動きません。

「RPGツクール」は、株式会社アスキー、および株式会社エンターブレインの登録商標です。

* 使用方法
まず ext_rgss.dll を Game.exe と同じディレクトリに置いてください。
次にツクールのスクリプトエディタ先頭に ext_rgss.rb を張り付けてください。
または require や load で読み込むようにしても構いません。
必ず他のスクリプトより先に読み込むようにしてください。

* ビルド方法
ext_rgss.dll のビルドから始める場合は、

 + Makefile が読める make（GNU Make で動作確認しています）
 + Windows DLL が出力可能な C コンパイラ（MinGW-w64 で動作確認しています）

が必要になります。用意できたらソースコードのあるディレクトリで
make コマンドを実行するだけで dll が出来上がると思います。

* ライセンス
LICENSE.txt または LICENSE.ja.txt いずれかのファイルに従ってください。
どちらも基本的に同じものですが、英文を読むのが嫌な方のための和訳が .ja の方です。
訳に不備があって違うものになっている可能性もあります。
その場合、教えていただけると助かります。

本ライブラリには ruby 1.9.2 からの引用を多く含みます。
http://svn.ruby-lang.org/cgi-bin/viewvc.cgi/branches/ruby_1_9_2/
Ruby : Copyright (C) 1993-2010 Yukihiro Matsumoto. All rights reserved.
