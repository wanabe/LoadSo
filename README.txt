* LoadSo �ɂ���
LoadSo �́ARPG�c�N�[���̃Q�[���G���W���� Ruby �̊g�����C�u������ǂݍ��ނ��Ƃ�ړI�Ƃ����v���W�F�N�g�ł��B
���݁ARPG�c�N�[��VX Ace�݂̂�ΏۂɊJ�����Ă��܂��B
����AExtRgss ���番�������΂���ŁA�܂��������Ă����܂���B���������܂���B

�uRPG�c�N�[���v�́A������ЃA�X�L�[�A����ъ�����ЃG���^�[�u���C���̓o�^���W�ł��B

* �g�p���@
�܂� msvcrt-ruby191.dll �� Game.exe �Ɠ����f�B���N�g���ɒu���Ă��������B
���Ƀc�N�[���̃X�N���v�g�G�f�B�^�擪�� load_so.rb �𒣂�t���Ă��������B
�܂��� require �� load �œǂݍ��ނ悤�ɂ��Ă��\���܂���B
�K�����̃X�N���v�g����ɓǂݍ��ނ悤�ɂ��Ă��������B
�Ō�ɁAdll �Ɠ����ꏊ�ɓǂݍ��݂����g�����C�u������u���A�X�N���v�g����
  require_so "�i���C�u�������j"
�Ƃ��Ă��������B

* �r���h���@
msvcrt-ruby191.dll �̃r���h����n�߂�ꍇ�́A

 + Makefile ���ǂ߂� make�iGNU Make �œ���m�F���Ă��܂��j
 + Windows DLL ���o�͉\�� C �R���p�C���iMinGW-w64 �œ���m�F���Ă��܂��j

���K�v�ɂȂ�܂��B�p�ӂł�����\�[�X�R�[�h�̂���f�B���N�g����
make �R�}���h�����s���邾���� dll ���o���オ��Ǝv���܂��B

* ���C�Z���X
COPYING.txt �܂��� COPYING.ja.txt �����ꂩ�̃t�@�C���ɏ]���Ă��������B
�P���� ruby �� COPYING, COPYING.ja �̓�̃t�@�C���� Windows �œǂ݂₷���悤��
���s�R�[�h�ƕ����R�[�h��ς������̂ł��B

�{���C�u�����ɂ� ruby 1.9.2 ����̈��p�𑽂��܂݂܂��B
http://svn.ruby-lang.org/cgi-bin/viewvc.cgi/branches/ruby_1_9_2/
Ruby : Copyright (C) 1993-2010 Yukihiro Matsumoto. All rights reserved.
