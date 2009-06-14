
extern char*						szClassName3;
extern char							rbuffer2[1024];
extern char							rbuffer2b[256];
extern BOOL							MemoryWinUpdate;
extern DWORD						memory_win_begin,memory_win_end;
extern DWORD						memory_window_cs;
extern DWORD						memory_window_eip;
extern int							MemWindowRect_w,MemWindowRect_h;
extern int							cursor_mem_x,cursor_mem_y;
extern HWND							hwndMainMem;
extern HDC							MemWinDC;
extern BOOL							memory_win_showing;

long FAR PASCAL MainWndProcMem(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
