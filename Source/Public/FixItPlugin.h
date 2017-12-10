// Copyright (C) 2015 Skunkwerks Kinetic Industries Inc. All Rights Reserved.

#pragma once

#include "FixItPluginPCH.h"

#include "ModuleManager.h"
#include "LevelEditor.h"
#include "LevelEditorActions.h"
#include "SharedPointer.h"
#include "FFixItPluginCommands.h"
#include "Internationalization.h"
#include "MultiBoxExtender.h"
#include "SWindow.h"
#include "SViewport.h"

 
class FixItPlugin : public IModuleInterface {
public:
	TSharedPtr <class SViewport> Viewport;
	TSharedRef <SDockTab> SpawnTab (const FSpawnTabArgs& TabSpawnArgs);

public:
	/** IModuleInterface implementation */
	virtual void StartupModule () override;
	virtual void ShutdownModule () override;

    void PreExit ();
    void PreSave (uint32 flags, UWorld *world);
    void PostSave (uint32 flags, UWorld *world, bool bSuccess);
    void MapOpened (const FString &, bool);
//    void CameraMoved (const FVector &, const FRotator &, ELevelViewportType, int32);
//    void Refresh ();
    void EndPIE (bool);

	void MyButton_Clicked ();

	/**	* Singleton-like access to ISpeedTreeImporter
	*	* @return Returns SpeedTreeImporter singleton instance, loading the module on demand if needed	*/
	static inline FixItPlugin &Get () {
		return FModuleManager::LoadModuleChecked <FixItPlugin> ("FixItNotePlugin");
	}

	/**	* Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	*	* @return True if the module is loaded and ready to use	*/
	static inline bool IsAvailable () {
		return FModuleManager::Get ().IsModuleLoaded ("FixItNotePlugin");
	}

private:
    void AddMainToolbarTab ();
    void RegisterEdModeUsingFactory ();
    void RegisterEdMode ();
    void AddEditorModeExtension ();
    void RegisterCommands ();

	void AddToolbarExtension (FToolBarBuilder &);

	TSharedPtr <FUICommandList> MyPluginCommands;
	TSharedPtr <FExtensibilityManager> MyExtensionManager;

    TSharedPtr <FExtender> stickyNoteModeExtender;
    TSharedPtr <const FExtensionBase> ToolbarExtension;

    void ShutDownMainToolbarTab ();
};