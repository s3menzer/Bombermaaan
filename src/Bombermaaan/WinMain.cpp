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
#include <string>
#include <unistd.h>

namespace
{
    std::string directoryOf(const char* path)
    {
        if (path == NULL || path[0] == '\0')
            return ".";

        const std::string value(path);
        const std::string::size_type slash = value.find_last_of("/\\");
        if (slash == std::string::npos)
            return ".";
        if (slash == 0)
            return "/";
        return value.substr(0, slash);
    }

    bool restoreWorkingDirectory(int argc, char** argv)
    {
        // hbloader passes the selected NRO as argv[1].  The launcher uses
        // that argument to pass the absolute game.nro path to this process,
        // so prefer it over argv[0], which may identify hbmenu instead.
        const char* executablePath = NULL;
        if (argc > 1 && argv != NULL && argv[1] != NULL &&
            argv[1][0] != '\0')
        {
            executablePath = argv[1];
        }
        else if (argc > 0 && argv != NULL && argv[0] != NULL &&
                 argv[0][0] != '\0')
        {
            executablePath = argv[0];
        }

        if (executablePath == NULL)
            return false;

        const std::string directory = directoryOf(executablePath);
        return directory == "." || chdir(directory.c_str()) == 0;
    }
}
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
    fsdevMountSdmc();
    restoreWorkingDirectory(argc, argv);
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
    romfsExit();
#endif
    return true;
}

//******************************************************************************************************************************
//******************************************************************************************************************************
//******************************************************************************************************************************
