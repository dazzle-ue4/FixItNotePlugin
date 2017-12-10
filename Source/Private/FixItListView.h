// Copyright (C) 2015 Skunkwerks Kinetic Industries Inc. All Rights Reserved.

#pragma once
#include "FixItPluginPCH.h"
#include "FixItCommon.h"
#include "FixIt.h"
#include "ObjectKey.h"


/** Variant type that defines an identifier for a list item. Assumes 'trivial relocatability' as with many unreal containers. */
/** Copied from SceneOutlinerStandaloneTypes.h, struct FTreeItemID */
struct FListItemID {
public:
    /** Default constructed null item ID */
    FListItemID () : Type (EType::Null), CachedHash(0) {}

    /** ID representing a UObject */
    FListItemID (const UObject* InObject) : Type (EType::Object) {
        check (InObject);
        new (Data) FObjectKey (InObject);
        CachedHash = CalculateTypeHash ();
    }
    FListItemID (const FObjectKey &InKey) : Type (EType::Object) {
        new (Data) FObjectKey (InKey);
        CachedHash = CalculateTypeHash ();
    }

    /** Copy construction / assignment */
    FListItemID (const FListItemID &Other) {
        *this = Other;
    }
    FListItemID &operator = (const FListItemID &Other) {
        Type = Other.Type;
        new (Data)FObjectKey (Other.GetAsObjectKey ());

        CachedHash = CalculateTypeHash ();
        return *this;
    }

    /** Move construction / assignment */
    FListItemID (FListItemID &&Other) {
        *this = MoveTemp (Other);
    }
    FListItemID &operator = (FListItemID &&Other) {
        FMemory::Memswap (this, &Other, sizeof (FListItemID));
        return *this;
    }

    ~FListItemID () {
        GetAsObjectKey ().~FObjectKey ();
    }

    friend bool operator == (const FListItemID &One, const FListItemID &Other) {
        return One.Type == Other.Type && One.CachedHash == Other.CachedHash && One.Compare (Other);
    }
    friend bool operator!=(const FListItemID &One, const FListItemID &Other) {
        return One.Type != Other.Type || One.CachedHash != Other.CachedHash || !One.Compare (Other);
    }

    uint32 CalculateTypeHash () const {
        uint32 Hash = 0;
        Hash = GetTypeHash (GetAsObjectKey ());

        return HashCombine ((uint8) Type, Hash);
    }

    friend uint32 GetTypeHash (const FListItemID &ItemID) {
        return ItemID.CachedHash;
    }

private:
    FObjectKey &GetAsObjectKey () const {
        return *reinterpret_cast <FObjectKey *> (Data);
    }

    /** Compares the specified ID with this one - assumes matching types */
    bool Compare (const FListItemID &Other) const {
        return GetAsObjectKey () == Other.GetAsObjectKey ();
    }

    enum class EType : uint8 { Object, Null };
    EType Type;

    uint32 CachedHash;
    static const uint32 MaxSize = sizeof (FObjectKey);
    mutable uint8 Data [MaxSize];
};

// An item displayed in the sticky note list
struct FFixItNoteListItem {
    /** The sticky note this list item is associated with. */
    mutable TWeakObjectPtr <AFixItNote> FixItNote;

    /** Constant identifier for this list item */
    const FObjectKey ID;

    /** Get the ID that represents this tree item. Used to reference this item in a map */
    FListItemID GetID () const;

    /** Construct this item from a sticky note */
    FFixItNoteListItem (AFixItNote* InNote);

};

typedef TSharedPtr<FFixItNoteListItem> FFixItNoteListItemPtr;
typedef TMap<FListItemID, FFixItNoteListItemPtr> ListItemMap;

class SFixItListView : public SListView<FFixItNoteListItemPtr> {
public:
    void Filter (EFixItNotePriorityFilter filter);
    FFixItNoteListItemPtr ItemAt (int i);
    void RegenerateWidget (AFixItNote* n);
};
