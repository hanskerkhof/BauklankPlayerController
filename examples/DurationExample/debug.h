// debug.h
#ifndef DEBUG_H
#define DEBUG_H

/*
This header file contains debug-related configuration settings for the BauklankPlayerController library.
It allows users to control the verbosity of debug output and enable/disable specific debug features.

How to use:
1. Include this file in your project to enable debug features.
2. Adjust the defined values to change debug behavior:
   - PLAYER_CONTROLLER_DEBUG_LEVEL: Sets the verbosity of debug output (higher = more verbose)
   - DISPLAY_PLAYER_STATUS_ENABLED: Enables/disables display of player status on state changes
   - DISPLAY_PLAYER_STATUS_PERIODIC: Enables/disables periodic display of player status

Example:
#include "debug.h"
*/

// Enable or disable debug output for the BauklankPlayerController library
#define BAUKLANK_PLAYER_CONTROLLER_DEBUG true
// Set debug level of the player controller
#define PLAYER_CONTROLLER_DEBUG_LEVEL 1  // or whatever level you want
// Display the the player status when the playerState changes
#define DISPLAY_PLAYER_STATUS_ENABLED true
// Display player status every 5 seconds.
#define DISPLAY_PLAYER_STATUS_PERIODIC true

#endif // DEBUG_H
