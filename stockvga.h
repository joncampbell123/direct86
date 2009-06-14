
#define CHROMA_RED				((128*35)/100)
#define CHROMA_GREEN			((128*54)/100)
#define CHROMA_BLUE				((128*11)/100)

long FAR PASCAL MainWndProcVGA(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

extern char*						szClassNameVGA;
extern BOOL							display_win_showing;
extern HPALETTE						VGAPaletteObj;
extern LOGPALETTE*					VGAPaletteLog;
extern BITMAPINFO*					VGAParams;
extern BYTE*						VGARenderBuffer;
extern BYTE*						VGARAM;
extern int							VGARAMSize;
extern int							VGAScreenX,VGAScreenY;
extern int							VGARenderScaleX,VGARenderScaleY;		// fixed point
extern double						VGA_AspectRatio;
extern BOOL							VGA_TextDisplay;
extern int							VGA_TextFontHeight;
extern DWORD						VGA_OffsetBank;
extern int							VGA_Cursor_X,VGA_Cursor_Y;
extern int							VGA_Cursor_PX,VGA_Cursor_PY;
extern int							VGA_Cursor_visible,VGA_Cursor_visblink,VGA_Cursor_visdraw;
extern int							VGA_pan_x,VGA_pan_y;
extern BYTE							VGAPalette[768];
extern BYTE							VGACharacterRAM[256*32];
extern BYTE							BitRev[256];
extern BYTE							VGA_CRTCREGS[256];
extern int							VGA_CRTCIndx;
extern HWND							hwndMainVGA;

void VGARedraw(int x1,int y1,int x2,int y2);
void VGARenderByRealDisplay(int x1,int y1,int x2,int y2);
void VGARenderByVGA(int x1,int y1,int x2,int y2);
void VGAReset();
void VGAInit();
void VGAClose();
void VGAPowerOn();
void VGAPowerOff();
int VGAIsPowered();

long FAR PASCAL MainWndProcVGA(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
