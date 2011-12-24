#include "ext_rgss.h"

HWND hWndRgss;

static BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lp) {
  DWORD pId;
  GetWindowThreadProcessId(hWnd, &pId);
  if(pId == lp) {
    hWndRgss = hWnd;
    return FALSE;
  }
  return TRUE;
}

int Init_ext_rgss() {
  RECT rect;

  EnumWindows(EnumWindowsProc, (LPARAM)GetCurrentProcessId());
  GetWindowRect(hWndRgss, &rect);
  printf("%d %d %d %d\n", rect.left, rect.top, rect.right, rect.bottom);
  return 1;
}
