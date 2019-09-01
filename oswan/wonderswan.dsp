# Microsoft Developer Studio Project File - Name="wonderswan" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=wonderswan - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "wonderswan.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "wonderswan.mak" CFG="wonderswan - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "wonderswan - Win32 Release" (based on\
 "Win32 (x86) Console Application")
!MESSAGE "wonderswan - Win32 Debug" (based on\
 "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "wonderswan - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /G6 /MT /GX /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX /FD /c
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 comctl32.lib comdlg32.lib kernel32.lib user32.lib gdi32.lib winmm.lib winspool.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "wonderswan - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /Gm /GX /Zi /O2 /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FR /YX /FD /c
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 comctl32.lib comdlg32.lib kernel32.lib user32.lib gdi32.lib winspool.lib winmm.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "wonderswan - Win32 Release"
# Name "wonderswan - Win32 Debug"
# Begin Group "nec"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\source\nec\nec.cpp

!IF  "$(CFG)" == "wonderswan - Win32 Release"

# ADD CPP /W1

!ELSEIF  "$(CFG)" == "wonderswan - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\source\nec\nec.h
# End Source File
# Begin Source File

SOURCE=.\source\nec\necea.h
# End Source File
# Begin Source File

SOURCE=.\source\nec\necinstr.h
# End Source File
# Begin Source File

SOURCE=.\source\nec\necmodrm.h
# End Source File
# End Group
# Begin Group "documentation"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\documentation\ws_communication.txt
# End Source File
# Begin Source File

SOURCE=.\documentation\ws_controls.txt
# End Source File
# Begin Source File

SOURCE=.\documentation\ws_dma.txt
# End Source File
# Begin Source File

SOURCE=.\documentation\ws_external_eeprom.txt
# End Source File
# Begin Source File

SOURCE=.\documentation\ws_gpu.txt
# End Source File
# Begin Source File

SOURCE=.\documentation\ws_hardware_type.txt
# End Source File
# Begin Source File

SOURCE=.\documentation\ws_internal_eeprom.txt
# End Source File
# Begin Source File

SOURCE=.\documentation\ws_interrupts.txt
# End Source File
# Begin Source File

SOURCE=.\documentation\ws_lcd_control.txt
# End Source File
# Begin Source File

SOURCE=.\documentation\ws_line_execution_behaviour.txt
# End Source File
# Begin Source File

SOURCE=.\documentation\ws_palette.txt
# End Source File
# Begin Source File

SOURCE=.\documentation\ws_realtimeclock.txt
# End Source File
# Begin Source File

SOURCE=.\documentation\ws_rom_banks_selectors.txt
# End Source File
# Begin Source File

SOURCE=.\documentation\ws_rom_header.txt
# End Source File
# Begin Source File

SOURCE=.\documentation\ws_sound.txt
# End Source File
# Begin Source File

SOURCE=.\documentation\ws_tileformat.txt
# End Source File
# Begin Source File

SOURCE=.\documentation\wstech20.txt
# End Source File
# Begin Source File

SOURCE=.\documentation\wstech20work.txt
# End Source File
# End Group
# Begin Group "tools"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\source\log.cpp
# End Source File
# Begin Source File

SOURCE=.\source\LOG.H
# End Source File
# Begin Source File

SOURCE=.\source\ticker.h
# End Source File
# Begin Source File

SOURCE=.\source\ticker.obj
# End Source File
# Begin Source File

SOURCE=.\source\types.h
# End Source File
# End Group
# Begin Group "gui"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\gui.rc

!IF  "$(CFG)" == "wonderswan - Win32 Release"

!ELSEIF  "$(CFG)" == "wonderswan - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\source\gui\logo.bmp
# End Source File
# Begin Source File

SOURCE=.\source\gui\sponsor.bmp
# End Source File
# End Group
# Begin Group "sdl"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\source\sdl\include\SDL.h
# End Source File
# Begin Source File

SOURCE=.\source\sdl\SDL.lib
# End Source File
# Begin Source File

SOURCE=.\source\sdl\SDLmain.lib
# End Source File
# Begin Source File

SOURCE=.\source\sdl\include\SDLptc.h
# End Source File
# End Group
# Begin Group "wonderswan"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\source\audio.cpp
# End Source File
# Begin Source File

SOURCE=.\source\audio.h
# End Source File
# Begin Source File

SOURCE=.\source\gpu.cpp
# End Source File
# Begin Source File

SOURCE=.\source\gpu.h
# End Source File
# Begin Source File

SOURCE=.\source\ieeprom.h
# End Source File
# Begin Source File

SOURCE=.\source\initialIo.h
# End Source File
# Begin Source File

SOURCE=.\source\io.cpp
# End Source File
# Begin Source File

SOURCE=.\source\io.h
# End Source File
# Begin Source File

SOURCE=.\source\memory.cpp
# End Source File
# Begin Source File

SOURCE=.\source\memory.h
# End Source File
# Begin Source File

SOURCE=.\source\rom.cpp
# End Source File
# Begin Source File

SOURCE=.\source\rom.h
# End Source File
# Begin Source File

SOURCE=.\source\ws.cpp
# End Source File
# Begin Source File

SOURCE=.\source\ws.h
# End Source File
# End Group
# Begin Group "filters"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\source\2xSaI.cpp
# End Source File
# Begin Source File

SOURCE=.\source\2xSaI.h
# End Source File
# Begin Source File

SOURCE=.\source\filters\2xsai.h
# End Source File
# Begin Source File

SOURCE=.\source\filters\doubled.h
# End Source File
# Begin Source File

SOURCE=.\source\filters\filter_partA.h
# End Source File
# Begin Source File

SOURCE=.\source\filters\filter_partB.h
# End Source File
# Begin Source File

SOURCE=.\source\filters\filter_partC.h
# End Source File
# Begin Source File

SOURCE=.\source\filters\filter_partD.h
# End Source File
# Begin Source File

SOURCE=.\source\filters\filter_partE.h
# End Source File
# Begin Source File

SOURCE=.\source\filters\halfscanlines.h
# End Source File
# Begin Source File

SOURCE=.\source\filters\scanlines.h
# End Source File
# Begin Source File

SOURCE=.\source\filters\special.h
# End Source File
# Begin Source File

SOURCE=.\source\filters\standard.h
# End Source File
# Begin Source File

SOURCE=.\source\filters\super2xsai.h
# End Source File
# Begin Source File

SOURCE=.\source\filters\supereagle.h
# End Source File
# End Group
# Begin Group "seal"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\source\seal\AUDIO.H
# End Source File
# Begin Source File

SOURCE=.\source\seal\AUDW32VC.LIB
# End Source File
# End Group
# Begin Source File

SOURCE=.\source\temp\key.h
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# End Target
# End Project
