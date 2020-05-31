# Microsoft Developer Studio Project File - Name="mmio" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=mmio - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "mmio.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "mmio.mak" CFG="mmio - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "mmio - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "mmio - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "mmio - Win32 VectorC" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=E:\VECTORC\VECTORCL.EXE
RSC=rc.exe

!IF  "$(CFG)" == "mmio - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "lib"
# PROP Intermediate_Dir "temp"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /Zp4 /MT /W3 /GX /Zd /O2 /I "$(MMIO)\include" /D "_MBCS" /D "_LIB" /D "P_INTEL" /D "WIN32" /D "NDEBUG" /D "MM_LOG_VERBOSE" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "mmio - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "$(MMIO)\include" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "P_INTEL" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "mmio - Win32 VectorC"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "mmio___Win32_VectorC"
# PROP BASE Intermediate_Dir "mmio___Win32_VectorC"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "temp"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Zp4 /MT /W3 /GX /Zd /O2 /I "$(MMIO)\include" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "P_INTEL" /YX /FD $(VECTORC) /c
# ADD CPP /nologo /MT /W2 /GX /Zd /O2 /I "$(MMIO)\include" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "P_INTEL" /YX /FD $(VECTORC) /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "mmio - Win32 Release"
# Name "mmio - Win32 Debug"
# Name "mmio - Win32 VectorC"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\loadcfg.c
# End Source File
# Begin Source File

SOURCE=.\src\log.c
# End Source File
# Begin Source File

SOURCE=.\src\mmalloc.c
# End Source File
# Begin Source File

SOURCE=.\src\mmcopy.c
# End Source File
# Begin Source File

SOURCE=.\src\mmerror.c
# End Source File
# Begin Source File

SOURCE=.\src\mmio.c

!IF  "$(CFG)" == "mmio - Win32 Release"

# ADD CPP /G6 /O2 /Op- /Oy /Ob2

!ELSEIF  "$(CFG)" == "mmio - Win32 Debug"

!ELSEIF  "$(CFG)" == "mmio - Win32 VectorC"

# ADD BASE CPP /G6 /O2 /Op- /Oy /Ob2
# ADD CPP /G6 /O2 /Op- /Oy /Ob2

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\include\log.h
# End Source File
# Begin Source File

SOURCE=.\include\mmconfig.h
# End Source File
# Begin Source File

SOURCE=.\include\mminline.h
# End Source File
# Begin Source File

SOURCE=.\include\mmio.h
# End Source File
# Begin Source File

SOURCE=.\include\mmtypes.h
# End Source File
# Begin Source File

SOURCE=.\include\vectorc.h
# End Source File
# End Group
# End Target
# End Project
