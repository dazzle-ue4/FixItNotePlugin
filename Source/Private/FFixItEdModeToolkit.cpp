// Copyright (C) 2015 Skunkwerks Kinetic Industries Inc. All Rights Reserved.

#include "FixItPluginPCH.h"
#include "UnrealEd.h"
#include "FFixItEdModeToolkit.h"

#include "FixItWidget.h"

#define LOCTEXT_NAMESPACE "FFixItEdModeToolkit"


void FFixItEdModeToolkit::RegisterTabSpawners (const TSharedRef<class FTabManager>& TabManager) {
    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----FFixItEdModeToolkit::RegisterTabSpawners"));
}

void FFixItEdModeToolkit::UnregisterTabSpawners (const TSharedRef<class FTabManager>& TabManager) {

}

void FFixItEdModeToolkit::Init (const TSharedPtr <class IToolkitHost> &InitToolkitHost) {
    FixItEdWidget = SNew (SFixItWidget);

    FModeToolkit::Init (InitToolkitHost);
}

FName FFixItEdModeToolkit::GetToolkitFName () const {
    return FName ("FixItNoteEditMode");
}

FText FFixItEdModeToolkit::GetBaseToolkitName () const {
    return FText::FromString (TEXT ("FixItNoteEditMode"));
}

class FEdMode* FFixItEdModeToolkit::GetEditorMode() const {
    return GLevelEditorModeTools ().GetActiveMode (FFixItEdMode::EM_FixIt);
}

TSharedPtr <SWidget> FFixItEdModeToolkit::GetInlineContent () const {
    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----FFixItEdModeToolkit::GetInlineContent () Returning the widget to draw the content for this tab"));

    return FixItEdWidget;
}

void FFixItEdModeToolkit::PostUndo () {
    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----FFixItEdModeToolkit::PostUndo ()"));

    // When an undo relates to the StickyNote Edit mode, refresh the list.
    //StickyNoteEdWidget->RefreshFullList();
}

#undef LOCTEXT_NAMESPACE