# Microsoft Developer Studio Project File - Name="chicken little" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=chicken little - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "chicken little.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "chicken little.mak" CFG="chicken little - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "chicken little - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "chicken little - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "chicken little - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "temp"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "include" /I "$(APP)\include" /I "$(KEYSTICK)\include" /I "$(MMIO)\include" /I "$(VDRIVER)\include" /I "$(MIKMOD)\include" /D "_WINDOWS" /D "WIN32" /D "NDEBUG" /D "MM_LOG_VERBOSE" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib dxguid.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "chicken little - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "include" /I "$(APP)\include" /I "$(KEYSTICK)\include" /I "$(MMIO)\include" /I "$(VDRIVER)\include" /I "$(MIKMOD)\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib dxguid.lib /nologo /subsystem:windows /profile /debug /machine:I386

!ENDIF 

# Begin Target

# Name "chicken little - Win32 Release"
# Name "chicken little - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "ResourceLoading"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\loadres.c
# End Source File
# Begin Source File

SOURCE=.\src\mdrespak.c
# End Source File
# End Group
# Begin Group "Entities"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\birdies.c
# End Source File
# Begin Source File

SOURCE=.\src\eggs.c
# End Source File
# Begin Source File

SOURCE=.\src\gamepiece.c
# End Source File
# Begin Source File

SOURCE=.\src\player.c
# End Source File
# Begin Source File

SOURCE=.\src\stone.c
# End Source File
# Begin Source File

SOURCE=.\src\textinfo.c
# End Source File
# Begin Source File

SOURCE=.\src\title.c
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\ai.c
# End Source File
# Begin Source File

SOURCE=.\src\animation.c
# End Source File
# Begin Source File

SOURCE=.\src\clstates.c
# End Source File
# Begin Source File

SOURCE=.\src\display.c
# End Source File
# Begin Source File

SOURCE=.\src\entity.c
# End Source File
# Begin Source File

SOURCE=.\src\gameplay.c
# End Source File
# Begin Source File

SOURCE=.\src\main.c
# End Source File
# Begin Source File

SOURCE=.\src\physics.c
# End Source File
# Begin Source File

SOURCE=.\src\piecedump.c
# End Source File
# Begin Source File

SOURCE=.\src\random.c
# End Source File
# Begin Source File

SOURCE=.\src\stat.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\include\ai.h
# End Source File
# Begin Source File

SOURCE=.\include\animation.h
# End Source File
# Begin Source File

SOURCE=.\include\chicken.h
# End Source File
# Begin Source File

SOURCE=.\include\clgfxres.h
# End Source File
# Begin Source File

SOURCE=.\include\clmenu.h
# End Source File
# Begin Source File

SOURCE=.\include\clres.h
# End Source File
# Begin Source File

SOURCE=.\include\clsfxres.h
# End Source File
# Begin Source File

SOURCE=.\include\entity.h
# End Source File
# Begin Source File

SOURCE=.\include\gameplay.h
# End Source File
# Begin Source File

SOURCE=.\include\physics.h
# End Source File
# Begin Source File

SOURCE=.\include\player.h
# End Source File
# Begin Source File

SOURCE=.\include\random.h
# End Source File
# Begin Source File

SOURCE=.\include\stat.h
# End Source File
# Begin Source File

SOURCE=.\include\textinfo.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\scripts\gamegfx.brk
# End Source File
# Begin Source File

SOURCE=.\scripts\gamesfx.brk
# End Source File
# Begin Source File

SOURCE=.\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\test.rc
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409 /i "include"
# End Source File
# End Group
# Begin Source File

SOURCE=.\ai.txt
# End Source File
# Begin Source File

SOURCE=.\bugs.txt
# End Source File
# Begin Source File

SOURCE=.\todo.txt
# End Source File
# End Target
# End Project
