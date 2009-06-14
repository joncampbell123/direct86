// fdsim.c header

typedef struct {
	BOOL			diskindrive;
	FILE			*file;
	int				filelen;
// drive parms
	int				heads,sectors,tracks;
	int				changeline;
} floppydrv;

floppydrv *floppydrv_alloc();
void floppydrv_free(floppydrv *drv);
BOOL floppydrv_selectfile(floppydrv *drv,char*);
int floppydrv_read(floppydrv *drv,int sector,int head,int track,int *num,void *buffer);
int floppydrv_write(floppydrv *drv,int sector,int head,int track,int *num,void *buffer);
