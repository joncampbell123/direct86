
void ExecuteMODREGRM(void(*callfunc)(void*,void*,int));
void ExecuteMODREGRMBT(void(*callfunc)(void*,void*,int,int));
void ExecuteMODMOVZX(void(*callfunc)(void*,void*,int,int));
void Execute386MODREGRM1(void(*callfunc)(void*,void*,int));
void ExecuteMODREGRMSHXD(void(*callfunc)(void*,void*,int,BYTE));
void ExecuteMODSREGRM(void(*callfunc)(void*,void*,int));
void ExecuteMODREGRMXCHG(void(*callfunc)(void*,void*,int));
void ExecuteMODS1RM(void(*callfunc)(void*,void*,int));
void ExecuteMODS1RMRDONLY(void(*callfunc)(void*,void*,int));
void ExecuteMODS1RMD(void(*callfunc)(void*,void*,int));
void ExecuteMODS1RMDRDONLY(void(*callfunc)(void*,void*,int));
void ExecuteMODS1RMF(void(*callfunc)(void*,void*,int));
void ExecuteMODLDS(void(*callfunc)(void*,void*,int));
void ExecuteMODLEA(void(*callfunc)(void*,void*,int));
void ExecuteMODSHII(void(*callfunc)(void*,void*,int));
void ExecuteMOD010RM(void(*callfunc)(void*,void*,int));
DWORD RMEAddress(int fl_rm,DWORD ofs);
DWORD RMAddressbfar(int fl_rm,int ofs);
DWORD RMAddressbfar32(int fl_rm,int ofs);
DWORD RMAddresswfar(int fl_rm,int ofs);
DWORD RMAddresswfar32(int fl_rm,int ofs);
DWORD RMAddressdfar(int fl_rm,int ofs);
DWORD RMAddressdfar32(int fl_rm,int ofs);
void RMAddressbfarw(int fl_rm,int ofs,DWORD d);
void RMAddressbfarw32(int fl_rm,int ofs,DWORD d);
void RMAddresswfarw(int fl_rm,int ofs,DWORD w);
void RMAddresswfarw32(int fl_rm,int ofs,DWORD w);
void RMAddressdfarw(int fl_rm,int ofs,DWORD d);
void RMAddressdfarw32(int fl_rm,int ofs,DWORD d);

#define RMAddressbfare RMAddressbfar
#define RMAddresswfare RMAddresswfar
#define RMAddressdfare RMAddressdfar
#define RMAddressbfarew RMAddressbfarw
#define RMAddresswfarew RMAddresswfarw
#define RMAddressdfarew RMAddressdfarw
