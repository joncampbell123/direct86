
void PushWord(DWORD v);
void PushDword(DWORD o);
DWORD PopWord();
DWORD PopDword();
void InterruptFrameCall16(DWORD new_cs,DWORD new_ip);
void InterruptFrameRet16();
void SignalInterrupt16(int num);
void NearCall16(int newip);
void FarCall16(int new_cs,int new_ip);
