// floppy drive emulator

#include "global.h"
#include <stdio.h>
#include "hdsim.h"

harddiskdrv *harddiskdrv_alloc()
{
	harddiskdrv *tmp;

	tmp = malloc(sizeof(harddiskdrv));
	if (!tmp) return NULL;

	tmp->file=NULL;
	tmp->diskindrive=FALSE;
	tmp->heads = 0;
	tmp->tracks = 0;
	tmp->sectors = 0;

	return tmp;
}

void harddiskdrv_free(harddiskdrv *drv)
{
	if (!drv) return;

	if (drv->file) fclose(drv->file);
	free(drv);
}

BOOL harddiskdrv_selectfile(harddiskdrv *drv,char *name)
{
	char tmp[256];
	int i;

	if (!drv) return FALSE;

	if (drv->file) fclose(drv->file);

	drv->heads = 0;
	drv->tracks = 0;
	drv->sectors = 0;

	drv->diskindrive=FALSE;
	if (name != NULL)	drv->file = fopen(name,"rb+");
	else				drv->file = NULL;
	if (!drv->file) {
		return FALSE;
	}

	fseek(drv->file,0,SEEK_END);
	drv->filelen = ftell(drv->file);
	fseek(drv->file,0,SEEK_SET);

// 10-1-2k: read hard disk parameters from separate INI file
	strcpy(tmp,name);
	i=strlen(tmp)-1;
	while (i > 0 && tmp[i] != '.') {
		tmp[i]=0;
		i--;
	}
	strcat(tmp,"ini");

	drv->heads = GetPrivateProfileInt("Geometry","heads",0,tmp);
	drv->tracks = GetPrivateProfileInt("Geometry","tracks",0,tmp);
	drv->sectors = GetPrivateProfileInt("Geometry","sectors",0,tmp);

	if (drv->heads && drv->tracks && drv->sectors) {
// accept what the INI tells us
	}
	else if (drv->filelen == 368640) {
		if (MessageBox(NULL,"Is the virtual floppy double-sided?","Need to know",MB_YESNO) == IDYES) {
			drv->heads = 2;
			drv->tracks = 40;
			drv->sectors = 9;
		}
		else {
			drv->heads = 1;
			drv->tracks = 80;
			drv->sectors = 9;
		}
	}
	else if (drv->filelen == 1228800) {
		drv->heads = 2;
		drv->tracks = 80;
		drv->sectors = 15;
	}
	else if (drv->filelen == 737280) {
		drv->heads = 2;
		drv->tracks = 80;
		drv->sectors = 9;
	}
	else if (drv->filelen == 1474560) {
		drv->heads = 2;
		drv->tracks = 80;
		drv->sectors = 18;
	}
	else if (drv->filelen == 2949120) {
		drv->heads = 2;
		drv->tracks = 80;
		drv->sectors = 36;
	}
	else {
		drv->heads = 1;
		drv->tracks = drv->filelen/512;
		drv->sectors = 1;
	}

	drv->diskindrive=TRUE;

	return TRUE;
}

int harddiskdrv_read(harddiskdrv *drv,int sector,int head,int track,int *num,void *buffer)
{
	int fo,sn;
	char *dataptr = (char*)buffer;

	if (!drv) return 0xFF;
	if (!drv->diskindrive) return 0xFF;

	sector--;

	if (head < 0 || head >= drv->heads)
		{ return 2; *num=0; }
	if (track < 0 || track >= drv->tracks)
		{ return 2; *num=0; }
	if (sector < 0 || sector >= drv->sectors)
		{ return 2; *num=0; }
	if (*num <= 0)
		{ return 0; *num=0; }

	if (*num > (drv->sectors - sector)) *num = drv->sectors - sector;

	fo = track;
	fo *= drv->heads;
	fo += head;
	fo *= drv->sectors;
	fo += sector;
	fseek(drv->file,fo * 512,SEEK_SET);

	sn = *num;
	while (sector < drv->sectors && sn > 0) {
		fread(dataptr,512,1,drv->file);
		dataptr += 512;
		sector++;
		sn--;
	}
	*num -= sn;

	return 0;		// 0 = sucess!
}

int harddiskdrv_write(harddiskdrv *drv,int sector,int head,int track,int *num,void *buffer)
{
	int fo,sn;
	char *dataptr = (char*)buffer;

	if (!drv) return 0xFF;
	if (!drv->diskindrive) return 0xFF;

	sector--;

	if (head < 0 || head >= drv->heads)
		{ return 2; *num=0; }
	if (track < 0 || track >= drv->tracks)
		{ return 2; *num=0; }
	if (sector < 0 || sector >= drv->sectors)
		{ return 2; *num=0; }
	if (*num <= 0)
		{ return 0; *num=0; }

	if (*num > (drv->sectors - sector)) *num = drv->sectors - sector;

	fo = track;
	fo *= drv->heads;
	fo += head;
	fo *= drv->sectors;
	fo += sector;
	fseek(drv->file,fo * 512,SEEK_SET);

	sn = *num;
	while (sector < drv->sectors && sn > 0) {
		fwrite(dataptr,512,1,drv->file);
		dataptr += 512;
		sector++;
		sn--;
	}
	*num -= sn;

	return 0;		// 0 = sucess!
}
