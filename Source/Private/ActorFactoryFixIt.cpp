// Copyright (C) 2015 Skunkwerks Kinetic Industries Inc. All Rights Reserved.

#include "FixItPluginPCH.h"
#include "FixItPlugin.h"
#include "FixIt.h"
#include "ActorFactoryFixIt.h"


#define LOCTEXT_NAMESPACE "UActorFactoryFixItNote" 

UActorFactoryFixItNote::UActorFactoryFixItNote (const FObjectInitializer &ObjectInitializer) : Super (ObjectInitializer) {
    DisplayName = LOCTEXT ("FixItNoteDisplayName", "FixIt Note");
    NewActorClass = AFixItNote::StaticClass ();
}

bool UActorFactoryFixItNote::CanCreateActorFrom (const FAssetData &AssetData, FText &OutErrorMsg) {
    return AssetData.ObjectPath == FName (*AFixItNote::StaticClass ()->GetPathName ());
}

#undef LOCTEXT_NAMESPACE