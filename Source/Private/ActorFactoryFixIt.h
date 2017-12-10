// Copyright (C) 2015 Skunkwerks Kinetic Industries Inc. All Rights Reserved.

#pragma once

//#include "ActorFactories/ActorFactory.h"
//#include "FixItPluginPCH.h"
//#include "FixItPlugin.h"
#include "ActorFactoryFixIt.generated.h"

/**
*
*/
UCLASS()
class UActorFactoryFixItNote : public UActorFactory {
    GENERATED_UCLASS_BODY()

public:
    virtual bool CanCreateActorFrom (const FAssetData &AssetData, FText &OutErrorMsg) override;
};
