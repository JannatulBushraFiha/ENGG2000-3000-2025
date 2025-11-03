// state.h
#pragma once
#include <Arduino.h>

enum SystemMode : uint8_t { MODE_AUTO, MODE_MANUAL };
enum BridgeCmd  : uint8_t { CMD_IDLE, CMD_OPEN, CMD_CLOSE, CMD_STOP };
enum MarineStatus: uint8_t {MARINE_DETECTED, MARINE_PASSING, MARINE_DEPARTED, MARINE_CLEAR }; 

// Defined ONCE in your main .ino:
extern SystemMode g_mode;

// Distance shared across modules (DEFINED in main)
extern float g_distance_cm;
extern volatile BridgeCmd  g_cmd_manual;  // set by UI handlers
extern volatile BridgeCmd  g_cmd_auto;
extern volatile bool g_emergency;   // <-- NEW

//Marine Status 
extern volatile MarineStatus g_marine_status; 