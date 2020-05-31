# Microsoft Developer Studio Project File - Name="vdriver" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=vdriver - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "vdriver.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "vdriver.mak" CFG="vdriver - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "vdriver - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "vdriver - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "vdriver - Win32 VectorC" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=E:\VECTORC\VECTORCL.EXE
RSC=rc.exe

!IF  "$(CFG)" == "vdriver - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "lib"
# PROP Intermediate_Dir "../temp"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /Zp4 /MT /W3 /GX /Zd /O2 /I "$(MMIO)\include" /I "$(VDRIVER)\include" /D "_MBCS" /D "_LIB" /D "WIN32" /D "NDEBUG" /D "MM_LOG_VERBOSE" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "vdriver - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "lib"
# PROP Intermediate_Dir "../temp/debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "$(MMIO)\include" /I "$(VDRIVER)\include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"lib\vdriverd.lib"

!ELSEIF  "$(CFG)" == "vdriver - Win32 VectorC"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "vdriver___Win32_VectorC"
# PROP BASE Intermediate_Dir "vdriver___Win32_VectorC"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "lib"
# PROP Intermediate_Dir "../temp"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Zp4 /MT /W3 /GX /Zd /O2 /I "$(MMIO)\include" /I "$(VDRIVER)\include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD $(VECTORC) /c
# ADD CPP /nologo /MT /W2 /GX /Zd /O2 /I "$(MMIO)\include" /I "$(VDRIVER)\include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD $(VECTORC) /c
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

# Name "vdriver - Win32 Release"
# Name "vdriver - Win32 Debug"
# Name "vdriver - Win32 VectorC"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Primatives"

# PROP Default_Filter ""
# Begin Group "MMX"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\src\prims\i386-mmx\spr32add.h"

!IF  "$(CFG)" == "vdriver - Win32 Release"

# PROP Intermediate_Dir "../temp/mmx"

!ELSEIF  "$(CFG)" == "vdriver - Win32 Debug"

# PROP Intermediate_Dir "..\temp\debug\mmx"

!ELSEIF  "$(CFG)" == "vdriver - Win32 VectorC"

# PROP Intermediate_Dir "../temp/mmx"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=".\src\prims\i386-mmx\spr32add_bgr.c"

!IF  "$(CFG)" == "vdriver - Win32 Release"

# PROP Intermediate_Dir "../temp/mmx"

!ELSEIF  "$(CFG)" == "vdriver - Win32 Debug"

# PROP Intermediate_Dir "..\temp\debug\mmx"

!ELSEIF  "$(CFG)" == "vdriver - Win32 VectorC"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=".\src\prims\i386-mmx\spr32add_lst.c"

!IF  "$(CFG)" == "vdriver - Win32 Release"

# PROP Intermediate_Dir "../temp/mmx"

!ELSEIF  "$(CFG)" == "vdriver - Win32 Debug"

# PROP Intermediate_Dir "..\temp\debug\mmx"

!ELSEIF  "$(CFG)" == "vdriver - Win32 VectorC"

# PROP Intermediate_Dir "../temp/mmx"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=".\src\prims\i386-mmx\spr32add_rgb.c"

!IF  "$(CFG)" == "vdriver - Win32 Release"

# PROP Intermediate_Dir "../temp/mmx"

!ELSEIF  "$(CFG)" == "vdriver - Win32 Debug"

# PROP Intermediate_Dir "..\temp\debug\mmx"

!ELSEIF  "$(CFG)" == "vdriver - Win32 VectorC"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=".\src\prims\i386-mmx\spr32alpha_bgr.c"

!IF  "$(CFG)" == "vdriver - Win32 Release"

# PROP Intermediate_Dir "../temp/mmx"

!ELSEIF  "$(CFG)" == "vdriver - Win32 Debug"

# PROP Intermediate_Dir "..\temp\debug\mmx"

!ELSEIF  "$(CFG)" == "vdriver - Win32 VectorC"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=".\src\prims\i386-mmx\spr32alpha_lst.c"

!IF  "$(CFG)" == "vdriver - Win32 Release"

# PROP Intermediate_Dir "../temp/mmx"

!ELSEIF  "$(CFG)" == "vdriver - Win32 Debug"

# PROP Intermediate_Dir "..\temp\debug\mmx"

!ELSEIF  "$(CFG)" == "vdriver - Win32 VectorC"

# PROP Intermediate_Dir "../temp/mmx"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=".\src\prims\i386-mmx\spr32alpha_rgb.c"

!IF  "$(CFG)" == "vdriver - Win32 Release"

# PROP Intermediate_Dir "../temp/mmx"

!ELSEIF  "$(CFG)" == "vdriver - Win32 Debug"

# PROP Intermediate_Dir "..\temp\debug\mmx"

!ELSEIF  "$(CFG)" == "vdriver - Win32 VectorC"

!ENDIF 

# End Source File
# End Group
# Begin Group "Sprite16"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\prims\spr16.c
# End Source File
# Begin Source File

SOURCE=.\src\prims\spr16add.c
# End Source File
# Begin Source File

SOURCE=.\src\prims\spr16add.h
# End Source File
# Begin Source File

SOURCE=.\src\prims\spr16add_555.c
# End Source File
# Begin Source File

SOURCE=.\src\prims\spr16add_565.c
# End Source File
# Begin Source File

SOURCE=.\src\prims\spr16add_lst.c
# End Source File
# Begin Source File

SOURCE=.\src\prims\spr16alpha.h
# End Source File
# Begin Source File

SOURCE=.\src\prims\spr16alpha_555.c
# End Source File
# Begin Source File

SOURCE=.\src\prims\spr16alpha_565.c
# End Source File
# Begin Source File

SOURCE=.\src\prims\spr16alpha_lst.c
# End Source File
# End Group
# Begin Group "Sprite32"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\prims\spr32.c
# End Source File
# Begin Source File

SOURCE=.\src\prims\spr32add.c
# End Source File
# Begin Source File

SOURCE=.\src\prims\spr32add.h
# End Source File
# Begin Source File

SOURCE=.\src\prims\spr32add_bgr.c
# End Source File
# Begin Source File

SOURCE=.\src\prims\spr32add_lst.c
# End Source File
# Begin Source File

SOURCE=.\src\prims\spr32add_rgb.c
# End Source File
# Begin Source File

SOURCE=.\src\prims\spr32alpha.h
# End Source File
# Begin Source File

SOURCE=.\src\prims\spr32alpha_bgr.c
# End Source File
# Begin Source File

SOURCE=.\src\prims\spr32alpha_lst.c
# End Source File
# Begin Source File

SOURCE=.\src\prims\spr32alpha_rgb.c
# End Source File
# End Group
# Begin Source File

SOURCE=".\src\prims\16-bit.c"
# End Source File
# Begin Source File

SOURCE=".\src\prims\32-bit.c"
# End Source File
# Begin Source File

SOURCE=.\src\prims\prims.h
# End Source File
# Begin Source File

SOURCE=.\src\prims\rect16.c
# End Source File
# Begin Source File

SOURCE=.\src\prims\rect32.c
# End Source File
# End Group
# Begin Group "Drivers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\drivers\win32\dib.c
# End Source File
# Begin Source File

SOURCE=.\src\drivers\win32\dx3.c
# End Source File
# Begin Source File

SOURCE=.\src\drivers\font50.c
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\cvt_rle.c
# End Source File
# Begin Source File

SOURCE=.\src\fontspr.c
# End Source File
# Begin Source File

SOURCE=.\src\gamegfx.c
# End Source File
# Begin Source File

SOURCE=.\src\gfx.c
# End Source File
# Begin Source File

SOURCE=.\src\image.c
# End Source File
# Begin Source File

SOURCE=.\src\loadbmp.c
# End Source File
# Begin Source File

SOURCE=.\src\loadpcx.c
# End Source File
# Begin Source File

SOURCE=.\src\pal_opt.c
# End Source File
# Begin Source File

SOURCE=.\src\vdrespak.c
# End Source File
# Begin Source File

SOURCE=.\src\vdriver.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\include\image.h
# End Source File
# Begin Source File

SOURCE=.\include\sprite.h
# End Source File
# Begin Source File

SOURCE=.\include\vdfont.h
# End Source File
# Begin Source File

SOURCE=.\include\vdrespak.h
# End Source File
# Begin Source File

SOURCE=.\include\vdriver.h
# End Source File
# End Group
# End Target
# End Project
