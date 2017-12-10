// Copyright (C) 2015 Skunkwerks Kinetic Industries Inc. All Rights Reserved.

#pragma once
#include "FixItPluginPCH.h"

class SFixItDraggable : public SCompoundWidget {
public:
    SLATE_BEGIN_ARGS(SFixItDraggable)
        : _Text()
    {}

    SLATE_ATTRIBUTE(FText, Text)

    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, UActorFactory* InFactory, const FAssetData& InAsset);

    virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

    bool IsPressed() const;

    FText AssetDisplayName;

    UActorFactory* FactoryToUse;
    FAssetData AssetData;

private:
    const FSlateBrush* GetBorder() const;

    bool bIsPressed;

    /** Brush resource that represents a button */
    const FSlateBrush* NormalImage;
    /** Brush resource that represents a button when it is hovered */
    const FSlateBrush* HoverImage;
    /** Brush resource that represents a button when it is pressed */
    const FSlateBrush* PressedImage;
};

