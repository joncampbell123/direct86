
void FlushCPUQueue();
BYTE CPUQueueFetch();

void BeginDecoderQueue(DWORD seg,DWORD ofs);
void FlushCPUDecoderQueue();
BYTE CPUDecoderQueueFetch();
void EndDecoderQueue(DWORD *seg,DWORD *ofs);

extern DWORD deci_eip;
extern DWORD deci_cs;

