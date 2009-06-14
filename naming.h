
/* used for storing proc, label, etc. attributes to an address */
typedef struct {
	char	inuse;			/* 1=entry is valid 0=entry invalid */
	char	enabled;		/* 1=enabled 0=disabled. Duh. */
	int		type;			/* 0=procedure, 1=label, 2=variable, 3=annotation */
	char*	name;			/* string containing "name" to associate (or note if type NAS_NOTES) */
	DWORD	seg_begin;		/* beginning of segment range to associate with */
	DWORD	seg_end;		/* end of segment range to associate with */
	DWORD	ofs_begin;		/* beginning of offset range (within segment) to associate with */
	DWORD	ofs_end;		/* end of offset range (within segment) to associate with */
	char	seg_constrain;	/* is set, entry applies ONLY to variables with segments between seg_begin and seg_end. If clear, absolute addresses are used in determining relationship. */
	char	var_string;		/* 0=display as byte sequence, 1=display as ASCII string */
	int		pref_typesize;	/* 0=default 1=byte sequence, 2=word sequence, 4=dword sequence */
} NAMEASSOCMEM;

/* used for storing "variables" in naming symbols */
typedef struct {
	char	inuse;			/* 1=entry is valid, 0=entry invalid */
	char*	varname;		/* variable name */
	DWORD	data;			/* variable value */
} NAMEASSOCVAR;

/* code sequence recognition */
typedef struct {
	char	inuse;			/* what do YOU think? */
	int		opcode[256];	/* -1 for unknown or 0...255 for opcode sequence */
	int		opcode_seq_length;
	int		opcode_match_idx;
	char	varstatement[512]; /* expression string [varname]=[value] */
	DWORD	ofs_begin,ofs_end;
	DWORD	seg_begin,seg_end;
} NAMEASSOCSEQUENCE;

#define NAS_PROC			0
#define NAS_LABEL			1
#define NAS_MEMVAR			2
#define NAS_NOTES			3

#define NAMEASSOCMEMMAX		4096
#define NAMEASSOCVARMAX		4096
#define NAMEASSOCSEQMAX		4096

extern NAMEASSOCMEM			name_assoc_mem[NAMEASSOCMEMMAX];
extern NAMEASSOCVAR			name_assoc_var[NAMEASSOCVARMAX];
extern NAMEASSOCSEQUENCE	name_assoc_seq[NAMEASSOCSEQMAX];
extern int					name_assoc_mem_next_alloc;
extern int					name_assoc_var_next_alloc;
extern int					name_assoc_seq_next_alloc;
extern int					name_assoc_seq_matches[32];
extern int					name_assoc_seq_matches_cnt;
extern int					naming_enabled;

void InitNaming();
void CloseNaming();
void LoadSavedNaming();

void AppendCallProcName(DWORD seg,DWORD ofs,char *buf);
int GetSkippedItems(DWORD sego,DWORD ofso,DWORD segn,DWORD ofsn,DWORD *newseg,DWORD *newofs);

int AddMemNamingAssoc(int type,char *name,DWORD segb,DWORD sege,DWORD ofsb,DWORD ofse,char seg_con);
int GetNamingMemAssocInfo(DWORD seg,DWORD ofs,char *buf,int bufl,int *type);
int GetNamingMemAssocStrPreIns(DWORD seg,DWORD ofs,char *buf,int bufl);
int GetNamingMemAssocStrComments(DWORD seg,DWORD ofs,char *buf,int bufl);
int GetNamingMemAssocStrPostIns(DWORD seg,DWORD ofs,char *buf,int bufl);

void NamingConfig();
void NamingVarsConfig();

void ClearNaming();
void ClearVarNames();

int AddNamingVar(char *name,DWORD value);
int GetNamingVar(char *name,DWORD *val);
int SetNamingVar(char *name,DWORD val);
int DeleteNamingVar(char *name);

int AddNamingSeq(int opcode_len,int *opcodes,char *varexp);
void MatchNamingSeq(BYTE b,DWORD cs,DWORD ip);
void ClearSeqMatches();
void HandleSeqMatches();
