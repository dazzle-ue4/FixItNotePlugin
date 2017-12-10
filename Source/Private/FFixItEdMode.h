// Copyright (C) 2015 Skunkwerks Kinetic Industries Inc. All Rights Reserved.

#pragma once

#include "FixItPluginPCH.h"
#include "FixItCommon.h"       // For EFixItNotePriority
#include "FixIt.h"
#include "FixItListView.h"
#include "EdMode.h"
#include "XmlParser.h"


class SFixItWidget;


class FFixItEdMode : public FEdMode {
public:
    FFixItEdMode ();
    virtual ~FFixItEdMode ();

    virtual void AddReferencedObjects (FReferenceCollector &Collector) override;    // FGCObject interface

    virtual void Enter () override;         // Called when the mode is entered
    virtual void Exit () override;          // Called when the mode is exited

    virtual void PostUndo () override;      // Called after an Undo operation

    /**
    * Called when the mouse is moved over the viewport
    *
    * @param	InViewportClient	Level editor viewport client that captured the mouse input
    * @param	InViewport			Viewport that captured the mouse input
    * @param	InMouseX			New mouse cursor X coordinate
    * @param	InMouseY			New mouse cursor Y coordinate
    *
    * @return	true if input was handled
    */
    virtual bool MouseMove (FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 x, int32 y) override;
    virtual bool CapturedMouseMove (FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 MouseX, int32 MouseY) override;
    virtual bool InputDelta (FEditorViewportClient* InViewportClient, FViewport* InViewport, FVector& InDrag, FRotator& InRot, FVector& InScale) override;
    //virtual void Tick (FEditorViewportClient* ViewportClient, float DeltaTime) override;
    virtual bool InputKey (FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)override;

    void RotateTextToCamera ();
    static void RotateTextToCameraStatic ();

	const static FEditorModeID EM_FixIt;


    static void FilterStickyNotesInWorld (EFixItNotePriorityFilter filter);
    static void HideAllNotes (UWorld *world);
    static void InitStickyNotes ();
    void RestoreAllStickyNotes ();

    static void StartupHook (UWorld *);                 // Called at startup, from the FFixItEditorModeFactory
    static void PreShutdownHook ();                     // Called at shutdown, before anything is destroyed or saved
    static void ShutdownHook ();                        // Called at shutdown, after the level is gone
    static void PreSaveHook ();                         // Called prior to saving.  To avoid saving sticky notes in level, remove them here and add them back in PostSaveHook
    static void PostSaveHook ();                        // Called after saving.  It saves savedNotes to xml file.  It reloads all the sticky notes from the xml file
    static void MapOpenedHook();                        // Called when a new map is opened.  It reloads all the sticky notes from the xml file
    //    static void AssetsDeletedHook (const TArray<UClass*>& DeletedAssetClasses);     // Called whenever anything is deleted.  Need to update our StickNote list when this happens.

    static TArray<TSharedRef<FixItNoteInfo>> savedNotes;
    static void CopyNoteInfo();                        // Copy active stickynote information to savedNotes
    static void SaveInfoToXMLFile();                   // Save the note information in savedNotes to XML file
    static void SaveToXMLFile();                       // When SAVE_TO_XML_FILE__DONT_SAVE_IN_LEVEL is defined, we save/load sticky notes to XML file here
    static void LoadFromXMLFile ();
    static FString PathToSaveXml ();

    static FFixItEdMode *edMode;                   // If the editor mode has been created, then set this static to point to it, so we can reach it from static methods if need be


    //static TArray<AFixItNote *> stickysNotInWorld;   // A list of all the StickyNotes that are currently hidden or filtered out of the world
    static bool startupComplete;                        // So I don't re-run startup code (in StartupHook) multiple times

private:
    static ListItemMap FixItNoteMap;
    static TArray<FFixItNoteListItemPtr> FixItNotes;  // A list of the stickyNotes that are visible in the world at the moment

public:
    static void ClearStickyNotes();
    static bool AddStickyNote(AActor* note);
    static bool RemoveStickyNote(AActor* note);
    static const TArray<FFixItNoteListItemPtr>* GetStickyNotes() {
        return &FixItNotes;
    }

};