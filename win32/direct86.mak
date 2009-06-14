# Microsoft Visual C++ Generated NMAKE File, Format Version 2.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=Win32 Debug
!MESSAGE No configuration specified.  Defaulting to Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Win32 Release" && "$(CFG)" != "Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "direct86.mak" CFG="Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

################################################################################
# Begin Project
MTL=MkTypLib.exe
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WinRel"
# PROP BASE Intermediate_Dir "WinRel"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "WinRel"
# PROP Intermediate_Dir "WinRel"
OUTDIR=.\WinRel
INTDIR=.\WinRel

ALL : \JMC\Winprogs\DIRECT86\D86Win32.exe $(OUTDIR)/direct86.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE CPP /nologo /ML /W3 /GX /YX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /c
# ADD CPP /nologo /ML /W3 /GX /YX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /c
CPP_PROJ=/nologo /ML /W3 /GX /YX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /FR$(INTDIR)/ /Fp$(OUTDIR)/"direct86.pch" /Fo$(INTDIR)/ /c 
CPP_OBJS=.\WinRel/
# ADD BASE RSC /l 0x80a /d "NDEBUG"
# ADD RSC /l 0x80a /d "NDEBUG"
RSC_PROJ=/l 0x80a /fo$(INTDIR)/"direct86.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"direct86.bsc" 
BSC32_SBRS= \
	$(INTDIR)/CPURGWIN.SBR \
	$(INTDIR)/8086.SBR \
	$(INTDIR)/VGA_PAL8.SBR \
	$(INTDIR)/Ram.sbr \
	$(INTDIR)/Bios.sbr \
	$(INTDIR)/VGA_TXT.SBR \
	$(INTDIR)/Stackops.sbr \
	$(INTDIR)/Brkpts.sbr \
	$(INTDIR)/Cpustkw.sbr \
	$(INTDIR)/TIMER.SBR \
	$(INTDIR)/FLAGOPS.SBR \
	$(INTDIR)/Keyboard.sbr \
	$(INTDIR)/Fdsim.sbr \
	$(INTDIR)/MEMWIN.SBR \
	$(INTDIR)/WCACHE.SBR \
	$(INTDIR)/Cpudec.sbr \
	$(INTDIR)/ADDRBUS.SBR \
	$(INTDIR)/LIB.SBR \
	$(INTDIR)/HARDWARE.SBR \
	$(INTDIR)/Cpuexec.sbr \
	$(INTDIR)/HDSIM.SBR \
	$(INTDIR)/Direct86.sbr \
	$(INTDIR)/Interrm.sbr \
	$(INTDIR)/WCACHEW.SBR \
	$(INTDIR)/Execops.sbr \
	$(INTDIR)/CPUTAIL.SBR \
	$(INTDIR)/Kbdwin.sbr \
	$(INTDIR)/Pic.sbr \
	$(INTDIR)/CMOS.SBR \
	$(INTDIR)/Execrm.sbr \
	$(INTDIR)/Stockvga.sbr \
	$(INTDIR)/Cpuqueue.sbr \
	$(INTDIR)/Mother.sbr \
	$(INTDIR)/VGA_NADA.SBR \
	$(INTDIR)/Cpumodel.sbr \
	$(INTDIR)/Naming.sbr

$(OUTDIR)/direct86.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /NOLOGO /SUBSYSTEM:windows /MACHINE:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /NOLOGO /SUBSYSTEM:windows /MACHINE:I386 /OUT:"..\D86Win32.exe"
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /NOLOGO\
 /SUBSYSTEM:windows /INCREMENTAL:no /PDB:$(OUTDIR)/"direct86.pdb" /MACHINE:I386\
 /OUT:"..\D86Win32.exe" 
DEF_FILE=
LINK32_OBJS= \
	$(INTDIR)/CPURGWIN.OBJ \
	$(INTDIR)/8086.OBJ \
	$(INTDIR)/VGA_PAL8.OBJ \
	$(INTDIR)/Ram.obj \
	$(INTDIR)/Bios.obj \
	$(INTDIR)/VGA_TXT.OBJ \
	$(INTDIR)/Stackops.obj \
	$(INTDIR)/Brkpts.obj \
	$(INTDIR)/Cpustkw.obj \
	$(INTDIR)/TIMER.OBJ \
	$(INTDIR)/FLAGOPS.OBJ \
	$(INTDIR)/Keyboard.obj \
	$(INTDIR)/Fdsim.obj \
	$(INTDIR)/MEMWIN.OBJ \
	$(INTDIR)/WCACHE.OBJ \
	$(INTDIR)/Cpudec.obj \
	$(INTDIR)/ADDRBUS.OBJ \
	$(INTDIR)/LIB.OBJ \
	$(INTDIR)/HARDWARE.OBJ \
	$(INTDIR)/Cpuexec.obj \
	$(INTDIR)/HDSIM.OBJ \
	$(INTDIR)/Direct86.obj \
	$(INTDIR)/Interrm.obj \
	$(INTDIR)/WCACHEW.OBJ \
	$(INTDIR)/Execops.obj \
	$(INTDIR)/CPUTAIL.OBJ \
	$(INTDIR)/Kbdwin.obj \
	$(INTDIR)/Pic.obj \
	$(INTDIR)/CMOS.OBJ \
	$(INTDIR)/Execrm.obj \
	$(INTDIR)/Stockvga.obj \
	$(INTDIR)/Cpuqueue.obj \
	$(INTDIR)/Mother.obj \
	$(INTDIR)/VGA_NADA.OBJ \
	$(INTDIR)/Cpumodel.obj \
	$(INTDIR)/Naming.obj \
	$(INTDIR)/direct86.res

\JMC\Winprogs\DIRECT86\D86Win32.exe : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WinDebug"
# PROP BASE Intermediate_Dir "WinDebug"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WinDebug"
# PROP Intermediate_Dir "WinDebug"
OUTDIR=.\WinDebug
INTDIR=.\WinDebug

ALL : \JMC\Winprogs\DIRECT86\D86Win32.exe $(OUTDIR)/direct86.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE CPP /nologo /ML /W3 /GX /Zi /YX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /c
# ADD CPP /nologo /ML /W3 /GX /Zi /YX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /c
CPP_PROJ=/nologo /ML /W3 /GX /Zi /YX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /FR$(INTDIR)/ /Fp$(OUTDIR)/"direct86.pch" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"direct86.pdb" /c 
CPP_OBJS=.\WinDebug/
# ADD BASE RSC /l 0x80a /d "_DEBUG"
# ADD RSC /l 0x80a /d "_DEBUG"
RSC_PROJ=/l 0x80a /fo$(INTDIR)/"direct86.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"direct86.bsc" 
BSC32_SBRS= \
	$(INTDIR)/CPURGWIN.SBR \
	$(INTDIR)/8086.SBR \
	$(INTDIR)/VGA_PAL8.SBR \
	$(INTDIR)/Ram.sbr \
	$(INTDIR)/Bios.sbr \
	$(INTDIR)/VGA_TXT.SBR \
	$(INTDIR)/Stackops.sbr \
	$(INTDIR)/Brkpts.sbr \
	$(INTDIR)/Cpustkw.sbr \
	$(INTDIR)/TIMER.SBR \
	$(INTDIR)/FLAGOPS.SBR \
	$(INTDIR)/Keyboard.sbr \
	$(INTDIR)/Fdsim.sbr \
	$(INTDIR)/MEMWIN.SBR \
	$(INTDIR)/WCACHE.SBR \
	$(INTDIR)/Cpudec.sbr \
	$(INTDIR)/ADDRBUS.SBR \
	$(INTDIR)/LIB.SBR \
	$(INTDIR)/HARDWARE.SBR \
	$(INTDIR)/Cpuexec.sbr \
	$(INTDIR)/HDSIM.SBR \
	$(INTDIR)/Direct86.sbr \
	$(INTDIR)/Interrm.sbr \
	$(INTDIR)/WCACHEW.SBR \
	$(INTDIR)/Execops.sbr \
	$(INTDIR)/CPUTAIL.SBR \
	$(INTDIR)/Kbdwin.sbr \
	$(INTDIR)/Pic.sbr \
	$(INTDIR)/CMOS.SBR \
	$(INTDIR)/Execrm.sbr \
	$(INTDIR)/Stockvga.sbr \
	$(INTDIR)/Cpuqueue.sbr \
	$(INTDIR)/Mother.sbr \
	$(INTDIR)/VGA_NADA.SBR \
	$(INTDIR)/Cpumodel.sbr \
	$(INTDIR)/Naming.sbr

$(OUTDIR)/direct86.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /NOLOGO /SUBSYSTEM:windows /DEBUG /MACHINE:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /NOLOGO /SUBSYSTEM:windows /DEBUG /MACHINE:I386 /OUT:"..\D86Win32.exe"
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /NOLOGO\
 /SUBSYSTEM:windows /INCREMENTAL:yes /PDB:$(OUTDIR)/"direct86.pdb" /DEBUG\
 /MACHINE:I386 /OUT:"..\D86Win32.exe" 
DEF_FILE=
LINK32_OBJS= \
	$(INTDIR)/CPURGWIN.OBJ \
	$(INTDIR)/8086.OBJ \
	$(INTDIR)/VGA_PAL8.OBJ \
	$(INTDIR)/Ram.obj \
	$(INTDIR)/Bios.obj \
	$(INTDIR)/VGA_TXT.OBJ \
	$(INTDIR)/Stackops.obj \
	$(INTDIR)/Brkpts.obj \
	$(INTDIR)/Cpustkw.obj \
	$(INTDIR)/TIMER.OBJ \
	$(INTDIR)/FLAGOPS.OBJ \
	$(INTDIR)/Keyboard.obj \
	$(INTDIR)/Fdsim.obj \
	$(INTDIR)/MEMWIN.OBJ \
	$(INTDIR)/WCACHE.OBJ \
	$(INTDIR)/Cpudec.obj \
	$(INTDIR)/ADDRBUS.OBJ \
	$(INTDIR)/LIB.OBJ \
	$(INTDIR)/HARDWARE.OBJ \
	$(INTDIR)/Cpuexec.obj \
	$(INTDIR)/HDSIM.OBJ \
	$(INTDIR)/Direct86.obj \
	$(INTDIR)/Interrm.obj \
	$(INTDIR)/WCACHEW.OBJ \
	$(INTDIR)/Execops.obj \
	$(INTDIR)/CPUTAIL.OBJ \
	$(INTDIR)/Kbdwin.obj \
	$(INTDIR)/Pic.obj \
	$(INTDIR)/CMOS.OBJ \
	$(INTDIR)/Execrm.obj \
	$(INTDIR)/Stockvga.obj \
	$(INTDIR)/Cpuqueue.obj \
	$(INTDIR)/Mother.obj \
	$(INTDIR)/VGA_NADA.OBJ \
	$(INTDIR)/Cpumodel.obj \
	$(INTDIR)/Naming.obj \
	$(INTDIR)/direct86.res

\JMC\Winprogs\DIRECT86\D86Win32.exe : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Group "Source Files"

################################################################################
# Begin Source File

SOURCE=\JMC\Winprogs\DIRECT86\CPURGWIN.C
DEP_CPURG=\
	\JMC\Winprogs\DIRECT86\global.h\
	\JMC\Winprogs\DIRECT86\direct86.h\
	\JMC\Winprogs\DIRECT86\cpurgwin.h\
	\JMC\Winprogs\DIRECT86\cpuexec.h\
	\JMC\Winprogs\DIRECT86\lib.h

$(INTDIR)/CPURGWIN.OBJ :  $(SOURCE)  $(DEP_CPURG) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\JMC\Winprogs\DIRECT86\8086.C
DEP_8086_=\
	\JMC\Winprogs\DIRECT86\global.h\
	\JMC\Winprogs\DIRECT86\addrbus.h\
	\JMC\Winprogs\DIRECT86\hardware.h\
	\JMC\Winprogs\DIRECT86\cpustkw.h\
	\JMC\Winprogs\DIRECT86\cpudec.h\
	\JMC\Winprogs\DIRECT86\cpuexec.h\
	\JMC\Winprogs\DIRECT86\cpuqueue.h\
	\JMC\Winprogs\DIRECT86\cpumodel.h\
	\JMC\Winprogs\DIRECT86\execrm.h\
	\JMC\Winprogs\DIRECT86\execops.h\
	\JMC\Winprogs\DIRECT86\flagops.h\
	\JMC\Winprogs\DIRECT86\stackops.h\
	\JMC\Winprogs\DIRECT86\interrm.h\
	\JMC\Winprogs\DIRECT86\8086.h\
	\JMC\Winprogs\DIRECT86\brkpts.h\
	\JMC\Winprogs\DIRECT86\naming.h

$(INTDIR)/8086.OBJ :  $(SOURCE)  $(DEP_8086_) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\JMC\Winprogs\DIRECT86\VGA_PAL8.C
DEP_VGA_P=\
	\JMC\Winprogs\DIRECT86\global.h\
	\JMC\Winprogs\DIRECT86\stockvga.h

$(INTDIR)/VGA_PAL8.OBJ :  $(SOURCE)  $(DEP_VGA_P) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\JMC\Winprogs\DIRECT86\Ram.C
DEP_RAM_C=\
	\JMC\Winprogs\DIRECT86\global.h\
	\JMC\Winprogs\DIRECT86\direct86.h\
	\JMC\Winprogs\DIRECT86\ram.h\
	\JMC\Winprogs\DIRECT86\addrbus.h

$(INTDIR)/Ram.obj :  $(SOURCE)  $(DEP_RAM_C) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\JMC\Winprogs\DIRECT86\Bios.C
DEP_BIOS_=\
	\JMC\Winprogs\DIRECT86\global.h\
	\JMC\Winprogs\DIRECT86\addrbus.h\
	\JMC\Winprogs\DIRECT86\hardware.h\
	\JMC\Winprogs\DIRECT86\cpuexec.h\
	\JMC\Winprogs\DIRECT86\ram.h\
	\JMC\Winprogs\DIRECT86\fdsim.h\
	\JMC\Winprogs\DIRECT86\hdsim.h\
	\JMC\Winprogs\DIRECT86\bios.h\
	\JMC\Winprogs\DIRECT86\cmos.h\
	\JMC\Winprogs\DIRECT86\stackops.h\
	\JMC\Winprogs\DIRECT86\lib.h\
	\JMC\Winprogs\DIRECT86\direct86.h

$(INTDIR)/Bios.obj :  $(SOURCE)  $(DEP_BIOS_) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\JMC\Winprogs\DIRECT86\VGA_TXT.C
DEP_VGA_T=\
	\JMC\Winprogs\DIRECT86\global.h\
	\JMC\Winprogs\DIRECT86\stockvga.h

$(INTDIR)/VGA_TXT.OBJ :  $(SOURCE)  $(DEP_VGA_T) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\JMC\Winprogs\DIRECT86\Stackops.C
DEP_STACK=\
	\JMC\Winprogs\DIRECT86\global.h\
	\JMC\Winprogs\DIRECT86\stackops.h\
	\JMC\Winprogs\DIRECT86\addrbus.h\
	\JMC\Winprogs\DIRECT86\memwin.h\
	\JMC\Winprogs\DIRECT86\cpustkw.h\
	\JMC\Winprogs\DIRECT86\cpuexec.h

$(INTDIR)/Stackops.obj :  $(SOURCE)  $(DEP_STACK) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\JMC\Winprogs\DIRECT86\Brkpts.C
DEP_BRKPT=\
	\JMC\Winprogs\DIRECT86\global.h\
	\JMC\Winprogs\DIRECT86\brkpts.h\
	\JMC\Winprogs\DIRECT86\direct86.h\
	\JMC\Winprogs\DIRECT86\cpuexec.h

$(INTDIR)/Brkpts.obj :  $(SOURCE)  $(DEP_BRKPT) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\JMC\Winprogs\DIRECT86\Cpustkw.C
DEP_CPUST=\
	\JMC\Winprogs\DIRECT86\global.h\
	\JMC\Winprogs\DIRECT86\direct86.h\
	\JMC\Winprogs\DIRECT86\cpustkw.h

$(INTDIR)/Cpustkw.obj :  $(SOURCE)  $(DEP_CPUST) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\JMC\Winprogs\DIRECT86\TIMER.C
DEP_TIMER=\
	\JMC\Winprogs\DIRECT86\global.h\
	\JMC\Winprogs\DIRECT86\cpuexec.h\
	\JMC\Winprogs\DIRECT86\timer.h\
	\JMC\Winprogs\DIRECT86\addrbus.h\
	\JMC\Winprogs\DIRECT86\hardware.h\
	\JMC\Winprogs\DIRECT86\pic.h

$(INTDIR)/TIMER.OBJ :  $(SOURCE)  $(DEP_TIMER) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\JMC\Winprogs\DIRECT86\FLAGOPS.C
DEP_FLAGO=\
	\JMC\Winprogs\DIRECT86\global.h\
	\JMC\Winprogs\DIRECT86\cpuexec.h\
	\JMC\Winprogs\DIRECT86\flagops.h

$(INTDIR)/FLAGOPS.OBJ :  $(SOURCE)  $(DEP_FLAGO) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\JMC\Winprogs\DIRECT86\Keyboard.C
DEP_KEYBO=\
	\JMC\Winprogs\DIRECT86\global.h\
	\JMC\Winprogs\DIRECT86\kbdwin.h\
	\JMC\Winprogs\DIRECT86\keyboard.h\
	\JMC\Winprogs\DIRECT86\addrbus.h\
	\JMC\Winprogs\DIRECT86\hardware.h\
	\JMC\Winprogs\DIRECT86\mother.h\
	\JMC\Winprogs\DIRECT86\pic.h

$(INTDIR)/Keyboard.obj :  $(SOURCE)  $(DEP_KEYBO) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\JMC\Winprogs\DIRECT86\Fdsim.C
DEP_FDSIM=\
	\JMC\Winprogs\DIRECT86\global.h\
	\JMC\Winprogs\DIRECT86\fdsim.h

$(INTDIR)/Fdsim.obj :  $(SOURCE)  $(DEP_FDSIM) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\JMC\Winprogs\DIRECT86\MEMWIN.C
DEP_MEMWI=\
	\JMC\Winprogs\DIRECT86\global.h\
	\JMC\Winprogs\DIRECT86\brkpts.h\
	\JMC\Winprogs\DIRECT86\direct86.h\
	\JMC\Winprogs\DIRECT86\cpurgwin.h\
	\JMC\Winprogs\DIRECT86\addrbus.h\
	\JMC\Winprogs\DIRECT86\cpuexec.h\
	\JMC\Winprogs\DIRECT86\memwin.h

$(INTDIR)/MEMWIN.OBJ :  $(SOURCE)  $(DEP_MEMWI) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\JMC\Winprogs\DIRECT86\WCACHE.C
DEP_WCACH=\
	\JMC\Winprogs\DIRECT86\global.h\
	\JMC\Winprogs\DIRECT86\addrbus.h\
	\JMC\Winprogs\DIRECT86\wcache.h\
	\JMC\Winprogs\DIRECT86\wcachew.h

$(INTDIR)/WCACHE.OBJ :  $(SOURCE)  $(DEP_WCACH) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\JMC\Winprogs\DIRECT86\Cpudec.C
DEP_CPUDE=\
	\JMC\Winprogs\DIRECT86\global.h\
	\JMC\Winprogs\DIRECT86\addrbus.h\
	\JMC\Winprogs\DIRECT86\mother.h\
	\JMC\Winprogs\DIRECT86\cpuqueue.h\
	\JMC\Winprogs\DIRECT86\cpumodel.h\
	\JMC\Winprogs\DIRECT86\cpudec.h\
	\JMC\Winprogs\DIRECT86\cpuexec.h\
	\JMC\Winprogs\DIRECT86\direct86.h\
	\JMC\Winprogs\DIRECT86\interrm.h\
	\JMC\Winprogs\DIRECT86\ram.h\
	\JMC\Winprogs\DIRECT86\naming.h

$(INTDIR)/Cpudec.obj :  $(SOURCE)  $(DEP_CPUDE) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\JMC\Winprogs\DIRECT86\ADDRBUS.C
DEP_ADDRB=\
	\JMC\Winprogs\DIRECT86\global.h\
	\JMC\Winprogs\DIRECT86\wcache.h\
	\JMC\Winprogs\DIRECT86\cpuexec.h\
	\JMC\Winprogs\DIRECT86\memwin.h\
	\JMC\Winprogs\DIRECT86\ram.h\
	\JMC\Winprogs\DIRECT86\brkpts.h\
	\JMC\Winprogs\DIRECT86\addrbus.h\
	\JMC\Winprogs\DIRECT86\direct86.h

$(INTDIR)/ADDRBUS.OBJ :  $(SOURCE)  $(DEP_ADDRB) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\JMC\Winprogs\DIRECT86\LIB.C
DEP_LIB_C=\
	\JMC\Winprogs\DIRECT86\global.h\
	\JMC\Winprogs\DIRECT86\direct86.h\
	\JMC\Winprogs\DIRECT86\lib.h

$(INTDIR)/LIB.OBJ :  $(SOURCE)  $(DEP_LIB_C) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\JMC\Winprogs\DIRECT86\HARDWARE.C
DEP_HARDW=\
	\JMC\Winprogs\DIRECT86\global.h\
	\JMC\Winprogs\DIRECT86\brkpts.h\
	\JMC\Winprogs\DIRECT86\addrbus.h\
	\JMC\Winprogs\DIRECT86\hardware.h\
	\JMC\Winprogs\DIRECT86\mother.h

$(INTDIR)/HARDWARE.OBJ :  $(SOURCE)  $(DEP_HARDW) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\JMC\Winprogs\DIRECT86\Cpuexec.C
DEP_CPUEX=\
	\JMC\Winprogs\DIRECT86\global.h\
	\JMC\Winprogs\DIRECT86\fdsim.h\
	\JMC\Winprogs\DIRECT86\hdsim.h\
	\JMC\Winprogs\DIRECT86\cputail.h\
	\JMC\Winprogs\DIRECT86\cpustkw.h\
	\JMC\Winprogs\DIRECT86\bios.h\
	\JMC\Winprogs\DIRECT86\flagops.h\
	\JMC\Winprogs\DIRECT86\cpumodel.h\
	\JMC\Winprogs\DIRECT86\cpuqueue.h\
	\JMC\Winprogs\DIRECT86\cpudec.h\
	\JMC\Winprogs\DIRECT86\cpuexec.h\
	\JMC\Winprogs\DIRECT86\stackops.h\
	\JMC\Winprogs\DIRECT86\direct86.h\
	\JMC\Winprogs\DIRECT86\execops.h\
	\JMC\Winprogs\DIRECT86\execrm.h\
	\JMC\Winprogs\DIRECT86\brkpts.h\
	\JMC\Winprogs\DIRECT86\addrbus.h\
	\JMC\Winprogs\DIRECT86\hardware.h\
	\JMC\Winprogs\DIRECT86\mother.h\
	\JMC\Winprogs\DIRECT86\8086.h

$(INTDIR)/Cpuexec.obj :  $(SOURCE)  $(DEP_CPUEX) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\JMC\Winprogs\DIRECT86\HDSIM.C
DEP_HDSIM=\
	\JMC\Winprogs\DIRECT86\global.h\
	\JMC\Winprogs\DIRECT86\hdsim.h

$(INTDIR)/HDSIM.OBJ :  $(SOURCE)  $(DEP_HDSIM) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\JMC\Winprogs\DIRECT86\Direct86.C
DEP_DIREC=\
	\JMC\Winprogs\DIRECT86\global.h\
	\JMC\Winprogs\DIRECT86\addrbus.h\
	\JMC\Winprogs\DIRECT86\fdsim.h\
	\JMC\Winprogs\DIRECT86\hdsim.h\
	\JMC\Winprogs\DIRECT86\bios.h\
	\JMC\Winprogs\DIRECT86\brkpts.h\
	\JMC\Winprogs\DIRECT86\cmos.h\
	\JMC\Winprogs\DIRECT86\cpudec.h\
	\JMC\Winprogs\DIRECT86\cpuexec.h\
	\JMC\Winprogs\DIRECT86\cpurgwin.h\
	\JMC\Winprogs\DIRECT86\cputail.h\
	\JMC\Winprogs\DIRECT86\cpustkw.h\
	\JMC\Winprogs\DIRECT86\direct86.h\
	\JMC\Winprogs\DIRECT86\execops.h\
	\JMC\Winprogs\DIRECT86\execrm.h\
	\JMC\Winprogs\DIRECT86\flagops.h\
	\JMC\Winprogs\DIRECT86\hardware.h\
	\JMC\Winprogs\DIRECT86\interrm.h\
	\JMC\Winprogs\DIRECT86\kbdwin.h\
	\JMC\Winprogs\DIRECT86\keyboard.h\
	\JMC\Winprogs\DIRECT86\lib.h\
	\JMC\Winprogs\DIRECT86\memwin.h\
	\JMC\Winprogs\DIRECT86\mother.h\
	\JMC\Winprogs\DIRECT86\pic.h\
	\JMC\Winprogs\DIRECT86\ram.h\
	\JMC\Winprogs\DIRECT86\stackops.h\
	\JMC\Winprogs\DIRECT86\stockvga.h\
	\JMC\Winprogs\DIRECT86\timer.h\
	\JMC\Winprogs\DIRECT86\wcache.h\
	\JMC\Winprogs\DIRECT86\wcachew.h\
	\JMC\Winprogs\DIRECT86\naming.h

$(INTDIR)/Direct86.obj :  $(SOURCE)  $(DEP_DIREC) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\JMC\Winprogs\DIRECT86\Interrm.C
DEP_INTER=\
	\JMC\Winprogs\DIRECT86\global.h\
	\JMC\Winprogs\DIRECT86\interrm.h\
	\JMC\Winprogs\DIRECT86\cpudec.h\
	\JMC\Winprogs\DIRECT86\cpuexec.h\
	\JMC\Winprogs\DIRECT86\cpuqueue.h\
	\JMC\Winprogs\DIRECT86\addrbus.h

$(INTDIR)/Interrm.obj :  $(SOURCE)  $(DEP_INTER) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\JMC\Winprogs\DIRECT86\WCACHEW.C
DEP_WCACHE=\
	\JMC\Winprogs\DIRECT86\global.h\
	\JMC\Winprogs\DIRECT86\cpuexec.h\
	\JMC\Winprogs\DIRECT86\direct86.h\
	\JMC\Winprogs\DIRECT86\wcachew.h

$(INTDIR)/WCACHEW.OBJ :  $(SOURCE)  $(DEP_WCACHE) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\JMC\Winprogs\DIRECT86\Execops.C
DEP_EXECO=\
	\JMC\Winprogs\DIRECT86\global.h\
	\JMC\Winprogs\DIRECT86\cpuexec.h\
	\JMC\Winprogs\DIRECT86\cpustkw.h\
	\JMC\Winprogs\DIRECT86\stackops.h\
	\JMC\Winprogs\DIRECT86\flagops.h\
	\JMC\Winprogs\DIRECT86\brkpts.h

$(INTDIR)/Execops.obj :  $(SOURCE)  $(DEP_EXECO) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\JMC\Winprogs\DIRECT86\CPUTAIL.C
DEP_CPUTA=\
	\JMC\Winprogs\DIRECT86\global.h\
	\JMC\Winprogs\DIRECT86\direct86.h\
	\JMC\Winprogs\DIRECT86\cputail.h

$(INTDIR)/CPUTAIL.OBJ :  $(SOURCE)  $(DEP_CPUTA) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\JMC\Winprogs\DIRECT86\Kbdwin.C
DEP_KBDWI=\
	\JMC\Winprogs\DIRECT86\global.h\
	\JMC\Winprogs\DIRECT86\direct86.h\
	\JMC\Winprogs\DIRECT86\kbdwin.h\
	\JMC\Winprogs\DIRECT86\keyboard.h\
	\JMC\Winprogs\DIRECT86\mother.h

$(INTDIR)/Kbdwin.obj :  $(SOURCE)  $(DEP_KBDWI) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\JMC\Winprogs\DIRECT86\Pic.C
DEP_PIC_C=\
	\JMC\Winprogs\DIRECT86\global.h\
	\JMC\Winprogs\DIRECT86\direct86.h\
	\JMC\Winprogs\DIRECT86\cpuexec.h\
	\JMC\Winprogs\DIRECT86\brkpts.h\
	\JMC\Winprogs\DIRECT86\pic.h\
	\JMC\Winprogs\DIRECT86\stackops.h\
	\JMC\Winprogs\DIRECT86\addrbus.h\
	\JMC\Winprogs\DIRECT86\hardware.h

$(INTDIR)/Pic.obj :  $(SOURCE)  $(DEP_PIC_C) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\JMC\Winprogs\DIRECT86\CMOS.C
DEP_CMOS_=\
	\JMC\Winprogs\DIRECT86\global.h\
	\JMC\Winprogs\DIRECT86\addrbus.h\
	\JMC\Winprogs\DIRECT86\hardware.h\
	\JMC\Winprogs\DIRECT86\direct86.h\
	\JMC\Winprogs\DIRECT86\cpuexec.h\
	\JMC\Winprogs\DIRECT86\lib.h\
	\JMC\Winprogs\DIRECT86\cmos.h

$(INTDIR)/CMOS.OBJ :  $(SOURCE)  $(DEP_CMOS_) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\JMC\Winprogs\DIRECT86\Execrm.C
DEP_EXECR=\
	\JMC\Winprogs\DIRECT86\global.h\
	\JMC\Winprogs\DIRECT86\direct86.h\
	\JMC\Winprogs\DIRECT86\stackops.h\
	\JMC\Winprogs\DIRECT86\execrm.h\
	\JMC\Winprogs\DIRECT86\cpudec.h\
	\JMC\Winprogs\DIRECT86\cpuexec.h\
	\JMC\Winprogs\DIRECT86\addrbus.h

$(INTDIR)/Execrm.obj :  $(SOURCE)  $(DEP_EXECR) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\JMC\Winprogs\DIRECT86\Stockvga.C
DEP_STOCK=\
	\JMC\Winprogs\DIRECT86\global.h\
	\JMC\Winprogs\DIRECT86\direct86.h\
	\JMC\Winprogs\DIRECT86\stockvga.h\
	\JMC\Winprogs\DIRECT86\addrbus.h\
	\JMC\Winprogs\DIRECT86\hardware.h\
	\JMC\Winprogs\DIRECT86\cpuexec.h\
	\JMC\Winprogs\DIRECT86\kbdwin.h

$(INTDIR)/Stockvga.obj :  $(SOURCE)  $(DEP_STOCK) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\JMC\Winprogs\DIRECT86\Cpuqueue.C
DEP_CPUQU=\
	\JMC\Winprogs\DIRECT86\global.h\
	\JMC\Winprogs\DIRECT86\direct86.h\
	\JMC\Winprogs\DIRECT86\cpuexec.h\
	\JMC\Winprogs\DIRECT86\cpuqueue.h\
	\JMC\Winprogs\DIRECT86\stackops.h\
	\JMC\Winprogs\DIRECT86\addrbus.h

$(INTDIR)/Cpuqueue.obj :  $(SOURCE)  $(DEP_CPUQU) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\JMC\Winprogs\DIRECT86\Mother.C
DEP_MOTHE=\
	\JMC\Winprogs\DIRECT86\global.h\
	\JMC\Winprogs\DIRECT86\direct86.h\
	\JMC\Winprogs\DIRECT86\fdsim.h\
	\JMC\Winprogs\DIRECT86\hdsim.h\
	\JMC\Winprogs\DIRECT86\bios.h\
	\JMC\Winprogs\DIRECT86\mother.h\
	\JMC\Winprogs\DIRECT86\cpuexec.h\
	\JMC\Winprogs\DIRECT86\cpudec.h\
	\JMC\Winprogs\DIRECT86\cmos.h\
	\JMC\Winprogs\DIRECT86\keyboard.h\
	\JMC\Winprogs\DIRECT86\addrbus.h\
	\JMC\Winprogs\DIRECT86\hardware.h\
	\JMC\Winprogs\DIRECT86\ram.h\
	\JMC\Winprogs\DIRECT86\timer.h\
	\JMC\Winprogs\DIRECT86\stockvga.h\
	\JMC\Winprogs\DIRECT86\pic.h\
	\JMC\Winprogs\DIRECT86\brkpts.h\
	\JMC\Winprogs\DIRECT86\cpustkw.h\
	\JMC\Winprogs\DIRECT86\cputail.h\
	\JMC\Winprogs\DIRECT86\lib.h

$(INTDIR)/Mother.obj :  $(SOURCE)  $(DEP_MOTHE) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\JMC\Winprogs\DIRECT86\VGA_NADA.C
DEP_VGA_N=\
	\JMC\Winprogs\DIRECT86\global.h\
	\JMC\Winprogs\DIRECT86\stockvga.h

$(INTDIR)/VGA_NADA.OBJ :  $(SOURCE)  $(DEP_VGA_N) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\JMC\Winprogs\DIRECT86\Cpumodel.C
DEP_CPUMO=\
	\JMC\Winprogs\DIRECT86\global.h\
	\JMC\Winprogs\DIRECT86\cpuexec.h\
	\JMC\Winprogs\DIRECT86\cpumodel.h\
	\JMC\Winprogs\DIRECT86\cpuqueue.h

$(INTDIR)/Cpumodel.obj :  $(SOURCE)  $(DEP_CPUMO) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\JMC\Winprogs\DIRECT86\Naming.C
DEP_NAMIN=\
	\JMC\Winprogs\DIRECT86\global.h\
	\JMC\Winprogs\DIRECT86\naming.h\
	\JMC\Winprogs\DIRECT86\direct86.h

$(INTDIR)/Naming.obj :  $(SOURCE)  $(DEP_NAMIN) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\direct86.rc
DEP_DIRECT=\
	.\APPICON.ICO\
	.\MEMICON.ICO\
	.\REGICON.ICO

$(INTDIR)/direct86.res :  $(SOURCE)  $(DEP_DIRECT) $(INTDIR)
   $(RSC) $(RSC_PROJ)  $(SOURCE) 

# End Source File
# End Group
# End Project
################################################################################
