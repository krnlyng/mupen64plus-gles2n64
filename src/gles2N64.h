#ifndef GLN64_H
#define GLN64_H

#include "winlnxdefs.h"
#include "m64p_config.h"
#include "stdio.h"

#include "m64p_config.h"
#include "m64p_vidext.h"
#
#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

#define PLUGIN_NAME     "gles2n64"
#define PLUGIN_VERSION  0x000005
#define PLUGIN_API_VERSION 0x020200

extern ptr_ConfigGetSharedDataFilepath ConfigGetSharedDataFilepath;

extern HWND         hWnd;
extern char *screenDirectory;
extern char configdir[PATH_MAX];

extern void (*CheckInterrupts)( void );
extern void (*renderCallback)();

extern ptr_VidExt_Init                  CoreVideo_Init;
extern ptr_VidExt_Quit                  CoreVideo_Quit;
extern ptr_VidExt_ListFullscreenModes   CoreVideo_ListFullscreenModes;
extern ptr_VidExt_SetVideoMode          CoreVideo_SetVideoMode;
extern ptr_VidExt_SetCaption            CoreVideo_SetCaption;
extern ptr_VidExt_ToggleFullScreen      CoreVideo_ToggleFullScreen;
extern ptr_VidExt_GL_GetProcAddress     CoreVideo_GL_GetProcAddress;
extern ptr_VidExt_GL_SetAttribute       CoreVideo_GL_SetAttribute;
extern ptr_VidExt_GL_GetAttribute       CoreVideo_GL_GetAttribute;
extern ptr_VidExt_GL_SwapBuffers        CoreVideo_GL_SwapBuffers;



#endif
