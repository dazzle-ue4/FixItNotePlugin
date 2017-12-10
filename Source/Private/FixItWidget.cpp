// Copyright (C) 2015 Skunkwerks Kinetic Industries Inc. All Rights Reserved.

#include "FixItPluginPCH.h"

#include "ActorFactoryFixIt.h"
#include "FFixItEdMode.h"
#include "FFixItEdModeToolkit.h"
#include "FixItPlugin.h"
#include "FixItDraggable.h"
#include "FixItWidget.h"
#include "FixIt.h"
//#include "DetailsCustomization.h"
#include "SSearchBox.h"

#include "GameDelegates.h"

#include <string.h>


#define LOCTEXT_NAMESPACE "SFixItWidget"


FFixItEdMode *SFixItWidget::GetEditorMode () {
	return (FFixItEdMode *) GLevelEditorModeTools ().GetActiveMode (FFixItEdMode::EM_FixIt);
}

// This widget is the form that is this tab


//DECLARE_DELEGATE_OneParam(FOnPriorityChanged, EFixItNotePriority);


// fill in combobox items
void SFixItWidget::GeneratePriorityComboItems () {
    PriorityComboItems.Empty ();
    PriorityComboItems.Add (MakeShareable (new EFixItNotePriority (EFixItNotePriority::Urgent)));
    PriorityComboItems.Add (MakeShareable (new EFixItNotePriority (EFixItNotePriority::Moderate)));
    PriorityComboItems.Add (MakeShareable (new EFixItNotePriority (EFixItNotePriority::Low)));
    //PriorityComboItems.Add (MakeShareable (new EFixItNotePriority (EFixItNotePriority::Comment)));
}
/****Changed By Linda Rename Priority to myPriority
 because Priority has been used to define a field at line 73 in FixItWidget.h ***/
TSharedRef<SWidget> SFixItWidget::MakePriorityComboItemWidget (TSharedPtr<EFixItNotePriority> myPriority) {
    return
        SNew (STextBlock)
        .Text (MakePriorityComboText (*myPriority));
}

// fill in combobox items
void SFixItWidget::GeneratePriorityFilterComboItems() {
	PriorityFilterComboItems.Empty();
	PriorityFilterComboItems.Add(MakeShareable(new EFixItNotePriorityFilter(EFixItNotePriorityFilter::All)));
	PriorityFilterComboItems.Add(MakeShareable(new EFixItNotePriorityFilter(EFixItNotePriorityFilter::Urgent)));
	PriorityFilterComboItems.Add(MakeShareable(new EFixItNotePriorityFilter(EFixItNotePriorityFilter::Moderate)));
	PriorityFilterComboItems.Add(MakeShareable(new EFixItNotePriorityFilter(EFixItNotePriorityFilter::Low)));
}
/****Changed By Linda Rename Priority to myPriority 
 because Priority has been used to define a field at line 73 in FixItWidget.h ***/
TSharedRef<SWidget> SFixItWidget::MakePriorityFilterComboItemWidget(TSharedPtr<EFixItNotePriorityFilter> myPriority) {
	return
		SNew(STextBlock)
		.Text(MakePriorityFilterComboText(*myPriority));
}

//bool SFixItWidget::watchdogDelegateInit = false;

SFixItWidget::~SFixItWidget () {
    FCoreUObjectDelegates::OnObjectModified.RemoveAll (watchdog.Get ());
    watchdog->RemoveFromRoot ();

    GEngine->OnLevelActorAdded ().RemoveAll (this);
    GEngine->OnLevelActorDeleted ().RemoveAll (this);

    USelection::SelectionChangedEvent.RemoveAll (this);
    USelection::SelectObjectEvent.RemoveAll (this);
}

// Create the widget (aka form) controls
void SFixItWidget::Construct (const FArguments& InArgs) {
    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----SFixItWidget::Construct..."));

    ViewPriority = EFixItNotePriorityFilter::All;

    Priority = EFixItNotePriority::Low;

    GeneratePriorityComboItems ();
        
	GeneratePriorityFilterComboItems();

	bIsReentrant = false;

    worldOutlinerSelectionChanging = false;
    listSelectionChanging = false;

    // Register commands to be called from the controls
    //FFixItPluginCommands::Register ();
    //TSharedPtr <FUICommandList> cmds = MakeShareable (new FUICommandList);
    //cmds->MapAction (
    //    FFixItPluginCommands::Get ().MyButton,
    //    FExecuteAction::CreateRaw (this, &SFixItWidget::OnCreateButtonClicked),
    //    FCanExecuteAction ());


	FMargin StandardPadding (0.0f, 4.0f, 0.0f, 4.0f);

    // For draggable
    UActorFactory *Factory = GEditor->FindActorFactoryByClass (UActorFactoryFixItNote::StaticClass());
    FAssetData AssetData = FAssetData (Factory->GetDefaultActorClass (FAssetData ()));

    FFixItEdMode *StickyNoteEdMode = GetEditorMode ();

TSharedPtr <class SSearchBox> SearchTextField;

if (StickyNoteEdMode != NULL) {

    ChildSlot[

        SNew (SScrollBox)
            .ScrollBarAlwaysVisible (true)
            + SScrollBox::Slot ()
            .Padding (0.0f)[


                SNew (SVerticalBox)

#pragma region New FixIt Note Options
                    + SVerticalBox::Slot ()
                    .AutoHeight ()
                    .Padding (StandardPadding) [
                        //SNew (SBorder)
                        //    .Padding (14)
                        //    .BorderImage (FEditorStyle::GetBrush ("ToolPanel.DarkGroupBorder")) [

                        SNew (SVerticalBox)
                            //-----------------------------------------------------------------------------------------
                            // Below here are settings for creating a new Sticky Note
                            //

                            // Heading/title
                            +SVerticalBox::Slot ()
                            .Padding (2.0f, 20.0f)
                            .AutoHeight ()
                            .HAlign (HAlign_Center) [
                                SNew (STextBlock)
                                    .Text (LOCTEXT ("New FixIt Note", "New FixIt Note"))
                            ]

                            // Priority------------------------------
                            + SVerticalBox::Slot ()
                            .AutoHeight ()
                            .Padding (StandardPadding) [
                                // This way shows them side by side
                                SNew (SHorizontalBox)
                                    + SHorizontalBox::Slot ()
                                    .MaxWidth (15)
                                    .FillWidth (0.1f)
                                    + SHorizontalBox::Slot ()
                                    .MaxWidth (120)
                                    .FillWidth (0.29f)
                                    .VAlign (VAlign_Center) [             // Causes this to only take 0.5x of this slot's width
                                        SNew (STextBlock)
                                            .Text (LOCTEXT ("PriorityLabel", "Priority"))
                                    ]
                                    + SHorizontalBox::Slot ()
                                    .FillWidth (0.7f) [
                                        SNew (SComboBox< TSharedPtr<EFixItNotePriority> >)
                                            .OptionsSource (&PriorityComboItems)
                                            .OnGenerateWidget (this, &SFixItWidget::MakePriorityComboItemWidget)
                                            .InitiallySelectedItem (PriorityComboItems[2])
                                            .OnSelectionChanged (this, &SFixItWidget::OnStickyNotePriorityChanged)
                                            .Content () [
                                                SNew (STextBlock)
                                                    .Text (this, &SFixItWidget::GetPriorityComboBoxContent)
                                            ]
                                    ]
                                    + SHorizontalBox::Slot ()
                                    .MaxWidth (5)
                                    .FillWidth (0.01f)// [             // Causes this to only take 0.5x of this slot's width
                                    //]
                            ]

                            // Spacer -----------------------------
                            //+ SVerticalBox::Slot ()
                            //.AutoHeight ()
                            //.Padding (StandardPadding) [
                            //    SNew (SSeparator)
                            //        .Orientation (Orient_Horizontal)
                            //]

                            // Recipient-----------------------------
                            +SVerticalBox::Slot ()
                            .AutoHeight ()
                            .Padding (StandardPadding) [
                                SNew (SHorizontalBox)
                                    + SHorizontalBox::Slot ()
                                    .MaxWidth (15)
                                    .FillWidth (0.1f)
                                    + SHorizontalBox::Slot ()
                                    .MaxWidth (120)
                                    .FillWidth (0.29f)
                                    .VAlign (VAlign_Center) [             // Causes this to only take 0.5x of this slot's width
                                        SNew (STextBlock)
                                            .Text (LOCTEXT ("Recipient", "Recipient"))
                                    ]
                                    + SHorizontalBox::Slot ()
                                    .FillWidth (0.7f) [
                                        SNew (SMultiLineEditableTextBox)
                                            .Text (LOCTEXT ("RecipientHere", ""))
                                            .ToolTipText (LOCTEXT ("RecipientHereTooltip", "Recipient of the FixIt note"))
                                            .OnTextChanged (this, &SFixItWidget::OnStickyNoteRecipientChanged)
                                    ]
                                    + SHorizontalBox::Slot ()
                                    .MaxWidth (5)
                                    .FillWidth (0.01f) //[             // Causes this to only take 0.5x of this slot's width
                                    //]
                            ]

                            // Spacer -----------------------------
                            //+ SVerticalBox::Slot ()
                            //.AutoHeight ()
                            //.Padding (StandardPadding) [
                            //    SNew (SSeparator)
                            //        .Orientation (Orient_Horizontal)
                            //]

                            // Text Message--------------------------
                            +SVerticalBox::Slot ()
                            .AutoHeight ()[
                                SNew (SHorizontalBox)
                                    + SHorizontalBox::Slot ()
                                    .MaxWidth (15)
                                    .FillWidth (0.1f)
                                    + SHorizontalBox::Slot ()
                                    .MaxWidth (120)
                                    .FillWidth (0.29f) [             // Causes this to only take 0.5x of this slot's width
                                        SNew (STextBlock)
                                            .Text (LOCTEXT ("Text", "Text"))
                                    ]
                                    + SHorizontalBox::Slot ()
                                    .FillWidth (0.7)[
                                        SNew (SBox)
                                            .MinDesiredHeight (60.0f) [      // How tall to make this text edit field

                                                SNew (SMultiLineEditableTextBox)
                                                    .Text (LOCTEXT ("MessageHere", ""))
                                                    .ToolTipText (LOCTEXT ("MessageHereTooltip", "The text message that will be displayed on the FixIt note"))
                                                    .OnTextChanged (this, &SFixItWidget::OnStickyNoteTextChanged)
                                            ]
                                    ]
                                    + SHorizontalBox::Slot ()
                                    .MaxWidth (5)
                                    .FillWidth (0.01f) //[             // Causes this to only take 0.5x of this slot's width
                                    //]
                            ]

                            // Spacer -----------------------------
                            //+ SVerticalBox::Slot ()
                            //.AutoHeight ()
                            //.Padding (StandardPadding) [
                            //    SNew (SSeparator)
                            //        .Orientation (Orient_Horizontal)
                            //]

                            // Tags----------------------------------
                            +SVerticalBox::Slot ()
                            .AutoHeight ()
                            .Padding (StandardPadding) [
                                SNew (SHorizontalBox)
                                    + SHorizontalBox::Slot ()
                                    .MaxWidth (15)
                                    .FillWidth (0.1f)
                                    + SHorizontalBox::Slot ()
                                    .MaxWidth (120)
                                    .FillWidth (0.29f)
                                    .VAlign (VAlign_Center) [
                                        SNew (STextBlock)
                                            .Text (LOCTEXT ("Tags", "Search Tags"))
                                    ]
                                    + SHorizontalBox::Slot ()
                                    .FillWidth (0.7f) [
                                        SNew (SMultiLineEditableTextBox)
                                            .Text (LOCTEXT ("TagsHere", ""))
                                            .ToolTipText (LOCTEXT ("TagsHereTooltip", "Tags for the FixIt note"))
                                            .OnTextChanged (this, &SFixItWidget::OnStickyNoteTagsChanged)
                                    ]
                                    + SHorizontalBox::Slot ()
                                    .MaxWidth (5)
                                    .FillWidth (0.01f) //[             // Causes this to only take 0.5x of this slot's width
                                    //]
                            ]

                            // Spacer -----------------------------
                            //+ SVerticalBox::Slot ()
                            //.AutoHeight ()
                            //.Padding (StandardPadding) [
                            //    SNew (SSeparator)
                            //        .Orientation (Orient_Horizontal)
                            //]

                            // Draggable area to create new note
                            +SVerticalBox::Slot ()
                            .HAlign (HAlign_Center)
                            .AutoHeight () [
                                SNew (SFixItDraggable, Factory, AssetData)
                                    .Text (LOCTEXT ("Drag Me", "FixIt Note [Drag]"))
                            ]

                            // Spacer -----------------------------
                            //+ SVerticalBox::Slot ()
                            //.AutoHeight ()
                            //.Padding (StandardPadding) [
                            //    SNew (SSeparator)
                            //        .Orientation (Orient_Horizontal)
                            //]

                            // Button to create new note
                            +SVerticalBox::Slot ()
                            .HAlign (HAlign_Center)
                            .AutoHeight () [
                                SNew (SHorizontalBox)
                                    + SHorizontalBox::Slot ()
                                    .Padding (0.0f, 0.5f, 0.0f, 0.5f)
                                    .MaxWidth (250) [
                                        SNew (SButton)
                                            .Text (FText::FromString (TEXT ("Create new FixIt Note")))
                                            .ContentPadding (3)
                                            .OnClicked (this, &SFixItWidget::OnCreateButtonClicked)
                                    ]
                            ]
                            //]         // Dark border
                    ]
#pragma endregion

                    // Spacer -----------------------------
                    + SVerticalBox::Slot ()
                    .AutoHeight ()
                    .Padding (StandardPadding) [
                        SNew (SSeparator)
                            .Orientation (Orient_Horizontal)
                    ]

#pragma region Search and Viewing Options
                    + SVerticalBox::Slot ()
                    .AutoHeight ()
                    .Padding (StandardPadding) [
                        //SNew (SBorder)
                        //.Padding (14)
                        //.BorderImage (FEditorStyle::GetBrush ("ToolPanel.DarkGroupBorder")) [

                        SNew (SVerticalBox)
                            + SVerticalBox::Slot ()
                            .AutoHeight ()
                            .Padding (StandardPadding) [
                                // Text title at the top of this section
                                SNew (SVerticalBox)
                                    + SVerticalBox::Slot ()
                                    .Padding (2.0f, 20.0f)
                                    .AutoHeight ()
                                    .HAlign (HAlign_Center) [
                                        SNew (STextBlock)
                                            .Text (LOCTEXT ("ViewLabel", "View / Search"))
                                    ]

                                    // Search-----------------------------
                                    + SVerticalBox::Slot ()
                                    .AutoHeight ()
                                    .Padding (StandardPadding) [
                                        SNew (SHorizontalBox)
                                            + SHorizontalBox::Slot ()
                                            .MaxWidth (15)
                                            .FillWidth (0.1f)
                                            // Label
                                            + SHorizontalBox::Slot ()
                                            .MaxWidth (120)
                                            .FillWidth (0.29f)
                                            .VAlign (VAlign_Center) [             // Causes this to only take 0.5x of this slot's width
                                                SNew (STextBlock)
                                                    .Text (LOCTEXT ("Search", "Search"))
                                            ]
                                            // Search text
                                            + SHorizontalBox::Slot ()
                                            .FillWidth (0.7f) [
                                                //SNew (SMultiLineEditableTextBox)
                                                SAssignNew (SearchTextField, SSearchBox)
                                                    //.Text (LOCTEXT ("SearchHere", ""))
                                                    .ToolTipText (LOCTEXT ("Search for a FixIt note containing this text or tag", "Search for a FixIt note containing this text or tag"))
                                                    .HintText (LOCTEXT ("Search Notes", "Search Notes"))
                                                    .OnTextChanged (this, &SFixItWidget::OnStickyNoteSearchStringChanged)
                                            ]
                                            + SHorizontalBox::Slot ()
                                            .MaxWidth (5)
                                            .FillWidth (0.01f)
                                    ]

                                    // Spacer -----------------------------
                                    //+ SVerticalBox::Slot ()
                                    //.AutoHeight ()
                                    //.Padding (StandardPadding) [
                                    //    SNew (SSeparator)
                                    //        .Orientation (Orient_Horizontal)
                                    //]

                                    // Priority------------------------------
                                    +SVerticalBox::Slot ()
                                    .AutoHeight ()
                                    .Padding (StandardPadding) [
                                        // This way shows them side by side
                                        SNew (SHorizontalBox)
                                            + SHorizontalBox::Slot ()
                                            .MaxWidth (15)
                                            .FillWidth (0.1f)
                                            + SHorizontalBox::Slot ()
                                            .MaxWidth (120)
                                            .FillWidth (0.29f)
                                            .VAlign (VAlign_Center) [             // Causes this to only take 0.5x of this slot's width (which is 1/2 of the total, since there are 2 here)
                                                SNew (STextBlock)
                                                    .Text (LOCTEXT ("PriorityLabel", "Priority"))
                                            ]
                                            + SHorizontalBox::Slot ()
                                            .FillWidth (0.7f) [
                                                SNew (SComboBox <TSharedPtr <EFixItNotePriorityFilter>>)                    // Select (lowest) priority of notes to list
                                                    .OptionsSource (&PriorityFilterComboItems)
                                                    .OnGenerateWidget (this, &SFixItWidget::MakePriorityFilterComboItemWidget)
                                                    .InitiallySelectedItem (PriorityFilterComboItems [0])
                                                    .OnSelectionChanged (this, &SFixItWidget::OnStickyNoteViewPriorityChanged)
                                                    .Content () [
                                                        SNew (STextBlock)
                                                            .Text (this, &SFixItWidget::GetViewPriorityComboBoxContent)
                                                    ]
                                            ]
                                            + SHorizontalBox::Slot ()
                                            .MaxWidth (5)
                                            .FillWidth (0.01f)
                                    ]
                            ]

                            // List of Sticky Notes
                            + SVerticalBox::Slot ()
                            //.AutoHeight ()
                            .MaxHeight (1500.0)              // How much vertical space this listbox will use.  Need to limit it, or it grows too tall.
                            .Padding (StandardPadding) [

                                SNew (SBorder)
                                    .BorderImage (FEditorStyle::GetBrush ("ToolPanel.GroupBorder"))
                                    .Padding (0.0f) [

                                        SAssignNew (FixItList, SFixItListView)
                                            .ItemHeight (5)                         // List view items are this tall.  Doesn't seem to actually have any effect.
                                            .SelectionMode (ESelectionMode::Multi)
                                            .ListItemsSource (FFixItEdMode::GetStickyNotes ())
                                            .OnGenerateRow (this, &SFixItWidget::MakeListWidget)
                                            .OnSelectionChanged (this, &SFixItWidget::HandleListSelectionChanged)
                                            //.OnSelectionChanged_Raw (this, &SFixItWidget::HandleListSelectionChanged)
                                            .OnMouseButtonClick (this, &SFixItWidget::HandleListMouseClick)
                                            .HeaderRow (
                                            SNew (SHeaderRow)
                                            + SHeaderRow::Column ("Name")
                                            .DefaultLabel (LOCTEXT ("FixItNoteListColumnHeader", "FixIt Note List"))
                                            .VAlignHeader (VAlign_Center)
                                            .HAlignHeader (HAlign_Left)
                                            .FillWidth (1.0f)
                                            )
                                    ]
                            ]
                            //]      // Dark border
                    ]
#pragma endregion

            ]];


        GEngine->OnLevelActorAdded ().AddSP (this, &SFixItWidget::OnLevelActorsAdded);
        GEngine->OnLevelActorDeleted ().AddSP (this, &SFixItWidget::OnLevelActorsRemoved);

        watchdog = TWeakObjectPtr<UWatchDog> (NewObject<UWatchDog> ((UObject*)GetTransientPackage (), TEXT ("WatchDog"), RF_ClassDefaultObject));
        watchdog->AddToRoot ();
        watchdog->widget = this;
        FCoreUObjectDelegates::OnObjectModified.AddUObject (watchdog.Get (), &UWatchDog::OnObjectModified);

        // Populate and register to find out when the level's selection changes
        // Should be only when in ActorBrowsing mode?
        OnLevelSelectionChanged (NULL);
        USelection::SelectionChangedEvent.AddRaw (this, &SFixItWidget::OnLevelSelectionChanged);
        USelection::SelectObjectEvent.AddRaw (this, &SFixItWidget::OnLevelSelectionChanged);

		FixItList->Filter (ViewPriority);
    }

    //FEditorDelegates::OnAssetsDeleted.AddSP (this, &SFixItWidget::OnAssetsDeleted);
    //GEditor->OnObjectsReplaced ().AddRaw (this, &SFixItWidget::OnObjectsReplaced);
}

// This is to detect if they click on an entry when it is already selected.  The desire was to center on this note as if you'd just selected it.
void SFixItWidget::HandleListMouseClick (FFixItNoteListItemPtr sn) {
    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----SFixItWidget::HandleListMouseClick"));

    HandleListSelectionChanged (sn, ESelectInfo::OnMouseClick);
}

void SFixItWidget::UpdateList () {
    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----SFixItWidget::UpdateList"));

    // Search the FixItList and select the found ones in FixItList and in the editor
    for (int32 i = 0; i < FixItList->GetNumItemsBeingObserved (); i++) {
        FFixItNoteListItemPtr a = FixItList->ItemAt (i);

        if (a.IsValid () && a->FixItNote.IsValid ()) {
            //FString txt = (a->StickyNote)->Text.ToString ().ToLower ();
            //FString tag = (a->StickyNote)->SearchTags.ToString ();
            //FString recipient = (a->StickyNote)->Recipient.ToString ();

            UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----SFixItWidget::UpdateList list contains item: Text '%s' or Recipient '%s' or Tags '%s'"), *a->FixItNote->Text.ToString ().ToLower (), *a->FixItNote->Recipient.ToString ().ToLower (), *a->FixItNote->SearchTags.ToString ());
        }
    }
    FixItList->RequestListRefresh ();
}


void SFixItWidget::RegenerateWidget (AFixItNote* n) {
    FixItList->RegenerateWidget (n);
}


//void SFixItWidget::HandleListSelectionChanged (IMessageTracerBreakpointPtr InItem, ESelectInfo::Type SelectInfo) {
void SFixItWidget::HandleListSelectionChanged (FFixItNoteListItemPtr sn, ESelectInfo::Type SelectInfo) {
    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----SFixItWidget::HandleListSelectionChanged ESelectInfo = %s"), (SelectInfo == ESelectInfo::OnMouseClick ? TEXT("OnMouseClick") : TEXT("Other")));

    if (worldOutlinerSelectionChanging)
        return;

    if (!sn.IsValid () || !sn->FixItNote.IsValid ())
        return;

    listSelectionChanging = true;       // Prevent recursion selecting it in the World Outliner, triggering a call to select it in our list again...

    // TODO: Select in the editor, the sticky note they selected in the list...
    //GWorld->GetCurrentLevel ()->
    //sn->
    //TArray <AActor *> actors = GWorld->GetCurrentLevel ()->Actors;
    //for (int i = 0; i < actors.Num (); i++) {
    //    AActor *a = actors [i];
    //    if (a->IsSelected)
    //}

    // Move to face the selected sticky note if they clicked on it
    if (SelectInfo == ESelectInfo::OnMouseClick) {
        FEditorViewportClient *client = (FEditorViewportClient *) GEditor->GetActiveViewport ()->GetClient ();
        client->SetViewRotation (sn->FixItNote->GetActorRotation ());
        client->SetViewLocation (sn->FixItNote->GetActorLocation () - sn->FixItNote->GetActorForwardVector () * 400);
    }

    // In the World Outliner, select only all of the FixIt notes selected in our list now
    GEditor->SelectNone (false, true, false);     // Deselect all other actors in the World Outliner
    for (int i = 0; i < FixItList->GetNumItemsSelected (); i++) {
        UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----SFixItWidget::HandleListSelectionChanged selected note %d in our list, in the World Outliner"), i+1);

        GEditor->SelectActor (FixItList->GetSelectedItems () [i]->FixItNote.Get (), true, true, true);
    }

    listSelectionChanging = false;
}

// Create a new sticky note
FReply SFixItWidget::OnCreateButtonClicked () {
    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----SFixItWidget::OnCreateButtonClicked"));


	FFixItEdMode *StickyNoteEdMode = GetEditorMode ();
	if (StickyNoteEdMode != NULL) {
		UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----SFixItWidget::OnCreateButtonClicked correct EditMode found"));

		// Position new actor relative to current view location/direction
		FEditorViewportClient *client = (FEditorViewportClient *) GEditor->GetActiveViewport ()->GetClient ();
		FVector editorCameraDirection = client->GetViewRotation ().Vector ();
		FVector editorCameraLocation = client->GetViewLocation ();

		FVector spawnLocation = editorCameraLocation + (editorCameraDirection * 300.0f);
		// Rotation not correct, but should not be issue if text can be tied to billboard
		FRotator spawnRotation = client->GetViewRotation ();
		AFixItNote *StickyNote = StickyNoteEdMode->GetWorld ()->SpawnActor <AFixItNote> (spawnLocation, spawnRotation);

        StickyNote->Priority = Priority;
        StickyNote->Text = Text;
        StickyNote->Recipient = Recipient;
        StickyNote->SearchTags = Tags;
        StickyNote->Update ();

//        FFixItEdMode::AddStickyNote (Text, Priority, spawnLocation, StickyNote);

	} else {
		UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----SFixItWidget::OnCreateButtonClicked wrong EditMode found"));
	}

    return FReply::Handled ();
}

//void SFixItWidget::OnAssetsDeleted (const TArray<UClass*>& DeletedAssetClasses) {
//    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----SFixItWidget::OnAssetsDeleted..."));
//
//    FFixItEdMode::AssetsDeletedHook (DeletedAssetClasses);
//}
//void SFixItWidget::OnObjectsReplaced (const TMap<UObject*, UObject*>& ReplacementMap) {
//    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----SFixItWidget::OnObjectsReplaced..."));
//}

void SFixItWidget::OnStickyNoteSearchStringChanged(const FText &InText) {
    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----SFixItWidget::OnStickyNoteSearchStringChanged to: '%s'"), *InText.ToString ());

    SearchString = InText;

    OnSearchButtonClicked ();
}

FReply SFixItWidget::OnSearchButtonClicked() {
    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----SFixItWidget::OnSearchButtonClicked: Search for %s"), *SearchString.ToString ());

    // Search the FixItList and select the found ones in FixItList and in the editor
    FixItList->ClearSelection ();
    //FFixItEdMode *em = GetEditorMode ();
    for (int32 i = 0; i < FixItList->GetNumItemsBeingObserved (); i++) {
        FFixItNoteListItemPtr a = FixItList->ItemAt(i);

        if (a.IsValid() && a->FixItNote.IsValid()) {
            FString txt = (a->FixItNote)->Text.ToString().ToLower();
            FString tag = (a->FixItNote)->SearchTags.ToString();
            FString recipient = (a->FixItNote)->Recipient.ToString ();
            FString search = SearchString.ToString().ToLower();
            if (txt.Contains (search) || tag.Contains (search) || recipient.Contains (search)) {
                UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----Search found item: Text '%s' or Recipient '%s' or Tags '%s' contain '%s'"), *a->FixItNote->Text.ToString ().ToLower (), *a->FixItNote->Recipient.ToString ().ToLower (), *a->FixItNote->SearchTags.ToString (), *SearchString.ToString ().ToLower ());

                // Select it in the list
                FixItList->SetItemSelection(a, true);

                // Select it in the editor window
                AFixItNote *sn = a->FixItNote.Get ();
                GEditor->SelectActor (sn, true, true);
            }
        }
    }

    // Sometimes it doesn't redraw the list and this is to fix that
    //FixItList->RequestListRefresh ();


    return FReply::Handled ();
}

EFixItNotePriorityFilter SFixItWidget::ViewPriority;

void SFixItWidget::OnStickyNoteViewPriorityChanged (TSharedPtr<EFixItNotePriorityFilter> NewSelection, ESelectInfo::Type /*SelectInfo*/) {
    ViewPriority = *NewSelection;

    // Update list to reflect new filter settings
    // Hide the sticky notes with filtered priorities
    FFixItEdMode::FilterStickyNotesInWorld (ViewPriority);

    // Display the list of visible sticky notes on the editor mode tab
    FixItList->Filter (ViewPriority);
}

void SFixItWidget::OnStickyNotePriorityChanged (TSharedPtr<EFixItNotePriority> NewSelection, ESelectInfo::Type /*SelectInfo*/) {
	Priority = *NewSelection;
    AFixItNote::ConstructorStatics.Priority = Priority;
}

void SFixItWidget::OnStickyNoteRecipientChanged (const FText &InText) {
    Recipient = InText;
    AFixItNote::ConstructorStatics.Recipient = InText;
}

void SFixItWidget::OnStickyNoteTextChanged (const FText &InText) {
    Text = InText;
    AFixItNote::ConstructorStatics.Text = InText;
}

void SFixItWidget::OnStickyNoteTagsChanged (const FText &InText) {
    Tags = InText;
    AFixItNote::ConstructorStatics.SearchTags = InText;
}

TSharedRef<ITableRow> SFixItWidget::MakeListWidget (FFixItNoteListItemPtr InItem, const TSharedRef<STableViewBase>& Owner) {
    if (InItem.IsValid ()) {
        FText widgetWords = InItem->FixItNote->Text;
        return
            SNew (STableRow <FFixItNoteListItemPtr>, Owner) [
                SNew (STextBlock)
                .Text (widgetWords)
                .ToolTipText (widgetWords)
            ];
    } else {
        return
            SNew (STableRow<FFixItNoteListItemPtr>, Owner);
    }
}

void SFixItWidget::OnLevelActorsAdded (AActor* InActor) {
    if (!bIsReentrant) {
        if (FFixItEdMode::AddStickyNote (InActor)) {
            FixItList->RequestListRefresh ();
        }
    }
}

void SFixItWidget::OnLevelActorsRemoved (AActor* InActor) {
    if (!bIsReentrant) {
        if (FFixItEdMode::RemoveStickyNote (InActor)) {
            FixItList->RequestListRefresh ();
        }
    }
}

void UWatchDog::OnObjectModified (UObject *ObjectModified)
{
    if (widget && ObjectModified && ObjectModified->IsA (UTextRenderComponent::StaticClass ())) {
        UObject* parent = ObjectModified->GetOuter ();
        if (parent && parent->IsA (AFixItNote::StaticClass ())) {
            widget->RegenerateWidget ((AFixItNote*) parent);
        }
    }
}

void SFixItWidget::OnLevelSelectionChanged (UObject* Obj) {
    if (listSelectionChanging)
        return;

    if (!bIsReentrant) {
        UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----SFixItWidget::OnLevelSelectionChanged"));

        worldOutlinerSelectionChanging = true;      // Prevent recursion selecting in our list triggering a call to select it in the World Outliner again...

        TArray <UObject *> selectedFixItNotes;
        GEditor->GetSelectedActors ()->GetSelectedObjects (AFixItNote::StaticClass (), /*out*/ selectedFixItNotes);
        FixItList->ClearSelection ();      // Start by clearing all selections in our list
        for (int j = 0; j < selectedFixItNotes.Num (); j++) {       // For each selected AFixItNote in the World Outliner
            AFixItNote *fin = (AFixItNote *) selectedFixItNotes [j];
            //UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----SFixItWidget::OnLevelSelectionChanged looking at note %d selected in World Outliner"), j);

            // Search for it in the FixItList selected items
            for (int i = 0; i < FixItList->GetNumItemsBeingObserved (); i++) {
                if (FixItList->ItemAt (i)->FixItNote.Get () == fin) {     // Found this note from the World Outliner, in our list
                    //UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----SFixItWidget::OnLevelSelectionChanged found same note in our list at line %d"), i);

                    if (!FixItList->GetSelectedItems ().Contains (FixItList->ItemAt (i))) {
                        //UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----SFixItWidget::OnLevelSelectionChanged selecting it"));

                        // If it's not selected in our list, select it now
                        FixItList->SetItemSelection (FixItList->ItemAt (i), true);
                    }
                    break;
                }
            }
        }

        worldOutlinerSelectionChanging = false;
    }
}

#undef LOCTEXT_NAMESPACE