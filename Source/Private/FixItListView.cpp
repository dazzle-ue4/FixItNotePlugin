// Copyright (C) 2015 Skunkwerks Kinetic Industries Inc. All Rights Reserved.

#include "FixItPluginPCH.h"
#include "FixItListView.h"
#include "FixItCommon.h"
#include "FixIt.h"


#define LOCTEXT_NAMESPACE "SFixItListView"

/** Construct this item from a sticky note */
FFixItNoteListItem::FFixItNoteListItem (AFixItNote* InNote) : FixItNote (InNote), ID (InNote) {
}

FListItemID FFixItNoteListItem::GetID () const {
    return ID;
}


// Does the priority match the filter?
bool PriorityMatchesFilter2(EFixItNotePriority priority, EFixItNotePriorityFilter filter) {
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


void SFixItListView::Filter(EFixItNotePriorityFilter filter) {
    for (int32 i = 0; i < GetNumItemsBeingObserved (); i++) {
        FFixItNoteListItemPtr a = (*ItemsSource) [i];
        if (a.IsValid() && a->FixItNote.IsValid() && PriorityMatchesFilter2(a->FixItNote->Priority, filter)) {
            //UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----Filter list: IN: %s"), *a->Text.ToString ());
            TSharedPtr<ITableRow> w = WidgetFromItem (a);
            if (w.IsValid ()) {
                w->AsWidget ()->SetVisibility (EVisibility::Visible);
            }
        } else {
            //UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----Filter list: OUT: %s"), *a->Text.ToString ());
            TSharedPtr<ITableRow> w = WidgetFromItem (a);
            if (w.IsValid ()) {
                w->AsWidget ()->SetVisibility (EVisibility::Collapsed);
            }
        }
    }

    // Sometimes it doesn't redraw the list and this is to fix that
    RequestListRefresh ();
}

FFixItNoteListItemPtr SFixItListView::ItemAt (int i) {
    return (*ItemsSource) [i];
}

/** Regenerate the widget for the given FixItNote **/
void SFixItListView::RegenerateWidget (AFixItNote* n) {
    WidgetGenerator.Clear ();
    //for (int32 i = 0; i < GetNumItemsBeingObserved (); i++) {
    //    FFixItNoteListItemPtr a = (*ItemsSource)[i];
    //    //if (a.IsValid () && a->StickyNote.IsValid () && a->StickyNote->ID == n->ID) {
    //        GenerateNewWidget ((*ItemsSource)[i]);
    //    //    break;
    //    //}
    //}
    RequestListRefresh ();
}

#undef LOCTEXT_NAMESPACE
