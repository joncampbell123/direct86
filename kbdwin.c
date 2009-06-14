// keyboard window

#include "global.h"
#include "resource.h"
#include "direct86.h"
#include "kbdwin.h"
#include "keyboard.h"
#include "mother.h"

char* szClassNameKBW =		"JMC_PROG_SOTHERE_DEDITx86_KEYB";
BYTE						keyboard_buffer[KEYBUF_MAX];
int							keyboard_buffer_head=0;
int							keyboard_buffer_tail=0;
BOOL						keyb_ack=FALSE;
HWND						hwndMainKeyb;
HDC							KeybWinDC;
BOOL						keyboard_win_showing=FALSE;

BYTE keybalpha2scan[26] = {
// A,B,C,D,E,F,G,H
	0x1E,0x30,0x2E,0x20,0x12,0x21,0x22,0x23,
// I,J,K,L,M,N,O,P
	0x17,0x24,0x25,0x26,0x32,0x31,0x18,0x19,
// Q,R,S,T,U,V,W,X
	0x10,0x13,0x1F,0x14,0x16,0x2F,0x11,0x2D,
// Y,Z
	0x15,0x2C};

void KeyboardBufferAdd(BYTE code)
{
	if (keyboard_buffer_head == ((keyboard_buffer_tail + KEYBUF_MAX - 1) % KEYBUF_MAX)) {
		MessageBeep(0xFFFFFFFF);
		return;
	}
	keyboard_buffer[keyboard_buffer_head] = code;
	keyboard_buffer_head++;
	keyboard_buffer_head %= KEYBUF_MAX;
}

int KeyboardBufferRead()
{
	int d;

	if (keyboard_buffer_head == keyboard_buffer_tail) {
		return -1;
	}

	d = (int)keyboard_buffer[keyboard_buffer_tail];
	keyboard_buffer_tail++;
	keyboard_buffer_tail %= KEYBUF_MAX;
	return d;
}

int KeyboardBufferPeek()
{
	int d;

	if (keyboard_buffer_head == keyboard_buffer_tail) {
		return -1;
	}

	d = (int)keyboard_buffer[keyboard_buffer_tail];
	return d;
}

void KeyboardBufferReset()
{
	keyboard_buffer_head = keyboard_buffer_tail = 0;
}

// Takes a Windows "Virtual Scan Code" and converts it into a stream of keyboard codes
void WinVscToScan(int d,int dn,DWORD lParam)
{
	if (d >= VK_F1 && d <= VK_F10)
		if (dn)
			KeyboardBufferAdd((BYTE)((d - VK_F1) + 59));
		else
			KeyboardBufferAdd((BYTE)(((d - VK_F1) + 59) | 0x80));

	if (d >= '1' && d <= '9')
		if (dn)
			KeyboardBufferAdd((BYTE)((d - '1') + 2));
		else
			KeyboardBufferAdd((BYTE)(((d - '1') + 2) | 0x80));

	if (d == '0') {
		if (dn)
			KeyboardBufferAdd(11);
		else
			KeyboardBufferAdd(11 | 0x80);
	}

	if (d >= 'A' && d <= 'Z') {
		if (dn)
			KeyboardBufferAdd(keybalpha2scan[d - 'A']);
		else
			KeyboardBufferAdd((BYTE)(keybalpha2scan[d - 'A'] | 0x80));
	}

	if (d == 0xC0) {			// Tilde (undefined)
		if (dn)
			KeyboardBufferAdd(41);
		else
			KeyboardBufferAdd(41 | 0x80);
	}
	if (d == 0xBB) {		// '='
		if (dn)
			KeyboardBufferAdd(13);
		else
			KeyboardBufferAdd(13 | 0x80);
	}
	if (d == 0xBA) {		// ';'
		if (dn)
			KeyboardBufferAdd(39);
		else
			KeyboardBufferAdd(39 | 0x80);
	}
	if (d == 0xBC) {		// ','
		if (dn)
			KeyboardBufferAdd(51);
		else
			KeyboardBufferAdd(51 | 0x80);
	}
	if (d == 0xBE) {		// '.'
		if (dn)
			KeyboardBufferAdd(52);
		else
			KeyboardBufferAdd(52 | 0x80);
	}
	if (d == 0xDE) {		// '''
		if (dn)
			KeyboardBufferAdd(40);
		else
			KeyboardBufferAdd(40 | 0x80);
	}
	if (d == 0xBD) {
		if (dn)
			KeyboardBufferAdd(12);
		else
			KeyboardBufferAdd(12 | 0x80);
	}
	if (d == 0xBF) {
		if (dn)
			KeyboardBufferAdd(53);
		else
			KeyboardBufferAdd(53 | 0x80);
	}
	if (d == 0xDB) {
		if (dn)
			KeyboardBufferAdd(26);
		else
			KeyboardBufferAdd(26 | 0x80);
	}
	if (d == 0xDC) {
		if (dn)
			KeyboardBufferAdd(43);
		else
			KeyboardBufferAdd(43 | 0x80);
	}
	if (d == 0xDD) {
		if (dn)
			KeyboardBufferAdd(27);
		else
			KeyboardBufferAdd(27 | 0x80);
	}
	if (d == VK_ESCAPE) {
		if (dn)
			KeyboardBufferAdd(1);
		else
			KeyboardBufferAdd(1 | 0x80);
	}
	if (d == VK_SPACE) {
		if (dn)
			KeyboardBufferAdd(57);
		else
			KeyboardBufferAdd(57 | 0x80);
	}
	if (d == VK_TAB) {
		if (dn)
			KeyboardBufferAdd(15);
		else
			KeyboardBufferAdd(15 | 0x80);
	}
	if (d == VK_BACK) {
		if (dn)
			KeyboardBufferAdd(14);
		else
			KeyboardBufferAdd(14 | 0x80);
	}
	if (d == VK_RETURN) {
		if (dn)
			KeyboardBufferAdd(28);
		else
			KeyboardBufferAdd(28 | 0x80);
	}
	if (d == VK_DELETE) {
		if (dn)
			KeyboardBufferAdd(0x53);
		else
			KeyboardBufferAdd(0x53 | 0x80);
	}

// shift stuff?
	if (d == VK_CAPITAL) {
		if (dn)
			KeyboardBufferAdd(0x3A);
		else
			KeyboardBufferAdd(0x3A | 0x80);
	}
	if (d == VK_SHIFT) {
		if (lParam & (1<<24)) {
			if (dn)
				KeyboardBufferAdd(0x36);
			else
				KeyboardBufferAdd(0x36 | 0x80);
		}
		else {
			if (dn)
				KeyboardBufferAdd(0x2A);
			else
				KeyboardBufferAdd(0x2A | 0x80);
		}
	}
// CTRL?
	if (d == VK_CONTROL) {
		if (lParam & (1<<24)) {
			KeyboardBufferAdd(0xE0);	// signal Right Ctrl
		}

		if (dn)
			KeyboardBufferAdd(0x1D);
		else
			KeyboardBufferAdd(0x1D | 0x80);
	}
}

void BitmapBlt(HDC hDC,BITMAPINFO *ptr,int x,int y,int w,int h,char *img)
{
	ptr->bmiHeader.biWidth = w;
	ptr->bmiHeader.biHeight = -h;
	ptr->bmiHeader.biSizeImage = abs(h)*w;
	SetDIBitsToDevice(hDC,x,y,(DWORD)w,(DWORD)abs(h),0,0,0,abs(h),img,ptr,DIB_PAL_COLORS);
}

long FAR PASCAL MainWndProcKeyb(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	int x,y;
	switch (iMessage) {
	
	case WM_CREATE:
		KeybWinDC = GetDC(hwnd);
		SelectObject(KeybWinDC,EditFont);
		SetTextColor(KeybWinDC,RGB(0,0,0));
		SetBkColor(KeybWinDC,RGB(255,255,255));
		break;

	case WM_PAINT:
		SendMessage(hwnd,WM_TIMER,0,0L);
		ValidateRect(hwnd,NULL);
		return DefWindowProc(hwnd,iMessage,wParam,lParam);

	case WM_KEYDOWN:
		x = (int)wParam;
		WinVscToScan(x,1,lParam);
		return DefWindowProc(hwnd,iMessage,wParam,lParam);

	case WM_KEYUP:
		x = (int)wParam;
		WinVscToScan(x,0,lParam);
		return DefWindowProc(hwnd,iMessage,wParam,lParam);

	case WM_LBUTTONDOWN:
		x = LOWORD(lParam);
		y = HIWORD(lParam);
		break;
	
	case WM_CLOSE:
// don't close, just hide yourself
		SendMessage(hwndMain,WM_COMMAND,IDM_WINDOWS_KEYB,0);
		break;
	
	case WM_TIMER:
		break;

	case WM_DESTROY:
		ReleaseDC(hwnd,KeybWinDC);
		break;

	case WM_SETCURSOR:
		SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));
		break;

default:
		return DefWindowProc(hwnd,iMessage,wParam,lParam);
	}
	return 0L;
}
