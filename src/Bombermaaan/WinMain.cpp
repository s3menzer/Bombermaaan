/************************************************************************************

    Copyright (C) 2000-2002, 2007 Thibaut Tollemer

    This file is part of Bombermaaan.

    Bombermaaan is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Bombermaaan is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Bombermaaan.  If not, see <http://www.gnu.org/licenses/>.

************************************************************************************/


/**
 *  \file WinMain.cpp
 *  \brief Entry point of the program for both Linux and Windows
 */

#include "STDAFX.H"
#include "CGame.h"

#ifdef BOMBERMAAAN_SWITCH
#include <switch.h>
#include "updater.hpp"
#include "version.hpp"
#endif

/**
 *  \mainpage Bombermaaan source code documentation
 *
 *  This is the source code documentation of Bombermaaan created by Doxygen.
 *
 *  Bombermaaan is a classic Bomberman game, also similar to Dynablaster.
 */


//******************************************************************************************************************************
//******************************************************************************************************************************
//******************************************************************************************************************************

/**
 *  \brief This is the main function of the executable file.
 */

#ifdef WIN32
int APIENTRY WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdline, int nCmdShow)
#else
int main (int argc, char **argv)
#endif
{
#ifdef BOMBERMAAAN_SWITCH
    updater::Config updateConfig;
    updateConfig.applicationName = "Bombermaaan";
    updateConfig.configFileBaseName = "bombermaaan";
    updateConfig.currentVersion = BOMBERMAAAN_VERSION_STRING;
    updateConfig.currentBuild = BOMBERMAAAN_BUILD_NUMBER;
    updateConfig.defaultManifestUrl = BOMBERMAAAN_DEFAULT_MANIFEST_URL;
    updateConfig.executablePath =
        (argc > 0 && argv != NULL && argv[0] != NULL) ? argv[0] : "";
    updateConfig.targetPath = "sdmc:/switch/Bombermaaan.nro";
    updateConfig.platform = "switch";

    // The updater mounts the SD card itself and runs before ROMFS/SDL
    // initialization, just like the Cross//Roads implementation.
    updater::run(updateConfig);

    if (romfsInit() != 0)
        return false;
#endif

#ifndef WIN32
    HINSTANCE hInstance = 0;
    char **lpCmdline = argv;
#endif
	
    // Create the CGame instance    
    CGame Game (hInstance, lpCmdline);

    // If creating the game failed
#ifdef WIN32
	if (!Game.Create(lpCmdline))
#else
	if (!Game.Create(lpCmdline, argc))
#endif
    {
        // Get out, failure
#ifdef BOMBERMAAAN_SWITCH
        romfsExit();
#endif
        return false;
    }

    //! Show the game window (CGame::ShowWindow())
    Game.ShowWindow ();

    // Update the game (CGame::MesagePump())
    Game.MessagePump ();

    // Destroy everything (CGame::Destroy())
    Game.Destroy ();

    // Everything went right
#ifdef BOMBERMAAAN_SWITCH
    // A running NRO cannot always be replaced immediately.  Running the
    // updater once more on exit lets it install the update or leave a
    // .pending file for the next launch.
    updater::run(updateConfig);
    romfsExit();
#endif
    return true;
}

//******************************************************************************************************************************
//******************************************************************************************************************************
//******************************************************************************************************************************
