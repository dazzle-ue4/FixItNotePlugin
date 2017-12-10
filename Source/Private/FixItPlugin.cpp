// Copyright (C) 2015 Skunkwerks Kinetic Industries Inc. All Rights Reserved.

#include "FixItPluginPCH.h"

#include "FixItPlugin.h"
#include "FixItWidget.h"
#include "FFixItEdMode.h"
#include "FixItEdModeStyle.h"

#include "EditorStyle.h"
//#include "LevelEditor.h"

#include "SlateBasics.h"
#include "SlotBase.h"
#include "SPanel.h"
#include "TabManager.h"
#include "SDockTab.h"
#include "SDockableTab.h"
#include "SDockTabStack.h"
#include "SlateApplication.h"

DEFINE_LOG_CATEGORY (SkunkLog_FixItNote);

static const FName StickyNoteTabName ("FixItNoteTab");


class FFixItEditorModeFactory : public IEditorModeFactory {
    virtual void OnSelectionChanged (FEditorModeTools& Tools, UObject* ItemUndergoingChange) const override;
    virtual FEditorModeInfo GetModeInfo() const override;
    virtual TSharedRef<FEdMode> CreateMode() const override;
};

// This is one method that is called at startup, after all the actors are created, so it
// can be a good place to intercept when the editor starts up.
void FFixItEditorModeFactory::OnSelectionChanged (FEditorModeTools &Tools, UObject *ItemUndergoingChange) const {
    UE_LOG (SkunkLog_FixItNote, Log, TEXT("-----FFixItEditorModeFactory::OnSelectionChanged"));

    IEditorModeFactory::OnSelectionChanged (Tools, ItemUndergoingChange);

    //// Check to see if this item was deleted
    //if (ItemUndergoingChange->IsA (AFixItNote::StaticClass ())) {
    //    AFixItNote *sn = (AFixItNote *) ItemUndergoingChange;

    //    // See if it exists in the world.  If not, then it must have been deleted
    //    bool exists = false;
    //    TArray <AActor *> actors = GWorld->GetCurrentLevel ()->Actors;
    //    for (int i = 0; i < actors.Num (); i++) {
    //        AActor *a = actors [i];
    //        if (sn == a) {
    //            exists = true;
    //            break;
    //        }
    //    }

    //    if (exists)
    //        return;

    //    // It has been deleted!
    //    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----FFixItEditorModeFactory::OnSelectionChanged found deleted sticky note"));
    //}
}

FEditorModeInfo FFixItEditorModeFactory::GetModeInfo () const {
    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----FFixItEditorModeFactory::GetModeInfo"));

    // This is not a bad place to hook at startup because it's called after the level loads, but it is called multiple times, so that must be taken into consideration
    FFixItEdMode::StartupHook (GWorld);

    // This registers the editor mode ID ("StickyNoteEditMode"), which it uses to select which widget to call to display the tab contents when someone clicks on our tab
    FEditorModeInfo info = FEditorModeInfo (FFixItEdMode::EM_FixIt, NSLOCTEXT ("EditorModes", "FixItNoteEditMode", "FixItNoteEditMode"));
    info.bVisible = true;
    // Set toolbar icon
    info.IconBrush = FSlateIcon (FixItEdModeStyle::GetStyleSetName (), "DefaultPluginIcon", "DefaultPluginIcon.small");


    return info;
}

TSharedRef<FEdMode> FFixItEditorModeFactory::CreateMode () const {
    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----FFixItEditorModeFactory::CreateMode"));

    FFixItEdMode *SNEdMode = new FFixItEdMode ();

    return MakeShareable (SNEdMode);
}



void FixItPlugin::StartupModule () {
    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----FixItPlugin::StartupModule"));

    // Register the style (for the mode tab)
    FixItEdModeStyle::Initialize();
    FixItEdModeStyle::ReloadTextures();

    RegisterEdModeUsingFactory();

    //AddMainToolbarTab ();

    AddEditorModeExtension ();

    //RegisterCommands ();

    // Register delegates to be notified when things happen in the editor or core.
    // NOTE: Must correspondingly remove these in ShutdownModule below
    FCoreDelegates::OnPreExit.AddRaw (this, &FixItPlugin::PreExit);
    FEditorDelegates::PreSaveWorld.AddRaw (this, &FixItPlugin::PreSave);
    FEditorDelegates::PostSaveWorld.AddRaw (this, &FixItPlugin::PostSave);
    FEditorDelegates::OnMapOpened.AddRaw (this, &FixItPlugin::MapOpened);
    FEditorDelegates::EndPIE.AddRaw (this, &FixItPlugin::EndPIE);
    //FEditorDelegates::OnNewAssetCreated.AddRaw (this, &FixItPlugin::OnNewAssetCreated);
    //FCoreUObjectDelegates::PostDemoPlay.AddRaw (this, &FixItPlugin::PreExit);
    //FWorldDelegates::
    //FEditorDelegates::RefreshEditor.AddRaw (this, &FixItPlugin::Refresh);
    //FEditorDelegates::OnEditorCameraMoved.AddRaw (this, &FixItPlugin::CameraMoved);
}

void FixItPlugin::PreExit () {
    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----FixItPlugin::PreExit..."));

    FFixItEdMode::PreShutdownHook ();
}
void FixItPlugin::PreSave (uint32 flags, UWorld *world) {
    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----FixItPlugin::PreSave..."));

    FFixItEdMode::PreSaveHook ();
}
void FixItPlugin::PostSave (uint32 flags, UWorld *world, bool) {
    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----FixItPlugin::PostSave..."));

    FFixItEdMode::PostSaveHook ();
}
void FixItPlugin::MapOpened (const FString &, bool) {
    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----FixItPlugin::MapOpened..."));

    FFixItEdMode::MapOpenedHook ();
}
//void FixItPlugin::CameraMoved (const FVector &, const FRotator &, ELevelViewportType, int32) {
//    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----FixItPlugin::CameraMoved..."));
//}
//void FixItPlugin::Refresh () {
//    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----FixItPlugin::Refresh Editor..."));
//}
void FixItPlugin::EndPIE (bool) {
    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----FixItPlugin::EndPIE..."));

    //FFixItEdMode::RotateTextToCameraStatic ();
}

void FixItPlugin::RegisterEdModeUsingFactory () {
    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----FixItPlugin::RegisterEdModeUsingFactory"));

    TSharedRef <FFixItEditorModeFactory> Factory = MakeShareable (new FFixItEditorModeFactory);
    // This FName here must match the one returned by GetModeInfo in the factory class:
    FEditorModeRegistry::Get ().RegisterMode (FFixItEdMode::EM_FixIt, Factory);
}

void FixItPlugin::AddEditorModeExtension () {
    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----FixItPlugin::AddEditorModeExtension"));

    stickyNoteModeExtender = MakeShareable (new FExtender);
    // In the ActorPainter example, he keeps a reference to ToolbarExtension here only to remove it in ShutdownModule
    //ToolbarExtension = stickyNoteModeExtender->AddToolBarExtension ("Settings", EExtensionHook::After, MyPluginCommands, FToolBarExtensionDelegate::CreateRaw (this, &FixItPlugin::AddToolbarExtension));

    FLevelEditorModule &LevelEditorModule = FModuleManager::LoadModuleChecked <FLevelEditorModule> ("LevelEditor");
    MyExtensionManager = LevelEditorModule.GetToolBarExtensibilityManager ();
    MyExtensionManager->AddExtender (stickyNoteModeExtender);
}

void FixItPlugin::RegisterCommands () {
    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----FixItPlugin::RegisterCommands"));

// Don't I need something like this:
//builder.AddToolBarButton (FFixItPluginCommands::Get ().MyButton, NAME_None, LOCTEXT ("MyButton_Override", "Sticky Note"), LOCTEXT ("MyButton_ToolTipOverride", "Click to toggle Sticky Note On/Off"), IconBrush, NAME_None);

    FFixItPluginCommands::Register ();

    MyPluginCommands = MakeShareable (new FUICommandList);
    // Register to call MyButton_Clicked when this button is clicked
    MyPluginCommands->MapAction (
        FFixItPluginCommands::Get ().MyButton, 
        FExecuteAction::CreateRaw (this, &FixItPlugin::MyButton_Clicked), 
        FCanExecuteAction ());


    // THis is how the Foliage Editor mode does this:
    //const FFixItPluginCommands& Commands = FFixItPluginCommands::Get ();
    //MyPluginCommands->MapAction (
    //    Commands.MyButton,
    //    FExecuteAction::CreateSP (this, &SFoliageEdit::OnSetPaint),
    //    FCanExecuteAction (),
    //    FIsActionChecked::CreateSP (this, &SFoliageEdit::IsPaintTool));
    //FEditorDelegates::NewCurrentLevel.AddSP (this, &SFoliageEdit::NotifyNewCurrentLevel);

}

void FixItPlugin::AddMainToolbarTab () {
	//PaintingEnabled = false;

	FFixItPluginCommands::Register ();

	MyPluginCommands = MakeShareable (new FUICommandList);

    // Register to call MyButton_Clicked when this button is clicked
	MyPluginCommands->MapAction (
		FFixItPluginCommands::Get ().MyButton,
		FExecuteAction::CreateRaw (this, &FixItPlugin::MyButton_Clicked),
		FCanExecuteAction ());

    stickyNoteModeExtender = MakeShareable (new FExtender);
    ToolbarExtension = stickyNoteModeExtender->AddToolBarExtension ("Settings", EExtensionHook::After, MyPluginCommands, FToolBarExtensionDelegate::CreateRaw (this, &FixItPlugin::AddToolbarExtension));

	FLevelEditorModule &LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule> ("LevelEditor");

    MyExtensionManager = LevelEditorModule.GetToolBarExtensibilityManager ();
    MyExtensionManager->AddExtender (stickyNoteModeExtender);


	//TSharedRef<FWorkspaceItem> cat = tm->AddLocalWorkspaceMenuCategory (FText::FromString (TEXT ("StickyNote")));
	//auto WorkspaceMenuCategoryRef = cat.ToSharedRef ();

    // This registers the SpawnTab function callback which creates the tab
    TSharedRef<class FGlobalTabmanager> tm = FGlobalTabmanager::Get ();
    // CreateRaw creates a delegate pointer so the editor can call the function
	//tm->RegisterTabSpawner (StickyNoteTabName, FOnSpawnTab::CreateRaw (this, &FixItPlugin::SpawnTab)).SetDisplayName (FText::FromString (TEXT ("Sticky Note")));
    // This one allows it to dock even on subwindows
    tm->RegisterNomadTabSpawner (StickyNoteTabName, FOnSpawnTab::CreateRaw (this, &FixItPlugin::SpawnTab)).SetDisplayName (FText::FromString (TEXT ("FixIt Note")));

    UE_LOG(SkunkLog_FixItNote, Log, TEXT("-----FixItPlugin: Tried to add main toolbar tab"));
}

// Tells the tab manager to invoke (open) our tab
void FixItPlugin::MyButton_Clicked () {
    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----FixItPlugin::MyButton_Clicked"));
return;

    TSharedRef<class FGlobalTabmanager> tm = FGlobalTabmanager::Get ();
	tm->InvokeTab (StickyNoteTabName);
}

// Creates and returns our main toolbar tab
TSharedRef<SDockTab> FixItPlugin::SpawnTab (const FSpawnTabArgs &TabSpawnArgs) {
    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----FixItPlugin::SpawnTab"));

	//TSharedRef<SDockTab> SpawnedTab = SNew (SDockTab) [
	//	SNew (SButton)
	//		.Text (FText::FromString (TEXT ("Push Me")))
	//		.ContentPadding (3)
	//];

    // This one is for docking even on subwindows
    //TSharedRef<SDockTab> SpawnedTab = SNew (SDockTab)
    //    .TabRole(ETabRole::NomadTab)
    //    [
    //        SNew (SButton)
    //        .Text (FText::FromString (TEXT ("Sticky Note Spawner")))
    //        .ContentPadding (3)
    //    ];

    // This one makes a tab panel and fills it in with the widget
    TSharedRef <SDockTab> SpawnedTab = SNew (SDockTab)
        .TabRole (ETabRole::NomadTab) [
            SNew (SFixItWidget)
        ];

	return SpawnedTab;
}

void FixItPlugin::AddToolbarExtension (FToolBarBuilder &builder) {
#define LOCTEXT_NAMESPACE "LevelEditorToolBar"

	UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----FixItPlugin::AddToolbarExtension"));

	FSlateIcon IconBrush = FSlateIcon (FEditorStyle::GetStyleSetName (), "LevelEditor.ViewOptions", "LevelEditor.ViewOptions.Small");

	builder.AddToolBarButton (FFixItPluginCommands::Get ().MyButton, NAME_None, LOCTEXT ("MyButton_Override", "FixIt Note"), LOCTEXT ("MyButton_ToolTipOverride", "Click to toggle FixIt Note On/Off"), IconBrush, NAME_None);

#undef LOCTEXT_NAMESPACE
}

void FixItPlugin::ShutdownModule () {
    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----FixItPlugin::ShutdownModule"));

    FixItEdModeStyle::Shutdown();

    FEditorModeRegistry::Get ().UnregisterMode (FName (TEXT ("FixItNoteEdMode")));

    // Remove the Level Mode extender we added:
    if (MyExtensionManager.IsValid ()) {
        FFixItPluginCommands::Unregister ();

        TSharedRef <class FGlobalTabmanager> tm = FGlobalTabmanager::Get ();
        tm->UnregisterTabSpawner (StickyNoteTabName);

        MyExtensionManager->RemoveExtender (stickyNoteModeExtender);
    } else {
        MyExtensionManager.Reset ();
    }

    FCoreDelegates::OnPreExit.RemoveAll (this);
    FEditorDelegates::PreSaveWorld.RemoveAll (this);
    FEditorDelegates::PostSaveWorld.RemoveAll (this);
    //FEditorDelegates::OnAssetsDeleted.RemoveAll (this);
    FEditorDelegates::OnMapOpened.RemoveAll (this);
    FEditorDelegates::EndPIE.RemoveAll (this);

    FFixItEdMode::ShutdownHook ();
}

IMPLEMENT_MODULE (FixItPlugin, Module)