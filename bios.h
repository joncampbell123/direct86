
extern BYTE*			ROM;
extern floppydrv*		bios_drive_a;
extern floppydrv*		bios_drive_b;
extern harddiskdrv*		bios_drive_c;
extern harddiskdrv*		bios_drive_d;

void ResetBIOS();
void InitBIOS();
void CloseBIOS();
void PowerDownBIOS();
void PowerOnBIOS();
int IsBIOSPowered();

void BIOS_Int10();
void BIOS_Int11();
void BIOS_Int13();
void BIOS_Int14();
void BIOS_Int15();
void BIOS_Int16();
void BIOS_Int17();
void BIOS_Int1A();
void ConfigureBIOS();
