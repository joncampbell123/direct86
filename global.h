// global stuff

#define LOINT16(d)	((short int)LOWORD(d))
#define HIINT16(d)	((short int)HIWORD(d))

#include <windows.h>

#define int32			int
#define uint32			unsigned int
#define int16			short int
#define uint16			unsigned short int

#ifndef WIN32

#define DWORD			uint32
#define WORD			uint16
#define BYTE			unsigned char
#define BOOL			BYTE

#endif
