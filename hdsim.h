// fdsim.cpp header

typedef struct {
	BOOL			diskindrive;
	FILE			*file;
	int				filelen;
// drive parms
	int				heads,sectors,tracks;
} harddiskdrv;

harddiskdrv *harddiskdrv_alloc();
void harddiskdrv_free(harddiskdrv *drv);
BOOL harddiskdrv_selectfile(harddiskdrv *drv,char *name);
int harddiskdrv_read(harddiskdrv *drv,int,int,int,int*,void*);
int harddiskdrv_write(harddiskdrv *drv,int,int,int,int*,void*);
