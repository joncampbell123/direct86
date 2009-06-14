// keyboard_window.cpp header file

#define KEYBUF_MAX		256

extern BOOL						keyboard_win_showing;
extern HWND						hwndMainKeyb;
extern char*					szClassNameKBW;

void KeyboardBufferAdd(BYTE code);
int KeyboardBufferRead();
void KeyboardBufferReset();

long FAR PASCAL MainWndProcKeyb(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
