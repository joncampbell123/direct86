
extern DWORD						debug_window_offset32;
extern DWORD						debug_window_offset_cs;
extern DWORD						debug_window_offset_ip;
extern BOOL							fl_d,fl_w,fl_s;
extern int							fl_mod,fl_reg,fl_rm,fl_fop;
extern DWORD						fl_bytes;
extern BOOL							db66,db67,db66a,db67a;
extern char							intcbuf[256];
extern char							fl_prefix[256];
extern char*						regs8[];
extern char*						regs16[];
extern char*						regs32[];
extern char*						regsegs[];
extern char*						bptr;
extern char*						wptr;
extern char*						fptr;
extern char*						dptr;
extern char*						displ[];
extern char*						displ32[];
extern char*						displ_67[];
extern char*						conjmp[];
extern char*						conjmpnear[];

void DecompileBytes(DWORD *o_cs,DWORD *o_ip,char *s);
void DecompileToFile(DWORD cs,DWORD from_ip,DWORD to_ip,char *path);
