// Copyright (C) 2015 Skunkwerks Kinetic Industries Inc. All Rights Reserved.

#include "FixItPluginPCH.h"

#include "UnrealEd.h"
#include "ToolkitManager.h"
#include "FFixItEdMode.h"
#include "FixItWidget.h"
#include "FFixItEdModeToolkit.h"
#include "SharedPointer.h"

#include "Editor/MainFrame/Public/Interfaces/IMainFrameModule.h"
//#include "Editor/MainFrame/Public/MainFrame.h"


#define XmlFileName "FixItNotes."
#define XmlFileExtension ".xml"
#define SAVE_TO_XML_FILE__DONT_SAVE_IN_LEVEL        1


static const FName MainFrameModuleName ("MainFrame");

const FEditorModeID FFixItEdMode::EM_FixIt (TEXT ("EM_FixItNote"));
FFixItEdMode *FFixItEdMode::edMode = NULL;

// Static variables:
ListItemMap FFixItEdMode::FixItNoteMap;
TArray<FFixItNoteListItemPtr> FFixItEdMode::FixItNotes;        // A list of the stickyNotes that are visible in the world at the moment

//TArray<AFixItNote *> FFixItEdMode::stickysNotInWorld;  // A list of all the StickyNotes that are currently hidden or filtered out of the world
bool FFixItEdMode::startupComplete = false;

FFixItEdMode::FFixItEdMode() : FEdMode () {
    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----FFixItEdMode::FFixItEdMode"));
    ClearStickyNotes ();

    edMode = this;

    // Don't erase the static variables that are already set before the editor mode is created!
    //stickysNotInWorld.Empty ();      // A list of all the StickyNotes that are currently hidden or filtered out of the world
}
FFixItEdMode::~FFixItEdMode () {
    edMode = NULL;

    FEditorDelegates::MapChange.RemoveAll (this);
}

void FFixItEdMode::AddReferencedObjects (FReferenceCollector &Collector) {
    // Call parent implementation
    FEdMode::AddReferencedObjects (Collector);
}

// This is called when they click on our tab, which selects our mode
void FFixItEdMode::Enter () {
    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----FFixItEdMode::Enter..."));

    FEdMode::Enter ();

    // Create our tookit and register it.  It's called (GetInlineContent), which returns the widget that draws the contents of our tab
    if (!Toolkit.IsValid ()) {
        Toolkit = MakeShareable (new FFixItEdModeToolkit);
        Toolkit->Init (Owner->GetToolkitHost ());
    }

// TODO: Instead call FilterStickyNotesInWorld and pass in the selected priority
RestoreAllStickyNotes ();
}

// Called when they switch away from our tab/mode
void FFixItEdMode::Exit () {
    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----FFixItEdMode::Exit..."));

    HideAllNotes (GetWorld ());

    FToolkitManager::Get ().CloseToolkit (Toolkit.ToSharedRef ());
    Toolkit.Reset ();

    // Call base Exit method to ensure proper cleanup
    FEdMode::Exit ();
}


// Does the priority match the filter?
bool PriorityMatchesFilter(EFixItNotePriority priority, EFixItNotePriorityFilter filter) {
	if (filter == EFixItNotePriorityFilter::All) {
		return true;
	}
	if (filter == EFixItNotePriorityFilter::Urgent && priority == EFixItNotePriority::Urgent) {
		return true;
	}
	if (filter == EFixItNotePriorityFilter::Moderate && priority == EFixItNotePriority::Moderate) {
		return true;
	}
	if (filter == EFixItNotePriorityFilter::Low && priority == EFixItNotePriority::Low) {
		return true;
	}

	return false;
}


// Remove Sticky Notes from the world that have a different priority than the one passed in
void FFixItEdMode::FilterStickyNotesInWorld (EFixItNotePriorityFilter filter) {
    ClearStickyNotes ();

    // Loop through all actors in the level, finding all the sticky notes
    TArray <AActor *> actors = GWorld->GetCurrentLevel ()->Actors;
    for (int i = 0; i < actors.Num (); i++) {
        AActor *a = actors [i];
        if (a && a->IsA (AFixItNote::StaticClass ())) {    //  Strangely, once one of the actors was null

            // It's a StickyNote all right, but is it one we want filtered out?
            AFixItNote *sn = (AFixItNote *) a;
            if (!PriorityMatchesFilter(sn->Priority, filter)) {
                UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----FFixItEdMode::FilterStickyNotesInWorld Removing FixIt note."));

                a->SetIsTemporarilyHiddenInEditor (true);
                //a->K2_DestroyActor ();

                //stickysNotInWorld.Add (sn);      // Save each one we remove from the world.  Then we can restore them later if the filter changes.
            } else {
                a->SetIsTemporarilyHiddenInEditor (false);
                sn->Update ();

                AddStickyNote(sn);
            }
        }
    }

//    // Also see if we should redisplay any that had been filtered out before:
//    for (int s = 0; s < stickysNotInWorld.Num (); s++) {
//        AFixItNote *sn = stickysNotInWorld [s];
//        if (sn->Priority <= priority) {
//            UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----FFixItEdMode::FilterStickyNotesInWorldStatic Redisplaying sticky note that had been filtered out."));
//
//            AFixItNote *snew = GWorld->SpawnActor <AFixItNote> (sn->GetActorLocation (), FRotator (0, 0, 0));
//            snew->Priority = sn->Priority;
//            snew->Text = sn->Text;
//            snew->Update ();
//
//            stickysNotInWorld.RemoveAt (s);
//            s--;    // So we don't skip one
//
//            StickyNotes.Add (snew);
//        }
//    }

    FFixItEdMode::RotateTextToCameraStatic ();
}

// Hide all the sticky notes when we leave our editor mode
void FFixItEdMode::HideAllNotes (UWorld *world) {
    ClearStickyNotes ();

    // Loop through all actors in the level, finding all the sticky notes
    TArray <AActor *> actors = world->GetCurrentLevel ()->Actors;
    for (int i = 0; i < actors.Num (); i++) {
        AActor *a = actors [i];
        if (a && a->IsA (AFixItNote::StaticClass ())) {    //  Strangely, once one of the actors was null
            UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----FFixItEdMode::HideAllNotes Removing FixIt note"));

            world->GetCurrentLevel ()->Actors [i]->SetIsTemporarilyHiddenInEditor (true);
            //a->K2_DestroyActor ();

            //AFixItNote *sn = (AFixItNote *) a;
            //stickysNotInWorld.Add (sn);      // Save each one we remove from the world.  Then we can restore them later if the filter changes.
        }
    }
}

// Set StickyNotes list to contain all that are now in the World.  The list is used to display them on the editor mode panel
// Never called?
void FFixItEdMode::InitStickyNotes () {
    ClearStickyNotes ();
    // Loop through all actors in the level, finding all the sticky notes
    TArray <AActor *> actors = GWorld->GetCurrentLevel ()->Actors;
    for (int i = 0; i < actors.Num (); i++) {
        AActor *a = actors [i];
        if (a && a->IsA (AFixItNote::StaticClass ())) {
            AddStickyNote((AFixItNote *)a);      // Save each one we remove from the world.  Then we can restore them later if the filter changes.
        }
    }
}

// Sets all sticky notes to be visible in the editor
void FFixItEdMode::RestoreAllStickyNotes () {
    ClearStickyNotes ();

    TArray <AActor *> actors = GWorld->GetCurrentLevel ()->Actors;
    for (int i = 0; i < actors.Num (); i++) {
        AActor *a = actors [i];
        if (a && a->IsA (AFixItNote::StaticClass ())) {    //  Strangely, once one of the actors was null

            AFixItNote *sn = (AFixItNote *) a;

            bool valid = sn->IsValidLowLevel ();
            if (!valid)
                UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----FFixItEdMode::RestoreAllStickyNotes problem!"));

            if (valid) {
                a->SetIsTemporarilyHiddenInEditor (false);
                sn->Update ();

                AddStickyNote(sn);
            }
        }
    }
    //stickysNotInWorld.Empty ();


    // Orient the text correctly for all these newly created sticky notes
    RotateTextToCamera ();
}


// Called at startup, but called repeatedly so I use startupComplete flag
void FFixItEdMode::StartupHook (UWorld *world) {
    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----FFixItEdMode::StartupHook"));

    if (!startupComplete) {
        UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----FFixItEdMode::StartupHook - First time"));

        startupComplete = true;

        LoadFromXMLFile ();

        FFixItEdMode::HideAllNotes (world);
    }
}

// Called at shutdown, before anything has been destroyed or saved
void FFixItEdMode::PreShutdownHook () {
    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----FFixItEdMode::PreShutdownHook"));

    SaveToXMLFile ();
}

// Called at shutdown, after the level is closed
void FFixItEdMode::ShutdownHook () {
    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----FFixItEdMode::ShutdownHook"));
}

void FFixItEdMode::PreSaveHook () {
    CopyNoteInfo ();
}
void FFixItEdMode::PostSaveHook () {
    SaveInfoToXMLFile ();
    LoadFromXMLFile ();

	// Filter the search results list, which will now include all notes.  This is messy, using a couple of static things, but should work:
	FilterStickyNotesInWorld (SFixItWidget::ViewPriority);
}
void FFixItEdMode::MapOpenedHook() {
    LoadFromXMLFile();
}
//void FFixItEdMode::AssetsDeletedHook (const TArray<UClass*>& DeletedAssetClasses) {
////    for (auto DeletedAssetClass : DeletedAssetClasses) {
//
//    InitStickyNotes ();
//}
//void FFixItEdMode::AssetsDeleted (const TArray<UClass*>& DeletedAssetClasses) {
//    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----FixItPlugin::AssetsDeleted..."));
//
//    FFixItEdMode::AssetsDeletedHook (DeletedAssetClasses);
//}

// Save the StickyNote info for the current level
TArray<TSharedRef<FixItNoteInfo>> FFixItEdMode::savedNotes;

// Copy the information for the StickyNotes on the current level
// Remove the original notes if notes are not being saved in the level
void FFixItEdMode::CopyNoteInfo () {
    if ((GWorld == NULL) || (GWorld->GetCurrentLevel () == NULL)) {
        UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----FFixItEdMode::CopyNoteInfo couldn't copy FixIt notes because level is already gone"));
        return;
    }

    FFixItEdMode::savedNotes.Empty ();

    // Loop through all actors in the level, finding all the sticky notes
    TArray <AActor *> actors = GWorld->GetCurrentLevel ()->Actors;
    for (int i = 0; i < actors.Num (); i++) {
        AActor *a = actors[i];
        if (a && a->IsA(AFixItNote::StaticClass ())) {    //  Strangely, once one of the actors was null
            AFixItNote *sn = (AFixItNote *) a;

            TSharedRef<FixItNoteInfo> note = MakeShareable (new FixItNoteInfo ());
            note->Priority = sn->Priority;
            note->Recipient = sn->Recipient;
            note->Text = sn->Text;
            note->SearchTags = sn->SearchTags;
            note->X = sn->GetActorLocation ().X;
            note->Y = sn->GetActorLocation ().Y;
            note->Z = sn->GetActorLocation ().Z;

            FFixItEdMode::savedNotes.Add (note);

#ifdef SAVE_TO_XML_FILE__DONT_SAVE_IN_LEVEL
            a->K2_DestroyActor ();
#endif
        }
    }
}

// Writes out the savedNotes list to FixItNotes.xml.  
// If we don't specify a path, it goes into the same folder as the engine exe: Program Files\Epic Games\4.8\Engine\Binaries\Win64
// But I'm using the GamePluginsDir as the default path, so we can have one per project
void FFixItEdMode::SaveInfoToXMLFile () {
	FString filename = PathToSaveXml();
	if (filename != "")
	{
		bool doesWorldHaveNotes = FFixItEdMode::savedNotes.Num() > 0;

		if (doesWorldHaveNotes)
		{
			FString XmlText = "<fixitnotes>\r\n<fixitversion>1.0.0</fixitversion>\r\n";

			// Loop through all actors in the level, finding all the sticky notes
			for (int i = 0; i < FFixItEdMode::savedNotes.Num(); i++) {
				TSharedRef <FixItNoteInfo> note = FFixItEdMode::savedNotes[i];

				XmlText += "\t<fixitnote>\r\n";
				XmlText += FString::Printf(TEXT("\t\t<text>%s</text>\r\n"), *note->Text.ToString());
				XmlText += FString::Printf(TEXT("\t\t<priority>%d</priority>\r\n"), (uint8)note->Priority);
				XmlText += FString::Printf(TEXT("\t\t<x>%f</x><y>%f</y><z>%f</z>\r\n"), note->X, note->Y, note->Z);
				XmlText += FString::Printf(TEXT("\t\t<recipient>%s</recipient>\r\n"), *note->Recipient.ToString());
				XmlText += FString::Printf(TEXT("\t\t<tags>%s</tags>\r\n"), *note->SearchTags.ToString());
				XmlText += "\t</fixitnote>\r\n";
			}

			XmlText += "</fixitnotes>\r\n";

			if (FFileHelper::SaveStringToFile(XmlText, *filename))
			{
				UE_LOG(SkunkLog_FixItNote, Log, TEXT("-----FFixItEdMode::SaveInfoToXMLFile wrote to XML file: %s"), *filename);
			}
			else
			{
				UE_LOG(SkunkLog_FixItNote, Log, TEXT("-----FFixItEdMode::SaveInfoToXMLFile could not write to XML file: %s"), *filename);
			}
		}
		else if (!doesWorldHaveNotes)
		{
			if (!FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*filename))
			{
				UE_LOG(SkunkLog_FixItNote, Log, TEXT("-----FFixItEdMode::SaveInfoToXMLFile could not delete XML file: %s"), *filename);
			}
		}
	}
}

// Writes out the StickyNote list to StickyNotes.xml.  
// If we don't specify a path, it goes into the same folder as the engine exe: Program Files\Epic Games\4.8\Engine\Binaries\Win64
// But I'm using the folder where the dll loaded from, as the default path, so we can have one per project
void FFixItEdMode::SaveToXMLFile () {
    if ((GWorld == NULL) || (GWorld->GetCurrentLevel () == NULL)) {
        UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----FFixItEdMode::SaveToXMLFile couldn't save fixit notes because level is already gone"));
        return;
    }

	FString filename = PathToSaveXml();

	if (filename != "")
	{
		bool doesWorldHaveNotes = false;
		{
			TArray <AActor *> actors = GWorld->GetCurrentLevel()->Actors;
			for (int i = 0; i < actors.Num(); i++) {
				AActor *a = actors[i];
				if (a && a->IsA(AFixItNote::StaticClass())) {
					doesWorldHaveNotes = true;
				}
			}
		}


		if (doesWorldHaveNotes)
		{
			FString XmlText = "<fixitnotes>\r\n<fixitversion>1.0.0</fixitversion>\r\n";

			// Loop through all actors in the level, finding all the sticky notes
			TArray <AActor *> actors = GWorld->GetCurrentLevel()->Actors;
			for (int i = 0; i < actors.Num(); i++) {
				AActor *a = actors[i];
				if (a && a->IsA(AFixItNote::StaticClass())) {    //  Strangely, once one of the actors was null
					AFixItNote *sn = (AFixItNote *)a;

					XmlText += "\t<fixitnote>\r\n";
					XmlText += FString::Printf(TEXT("\t\t<text>%s</text>\r\n"), *sn->Text.ToString());
					XmlText += FString::Printf(TEXT("\t\t<priority>%d</priority>\r\n"), (uint8)sn->Priority);
					XmlText += FString::Printf(TEXT("\t\t<x>%f</x><y>%f</y><z>%f</z>\r\n"), sn->GetActorLocation().X, sn->GetActorLocation().Y, sn->GetActorLocation().Z);
					XmlText += FString::Printf(TEXT("\t\t<recipient>%s</recipient>\r\n"), *sn->Recipient.ToString());
					XmlText += FString::Printf(TEXT("\t\t<tags>%s</tags>\r\n"), *sn->SearchTags.ToString());
					XmlText += "\t</fixitnote>\r\n";

#ifdef SAVE_TO_XML_FILE__DONT_SAVE_IN_LEVEL
					a->K2_DestroyActor();
#endif
				}
			}

			XmlText += "</fixitnotes>\r\n";

			if (FFileHelper::SaveStringToFile(XmlText, *filename))
			{
				UE_LOG(SkunkLog_FixItNote, Log, TEXT("-----FFixItEdMode::SaveToXMLFile wrote to XML file: %s"), *filename);
			}
			else
			{
				UE_LOG(SkunkLog_FixItNote, Log, TEXT("-----FFixItEdMode::SaveToXMLFile could not write to XML file: %s"), *filename);
			}
		}
		else if (!doesWorldHaveNotes)
		{
			if (!FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*filename))
			{
				UE_LOG(SkunkLog_FixItNote, Log, TEXT("-----FFixItEdMode::SaveToXMLFile could not delete XML file: %s"), *filename);
			}
		}
	}
}

// Reads in the StickyNote list from StickyNotes.xml
// By default (if I don't specify a path), it goes into the same folder as the engine exe: Program Files\Epic Games\4.8\Engine\Binaries\Win64
// But for now, I'm using the folder where the dll loaded from, as the default path
void FFixItEdMode::LoadFromXMLFile () {
    // First, remove all existing sticky notes:
    TArray <AActor *> actors = GWorld->GetCurrentLevel ()->Actors;
    for (int i = 0; i < actors.Num (); i++) {
        AActor *a = actors [i];
        if (a && a->IsA (AFixItNote::StaticClass ())) {    //  Strangely, once one of the actors was null
            AFixItNote *sn = (AFixItNote *) a;

#ifdef SAVE_TO_XML_FILE__DONT_SAVE_IN_LEVEL
            a->K2_DestroyActor ();
#endif
        }
    }

    bool stickyMode = false;
    if (FFixItEdMode::edMode != NULL)
        stickyMode = FFixItEdMode::edMode->GetModeManager ()->IsModeActive (EM_FixIt);

    FString filename = PathToSaveXml ();

	FXmlFile NoteXml;
	bool bLoadResult = NoteXml.LoadFile(filename);
	
	if (bLoadResult)
	{
		const FXmlNode* NoteRoot = NoteXml.GetRootNode();
		if (NoteRoot != nullptr)
		{
			for (const auto& NoteNode : NoteRoot->GetChildrenNodes())
			{
				const auto& NoteNodeName = NoteNode->GetTag();
				if (NoteNodeName.Equals(TEXT("fixitnote"), ESearchCase::IgnoreCase))
				{
					FString Text;
					uint8 Priority = 0;
					float X = 0.f, Y = 0.f, Z = 0.f;
					FString Recipient;
					FString Tags;

					for (const auto& NotePropertyNode : NoteNode->GetChildrenNodes())
					{
						const auto& NotePropertyName = NotePropertyNode->GetTag();
						if (NotePropertyName.Equals(TEXT("text"), ESearchCase::IgnoreCase))
						{
							Text = NotePropertyNode->GetContent();
						}
						if (NotePropertyName.Equals(TEXT("priority"), ESearchCase::IgnoreCase))
						{
							Priority = FCString::Atoi(*NotePropertyNode->GetContent());
						}
						if (NotePropertyName.Equals(TEXT("x"), ESearchCase::IgnoreCase))
						{
							X = FCString::Atof(*NotePropertyNode->GetContent());
						}
						if (NotePropertyName.Equals(TEXT("y"), ESearchCase::IgnoreCase))
						{
							Y = FCString::Atof(*NotePropertyNode->GetContent());
						}
						if (NotePropertyName.Equals(TEXT("z"), ESearchCase::IgnoreCase))
						{
							Z = FCString::Atof(*NotePropertyNode->GetContent());
						}
						if (NotePropertyName.Equals(TEXT("recipient"), ESearchCase::IgnoreCase))
						{
							Recipient = NotePropertyNode->GetContent();
						}
						if (NotePropertyName.Equals(TEXT("tags"), ESearchCase::IgnoreCase))
						{
							Tags = NotePropertyNode->GetContent();
						}
					}
					// Spawn a new stickynote with these settings
#ifdef SAVE_TO_XML_FILE__DONT_SAVE_IN_LEVEL
					AFixItNote *snew = GWorld->SpawnActor <AFixItNote>(FVector(X, Y, Z), FRotator(0, 0, 0));
					snew->Priority = (EFixItNotePriority)Priority;
					snew->Text = FText::FromString(Text);
					snew->Recipient = FText::FromString(Recipient);
					snew->SearchTags = FText::FromString(Tags);
					snew->Update();

					if (!stickyMode)
						snew->SetIsTemporarilyHiddenInEditor(true);    // Since our editor mode tab isn't open, hide the sticky note
#endif

																   // We don't need to add it to the StickyNote list now, since it is added in RestoreAllStickyNotes whenever we enter our editor mode

					UE_LOG(SkunkLog_FixItNote, Log, TEXT("-----FFixItEdMode::LoadFromXMLFile loaded fixit note txt=%s recipient=%s tags=%s pri=%d x=%f y=%f y=%f"), *Text, *Recipient, *Tags, Priority, X, Y, Z);

				}
			}
		}

		if (stickyMode)
			FFixItEdMode::RotateTextToCameraStatic();

		UE_LOG(SkunkLog_FixItNote, Log, TEXT("-----FFixItEdMode::LoadFromXMLFile read config from: %s"), *filename);
	}
    else
	{
        UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----FFixItEdMode::LoadFromXMLFile XML file not found: %s"), *filename);
    }

#ifdef SAVE_TO_XML_FILE__DONT_SAVE_IN_LEVEL
    // Don;t mark the level is dirty just from addin in the sticky notes from file
    GWorld->GetCurrentLevel ()->GetOutermost ()->SetDirtyFlag (false);
#endif
}


//Make sure to check that return value is not NULL
FString FFixItEdMode::PathToSaveXml () {
	// Insert the level name into the filename:     FixItNotes.<level-name>.xml
    const IMainFrameModule& MainFrameModule = FModuleManager::GetModuleChecked< IMainFrameModule >(MainFrameModuleName);
    FString LevelName = MainFrameModule.GetLoadedLevelName();

	return FPaths::GamePluginsDir() + "Fixit/Data/" + XmlFileName + LevelName + XmlFileExtension;
}

void FFixItEdMode::PostUndo () {
    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----FFixItEdMode::PostUndo..."));

    // Update the search results list.  This is messy, using a couple of static things, but should work:
    FilterStickyNotesInWorld (SFixItWidget::ViewPriority);

    FEdMode::PostUndo ();
}

bool FFixItEdMode::MouseMove (FEditorViewportClient *ViewportClient, FViewport *Viewport, int32 x, int32 y) {
//    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("FFixItEdMode::MouseMove %d %d..."), x, y);

    return false;
}

bool FFixItEdMode::CapturedMouseMove (FEditorViewportClient *ViewportClient, FViewport *Viewport, int32 x, int32 y) {
//    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----FFixItEdMode::CapturedMouseMove %d %d..."), x, y);

    return false;
}

bool FFixItEdMode::InputDelta (FEditorViewportClient *InViewportClient, FViewport *InViewport, FVector &InDrag, FRotator &InRot, FVector &InScale) {
    //UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----FFixItEdMode::InputDelta... InDrag=%s InRot=%s InScale=%s"), *InDrag.ToString (), *InRot.ToString (), *InScale.ToString ());

    RotateTextToCamera ();
    return false;
}

bool FFixItEdMode::InputKey (FEditorViewportClient *ViewportClient, FViewport *Viewport, FKey Key, EInputEvent Event) {
    //UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----FFixItEdMode::InputKey %s"), *Key.GetFName ().ToString ());

    //if (Key == EKeys::LeftMouseButton) {
    //    if (Event == EInputEvent::IE_Pressed) {
    //        bViewCameraIsLocked = true;
    //        LockedCameraLocation = ViewportClient->GetViewLocation ();
    //        LockedCameraRotation = ViewportClient->GetViewRotation ();
    //    } else if (Event == EInputEvent::IE_Released) {
    //        bViewCameraIsLocked = false;
    //    }
    //}

    //if (Key.GetFName () == FName ("j")) {
    //    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("j pressed"));

    //    // Register a custom layout class for the sticky note actor's Details panel:
    //    //        FPropertyEditorModule &PropertyModule = FModuleManager::LoadModuleChecked <FPropertyEditorModule> ("PropertyEditor");
    //    //        PropertyModule.RegisterCustomClassLayout ("Actor", FOnGetDetailCustomizationInstance::CreateStatic (&StickyNoteLayoutDetails::MakeInstance));

    //    UWorld *World = ViewportClient->GetWorld ();
    //    World->SpawnActor<ASolidBlock> (ASolidBlock::StaticClass ());
    //}

    RotateTextToCamera ();
    return false;
}

//void FFixItEdMode::Tick (FEditorViewportClient *ViewportClient, float DeltaTime) {
//    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----FFixItEdMode::Tick"));
//
//    FEdMode::Tick (ViewportClient, DeltaTime);
//}

void FFixItEdMode::RotateTextToCamera () {
    // Position sticky note text relative to current view location/direction
    FEditorViewportClient *client = (FEditorViewportClient *) GEditor->GetActiveViewport ()->GetClient ();
    FVector editorCameraDirection = client->GetViewRotation ().Vector ();
    FVector editorCameraLocation = client->GetViewLocation ();

    // Loop through all actors in the level, finding all the sticky notes
    TArray <AActor *> actors = GetWorld ()->GetCurrentLevel ()->Actors;
    for (int i = 0; i < actors.Num (); i++) {
        AActor *a = actors [i];
        if (a)      //  Strangely, once one of the actors was null
        if (a->GetClass ()->GetFName () == "FixItNote") {
            FVector location = a->GetActorLocation ();          // Where the sticky note text I'm rotating, is

            FRotator rotation = client->GetViewRotation ();     // What the camera rotation is

            a->SetActorRotation (rotation);                     // Set the actor (and hence it's text subcomponent) rotation, to point to the camera

            //UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----FFixItEdMode::RotateTextToCamera   location=%s rotation=%s"), *location.ToString (), *rotation.ToString ());
        }
    }
}
void FFixItEdMode::RotateTextToCameraStatic () {
    if (GEditor == NULL)
        return;
    if (GEditor->GetActiveViewport () == NULL)
        return;

    // Position sticky note text relative to current view location/direction
    FEditorViewportClient *client = (FEditorViewportClient *) GEditor->GetActiveViewport ()->GetClient ();
    FVector editorCameraDirection = client->GetViewRotation ().Vector ();
    FVector editorCameraLocation = client->GetViewLocation ();

    // Loop through all actors in the level, finding all the sticky notes
    TArray <AActor *> actors = GWorld->GetCurrentLevel ()->Actors;
    for (int i = 0; i < actors.Num (); i++) {
        AActor *a = actors [i];
        if (a)      //  Strangely, once one of the actors was null
            if (a->GetClass ()->GetFName () == "FixItNote") {
                FVector location = a->GetActorLocation ();          // Where the sticky note text I'm rotating, is

                FRotator rotation = client->GetViewRotation ();     // What the camera rotation is

                a->SetActorRotation (rotation);                     // Set the actor (and hence it's text subcomponent) rotation, to point to the camera

                //UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----FFixItEdMode::RotateTextToCamera   location=%s rotation=%s"), *location.ToString (), *rotation.ToString ());
            }
    }
}

void FFixItEdMode::ClearStickyNotes () {
    FixItNoteMap.Reset();
    FixItNotes.Empty();
}

bool FFixItEdMode::AddStickyNote (AActor* note) {
    if (note && note->IsA (AFixItNote::StaticClass ())) {
        TSharedPtr<FFixItNoteListItem> item = TSharedPtr <FFixItNoteListItem> (new FFixItNoteListItem ((AFixItNote *) note));
        if (!FixItNoteMap.Contains (note)) {
            FixItNoteMap.Emplace (item->GetID (), item);
            FixItNotes.Add (item);
            return true;
        }
    }
    return false;
}

bool FFixItEdMode::RemoveStickyNote (AActor *note) {
    if (note && note->IsA (AFixItNote::StaticClass ())) {
        FFixItNoteListItemPtr *item = FixItNoteMap.Find (note);
        if (item != nullptr) {
            FixItNoteMap.Remove ((*item)->GetID ());
            FixItNotes.Remove (*item);
            return true;
        }
    }
    return false;
}
