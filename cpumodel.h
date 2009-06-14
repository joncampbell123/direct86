
typedef struct {
	BOOL			lastbyte;						// last byte in instruction opcode sequence
	int				(*execute)(BYTE b);				// function to call to execute opcode
	int				(*decode)(BYTE b,char *buf);	// function to call to decode opcode
	void*			ref_table;						// model table to refer to if this opcode is recognized
} MODELOPCODE;

MODELOPCODE mdl_cpu_8086[256];

int mdl_exec_default(BYTE b);
int mdl_dec_default(BYTE b,char *buf);

int model_execute_opcode();
int model_decode_opcode(char *buffer);
void set_cpu_model(MODELOPCODE *model);

extern char				segment_prefix_dec[32];
