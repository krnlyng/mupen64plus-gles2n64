/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - Config_nogui.cpp                                        *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2008 Tillin9                                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "Config.h"
#include "gles2N64.h"
#include "RSP.h"
#include "Textures.h"
#include "OpenGL.h"

#include "winlnxdefs.h"
#include "Config.h"
#include "Common.h"


Config config;


static m64p_handle l_ConfigVideoGeneral = NULL;
static m64p_handle l_ConfigVideo = NULL;

struct option_mapping {
    const char *oldconfig;
    const char *newconfig;
    int *cfg;
};

option_mapping option_mappings[] = {
    {"window centre", "WindowCentre", &config.window.centre},
    {"window xpos", "WindowXPos", &config.window.xpos},
    {"window ypos", "WindowYPos", &config.window.ypos},
    {"window width", "WindowWidth", &config.window.width},
    {"window height", "WindowHeight", &config.window.height},

    {"framebuffer enable", "FramebufferEnable", &config.framebuffer.enable},
    {"framebuffer bilinear", "FramebufferBilinear", &config.framebuffer.bilinear},
    {"framebuffer width", "FramebufferWidth", &config.framebuffer.width},
    {"framebuffer height", "FramebufferHeight", &config.framebuffer.height},

    {"video force", "VIForce", &config.video.force},
    {"video width", "VIWidth", &config.video.width},
    {"video height", "VIHeight", &config.video.height},

    {"enable fog", "RenderFog", &config.enableFog},
    {"enable primitive z", "RenderPrimitiveZ", &config.enablePrimZ},
    {"enable lighting", "RenderLighting", &config.enableLighting},
    {"enable alpha test", "RenderAlphaTest", &config.enableAlphaTest},
    {"enable clipping", "RenderClipping", &config.enableClipping},
    {"enable face culling", "RenderFaceCulling", &config.enableFaceCulling},
    {"enable noise", "RenderNoise", &config.enableNoise},

    {"texture 2xSAI", "Texture2xSAI", &config.texture.sai2x},
    {"texture force bilinear", "TextureForceBilinear", &config.texture.forceBilinear},
    {"texture max anisotropy", "TextureForceMaxAnisotropy", &config.texture.maxAnisotropy},
    {"texture use IA", "TextureUseIA", &config.texture.useIA},
    {"texture fast CRC", "TextureFastCRC", &config.texture.fastCRC},
    {"texture pow2", "TexturePow2", &config.texture.pow2},

    {"auto frameskip", "FrameskipAuto", &config.autoFrameSkip},
    {"target FPS", "FrameskipTargetFPS", &config.targetFPS},
    {"frame render rate", "FrameskipRenderRate", &config.frameRenderRate},

    {"update mode", "UpdateMode", &config.updateMode},
    {"ignore offscreen rendering", "IgnoreOffscreenRendering", &config.ignoreOffscreenRendering},
    {"force screen clear", "ForceBufferClear", &config.forceBufferClear},
    {"flip vertical", "FlipVertical", &config.screen.flipVertical},

    {"hack banjo tooie", "HackBanjoTooie", &config.hackBanjoTooie},
    {"hack zelda", "HackZelda", &config.hackZelda},
    {"hack alpha", "HackAlpha", &config.hackAlpha},
    {"hack z", "HackZ", &config.zHack},
};

const int option_mapping_count = sizeof(option_mappings) / sizeof(option_mapping);

void Config_SetOption(const char *config, const char *value)
{
    int i = 0;
    int setting = atoi(value);
    for(i=0;i<option_mapping_count;i++) {
        if(0 == strcmp(config, option_mappings[i].oldconfig) || 0 == strcmp(config, option_mappings[i].newconfig)) {
            DebugMessage(M64MSG_WARNING, "WARNING: using rom database override setting for %s: %d", option_mappings[i].newconfig, setting);
	    *(option_mappings[i].cfg) = setting;
            break;
        }
    }
}

void Config_LoadRomConfig(unsigned char* header)
{
    char line[4096];
    const char *filename;

    // get the name of the ROM
    for (int i=0; i<20; i++) config.romName[i] = header[0x20+i];
config.romName[20] = '\0';
    while (config.romName[strlen(config.romName)-1] == ' ')
    {
    config.romName[strlen(config.romName)-1] = '\0';
    }

    switch(header[0x3e])
    {
        case 0x44: //Germany ('D')
            config.romPAL = true;
            break;
        case 0x45: //USA ('E')
            config.romPAL = false;
            break;
        case 0x4A: //= Japan ('J')
            config.romPAL = false;
            break;
        case 0x50: //= Europe ('P')
            config.romPAL = true;
            break;
        case 0x55: //= Australia ('U')
            config.romPAL = true;
            break;
        default:
            config.romPAL = false;
            break;
    }

    LOG(LOG_MINIMAL, "Rom is %s\n", config.romPAL ? "PAL" : "NTSC");

    filename = ConfigGetSharedDataFilepath("gles2n64rom.conf");
    FILE *f = fopen(filename,"r");
    if (!f)
    {
        LOG(LOG_MINIMAL, "Could not find %s Rom settings file, using global.\n", filename);
        return;
    }
    else
    {
        LOG(LOG_MINIMAL, "[gles2N64]: Searching %s Database for \"%s\" ROM\n", filename, config.romName);
        bool isRom = false;
        while (!feof(f))
        {
            fgets(line, 4096, f);
            if (line[0] == '\n') continue;

            if (strncmp(line,"rom name=", 9) == 0)
            {
                char* v = strchr(line, '\n');
                if (v) *v='\0';
                isRom = (strcasecmp(config.romName, line+9) == 0);
            }
            else
            {
                if (isRom)
                {
                    char* val = strchr(line, '=');
                    if (!val) continue;
                    *val++ = '\0';
                    Config_SetOption(line,val);
                }
            }
        }
    }
}


void Config_LoadConfig()
{
    if(M64ERR_SUCCESS != ConfigOpenSection("Video-General", &l_ConfigVideoGeneral))
    {
        DebugMessage(M64MSG_ERROR, "failed to open 'Video-General' configuration section");
        return;
    }
    if(M64ERR_SUCCESS != ConfigOpenSection("Video-gles2n64", &l_ConfigVideo))
    {
        DebugMessage(M64MSG_ERROR, "failed to open 'Video-gles2n64' configuration section");
        return;
    }

    // Video-General Settings
    // set default options
    ConfigSetDefaultBool(l_ConfigVideoGeneral, "Fullscreen", true, "Use fullscreen mode if True, or windowed mode if False");
    ConfigSetDefaultInt(l_ConfigVideoGeneral, "ScreenWidth", 640, "Width of output window or fullscreen width");
    ConfigSetDefaultInt(l_ConfigVideoGeneral, "ScreenHeight", 480, "Height of output window or fullscreen height");
    ConfigSetDefaultInt(l_ConfigVideoGeneral, "ScreenBPP", 0, "Screen bits per Pixel");
    ConfigSetDefaultBool(l_ConfigVideoGeneral, "VerticalSync", false, "Use vertical sync if True, or not if False");
    // end set default options

    // get settings
    config.window.fullscreen = ConfigGetParamBool(l_ConfigVideoGeneral, "Fullscreen");
    config.screen.width = ConfigGetParamInt(l_ConfigVideoGeneral, "ScreenWidth");
    config.screen.height = ConfigGetParamInt(l_ConfigVideoGeneral, "ScreenHeight");
    config.screen.bpp = ConfigGetParamInt(l_ConfigVideoGeneral, "ScreenBPP");
    config.verticalSync = ConfigGetParamBool(l_ConfigVideoGeneral, "VerticalSync");
    // end get settings
    // end Video-General Settings

    // Video-gles2n64 Settings
    // set default options
    // Window Settings
    ConfigSetDefaultBool(l_ConfigVideo, "WindowCentre", true, "Center the window if True");
    ConfigSetDefaultInt(l_ConfigVideo, "WindowXPos", 0, "X Position of the Window");
    ConfigSetDefaultInt(l_ConfigVideo, "WindowYPos", 0, "Y Position of the Window");
    ConfigSetDefaultInt(l_ConfigVideo, "WindowWidth", 800, "Window Width");
    ConfigSetDefaultInt(l_ConfigVideo, "WindowHeight", 480, "Window Height");

    // Framebuffer Settings
    ConfigSetDefaultBool(l_ConfigVideo, "FramebufferEnable", false, "Enable Framebuffer");
    ConfigSetDefaultBool(l_ConfigVideo, "FramebufferBilinear", false, "Bilinear Framebuffer");
    ConfigSetDefaultInt(l_ConfigVideo, "FramebufferWidth", 400, "Framebuffer Width");
    ConfigSetDefaultInt(l_ConfigVideo, "FramebufferHeight", 200, "Framebuffer Height");
    
    // VI Settings
    ConfigSetDefaultBool(l_ConfigVideo, "VIForce", false, "Force VI");
    ConfigSetDefaultInt(l_ConfigVideo, "VIWidth", 320, "VI Width");
    ConfigSetDefaultInt(l_ConfigVideo, "VIHeight", 240, "VI Height");

    // Render Settings
    ConfigSetDefaultBool(l_ConfigVideo, "RenderFog", false, "Enable Fog");
    ConfigSetDefaultBool(l_ConfigVideo, "RenderPrimitiveZ", true, "Enable primitive z");
    ConfigSetDefaultBool(l_ConfigVideo, "RenderLighting", true, "Enable lighting");
    ConfigSetDefaultBool(l_ConfigVideo, "RenderAlphaTest", true, "Enable alpha test");
    ConfigSetDefaultBool(l_ConfigVideo, "RenderClipping", false, "Enable clipping");
    ConfigSetDefaultBool(l_ConfigVideo, "RenderFaceCulling", true, "Enable face culling");
    ConfigSetDefaultBool(l_ConfigVideo, "RenderNoise", false, "Enable Noise");

    // Texture Settings
    ConfigSetDefaultBool(l_ConfigVideo, "Texture2xSAI", false, "Enable 2xSAI");
    ConfigSetDefaultBool(l_ConfigVideo, "TextureForceBilinear", false, "Force Bilinear");
    ConfigSetDefaultBool(l_ConfigVideo, "TextureForceMaxAnisotropy", false, "Force Maximum Anisotropy");
    ConfigSetDefaultBool(l_ConfigVideo, "TextureUseIA", false, "Use IA");
    ConfigSetDefaultBool(l_ConfigVideo, "TextureFastCRC", true, "Use fast CRC calculation");
    ConfigSetDefaultBool(l_ConfigVideo, "TexturePow2", true, "Use pow2");

    // Frameskip Settings
    ConfigSetDefaultBool(l_ConfigVideo, "FrameskipAuto", false, "Enable automatic Frameskipping");
    ConfigSetDefaultInt(l_ConfigVideo, "FrameskipTargetFPS", 20, "Choose FPS setting");
    ConfigSetDefaultInt(l_ConfigVideo, "FrameskipRenderRate", 1, "Choose frame render rate");

    // Other Settings
    ConfigSetDefaultInt(l_ConfigVideo, "UpdateMode", SCREEN_UPDATE_AT_VI_UPDATE, "Choose the screen update mode");
    ConfigSetDefaultBool(l_ConfigVideo, "IgnoreOffscreenRendering", false, "Ignore offscreen rendering");
    ConfigSetDefaultBool(l_ConfigVideo, "ForceBufferClear", false, "Force buffer clear");
    ConfigSetDefaultBool(l_ConfigVideo, "FlipVertical", false, "Flip Vertical");

    // hack settings
    ConfigSetDefaultBool(l_ConfigVideo, "HackBanjoTooie", false, "Enable Banjo Tooie Hack");
    ConfigSetDefaultBool(l_ConfigVideo, "HackZelda", false, "Enable Zelda Hack");
    ConfigSetDefaultBool(l_ConfigVideo, "HackAlpha", false, "Enable Alpha Hack");
    ConfigSetDefaultBool(l_ConfigVideo, "HackZ", false, "Enable Z Hack");
    // end set default options

    // get settings
    // Window Settings
    config.window.centre = ConfigGetParamBool(l_ConfigVideo, "WindowCentre");
    config.window.xpos = ConfigGetParamInt(l_ConfigVideo, "WindowXPos");
    config.window.ypos = ConfigGetParamInt(l_ConfigVideo, "WindowYPos");
    config.window.width = ConfigGetParamInt(l_ConfigVideo, "WindowWidth");
    config.window.height = ConfigGetParamInt(l_ConfigVideo, "WindowHeight");

    // Framebuffer Settings
    config.framebuffer.enable = ConfigGetParamBool(l_ConfigVideo, "FramebufferEnable");
    config.framebuffer.bilinear = ConfigGetParamBool(l_ConfigVideo, "FramebufferBilinear");
    config.framebuffer.width = ConfigGetParamInt(l_ConfigVideo, "FramebufferWidth");
    config.framebuffer.height = ConfigGetParamBool(l_ConfigVideo, "FramebufferHeight");

    // VI Settings
    config.video.force = ConfigGetParamBool(l_ConfigVideo, "VIForce");
    config.video.width = ConfigGetParamInt(l_ConfigVideo, "VIWidth");
    config.video.height = ConfigGetParamInt(l_ConfigVideo, "VIHeight");

    // Render Settings
    config.enableFog = ConfigGetParamBool(l_ConfigVideo, "RenderFog");
    config.enablePrimZ = ConfigGetParamBool(l_ConfigVideo, "RenderPrimitiveZ");
    config.enableLighting = ConfigGetParamBool(l_ConfigVideo, "RenderLighting");
    config.enableAlphaTest = ConfigGetParamBool(l_ConfigVideo, "RenderAlphaTest");
    config.enableClipping = ConfigGetParamBool(l_ConfigVideo, "RenderClipping");
    config.enableFaceCulling = ConfigGetParamBool(l_ConfigVideo, "RenderFaceCulling");
    config.enableNoise = ConfigGetParamBool(l_ConfigVideo, "RenderNoise");

    // Texture settings
    config.texture.sai2x = ConfigGetParamBool(l_ConfigVideo, "Texture2xSAI");
    config.texture.forceBilinear = ConfigGetParamBool(l_ConfigVideo, "TextureForceBilinear");
    config.texture.maxAnisotropy = ConfigGetParamBool(l_ConfigVideo, "TextureForceMaxAnisotropy");
    config.texture.useIA = ConfigGetParamBool(l_ConfigVideo, "TextureUseIA");
    config.texture.fastCRC = ConfigGetParamBool(l_ConfigVideo, "TextureFastCRC");
    config.texture.pow2 = ConfigGetParamBool(l_ConfigVideo, "TexturePow2");

    // Frameskip Settings
    config.autoFrameSkip = ConfigGetParamBool(l_ConfigVideo, "FrameskipAuto");
    config.targetFPS = ConfigGetParamInt(l_ConfigVideo, "FrameskipTargetFPS");
    config.frameRenderRate = ConfigGetParamInt(l_ConfigVideo, "FrameskipRenderRate");

    // Other Settings
    config.updateMode = ConfigGetParamInt(l_ConfigVideo, "UpdateMode");
    config.ignoreOffscreenRendering = ConfigGetParamBool(l_ConfigVideo, "IgnoreOffscreenRendering");
    config.forceBufferClear = ConfigGetParamBool(l_ConfigVideo, "ForceBufferClear");
    config.screen.flipVertical = ConfigGetParamBool(l_ConfigVideo, "FlipVertical");

    // Hack Settings
    config.hackBanjoTooie = ConfigGetParamBool(l_ConfigVideo, "HackBanjoTooie");
    config.hackZelda = ConfigGetParamBool(l_ConfigVideo, "HackZelda");
    config.hackAlpha = ConfigGetParamBool(l_ConfigVideo, "HackAlpha");
    config.zHack = ConfigGetParamBool(l_ConfigVideo, "HackZ");
    // end get settings
}

