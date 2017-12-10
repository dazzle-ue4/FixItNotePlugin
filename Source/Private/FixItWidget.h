// Copyright (C) 2015 Skunkwerks Kinetic Industries Inc. All Rights Reserved.

#pragma once
#include "FixItPluginPCH.h"
#include "FFixItEdMode.h"
#include "FixItListView.h"


#define LOCTEXT_NAMESPACE "SFixItWidget"

class SFixItWidget;

class UWatchDog : public UObject {
public:
    SFixItWidget* widget;

    /** Called by the engine when any object is modified. */
    void OnObjectModified (UObject *ObjectModified);

};

class SFixItWidget : public SCompoundWidget {
protected:
	// Copied from LandscapeEditorDetailCustomization_Base.h
	// Called from LandscapeEditorDetailCustomization_NewLandscape's OnCreateButtonClicked
	//   which is copied into this class' OnCreateButtonClicked
	static FFixItEdMode *GetEditorMode ();

public:
	SLATE_BEGIN_ARGS (SFixItWidget) {}
	SLATE_END_ARGS ()

public:
    ~SFixItWidget ();

	void Construct (const FArguments &InArgs);

    FText SearchString;
    void OnStickyNoteSearchStringChanged (const FText &InText);       // Called when the search string is changed
	FReply OnSearchButtonClicked ();

    FReply OnCreateButtonClicked ();

    //void OnAssetsDeleted (const TArray <UClass *> &DeletedAssetClasses);
    //void OnObjectsReplaced (const TMap <UObject *, UObject *> &ReplacementMap);

    ECheckBoxState GetPaintingActive () const {
        return PaintingEnabled ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
    }

    void OnPaintingCheckboxChanged (ECheckBoxState NewState) {
        UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----SFixItWidget::OnPaintingCheckboxChanged"));

        PaintingEnabled = (NewState == ECheckBoxState::Checked);
    }

    static EFixItNotePriorityFilter ViewPriority;
    void UpdateList ();

    // When a FixItNote has changed, regenerate its widget as displayed in the list
    void RegenerateWidget (AFixItNote* n);

    TWeakObjectPtr<UWatchDog> watchdog;

protected:
    // These are to prevent recursively selecting things in our search list and the world outliner when either one is changed
    // Otherwise changing either one fires an event that triggers the other and you get an infinite recursion
    bool worldOutlinerSelectionChanging;
    bool listSelectionChanging;

    void OnStickyNoteViewPriorityChanged (TSharedPtr <EFixItNotePriorityFilter> NewSelection, ESelectInfo::Type /*SelectInfo*/);    	/** Called when the Sticky Note priority radio Button selection is changed */

    EFixItNotePriority Priority;
    void OnStickyNotePriorityChanged (TSharedPtr <EFixItNotePriority> NewSelection, ESelectInfo::Type /*SelectInfo*/);    	/** Called when the Sticky Note priority radio Button selection is changed */

    FText Recipient;
    void OnStickyNoteRecipientChanged (const FText &InText);       // Called when recipient of the sticky note is changed

    FText Text;
    void OnStickyNoteTextChanged (const FText &InText);           // Called when text for the sticky note is changed

    FText Tags;
    void OnStickyNoteTagsChanged (const FText &InText);            // Called when tags for the sticky note is changed

    TSharedPtr <SFixItListView> FixItList;

    TSharedRef <ITableRow> MakeListWidget (FFixItNoteListItemPtr InItem, const TSharedRef <STableViewBase> &);

private:
    // combobox options for selecting priority
    TArray <TSharedPtr <EFixItNotePriority>> PriorityComboItems;

    // fill in combobox items
    void GeneratePriorityComboItems ();
        
    TSharedRef <SWidget> MakePriorityComboItemWidget (TSharedPtr <EFixItNotePriority> Priority);
        

	// combobox options for selecting priority filter
	TArray <TSharedPtr <EFixItNotePriorityFilter>> PriorityFilterComboItems;

	// fill in combobox items
	void GeneratePriorityFilterComboItems();

	TSharedRef <SWidget> MakePriorityFilterComboItemWidget(TSharedPtr <EFixItNotePriorityFilter> Filter);

	/**
    * Return the text for the specified priority
    */
    FText MakePriorityComboText (EFixItNotePriority PriorityToText) const {
        FText PriorityText;

        switch (PriorityToText) {
        case EFixItNotePriority::Urgent:
            PriorityText = NSLOCTEXT ("SFixItWidget", "PriorityHigh", "High");
            break;

        case EFixItNotePriority::Moderate:
            PriorityText = NSLOCTEXT ("SFixItWidget", "PriorityMedium", "Medium");
            break;

        case EFixItNotePriority::Low:
            PriorityText = NSLOCTEXT ("SFixItWidget", "PriorityLow", "Low");
            break;

        //case EFixItNotePriority::Comment:
        //    PriorityText = NSLOCTEXT ("SFixItWidget", "PriorityComment", "Comment");
        //    break;
        }

        return PriorityText;
    }

	/**
	* Return the text for the specified priority
	*/
	FText MakePriorityFilterComboText(EFixItNotePriorityFilter Filter) const {
		FText PriorityText;

		switch (Filter) {
		case EFixItNotePriorityFilter::Urgent:
			PriorityText = NSLOCTEXT("SFixItWidget", "PriorityHigh", "High");
			break;

		case EFixItNotePriorityFilter::Moderate:
			PriorityText = NSLOCTEXT("SFixItWidget", "PriorityMedium", "Medium");
			break;

		case EFixItNotePriorityFilter::Low:
			PriorityText = NSLOCTEXT("SFixItWidget", "PriorityLow", "Low");
			break;

		case EFixItNotePriorityFilter::All:
			PriorityText = NSLOCTEXT("SFixItWidget", "PriorityAll", "-- All --");
			break;

		}

		return PriorityText;
	}

	// value to display in view priority combobox
    FText GetViewPriorityComboBoxContent () const {
        return MakePriorityFilterComboText (ViewPriority);
    }

    // value to display in priority combobox
    FText GetPriorityComboBoxContent () const {
        return MakePriorityComboText (Priority);
    }

    void HandleListMouseClick (FFixItNoteListItemPtr sn);
    void HandleListSelectionChanged (FFixItNoteListItemPtr sn, ESelectInfo::Type SelectInfo);

    UPROPERTY (VisibleAnywhere, BlueprintReadWrite, Category = Rendering)
    //UMaterialBillboardComponent *Billboard;

    bool PaintingEnabled;

    /** Reentrancy guard */
    bool bIsReentrant;

    /** Called by the engine when an actor is added to the world. */
    void OnLevelActorsAdded (AActor *InActor);

    /** Called by the engine when an actor is remove from the world. */
    void OnLevelActorsRemoved (AActor *InActor);

    /** Called by USelection::SelectionChangedEvent delegate when the level's selection changes */
    void OnLevelSelectionChanged (UObject *Obj);

};

#undef LOCTEXT_NAMESPACE
