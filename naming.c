// this code allows the assocation of names and attributes with
// code sequences and addresses in memory
//
// VERY helpful for debugging

#include "global.h"
#include <stdio.h>
#include <string.h>
#include "naming.h"
#include "direct86.h"
#include "brkpts.h"
#include "cpuexec.h"
#include "addrbus.h"
#include "resource.h"

NAMEASSOCMEM		name_assoc_mem[NAMEASSOCMEMMAX];
NAMEASSOCVAR		name_assoc_var[NAMEASSOCVARMAX];
NAMEASSOCSEQUENCE	name_assoc_seq[NAMEASSOCSEQMAX];
int					name_assoc_mem_next_alloc=0;
int					name_assoc_var_next_alloc=0;
int					name_assoc_seq_next_alloc=0;
int					name_assoc_mem_next_l=0;
int					name_assoc_mem_next_sz=0;
int					name_assoc_mem_next_szsub=0;
DWORD				name_assoc_mem_should_be_ofs=0;
DWORD				name_assoc_mem_should_be_seg=0;
int					name_assoc_mem_itm_idx=0;
int					name_assoc_mem_itm_added=-1;
int					name_assoc_seq_itm_added=-1;
int					name_assoc_mem_search_typeconst = -1;
int					name_assoc_mem_search_no_notes = 0;
int					name_assoc_seq_matches[32];
int					name_assoc_seq_matches_cnt=0;
int					naming_enabled=0;

void InitNaming()
{
	int x;

	for (x=0;x < NAMEASSOCMEMMAX;x++) {
		name_assoc_mem[x].enabled = 0;
		name_assoc_mem[x].inuse = 0;
		name_assoc_mem[x].name = NULL;
		name_assoc_mem[x].type = 0;
	}
	for (x=0;x < NAMEASSOCSEQMAX;x++) {
		name_assoc_seq[x].inuse = 0;
		name_assoc_seq[x].opcode_seq_length = 0;
		name_assoc_seq[x].opcode_match_idx = 0;
	}

	for (x=0;x < NAMEASSOCVARMAX;x++) {
		name_assoc_var[x].inuse = 0;
		name_assoc_var[x].varname = NULL;
		name_assoc_var[x].data = 0;
	}
}

void ClearVarNames()
{
	int x;

	for (x=0;x < NAMEASSOCVARMAX;x++) {
		name_assoc_var[x].inuse = 0;
		if (name_assoc_var[x].varname) free(name_assoc_var[x].varname);
		name_assoc_var[x].varname=NULL;
		name_assoc_var[x].data = 0;
	}

	name_assoc_var_next_alloc=0;
}

void ClearNaming()
{
	int x;

	for (x=0;x < NAMEASSOCMEMMAX;x++) {
		if (name_assoc_mem[x].name) free(name_assoc_mem[x].name);
		name_assoc_mem[x].enabled = 0;
		name_assoc_mem[x].inuse = 0;
		name_assoc_mem[x].name = NULL;
		name_assoc_mem[x].type = 0;
	}
	for (x=0;x < NAMEASSOCSEQMAX;x++) {
		name_assoc_seq[x].inuse = 0;
		name_assoc_seq[x].opcode_seq_length = 0;
		name_assoc_seq[x].opcode_match_idx = 0;
	}
	name_assoc_mem_next_alloc=0;
	name_assoc_seq_next_alloc=0;
	name_assoc_seq_matches_cnt = 0;
}

void CloseNaming()
{
	ClearNaming();
	ClearVarNames();
}

void LoadSavedNamingFile(char *pathname)
{
	FILE *fp;
	char buf[1024];
	char name[512];
	char varname[512];
	char varexp[260];
	int c,segcon,strh,prefsz;
	unsigned long a1,a2,aei,aes,b1,b2,bei,bes;
	int l,x,type,nx,req_satisfied;
	int opcode_len;
	int opcodes[256];
	char old_title[1024];
	DWORD dd;
	char *ptr;

	fp=fopen(pathname,"rb");
	if (!fp) return;

	GetWindowText(hwndMain,old_title,1020);
	sprintf(buf,"Loading symbol name file %s",pathname);
	SetWindowText(hwndMain,buf);

	l=1;
	req_satisfied=1;
	while (l) {
/* read one line */
		x=0;
		c=fgetc(fp);
		if (c == 13) c=fgetc(fp);
		if (c == 10) c=fgetc(fp);
		while (x < 1020 && !(c == 13 || c == 10 || c == -1 || c == 26)) {
			buf[x++] = c;
			c=fgetc(fp);
		}
		buf[x] = 0;

		while (!(c == 13 || c == 10 || c == -1 || c == 26)) c=fgetc(fp);

		if (c == -1 || c == 26) l=0;

/* parse it */
		x=0;
		type = -1;
		strh=0;
		segcon=0;
		strh=0;
		prefsz=0;
		aei=bei=0;
		aes=bes=0;
		segcon=0;
		a1=a2=b1=b2=0;
		prefsz=0;
		while (buf[x] != 0 && buf[x] == 32) x++;

		if (!strnicmp(buf+x,"label ",6) && req_satisfied) {
			x += 6;
			type = NAS_LABEL;
		}
		else if (!strnicmp(buf+x,"proc ",5) && req_satisfied) {
			x += 5;
			type = NAS_PROC;
		}
		else if (!strnicmp(buf+x,"var ",4) && req_satisfied) {
			x += 4;
			type = NAS_MEMVAR;
		}
		else if (!strnicmp(buf+x,"annotation ",11) && req_satisfied) {
			x += 11;
			type = NAS_NOTES;
		}
		else if (!strnicmp(buf+x,"sequence ",9) && req_satisfied) {
			x += 9;

			ptr = buf+x;

			while (*ptr == 32 && *ptr && *ptr != ';' && *ptr != '[') ptr++;
			if (*ptr != '[') {
/* get address one */
				if (isdigit(*ptr)) {
					a1 = strtoul(ptr,&ptr,0);
					aei=1;
				}
				else {
					nx=0;
					while (*ptr && *ptr != 32 && *ptr != ';' && *ptr != ':') varname[nx++] = *ptr++;
					varname[nx] = 0;

					if (GetNamingVar(varname,&dd)) {
						a1 = (int)dd;
						aei=1;
					}
				}

/* get second param (only if first one existed) */
				if (aei) {
					while (*ptr && *ptr == 32) *ptr++;
					if (*ptr == ':') {
						ptr++;
						x++;
					}
					while (*ptr != 0 && *ptr == 32) *ptr++;

					if (isdigit(*ptr)) {
						a2 = strtoul(ptr,&ptr,0);
						aes=1;
					}
					else {
						nx=0;
						while (*ptr && *ptr != 32 && *ptr != ';' && *ptr != ':') varname[nx++] = *ptr++;
						varname[nx] = 0;

						if (GetNamingVar(varname,&dd)) {
							a2 = (int)dd;
							aes=1;
						}
					}
				}

/* get second address (if present) */
				while (*ptr && *ptr == 32) *ptr++;
				if (*ptr == '-') {
					ptr++;
					while (*ptr && *ptr == 32) *ptr++;

					if (isdigit(*ptr)) {
						b1 = strtoul(ptr,&ptr,0);
						bei=1;
					}
					else {
						nx=0;
						while (*ptr && *ptr != 32 && *ptr != ';' && *ptr != ':') varname[nx++] = *ptr++;
						varname[nx] = 0;

						if (GetNamingVar(varname,&dd)) {
							b1 = (int)dd;
							bei=1;
						}
					}

/* get second param (only if first one existed) */
					if (bei) {
						while (*ptr && *ptr == 32) *ptr++;
						if (*ptr == ':') {
							ptr++;
							x++;
						}
						while (*ptr != 0 && *ptr == 32) *ptr++;

						if (isdigit(*ptr)) {
							b2 = strtoul(ptr,&ptr,0);
							bes=1;
						}
						else {
							nx=0;
							while (*ptr && *ptr != 32 && *ptr != ';' && *ptr != ':') varname[nx++] = *ptr++;
							varname[nx] = 0;

							if (GetNamingVar(varname,&dd)) {
								b2 = (int)dd;
								bes=1;
							}
						}
					}
				}

				if (!aes) aei=0;
				if (!bes) bei=0;
			}

			while (*ptr == 32 && *ptr && *ptr != ';' && *ptr != '[') ptr++;
			while (*ptr && *ptr != ';' && *ptr != '[') ptr++;

			if (*ptr == '[') {
				ptr++;
				opcode_len=0;

				while (*ptr != ']' && *ptr) {
					while (*ptr == 32 && *ptr && *ptr != ';' && *ptr != ']') ptr++;

					if (opcode_len < 256) {
						if (isdigit(*ptr)) {
							opcodes[opcode_len] = strtoul(ptr,&ptr,0);
							opcode_len++;
						}
						else if (*ptr == '?') {
							ptr++;
							opcodes[opcode_len] = -1;
							opcode_len++;
						}
						else {
							if (*ptr != ']') ptr++;
						}
					}
					else {
						if (*ptr != ']') ptr++;
					}
				}
				ptr++;

				while (*ptr == 32 && *ptr && *ptr != ';') ptr++;

				nx=0;
				while (*ptr != 32 && *ptr && *ptr != ';') varexp[nx++] = *ptr++;
				varexp[nx++] = 0;
			}

			if (!aei) {
				a1 = 0x0000;
				a2 = 0x0;
			}
			if (!bei) {
				if (!aei) {
					b1 = 0xFFFFFFFF;
					b2 = 0xFFFFFFFF;
				}
				else {
					b1 = a1;
					b2 = a2 + opcode_len + -1;
				}
			}

			if (varexp[0] && opcode_len > 0) {
				if (AddNamingSeq(opcode_len,opcodes,varexp)) {
					name_assoc_seq[name_assoc_seq_itm_added].seg_begin = a1;
					name_assoc_seq[name_assoc_seq_itm_added].ofs_begin = a2;
					name_assoc_seq[name_assoc_seq_itm_added].seg_end   = b1;
					name_assoc_seq[name_assoc_seq_itm_added].ofs_end   = b2;
				}
			}
		}
		else if (!strnicmp(buf+x,"requires ",9)) {
			x += 9;

/* look at the variable name it requires */
			nx=0;
			while (buf[x] == 32) x++;
			while (buf[x] && buf[x] != ';') name[nx++] = buf[x++];
/* kill trailing spaces */
			while (nx > 0 && name[nx-1] == 32) nx--;
			name[nx] = 0;

			if (!name[0]) {
				req_satisfied=1;
			}
			else if (!strcmpi(name,"none")) {
				req_satisfied=1;
			}
			else {
				req_satisfied=GetNamingVar(name,NULL);
			}
		}

		if (type >= 0 && req_satisfied) {
/* get name */
			while (buf[x] != 0 && buf[x] == 32) x++;

			nx=0;
			if (buf[x] == '\"')	{
				x++;
				while (buf[x] != 0 && buf[x] != '\"') name[nx++] = buf[x++];
				x++;
				name[nx] = 0;
			}
			else {
				while (buf[x] != 0 && buf[x] != 32) name[nx++] = buf[x++];
				name[nx] = 0;
			}

/* get address one */
			ptr = buf+x;
			while (*ptr != 0 && *ptr == 32) *ptr++;

			if (isdigit(*ptr)) {
				a1 = strtoul(ptr,&ptr,0);
				aei=1;
			}
			else {
				nx=0;
				while (*ptr && *ptr != 32 && *ptr != ';' && *ptr != ':') varname[nx++] = *ptr++;
				varname[nx] = 0;

				if (GetNamingVar(varname,&dd)) {
					a1 = (int)dd;
					aei=1;
				}
			}

/* get second param (only if first one existed) */
			if (aei) {
				while (*ptr && *ptr == 32) *ptr++;
				if (*ptr == ':') {
					ptr++;

					while (*ptr != 0 && *ptr == 32) *ptr++;

					if (isdigit(*ptr)) {
						a2 = strtoul(ptr,&ptr,0);
						aes=1;
					}
					else {
						nx=0;
						while (*ptr && *ptr != 32 && *ptr != ';' && *ptr != ':') varname[nx++] = *ptr++;
						varname[nx] = 0;

						if (GetNamingVar(varname,&dd)) {
							a2 = (int)dd;
							aes=1;
						}
					}
				}
			}

/* get second address (if present) */
			while (*ptr && *ptr == 32) *ptr++;
			if (*ptr == '-') {
				ptr++;
				while (*ptr && *ptr == 32) *ptr++;

				if (isdigit(*ptr)) {
					b1 = strtoul(ptr,&ptr,0);
					bei=1;
				}
				else {
					nx=0;
					while (*ptr && *ptr != 32 && *ptr != ';' && *ptr != ':') varname[nx++] = *ptr++;
					varname[nx] = 0;

					if (GetNamingVar(varname,&dd)) {
						b1 = (int)dd;
						bei=1;
					}
				}

/* get second param (only if first one existed) */
				if (bei) {
					while (*ptr && *ptr == 32) *ptr++;
					if (*ptr == ':') {
						ptr++;

						while (*ptr != 0 && *ptr == 32) *ptr++;

						if (isdigit(*ptr)) {
							b2 = strtoul(ptr,&ptr,0);
							bes=1;
						}
						else {
							nx=0;
							while (*ptr && *ptr != 32 && *ptr != ';' && *ptr != ':') varname[nx++] = *ptr++;
							varname[nx] = 0;

							if (GetNamingVar(varname,&dd)) {
								b2 = (int)dd;
								bes=1;
							}
						}
					}
				}
			}

/* anything else */
			while (*ptr != 0 && *ptr == 32) *ptr++;
			while (*ptr != ';' && *ptr != 0) {
				while (*ptr != 0 && *ptr == 32) ptr++;

				if (!strnicmp(ptr,"constrain_seg",13))			{ segcon=1; ptr += 13; }
				else if (!strnicmp(ptr,"string",6))				{ strh=1; ptr += 6; }
				else if (!strnicmp(ptr,"byte_array",10))		{ prefsz=1; ptr += 10; }
				else if (!strnicmp(ptr,"word_array",10))		{ prefsz=2; ptr += 10; }
				else if (!strnicmp(ptr,"dword_array",11))		{ prefsz=4; ptr += 11; }
				else											ptr++;
			}

/* check entry and invalidate if bad */
			if (type == NAS_LABEL) {
				if (!aei) type = -1;
				if (!aes) type = -1;
				if (!bes) bei  = 0;
				b1 = a1;
				b2 = a2;
			}
			if (type == NAS_PROC) {
				if (!aei) type = -1;
				if (!aes) type = -1;
				if (!bes) bei  = 0;
				if (!bei) {
					b1 = a1;
					b2 = a2;
				}
			}
			if (type == NAS_MEMVAR) {
				if (!aei) type = -1;
				if (!aes) type = -1;
				if (!bes) bei  = 0;
				if (!bei) {
					b1 = a1;
					b2 = a2;
				}
			}
			if (type == NAS_NOTES) {
				if (!aei) type = -1;
				if (!aes) type = -1;
				if (!bes) bei  = 0;
				if (!bei) {
					b1 = a1;
					b2 = a2;
				}
			}

/* enter */
			if (type >= 0) {
				if (AddMemNamingAssoc(type,name,a1,b1,a2,b2,(char)segcon)) {
					name_assoc_mem[name_assoc_mem_itm_added].var_string = strh;
					name_assoc_mem[name_assoc_mem_itm_added].pref_typesize = prefsz;
				}
			}
		}
	}

	fclose(fp);
	SetWindowText(hwndMain,old_title);
}

void LoadSavedNaming()
{
	char paths[1024];
	int x;
	char buf[64];

	ClearNaming();

	SetCursor(LoadCursor(NULL,IDC_WAIT));

	x=0;
	sprintf(buf,"NameFile%d",x);
	GetPrivateProfileString("Naming",buf,"",paths,1000,common_ini_file);
	x++;
	while (paths[0]) {
		LoadSavedNamingFile(paths);

		sprintf(buf,"NameFile%d",x);
		GetPrivateProfileString("Naming",buf,"",paths,1000,common_ini_file);
		x++;
	}

	SetCursor(LoadCursor(NULL,IDC_ARROW));
}

int PickFileToAdd(char *buf,HWND parent)
{
	OPENFILENAME ofn;
	char *filterstr = " Text files\x00*.txt\x00 all files\x00*.*\x00\x00\x00\x00";
	char *title = "Add a name symbol file";
	char filename[1024];

	filename[0] = 0;

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = parent;
	ofn.hInstance = hInst;
	ofn.lpstrFilter = filterstr;
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = filename;
	ofn.nMaxFile = 1000;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = title;
#ifdef WIN95
	ofn.Flags = OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
#else
	ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
#endif
	ofn.lpstrDefExt = NULL;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lCustData = 0;
	ofn.lpfnHook = 0;
	ofn.lpTemplateName = NULL;

	if (GetOpenFileName(&ofn)) {
		strcpy(buf,filename);
		return 1;
	}

	return 0;
}

BOOL CALLBACK NamingConfigDlg(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	char paths[1024];
	int x;
	char buf[64];

	if (msg == WM_INITDIALOG) {
		x=0;
		sprintf(buf,"NameFile%d",x);
		GetPrivateProfileString("Naming",buf,"",paths,1000,common_ini_file);
		x++;
		while (paths[0]) {
			SendDlgItemMessage(hwnd,IDC_FILELIST,LB_ADDSTRING,0,(LPARAM)((DWORD)paths));
			sprintf(buf,"NameFile%d",x);
			GetPrivateProfileString("Naming",buf,"",paths,1000,common_ini_file);
			x++;
		}

		return TRUE;
	}
	else if (msg == WM_COMMAND) {
		if (wparam == IDOK) {
			x=0;
			sprintf(buf,"NameFile%d",x);
			paths[0] = 0; SendDlgItemMessage(hwnd,IDC_FILELIST,LB_GETTEXT,x,(LPARAM)((DWORD)paths));
			WritePrivateProfileString("Naming",buf,paths,common_ini_file);
			x++;
			while (paths[0]) {
				sprintf(buf,"NameFile%d",x);
				paths[0] = 0; SendDlgItemMessage(hwnd,IDC_FILELIST,LB_GETTEXT,x,(LPARAM)((DWORD)paths));
				WritePrivateProfileString("Naming",buf,paths,common_ini_file);
				x++;
			}

			LoadSavedNaming();

			EndDialog(hwnd,0);
		}
		if (wparam == IDADDFILE) {
			if (PickFileToAdd(paths,hwnd)) {
				SendDlgItemMessage(hwnd,IDC_FILELIST,LB_ADDSTRING,0,(LPARAM)((DWORD)paths));
			}
		}
		if (wparam == IDREMOVE) {
			x = (int)SendDlgItemMessage(hwnd,IDC_FILELIST,LB_GETCURSEL,0,0L);
			if (x >= 0) SendDlgItemMessage(hwnd,IDC_FILELIST,LB_DELETESTRING,x,0L);
		}
	}

	return FALSE;
}

void NamingConfig()
{
	DialogBox(hInst,MAKEINTRESOURCE(IDD_NAMINGCONFIG),hwndMain,NamingConfigDlg);
	SetFocus(hwndMain);
}

BOOL CALLBACK NamingVarsConfigDlg(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	char buf[512];
	int i;

	if (msg == WM_INITDIALOG) {
		SendMessage(hwnd,WM_USER+600,0,0L);
		return TRUE;
	}
	else if (msg == WM_USER+600) {
		SendDlgItemMessage(hwnd,IDC_VARLIST,LB_RESETCONTENT,0,(LPARAM)((DWORD)buf));
		i=0;
		while (name_assoc_var[i].inuse) {
			sprintf(buf,"%s=0x%X",name_assoc_var[i].varname,name_assoc_var[i].data);
			SendDlgItemMessage(hwnd,IDC_VARLIST,LB_ADDSTRING,0,(LPARAM)((DWORD)buf));
			i++;
		}
	}
	else if (msg == WM_COMMAND) {
		if (wparam == IDOK) {
			EndDialog(hwnd,0);
		}
		if (wparam == IDDELVAR) {
			i=SendDlgItemMessage(hwnd,IDC_VARLIST,LB_GETCURSEL,0,0L);
			if (i >= 0) {
				DeleteNamingVar(name_assoc_var[i].varname);
				LoadSavedNaming();
				SendDlgItemMessage(hwnd,IDC_VARLIST,LB_DELETESTRING,(WPARAM)i,0L);
			}
		}
		if (wparam == IDCLEARALL) {
			ClearVarNames();
			LoadSavedNaming();
			SendMessage(hwnd,WM_USER+600,0,0L);
		}
	}

	return FALSE;
}

void NamingVarsConfig()
{
	DialogBox(hInst,MAKEINTRESOURCE(IDD_NAMINGSYMVARS),hwndMain,NamingVarsConfigDlg);
	SetFocus(hwndMain);
}

int AddMemNamingAssoc(int type,char *name,DWORD segb,DWORD sege,DWORD ofsb,DWORD ofse,char seg_con)
{
	char *ptr;
	int l;

	l=strlen(name);
	if (l > 512) l = 512;
	ptr = malloc(l+1);
	if (!ptr) return 0;
	memcpy(ptr,name,l+1);

	while (name_assoc_mem[name_assoc_mem_next_alloc].inuse && name_assoc_mem_next_alloc < NAMEASSOCMEMMAX) name_assoc_mem_next_alloc++;

	if (name_assoc_mem_next_alloc >= NAMEASSOCMEMMAX) {
		name_assoc_mem_next_alloc = 0;
		while (name_assoc_mem[name_assoc_mem_next_alloc].inuse && name_assoc_mem_next_alloc < NAMEASSOCMEMMAX) name_assoc_mem_next_alloc++;
	}

	if (name_assoc_mem_next_alloc >= NAMEASSOCMEMMAX) {
		return 0;
	}

	name_assoc_mem[name_assoc_mem_next_alloc].inuse=1;
	name_assoc_mem[name_assoc_mem_next_alloc].var_string=0;
	name_assoc_mem[name_assoc_mem_next_alloc].enabled=1;
	name_assoc_mem[name_assoc_mem_next_alloc].name=ptr;
	name_assoc_mem[name_assoc_mem_next_alloc].ofs_begin=ofsb;
	name_assoc_mem[name_assoc_mem_next_alloc].ofs_end=ofse;
	name_assoc_mem[name_assoc_mem_next_alloc].seg_begin=segb;
	name_assoc_mem[name_assoc_mem_next_alloc].seg_end=sege;
	name_assoc_mem[name_assoc_mem_next_alloc].seg_constrain=seg_con;
	name_assoc_mem[name_assoc_mem_next_alloc].type=type;
	name_assoc_mem_itm_added = name_assoc_mem_next_alloc;

	while (name_assoc_mem[name_assoc_mem_next_alloc].inuse && name_assoc_mem_next_alloc < NAMEASSOCMEMMAX) name_assoc_mem_next_alloc++;

	if (name_assoc_mem_next_alloc >= NAMEASSOCMEMMAX) {
		name_assoc_mem_next_alloc = 0;
		while (name_assoc_mem[name_assoc_mem_next_alloc].inuse && name_assoc_mem_next_alloc < NAMEASSOCMEMMAX) name_assoc_mem_next_alloc++;
	}

	return 1;
}

int GetNamingMemAssocInfo(DWORD seg,DWORD ofs,char *buf,int bufl,int *type)
{
	int x,f,sf,a1,a2,a;

	if (!naming_enabled) return 0;

	f=0;
	x=0;
	name_assoc_mem_next_l=0;
	name_assoc_mem_next_sz=0;
	name_assoc_mem_next_szsub=0;
	name_assoc_mem_should_be_ofs=ofs;
	name_assoc_mem_should_be_seg=seg;
	while (x < NAMEASSOCMEMMAX && name_assoc_mem[x].inuse && !f) {
		if (name_assoc_mem[x].enabled) {
			sf=0;

			if (name_assoc_mem[x].seg_constrain) {
				sf = (seg >= name_assoc_mem[x].seg_begin && seg <= name_assoc_mem[x].seg_end);
				a = ofs;
				a1 = name_assoc_mem[x].ofs_begin;
				a2 = name_assoc_mem[x].ofs_end;
			}
			else {
				a1 = (name_assoc_mem[x].seg_begin<<4) + name_assoc_mem[x].ofs_begin;
				a2 = (name_assoc_mem[x].seg_end<<4) + name_assoc_mem[x].ofs_end;
				a = (seg << 4) + ofs;
				sf = 1;
			}

			if (name_assoc_mem[x].type == NAS_PROC || name_assoc_mem[x].type == NAS_LABEL) {
				name_assoc_mem_next_l = a == a2;

				if (sf)	f = (a == a1 || name_assoc_mem_next_l);
				else	f = 0;

				if (name_assoc_mem_next_l) name_assoc_mem_next_l = f;

				name_assoc_mem_next_sz = (a2-a1)+1;
				name_assoc_mem_next_szsub = 0;
				name_assoc_mem_should_be_ofs = a1 + name_assoc_mem_next_sz;
			}
			else if (name_assoc_mem[x].type == NAS_MEMVAR || name_assoc_mem[x].type == NAS_NOTES) {
				if (sf)	f = (a >= a1 && a <= a2);
				else	f = 0;

				name_assoc_mem_next_sz = (a2-a1)+1;
				name_assoc_mem_next_szsub = a-a1;
				if (name_assoc_mem[x].seg_constrain) {
					name_assoc_mem_should_be_ofs = name_assoc_mem[x].ofs_end+1;
					name_assoc_mem_should_be_seg = name_assoc_mem[x].seg_end;
				}
				else {
					name_assoc_mem_should_be_ofs = a1 - (seg<<4);
				}
			}

			if (name_assoc_mem_search_typeconst >= 0) {
				if (name_assoc_mem_search_typeconst != name_assoc_mem[x].type) f=0;
			}
			if (name_assoc_mem_search_no_notes) {
				if (name_assoc_mem[x].type == NAS_NOTES) f=0;
			}
		}

		if (!f) x++;
	}

	if (!f) {
		name_assoc_mem_itm_idx = -1;
		return 0;
	}

	strncpy(buf,name_assoc_mem[x].name,bufl);
	buf[bufl] = 0;
	name_assoc_mem_itm_idx = x;

	if (type) *type = name_assoc_mem[x].type;

	return 1;
}

int GetSkippedItems(DWORD sego,DWORD ofso,DWORD segn,DWORD ofsn,DWORD *newseg,DWORD *newofs)
{
	int x,f,sf,a1,a2,aa,ab,ans,ano;

	if (!naming_enabled) return 0;
	if (sego == segn && ofso == ofsn) return 0;

	f=0;
	x=0;
	name_assoc_mem_next_l=0;
	name_assoc_mem_next_sz=0;
	name_assoc_mem_next_szsub=0;
	while (x < NAMEASSOCMEMMAX && name_assoc_mem[x].inuse && !f) {
		if (name_assoc_mem[x].enabled) {
			sf=0;

			if (name_assoc_mem[x].seg_constrain) {
				sf = (sego >= name_assoc_mem[x].seg_begin && sego <= name_assoc_mem[x].seg_end) && (segn >= name_assoc_mem[x].seg_begin && segn <= name_assoc_mem[x].seg_end);
				aa = ofso;
				ab = ofsn;
				a1 = name_assoc_mem[x].ofs_begin;
				a2 = name_assoc_mem[x].ofs_end;
				ans = name_assoc_mem[x].seg_begin;
				ano = a1;
			}
			else {
				a1 = (name_assoc_mem[x].seg_begin<<4) + name_assoc_mem[x].ofs_begin;
				a2 = (name_assoc_mem[x].seg_end<<4) + name_assoc_mem[x].ofs_end;
				aa = (sego << 4) + ofso;
				ab = (segn << 4) + ofsn;
				sf = 1;
				ans = segn;
				ano = a1 - (segn<<4);
			}

			if (sf)	f = (aa < a1 && ab > a2);
			else	f = 0;
		}

		if (!f) x++;
	}

	if (!f) return 0;

	if (newofs) *newofs = ano;
	if (newseg) *newseg = ans;

	return 1;
}

int GetNamingMemAssocStrPreIns(DWORD seg,DWORD ofs,char *buf,int bufl)
{
	int tp,r,qut;
	char c;
	char tbuf[1024];
	char *ptr;

	name_assoc_mem_search_no_notes = 1;
	r=GetNamingMemAssocInfo(seg,ofs,tbuf,1023,&tp);
	name_assoc_mem_search_no_notes = 0;

	if (!r) return 0;
	if (!naming_enabled) return 0;

	if (tp == NAS_PROC)	{
		if (name_assoc_mem_next_l && name_assoc_mem_next_sz > 1)		return 0;
		else															sprintf(buf,"%s proc",tbuf);
	}
	else if (tp == NAS_LABEL) {
		sprintf(buf,"%s:",tbuf);
	}
	else if (tp == NAS_MEMVAR) {
		if (name_assoc_mem_next_l && name_assoc_mem_next_sz > 1) return 0;

		BreakpointTriggerCease(1);

		if (name_assoc_mem[name_assoc_mem_itm_idx].var_string) {
			sprintf(buf,"%04X:%04X  %-28s DB ",seg,ofs - name_assoc_mem_next_szsub,tbuf,name_assoc_mem_next_sz);
			ptr=strlen(buf)+buf;
			qut=0;
			bufl -= strlen(buf);
			for (r=0;r < name_assoc_mem_next_sz && bufl > 0;r++) {
				c = membytefarptr(seg,ofs); ofs++;

				if (c < 32 || c >= 128 || c == '\'' || c == '\"') {
					if (qut) {
						*ptr++ = '\'';
						*ptr++ = ',';
						qut=0;
					}
					else if (r > 0) {
						*ptr++ = ',';
					}

					sprintf(ptr,"0%02Xh",((unsigned)c)&0xFF);
					bufl -= strlen(ptr);
					ptr=strlen(ptr)+ptr;
				}
				else {
					if (!qut) {
						if (r > 0) *ptr++ = ',';
						*ptr++ = '\'';
						qut=1;
					}
					*ptr++ = c;
					bufl--;
				}
			}

			if (qut) {
				*ptr++ = '\'';
				*ptr++ = 0;
				qut=0;
			}
		}
		else if (name_assoc_mem[name_assoc_mem_itm_idx].pref_typesize > 0) {
			if (name_assoc_mem[name_assoc_mem_itm_idx].pref_typesize == 1) {
				sprintf(buf,"%04X:%04X  %-28s DB ",seg,ofs - name_assoc_mem_next_szsub,tbuf,name_assoc_mem_next_sz);
				ptr=strlen(buf)+buf;
				bufl -= strlen(buf);
				for (r=0;r < name_assoc_mem_next_sz && bufl > 0;r++) {
					if (r > 0) *ptr++ = ',';
					sprintf(ptr,"0%02Xh",membytefarptr(seg,ofs));
					ptr=strlen(ptr)+ptr;
					bufl -= strlen(ptr);
					ofs++;
				}
			}
			else if (name_assoc_mem[name_assoc_mem_itm_idx].pref_typesize == 2) {
				sprintf(buf,"%04X:%04X  %-28s DW ",seg,ofs - name_assoc_mem_next_szsub,tbuf,name_assoc_mem_next_sz);
				ptr=strlen(buf)+buf;
				bufl -= strlen(buf);
				for (r=0;r < name_assoc_mem_next_sz && bufl > 0;r += 2) {
					if (r > 0) *ptr++ = ',';
					sprintf(ptr,"0%04Xh",memwordfarptr(seg,ofs));
					ptr=strlen(ptr)+ptr;
					bufl -= strlen(ptr);
					ofs += 2;
				}
			}
			else if (name_assoc_mem[name_assoc_mem_itm_idx].pref_typesize == 4) {
				sprintf(buf,"%04X:%04X  %-28s DD ",seg,ofs - name_assoc_mem_next_szsub,tbuf,name_assoc_mem_next_sz);
				ptr=strlen(buf)+buf;
				bufl -= strlen(buf);
				for (r=0;r < name_assoc_mem_next_sz && bufl > 0;r += 4) {
					if (r > 0) *ptr++ = ',';
					sprintf(ptr,"0%08Xh",memdwordfarptr(seg,ofs));
					ptr=strlen(ptr)+ptr;
					bufl -= strlen(ptr);
					ofs += 4;
				}
			}
		}
		else {
			if (name_assoc_mem_next_sz == 1) {
				sprintf(buf,"%04X:%04X  %-28s DB 0%02Xh",seg,ofs - name_assoc_mem_next_szsub,tbuf,membytefarptr(seg,ofs));
			}
			else if (name_assoc_mem_next_sz == 2) {
				sprintf(buf,"%04X:%04X  %-28s DW 0%04Xh",seg,ofs - name_assoc_mem_next_szsub,tbuf,memwordfarptr(seg,ofs));
			}
			else if (name_assoc_mem_next_sz == 4) {
				sprintf(buf,"%04X:%04X  %-28s DD 0%08Xh",seg,ofs - name_assoc_mem_next_szsub,tbuf,memdwordfarptr(seg,ofs));
			}
			else {
				sprintf(buf,"%04X:%04X  %-28s DB ",seg,ofs - name_assoc_mem_next_szsub,tbuf,name_assoc_mem_next_sz);
				ptr=strlen(buf)+buf;
				for (r=0;r < name_assoc_mem_next_sz;r++) {
					if (r > 0) *ptr++ = ',';
					sprintf(ptr,"0%02Xh",membytefarptr(seg,ofs));
					ptr=strlen(ptr)+ptr;
					ofs++;
				}
			}
		}

		BreakpointTriggerCease(0);

		return 1 + (name_assoc_mem_next_sz - name_assoc_mem_next_szsub);
	}
	else if (tp == NAS_NOTES) {
		return 0;
	}

	return 1;
}

int GetNamingMemAssocStrPostIns(DWORD seg,DWORD ofs,char *buf,int bufl)
{
	int tp,r;
	char tbuf[1024];

	name_assoc_mem_search_typeconst = NAS_PROC;
	name_assoc_mem_search_no_notes = 1;
	r=GetNamingMemAssocInfo(seg,ofs,tbuf,1023,&tp);
	name_assoc_mem_search_no_notes = 0;
	name_assoc_mem_search_typeconst = -1;

	if (!r) return 0;
	if (!naming_enabled) return 0;

	if (tp == NAS_PROC)	{
		if (name_assoc_mem_next_l)		sprintf(buf,"%s endp",tbuf);
		else							return 0;
	}
	else if (tp == NAS_LABEL) {
		return 0;
	}
	else if (tp == NAS_MEMVAR) {
		return 0;
	}
	else if (tp == NAS_NOTES) {
		return 0;
	}

	return 1;
}

int GetNamingMemAssocStrComments(DWORD seg,DWORD ofs,char *buf,int bufl)
{
	int tp,r;
	char tbuf[1024];

	name_assoc_mem_search_typeconst = NAS_NOTES;
	name_assoc_mem_search_no_notes = 0;
	r=GetNamingMemAssocInfo(seg,ofs,tbuf,1023,&tp);
	name_assoc_mem_search_typeconst = -1;

	if (!r) return 0;
	if (!naming_enabled) return 0;

	if (tp == NAS_PROC)	{
		return 0;
	}
	else if (tp == NAS_LABEL) {
		return 0;
	}
	else if (tp == NAS_MEMVAR) {
		return 0;
	}
	else if (tp == NAS_NOTES) {
		strcpy(buf,tbuf);
		return 1;
	}

	return 1;
}

void AppendCallProcName(DWORD seg,DWORD ofs,char *buf)
{
	int typ,gat;
	char nbuf[1024];
	int x;

	name_assoc_mem_search_no_notes = 1;
	name_assoc_mem_search_typeconst = -1;
	gat=GetNamingMemAssocInfo(seg,ofs,nbuf,1023,&typ);
	name_assoc_mem_search_no_notes = 0;

	if (gat) {
		x=strlen(buf);
		sprintf(buf+x," (%s)",nbuf);
	}
}

int AddNamingVar(char *name,DWORD value)
{
	char *ptr;
	int l;

	l=strlen(name);
	if (l > 512) l = 512;
	ptr = malloc(l+1);
	if (!ptr) return 0;
	memcpy(ptr,name,l+1);

	while (name_assoc_var[name_assoc_var_next_alloc].inuse && name_assoc_var_next_alloc < NAMEASSOCVARMAX) name_assoc_var_next_alloc++;

	if (name_assoc_var_next_alloc >= NAMEASSOCVARMAX) {
		name_assoc_var_next_alloc = 0;
		while (name_assoc_var[name_assoc_var_next_alloc].inuse && name_assoc_var_next_alloc < NAMEASSOCVARMAX) name_assoc_var_next_alloc++;
	}

	if (name_assoc_var_next_alloc >= NAMEASSOCMEMMAX) {
		return 0;
	}

	name_assoc_var[name_assoc_var_next_alloc].inuse=1;
	name_assoc_var[name_assoc_var_next_alloc].data=value;
	name_assoc_var[name_assoc_var_next_alloc].varname=ptr;

	while (name_assoc_var[name_assoc_var_next_alloc].inuse && name_assoc_var_next_alloc < NAMEASSOCVARMAX) name_assoc_var_next_alloc++;

	if (name_assoc_var_next_alloc >= NAMEASSOCMEMMAX) {
		name_assoc_var_next_alloc = 0;
		while (name_assoc_var[name_assoc_var_next_alloc].inuse && name_assoc_var_next_alloc < NAMEASSOCVARMAX) name_assoc_var_next_alloc++;
	}

	return 1;
}

int GetNamingVar(char *name,DWORD *val)
{
	int x,f;

	if (!naming_enabled) return 0;

	f=0;
	x=0;
	while (x < NAMEASSOCVARMAX && name_assoc_var[x].inuse && !f) {
		f = !strcmpi(name,name_assoc_var[x].varname);
		if (!f) x++;
	}

	if (!f) return 0;

	if (val) *val = name_assoc_var[x].data;

	return 1;
}

int SetNamingVar(char *name,DWORD val)
{
	int x,f;

	if (!naming_enabled) return 0;

	f=0;
	x=0;
	while (x < NAMEASSOCVARMAX && name_assoc_var[x].inuse && !f) {
		f = !strcmpi(name,name_assoc_var[x].varname);
		if (!f) x++;
	}

	if (!f) return 0;

	name_assoc_var[x].data = val;

	return 1;
}

int DeleteNamingVar(char *name)
{
	int x,f;

	if (!naming_enabled) return 0;

	f=0;
	x=0;
	while (x < NAMEASSOCVARMAX && name_assoc_var[x].inuse && !f) {
		f = !strcmpi(name,name_assoc_var[x].varname);
		if (!f) x++;
	}

	if (!f) return 0;

	if (name_assoc_var[x].inuse) {
		if (name_assoc_var[x].varname) free(name_assoc_var[x].varname);
		name_assoc_var[x].varname=NULL;
		name_assoc_var[x].data=0;
	}

	while (name_assoc_var[x].inuse) {
		name_assoc_var_next_alloc = x;
		memcpy(&name_assoc_var[x],&name_assoc_var[x+1],sizeof(NAMEASSOCVAR));
		x++;
	}
	if (x < NAMEASSOCVARMAX) {
		memset(&name_assoc_var[x],0,sizeof(NAMEASSOCVAR));
	}

	return 1;
}

int AddNamingSeq(int opcode_len,int *opcodes,char *varexp)
{
	int l;

	name_assoc_seq_itm_added=-1;

	while (name_assoc_seq[name_assoc_seq_next_alloc].inuse && name_assoc_seq_next_alloc < NAMEASSOCSEQMAX) name_assoc_seq_next_alloc++;

	if (name_assoc_seq_next_alloc >= NAMEASSOCSEQMAX) {
		name_assoc_seq_next_alloc = 0;
		while (name_assoc_seq[name_assoc_seq_next_alloc].inuse && name_assoc_seq_next_alloc < NAMEASSOCSEQMAX) name_assoc_seq_next_alloc++;
	}

	if (name_assoc_seq_next_alloc >= NAMEASSOCSEQMAX) {
		return 0;
	}

	name_assoc_seq[name_assoc_seq_next_alloc].inuse=1;
	for (l=0;l < opcode_len;l++) name_assoc_seq[name_assoc_seq_next_alloc].opcode[l] = opcodes[l];
	name_assoc_seq[name_assoc_seq_next_alloc].opcode_seq_length=opcode_len;
	strcpy(name_assoc_seq[name_assoc_seq_next_alloc].varstatement,varexp);
	name_assoc_seq_itm_added = name_assoc_seq_next_alloc;

	while (name_assoc_seq[name_assoc_seq_next_alloc].inuse && name_assoc_seq_next_alloc < NAMEASSOCSEQMAX) name_assoc_seq_next_alloc++;

	if (name_assoc_seq_next_alloc >= NAMEASSOCSEQMAX) {
		name_assoc_seq_next_alloc = 0;
		while (name_assoc_seq[name_assoc_seq_next_alloc].inuse && name_assoc_seq_next_alloc < NAMEASSOCSEQMAX) name_assoc_seq_next_alloc++;
	}

	return 1;
}

void ClearSeqMatches()
{
	name_assoc_seq_matches_cnt = 0;
}

void MatchNamingSeq(BYTE b,DWORD cs,DWORD ip)
{
	int x,f;

	x=0;
	f=0;
	while (name_assoc_seq[x].inuse && !f) {
		if (name_assoc_seq[x].opcode_match_idx < name_assoc_seq[x].opcode_seq_length) {
			if (cs < name_assoc_seq[x].seg_begin || cs > name_assoc_seq[x].seg_end || ip < name_assoc_seq[x].ofs_begin || ip > name_assoc_seq[x].ofs_end) {
				name_assoc_seq[x].opcode_match_idx=0;
			}
			else if (name_assoc_seq[x].opcode[name_assoc_seq[x].opcode_match_idx] == -1) {
				name_assoc_seq[x].opcode_match_idx++;
			}
			else if (b == ((BYTE)name_assoc_seq[x].opcode[name_assoc_seq[x].opcode_match_idx])) {
				name_assoc_seq[x].opcode_match_idx++;
			}
			else {
				name_assoc_seq[x].opcode_match_idx=0;
			}

			if (name_assoc_seq[x].opcode_match_idx >= name_assoc_seq[x].opcode_seq_length) f=1;
		}

		if (!f) x++;
	}

	if (!f) return;

	name_assoc_seq_matches[name_assoc_seq_matches_cnt] = x;
	name_assoc_seq_matches_cnt++;

	return;
}

// expression decoder

// decodes only what it is given up to the next NULL or closing parenthesis
#define OPP_IGNORE		-1
#define OPP_ADD			0
#define OPP_SUB			1
#define OPP_MUL			2
#define OPP_DIV			3
DWORD DecodeExprToIntParenthesis(char **str)
{
	DWORD val,vtmp;
	int opp;
	DWORD dd;
	int x;
	char nm[128];
	char *ptr;

	ptr = *str;
	val = 0;
	opp = OPP_ADD;

	while (*ptr && *ptr != ')') {
		while (*ptr == 32) *ptr++;		// skip past white-spaces
		
/* opening parenthesis? */
		vtmp = 0;
		if (*ptr == '(') {
			*ptr++;
			vtmp = DecodeExprToIntParenthesis(&ptr);
			if (*ptr == ')') ptr++;
		}
		else if (isdigit(*ptr)) {
			vtmp = strtoul(ptr,&ptr,0);
		}
		else {
			x=0;
			while (!(*ptr == ')' || *ptr == '+' || *ptr == '-' || *ptr == '*' || *ptr == '/' || *ptr == 32 || *ptr == 0)) nm[x++] = *ptr++;
			nm[x]=0;

			if (GetNamingVar(nm,&dd)) {
				vtmp = dd;
			}
			else {
				if (!strnicmp(nm,"CS",2))		vtmp = ireg_cs;
				if (!strnicmp(nm,"IP",2))		vtmp = ireg_eip;
				if (!strnicmp(nm,"DS",2))		vtmp = ireg_ds;
				if (!strnicmp(nm,"ES",2))		vtmp = ireg_es;
				if (!strnicmp(nm,"SS",2))		vtmp = ireg_ss;
				if (!strnicmp(nm,"AX",2))		vtmp = ireg_eax;
				if (!strnicmp(nm,"BX",2))		vtmp = ireg_ebx;
				if (!strnicmp(nm,"CX",2))		vtmp = ireg_ecx;
				if (!strnicmp(nm,"DX",2))		vtmp = ireg_edx;
				if (!strnicmp(nm,"SI",2))		vtmp = ireg_esi;
				if (!strnicmp(nm,"DI",2))		vtmp = ireg_edi;
			}
		}

		if (opp == OPP_ADD)		val += vtmp;
		if (opp == OPP_SUB)		val -= vtmp;
		if (opp == OPP_MUL)		val *= vtmp;
		if (opp == OPP_DIV)		val /= vtmp;

		while (*ptr == 32) *ptr++;		// skip past white-spaces
		if (*ptr == '+') {
			ptr++;
			opp = OPP_ADD;
		}
		else if (*ptr == '-') {
			ptr++;
			opp = OPP_SUB;
		}
		else if (*ptr == '*') {
			ptr++;
			opp = OPP_MUL;
		}
		else if (*ptr == '/') {
			ptr++;
			opp = OPP_DIV;
		}
		else {
			opp = OPP_IGNORE;
		}
	}

	*str = ptr;
	return val;
}

DWORD DecodeExprToInt(char *str)
{
	DWORD val;

	val = DecodeExprToIntParenthesis(&str);
	return val;
}

void HandleSeqMatches()
{
	int x,f,i,delv,xx;
	char nm[512];
	char *ptr,*ptrn,*ptt;
	DWORD dd;

	f=0;
	for (x=0;x < name_assoc_seq_matches_cnt;x++) {
		i = name_assoc_seq_matches[x];
		ptrn=name_assoc_seq[i].varstatement;
		ptr=strstr(ptrn,"=");

		while (ptr) {
			*ptr = 0;
			delv=0;
			ptr++;
			ptt = strstr(ptr,",");

			if (ptt) {
				xx = (((int)ptt)-((int)ptr));
				strncpy(nm,ptr,(size_t)xx);
				nm[xx]=0;
			}
			else {
				strcpy(nm,ptr);
			}

/* match either variables, constants, or numbers */
			dd=0;
			if (nm[0] == 0 || nm[0] == 32) {	// assign to nothing?
				delv=1;							// delete it
			}
			else {
				dd=DecodeExprToInt(nm);
			}

			if (delv)								DeleteNamingVar(ptrn);
			else if (GetNamingVar(ptrn,NULL))		SetNamingVar(ptrn,dd);
			else									AddNamingVar(ptrn,dd);
			f=1;

// look for more params
			ptr = ptt;
			if (ptr) {
				ptrn=ptr+1;
				ptr=strstr(ptrn,"=");
			}
		}
	}

	if (f) LoadSavedNaming();
}
