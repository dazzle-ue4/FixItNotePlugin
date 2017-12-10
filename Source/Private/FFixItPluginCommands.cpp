// Copyright (C) 2015 Skunkwerks Kinetic Industries Inc. All Rights Reserved.

#include "FixItPluginPCH.h"

#include "FFixItPluginCommands.h"
 
#define LOCTEXT_NAMESPACE "FFixItNotePluginCommands"

PRAGMA_DISABLE_OPTIMIZATION
void FFixItPluginCommands::RegisterCommands () {
	UI_COMMAND (MyButton, "FixIt Note", "FixIt Note Blueprint", EUserInterfaceActionType::Button, FInputGesture ());
}
PRAGMA_ENABLE_OPTIMIZATION

#undef LOCTEXT_NAMESPACE

