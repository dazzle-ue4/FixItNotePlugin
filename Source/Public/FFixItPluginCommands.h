// Copyright (C) 2015 Skunkwerks Kinetic Industries Inc. All Rights Reserved.

#pragma once

#include "FixItPluginPCH.h"

#include "SlateBasics.h"
#include "Commands.h"
#include "EditorStyle.h"


class FFixItPluginCommands : public TCommands<FFixItPluginCommands> {
public:

	FFixItPluginCommands () : TCommands <FFixItPluginCommands> (TEXT ("FixItNote"), NSLOCTEXT("Contexts", "FixItNotePlugin", "FixIt Note"), NAME_None, FEditorStyle::GetStyleSetName()) {
	}

	virtual void RegisterCommands () override;

	TSharedPtr <FUICommandInfo> MyButton;

};