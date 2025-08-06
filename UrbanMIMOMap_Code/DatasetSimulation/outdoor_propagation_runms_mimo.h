#pragma once

#include "../Interface/Engine.h"
#include "../Interface/Convert.h"
#include "../Interface/OutdoorPlugIn.h"
#include "../Interface/OutdoorPlugInPrePro.h"
#include "../Net/Net.h"
#include "../Interface/Init.h"                                                         
#include "../Interface/Free.h"
#include "../Interface/Prepro.h"             
#include "../Public/Interface/Clutter.h"
#include "../Public/Interface/Topo.h"
#include "../SuperposeMS/winprop_superposems.hpp"
#include "../SuperposeMS/winprop_superposems_engine.h"

int _STD_CALL CallbackMessage(const char *Text);
int _STD_CALL CallbackProgress(int value, const char* text);
int _STD_CALL CallbackError(const char *Message, int Mode);
void write_ascii(const WinProp_ResultTrajectoryList* Result, const char* Filename);