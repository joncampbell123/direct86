# Microsoft Developer Studio Project File - Name="win95" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=win95 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "win95.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "win95.mak" CFG="win95 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "win95 - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "win95 - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "win95 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "WIN95" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x80a /d "NDEBUG"
# ADD RSC /l 0x80a /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386 /out:"..\D86Win95.exe"

!ELSEIF  "$(CFG)" == "win95 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "WIN95" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x80a /d "_DEBUG"
# ADD RSC /l 0x80a /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"..\D86Win95.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "win95 - Win32 Release"
# Name "win95 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\8086.c
# End Source File
# Begin Source File

SOURCE=..\Addrbus.c
# End Source File
# Begin Source File

SOURCE=..\Bios.C
# End Source File
# Begin Source File

SOURCE=..\Brkpts.C
# End Source File
# Begin Source File

SOURCE=..\Cmos.c
# End Source File
# Begin Source File

SOURCE=..\Cpudec.C
# End Source File
# Begin Source File

SOURCE=..\Cpuexec.C
# End Source File
# Begin Source File

SOURCE=..\Cpumodel.C
# End Source File
# Begin Source File

SOURCE=..\Cpuqueue.C
# End Source File
# Begin Source File

SOURCE=..\Cpurgwin.c
# End Source File
# Begin Source File

SOURCE=..\Cpustkw.C
# End Source File
# Begin Source File

SOURCE=..\Cputail.c
# End Source File
# Begin Source File

SOURCE=..\Direct86.C
# End Source File
# Begin Source File

SOURCE=.\direct86.rc
# End Source File
# Begin Source File

SOURCE=..\Execops.C
# End Source File
# Begin Source File

SOURCE=..\Execrm.C
# End Source File
# Begin Source File

SOURCE=..\Fdsim.C
# End Source File
# Begin Source File

SOURCE=..\Flagops.c
# End Source File
# Begin Source File

SOURCE=..\Hardware.c
# End Source File
# Begin Source File

SOURCE=..\Hdsim.c
# End Source File
# Begin Source File

SOURCE=..\Interrm.C
# End Source File
# Begin Source File

SOURCE=..\Kbdwin.C
# End Source File
# Begin Source File

SOURCE=..\Keyboard.C
# End Source File
# Begin Source File

SOURCE=..\Lib.c
# End Source File
# Begin Source File

SOURCE=..\Memwin.c
# End Source File
# Begin Source File

SOURCE=..\Mother.C
# End Source File
# Begin Source File

SOURCE=..\Naming.C
# End Source File
# Begin Source File

SOURCE=..\Pic.C
# End Source File
# Begin Source File

SOURCE=..\Ram.C
# End Source File
# Begin Source File

SOURCE=..\Stackops.C
# End Source File
# Begin Source File

SOURCE=..\Stockvga.C
# End Source File
# Begin Source File

SOURCE=..\Timer.c
# End Source File
# Begin Source File

SOURCE=..\Vga_nada.c
# End Source File
# Begin Source File

SOURCE=..\Vga_pal8.c
# End Source File
# Begin Source File

SOURCE=..\Vga_txt.c
# End Source File
# Begin Source File

SOURCE=..\Wcache.c
# End Source File
# Begin Source File

SOURCE=..\Wcachew.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\8086.h
# End Source File
# Begin Source File

SOURCE=..\Addrbus.h
# End Source File
# Begin Source File

SOURCE=..\Bios.h
# End Source File
# Begin Source File

SOURCE=..\Brkpts.h
# End Source File
# Begin Source File

SOURCE=..\Cmos.h
# End Source File
# Begin Source File

SOURCE=..\Cpudec.h
# End Source File
# Begin Source File

SOURCE=..\Cpuexec.h
# End Source File
# Begin Source File

SOURCE=..\Cpumodel.h
# End Source File
# Begin Source File

SOURCE=..\Cpuqueue.h
# End Source File
# Begin Source File

SOURCE=..\Cpurgwin.h
# End Source File
# Begin Source File

SOURCE=..\CPUSTKW.h
# End Source File
# Begin Source File

SOURCE=..\Cputail.h
# End Source File
# Begin Source File

SOURCE=..\Direct86.h
# End Source File
# Begin Source File

SOURCE=..\Execops.h
# End Source File
# Begin Source File

SOURCE=..\Execrm.h
# End Source File
# Begin Source File

SOURCE=..\Fdsim.h
# End Source File
# Begin Source File

SOURCE=..\Flagops.h
# End Source File
# Begin Source File

SOURCE=..\Global.h
# End Source File
# Begin Source File

SOURCE=..\Hardware.h
# End Source File
# Begin Source File

SOURCE=..\Hdsim.h
# End Source File
# Begin Source File

SOURCE=..\Interrm.h
# End Source File
# Begin Source File

SOURCE=..\Kbdwin.h
# End Source File
# Begin Source File

SOURCE=..\Keyboard.h
# End Source File
# Begin Source File

SOURCE=..\Lib.h
# End Source File
# Begin Source File

SOURCE=..\Memwin.h
# End Source File
# Begin Source File

SOURCE=..\Mother.h
# End Source File
# Begin Source File

SOURCE=..\naming.h
# End Source File
# Begin Source File

SOURCE=..\Pic.h
# End Source File
# Begin Source File

SOURCE=..\Ram.h
# End Source File
# Begin Source File

SOURCE=..\Resource.h
# End Source File
# Begin Source File

SOURCE=..\Stackops.h
# End Source File
# Begin Source File

SOURCE=..\Stockvga.h
# End Source File
# Begin Source File

SOURCE=..\Timer.h
# End Source File
# Begin Source File

SOURCE=..\Wcache.h
# End Source File
# Begin Source File

SOURCE=..\Wcachew.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\Appicon.ico
# End Source File
# Begin Source File

SOURCE=.\Memicon.ico
# End Source File
# Begin Source File

SOURCE=.\Regicon.ico
# End Source File
# End Group
# End Target
# End Project
