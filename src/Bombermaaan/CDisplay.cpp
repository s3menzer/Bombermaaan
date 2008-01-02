/************************************************************************************

    Copyright (C) 2000-2002, 2007 Thibaut Tollemer, Bernd Arnold

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


///////////////////////
// CDisplay.cpp

#include "stdafx.h"
#include "CDisplay.h"
#include "CDirectDraw.h"


//******************************************************************************************************************************
//******************************************************************************************************************************
//******************************************************************************************************************************

CDisplay::CDisplay (void)
{
    // No connection to the resources yet
    m_hModule = NULL;

    // Reset view origin
    m_ViewOriginX = 0;
    m_ViewOriginY = 0;
}

//******************************************************************************************************************************
//******************************************************************************************************************************
//******************************************************************************************************************************

CDisplay::~CDisplay (void)
{
    // Nothing to do
}

//******************************************************************************************************************************
//******************************************************************************************************************************
//******************************************************************************************************************************

bool CDisplay::Create (int Width, int Height, bool FullScreen)
{
    // Check if we have a connection with the resources
    ASSERT (m_hModule != NULL);
    
    int Depth = (FullScreen ? 16 : 0);
    
    // If no display mode has been set yet or the current display mode is not the right one
    if (!m_DirectDraw.IsModeSet (Width, Height, Depth, FullScreen))
    {
        // Destroy DirectDraw interface and the sprite tables
        Destroy ();        
                
        // If DirectDraw object creation failed
        if (!m_DirectDraw.Create (Width, Height, Depth, FullScreen))
        {
            // Get out
            return false;
        }

        // Set the RGB color for transparent pixels in sprites
        // If it failed
        if (!m_DirectDraw.SetTransparentColor (0, 255, 0))
        {
            // Get out, it failed
            return false;
        }
        

        // Load the sprite tables. If at least one sprite table could not be loaded
        if (
#ifdef USE_32_PIXELS_PER_BLOCK
            !LoadSprites (2,      1,      32,     32,     false,    BMP_ARENA_FLOOR               ) ||
            !LoadSprites (7,      1,      32,     32,     true,     BMP_ARENA_WALL                ) ||
            !LoadSprites (28,     1,      32,     32,     true,     BMP_ARENA_FLAME               ) ||
            !LoadSprites (14,     1,      32,     32,     false,    BMP_ARENA_ITEM                ) ||
            !LoadSprites (3,      1,      32,     32,     true,     BMP_ARENA_BOMB                ) ||
            !LoadSprites (12,     6,      42,     44,     true,     BMP_ARENA_BOMBER_WALK         ) ||
            !LoadSprites (7,      1,      52,     54,     true,     BMP_ARENA_FIRE                ) ||
            !LoadSprites (12,     6,      42,     44,     true,     BMP_ARENA_BOMBER_WALK_HOLD    ) ||
            !LoadSprites (4,      1,      32,     32,     true,     BMP_ARENA_FLY                 ) ||
#else
            !LoadSprites (2,      1,      16,     16,     false,    BMP_ARENA_FLOOR               ) ||
            !LoadSprites (7,      1,      16,     16,     true,     BMP_ARENA_WALL                ) ||
            !LoadSprites (28,     1,      16,     16,     true,     BMP_ARENA_FLAME               ) ||
            !LoadSprites (14,     1,      16,     16,     false,    BMP_ARENA_ITEM                ) ||
            !LoadSprites (3,      1,      16,     16,     true,     BMP_ARENA_BOMB                ) ||
            !LoadSprites (12,     6,      21,     22,     true,     BMP_ARENA_BOMBER_WALK         ) ||
            !LoadSprites (7,      1,      26,     27,     true,     BMP_ARENA_FIRE                ) ||
            !LoadSprites (12,     6,      21,     22,     true,     BMP_ARENA_BOMBER_WALK_HOLD    ) ||
            !LoadSprites (4,      1,      16,     16,     true,     BMP_ARENA_FLY                 ) ||
#endif
            !LoadSprites (1,      1,      240,    26,     false,    BMP_BOARD_BACKGROUND          ) ||
            !LoadSprites (12,     1,      7,      10,     true,     BMP_BOARD_TIME                ) || // 10
            !LoadSprites (2,      1,      15,     7,      true,     BMP_BOARD_CLOCK_TOP           ) ||
            !LoadSprites (8,      1,      15,     13,     true,     BMP_BOARD_CLOCK_BOTTOM        ) ||
            !LoadSprites (6,      1,      6,      8,      true,     BMP_BOARD_SCORE               ) ||
            !LoadSprites (5,      2,      14,     14,     true,     BMP_BOARD_HEADS               ) ||
            !LoadSprites (1,      1,      240,    234,    false,    BMP_DRAWGAME_MAIN             ) ||
            !LoadSprites (2,      1,      34,     48,     false,    BMP_DRAWGAME_FLAG             ) ||
            !LoadSprites (4,      1,      10,     31,     true,     BMP_DRAWGAME_FUMES            ) ||
            !LoadSprites (4,      5,      24,     32,     true,     BMP_WINNER_BOMBER             ) ||
            !LoadSprites (1,      1,      64,     32,     false,    BMP_WINNER_BACKGROUND_1       ) ||
            !LoadSprites (16,     1,      22,     22,     true,     BMP_WINNER_COIN               ) || // 20
            !LoadSprites (4,      1,      6,      6,      true,     BMP_WINNER_LIGHTS             ) ||
            !LoadSprites (4,      2,      16,     16,     true,     BMP_WINNER_SPARKS             ) ||
            !LoadSprites (1,      1,      158,    16,     true,     BMP_WINNER_TITLE              ) ||
            !LoadSprites (1,      1,      32,     135,    false,    BMP_VICTORY_WALL              ) ||
            !LoadSprites (9,      1,      14,     16,     true,     BMP_VICTORY_CROWD             ) ||
            !LoadSprites (14,     5,      36,     61,     true,     BMP_VICTORY_BOMBER            ) ||
            !LoadSprites (1,      1,      192,    60,     true,     BMP_VICTORY_TITLE             ) ||
            !LoadSprites (46,     6,      10,     10,     true,     BMP_GLOBAL_FONT               ) ||
            !LoadSprites (1,      1,      64,     32,     false,    BMP_MENU_BACKGROUND_1         ) ||
            !LoadSprites (5,      2,      21,     19,     true,     BMP_MENU_BOMBER               ) || // 30
            !LoadSprites (1,      1,      210,    181,    true,     BMP_MENU_FRAME_1              ) ||
            !LoadSprites (2,      1,      15,     16,     true,     BMP_MENU_HAND                 ) ||
            !LoadSprites (5,      1,      23,     23,     true,     BMP_WINNER_CROSS              ) ||
            !LoadSprites (5,      5,      14,     15,     true,     BMP_VICTORY_CONFETTIS_LARGE   ) ||
            !LoadSprites (5,      5,      13,     14,     true,     BMP_VICTORY_CONFETTIS_MEDIUM  ) ||
            !LoadSprites (5,      5,      10,     10,     true,     BMP_VICTORY_CONFETTIS_SMALL   ) ||
            !LoadSprites (1,      1,      69,     16,     true,     BMP_PAUSE                     ) ||
            !LoadSprites (1,      1,     105,     16,     true,     BMP_HURRY                     ) ||
            !LoadSprites (1,      1,     154,     93,     true,     BMP_MENU_FRAME_2              ) ||
            !LoadSprites (3,      4,      16,     16,     true,     BMP_ARENA_FUMES               ) || // 40
            !LoadSprites (1,      1,      14,     14,     true,     BMP_BOARD_DRAWGAME            ) ||
            !LoadSprites (1,      1,     240,    234,     false,    BMP_TITLE_BACKGROUND          ) ||
            !LoadSprites (1,      1,     240,    126,     true,     BMP_TITLE_BOMBERS             ) ||
            !LoadSprites (1,      1,     168,     77,     true,     BMP_TITLE_TITLE               ) ||
            !LoadSprites (2,      5,      64,     13,     true,     BMP_TITLE_MENU_ITEMS          ) ||
            !LoadSprites (1,      1,      64,     32,     false,    BMP_CONTROLS_BACKGROUND_1     ) ||
            !LoadSprites (1,      1,      82,     41,     false,    BMP_CONTROLS_BACKGROUND_2     ) ||
            !LoadSprites (1,      1,      82,     41,     false,    BMP_CONTROLS_BACKGROUND_3     ) ||
            !LoadSprites (1,      1,      82,     41,     false,    BMP_MENU_BACKGROUND_2         ) ||
            !LoadSprites (1,      1,      82,     41,     false,    BMP_MENU_BACKGROUND_3         ) || // 50
            !LoadSprites (1,      1,      82,     41,     false,    BMP_WINNER_BACKGROUND_2       ) ||
            !LoadSprites (1,      1,      82,     41,     false,    BMP_WINNER_BACKGROUND_3       ) ||
            !LoadSprites (1,      1,     138,     46,     true,     BMP_TITLE_CLOUD_1             ) ||
            !LoadSprites (1,      1,     106,     46,     true,     BMP_TITLE_CLOUD_2             ) ||
            !LoadSprites (1,      1,      66,     22,     true,     BMP_TITLE_CLOUD_3             ) ||
            !LoadSprites (4,      1,       8,      8,     false,    BMP_LEVEL_MINI_TILES          ) ||
            !LoadSprites (5,      1,      12,     10,     true,     BMP_LEVEL_MINI_BOMBERS        ) ||
#ifdef USE_32_PIXELS_PER_BLOCK
            !LoadSprites (7,      5,      42,     44,     true,     BMP_ARENA_BOMBER_DEATH        ) ||
            !LoadSprites (12,     6,      42,     44,     true,     BMP_ARENA_BOMBER_LIFT         ) || 
            !LoadSprites (20,     6,      42,     44,     true,     BMP_ARENA_BOMBER_THROW        ) || // 60
            !LoadSprites (8,      6,      42,     44,     true,     BMP_ARENA_BOMBER_PUNCH        ) ||
            !LoadSprites (4,      6,      42,     44,     true,     BMP_ARENA_BOMBER_STUNT        )
#else
            !LoadSprites (7,      5,      21,     22,     true,     BMP_ARENA_BOMBER_DEATH        ) ||
            !LoadSprites (12,     6,      21,     22,     true,     BMP_ARENA_BOMBER_LIFT         ) || 
            !LoadSprites (20,     6,      21,     22,     true,     BMP_ARENA_BOMBER_THROW        ) || // 60
            !LoadSprites (8,      6,      21,     22,     true,     BMP_ARENA_BOMBER_PUNCH        ) ||
            !LoadSprites (4,      6,      21,     22,     true,     BMP_ARENA_BOMBER_STUNT        )
#endif
           )
        {
            // Failure, get out (error is logged by the LoadSprites() method)
            return false;
        }

        // Save origin where to draw from
        m_ViewOriginX = (Width - VIEW_WIDTH) / 2;
        m_ViewOriginY = (Height - VIEW_HEIGHT) / 2;

        m_DirectDraw.SetOrigin (m_ViewOriginX, m_ViewOriginY);
    }

    // Everything went right
    return true;
}

//******************************************************************************************************************************
//******************************************************************************************************************************
//******************************************************************************************************************************

bool CDisplay::Create (EDisplayMode DisplayMode)
{
    ASSERT (DisplayMode != DISPLAYMODE_NONE);
    
    // According to the new display mode to set
    switch (DisplayMode)
    {
        case DISPLAYMODE_FULL1    : return Create (320, 240, true);
        case DISPLAYMODE_FULL2    : return Create (512, 384, true);
        case DISPLAYMODE_FULL3    : return Create (640, 480, true);
        case DISPLAYMODE_WINDOWED : return Create (VIEW_WIDTH, VIEW_HEIGHT, false);
    }

    // Should never happen
    return false;
}

//******************************************************************************************************************************
//******************************************************************************************************************************
//******************************************************************************************************************************

void CDisplay::Destroy (void)
{
    // Destroy DirectDraw interface and the sprite tables
    m_DirectDraw.Destroy ();
}

//******************************************************************************************************************************
//******************************************************************************************************************************
//******************************************************************************************************************************

bool CDisplay::IsDisplayModeAvailable (EDisplayMode DisplayMode)
{
    ASSERT (DisplayMode != DISPLAYMODE_NONE);

    // According to the display mode to test
    switch (DisplayMode)
    {
        case DISPLAYMODE_FULL1    : return m_DirectDraw.IsModeAvailable (320, 240, 16);
        case DISPLAYMODE_FULL2    : return m_DirectDraw.IsModeAvailable (512, 384, 16);
        case DISPLAYMODE_FULL3    : return m_DirectDraw.IsModeAvailable (640, 480, 16);
        case DISPLAYMODE_WINDOWED : return true;
    }

    // Should never happen
    return false;
}

//******************************************************************************************************************************
//******************************************************************************************************************************
//******************************************************************************************************************************

bool CDisplay::LoadSprites (int SpriteTableWidth, 
                            int SpriteTableHeight, 
                            int SpriteWidth, 
                            int SpriteHeight, 
                            bool Transparent, 
                            int BMP_ID)
{
    // Load the bitmap as a resource
    HBITMAP hBitmap = (HBITMAP) LoadImage (m_hModule, MAKEINTRESOURCE(BMP_ID), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);

    // If it failed
    if (hBitmap == NULL)
    {
        // Log failure
        theLog.WriteLine ("Display         => !!! Could not load resource image and create handle to bitmap.");
        theLog.LogLastError ();

        // Get out
        return false;
    }

    // Create the sprites by giving the sprite table information and the handle to the bitmap.
    // If it fails
    if (!m_DirectDraw.LoadSprites (SpriteTableWidth, 
                                   SpriteTableHeight, 
                                   SpriteWidth, 
                                   SpriteHeight, 
                                   Transparent, 
                                   hBitmap))
    {
        // Get out, failure
        return false;
    }

    // We no longer need the hBitmap so delete it
    // If it fails
    if (DeleteObject (hBitmap) == 0)
    {
        // Log failure
        theLog.WriteLine ("Display         => !!! Could not delete handle to bitmap.");
        theLog.LogLastError ();

        // Get out, failure
        return false;
    }

    // Everything went right
    return true;
}

//******************************************************************************************************************************
//******************************************************************************************************************************
//******************************************************************************************************************************

