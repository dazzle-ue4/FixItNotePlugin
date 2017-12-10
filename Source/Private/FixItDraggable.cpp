// Copyright (C) 2015 Skunkwerks Kinetic Industries Inc. All Rights Reserved.

#include "FixItPluginPCH.h"
#include "LevelEditor.h"
#include "AssetSelection.h"
#include "DragAndDrop/AssetDragDropOp.h"
#include "EditorClassUtils.h"
#include "FixItDraggable.h"


#define LOCTEXT_NAMESPACE "SFixItDraggable"

/**
* This is the asset thumbnail.
*/
class SFixItDraggableThumbnail : public SCompoundWidget {
public:
    SLATE_BEGIN_ARGS(SFixItDraggableThumbnail)
        : _Width(32)
        , _Height(32)
        , _AlwaysUseGenericThumbnail(false)
        , _AssetTypeColorOverride()
    {}

    SLATE_ARGUMENT(uint32, Width)

        SLATE_ARGUMENT(uint32, Height)

        SLATE_ARGUMENT(FName, ClassThumbnailBrushOverride)

        SLATE_ARGUMENT(bool, AlwaysUseGenericThumbnail)

        SLATE_ARGUMENT(TOptional<FLinearColor>, AssetTypeColorOverride)
        SLATE_END_ARGS()

     void Construct(const FArguments& InArgs, const FAssetData& InAsset) {
        Asset = InAsset;

        FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
        TSharedPtr<FAssetThumbnailPool> ThumbnailPool = LevelEditorModule.GetFirstLevelEditor()->GetThumbnailPool();

        Thumbnail = MakeShareable(new FAssetThumbnail(Asset, InArgs._Width, InArgs._Height, ThumbnailPool));

        FAssetThumbnailConfig Config;
        Config.bForceGenericThumbnail = InArgs._AlwaysUseGenericThumbnail;
        Config.ClassThumbnailBrushOverride = InArgs._ClassThumbnailBrushOverride;
        Config.AssetTypeColorOverride = InArgs._AssetTypeColorOverride;
        ChildSlot [
                Thumbnail->MakeThumbnailWidget(Config)
            ];
    }

private:

    FAssetData Asset;
    TSharedPtr< FAssetThumbnail > Thumbnail;
};

void SFixItDraggable::Construct(const FArguments& InArgs, UActorFactory* InFactory, const FAssetData& InAsset) {
    bIsPressed = false;

    FactoryToUse = InFactory;
    AssetData = InAsset;

    TSharedPtr< SHorizontalBox > ActorType = SNew(SHorizontalBox);

    const bool IsClass = AssetData.GetClass() == UClass::StaticClass();
    const bool IsVolume = IsClass ? CastChecked<UClass>(AssetData.GetAsset())->IsChildOf(AVolume::StaticClass()) : false;

    if (IsClass) {
        AssetDisplayName = FText::FromString(FName::NameToDisplayString(AssetData.AssetName.ToString(), false));
    } else {
        AssetDisplayName = FText::FromName(AssetData.AssetName);
    }

    FText ActorTypeDisplayName;
    AActor* DefaultActor = nullptr;
    if (FactoryToUse != nullptr) {
        DefaultActor = FactoryToUse->GetDefaultActor(AssetData);
        ActorTypeDisplayName = FactoryToUse->GetDisplayName();
    } else if (IsClass && CastChecked<UClass>(AssetData.GetAsset())->IsChildOf(AActor::StaticClass())) {
        DefaultActor = CastChecked<AActor>(CastChecked<UClass>(AssetData.GetAsset())->ClassDefaultObject);
        ActorTypeDisplayName = FText::FromString(FName::NameToDisplayString(DefaultActor->GetClass()->GetName(), false));
    }

    UClass* DocClass = nullptr;
    TSharedPtr<IToolTip> ToolTip;
    if (DefaultActor != nullptr) {
        DocClass = DefaultActor->GetClass();
        ToolTip = FEditorClassUtils::GetTooltip(DefaultActor->GetClass());
    }

    if (IsClass && !IsVolume && !ActorTypeDisplayName.IsEmpty()) {
        AssetDisplayName = ActorTypeDisplayName;
    }

    if (!ToolTip.IsValid()) {
        ToolTip = FSlateApplicationBase::Get().MakeToolTip(AssetDisplayName);
    }

    const FButtonStyle& ButtonStyle = FEditorStyle::GetWidgetStyle <FButtonStyle> ("PlacementBrowser.Asset");

    NormalImage = &ButtonStyle.Normal;
    HoverImage = &ButtonStyle.Hovered;
    PressedImage = &ButtonStyle.Pressed;

    // Create doc link widget if there is a class to link to
    TSharedRef<SWidget> DocWidget = SNew(SSpacer);
    if (DocClass != NULL) {
        DocWidget = FEditorClassUtils::GetDocumentationLinkWidget(DocClass);
        DocWidget->SetCursor(EMouseCursor::Default);
    }

    ChildSlot [
            SNew(SBorder)
            .BorderImage(this, &SFixItDraggable::GetBorder)
            .Cursor(EMouseCursor::GrabHand)
            .ToolTip(ToolTip) [
                SNew(SHorizontalBox)

                + SHorizontalBox::Slot()
                .Padding(0)
                .AutoWidth() [
                    // Drop shadow border
                    SNew(SBorder)
                    .Padding(4)
                    .BorderImage(FEditorStyle::GetBrush("ContentBrowser.ThumbnailShadow")) [
                        SNew(SBox)
                        .WidthOverride(35)
                        .HeightOverride(35) [
                            SNew(SFixItDraggableThumbnail, AssetData)
                            .ClassThumbnailBrushOverride("ClassThumbnail.Note")
                        ]
                    ]
                ]

                + SHorizontalBox::Slot()
                .VAlign(VAlign_Center)
                .Padding(2, 0, 4, 0) [

                    SNew(SVerticalBox)
                    + SVerticalBox::Slot()
                    .Padding(0, 0, 0, 1)
                    .AutoHeight() [
                        SNew(STextBlock)
                        .Text(InArgs._Text)
                    ]
                ]

                + SHorizontalBox::Slot()
                .VAlign(VAlign_Center)
                .AutoWidth() [
                    DocWidget
                ]
            ]
        ];
}

FReply SFixItDraggable::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
    if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton) {
        bIsPressed = true;

        return FReply::Handled().DetectDrag(SharedThis(this), MouseEvent.GetEffectingButton());
    }

    return FReply::Unhandled();
}

FReply SFixItDraggable::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
    if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton) {
        bIsPressed = false;
    }

    return FReply::Unhandled();
}

FReply SFixItDraggable::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
    bIsPressed = false;

    return FReply::Handled().BeginDragDrop(FAssetDragDropOp::New(AssetData, FactoryToUse));
}

bool SFixItDraggable::IsPressed() const {
    return bIsPressed;
}

const FSlateBrush* SFixItDraggable::GetBorder() const {
    if (IsPressed()) {
        return PressedImage;
    } else if (IsHovered()) {
        return HoverImage;
    } else {
        return NormalImage;
    }
}

#undef LOCTEXT_NAMESPACE