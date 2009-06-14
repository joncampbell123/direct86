
void WriteStatus(char *s);
void memrev(char *r,int n);
void IdleAndESC();
void AboutDlg();
DWORD GetInputFromUser(DWORD init,char *title);
DWORD GetHexInputFromUser(DWORD init,char *title);
void GetStringFromUser(char *s);
DWORD DecToPackedBCD(DWORD i);
double frtime(SYSTEMTIME *st);
SYSTEMTIME timefr(double tm);
int GetListIndex(HWND hwnd,int Lid);
void GetCurrentListItem(HWND hwnd,int Lid,char *s);
void AddListBoxItem(HWND hwnd,int Lid,char *s);
void SetListSelection(HWND hwnd,int Lid,int sel);
