// Copyright (C) 2015 Skunkwerks Kinetic Industries Inc. All Rights Reserved.

#include "FixItPluginPCH.h"

#include "FixItEdModeStyle.h"
#include "SlateGameResources.h"
#include "IPluginManager.h"

TSharedPtr< FSlateStyleSet > FixItEdModeStyle::StyleInstance = NULL;

void FixItEdModeStyle::Initialize () {
	if (!StyleInstance.IsValid ()) {
		StyleInstance = Create ();
		FSlateStyleRegistry::RegisterSlateStyle (*StyleInstance);
	}
}

void FixItEdModeStyle::Shutdown () {
	FSlateStyleRegistry::UnRegisterSlateStyle (*StyleInstance);
	ensure (StyleInstance.IsUnique ());
	StyleInstance.Reset ();
}

FName FixItEdModeStyle::GetStyleSetName () {
	static FName StyleSetName (TEXT ("FixItNoteEdModeStyle"));
	return StyleSetName;
}

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BORDER_BRUSH( RelativePath, ... ) FSlateBorderBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define TTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".ttf") ), __VA_ARGS__ )
#define OTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".otf") ), __VA_ARGS__ )

const FVector2D Icon16x16 (16.0f, 16.0f);
const FVector2D Icon20x20 (20.0f, 20.0f);
const FVector2D Icon40x40 (40.0f, 40.0f);
const FVector2D Icon128x128 (128.0f, 128.0f);

TSharedRef <FSlateStyleSet> FixItEdModeStyle::Create () {
	TSharedRef <FSlateStyleSet> Style = MakeShareable (new FSlateStyleSet ("FixItNoteEdModeStyle"));
    Style->SetContentRoot (IPluginManager::Get ().FindPlugin ("FixItNotePlugin")->GetBaseDir () / TEXT ("Resources"));

    Style->Set ("DefaultPluginIcon", new IMAGE_BRUSH (TEXT("DefaultIcon40"), Icon40x40));
    Style->Set ("DefaultPluginIcon.small", new IMAGE_BRUSH (TEXT ("DefaultIcon128"), Icon20x20));

    return Style;
}

#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef BORDER_BRUSH
#undef TTF_FONT
#undef OTF_FONT

void FixItEdModeStyle::ReloadTextures () {
	FSlateApplication::Get().GetRenderer ()->ReloadTextureResources ();
}

const ISlateStyle& FixItEdModeStyle::Get () {
	return *StyleInstance;
}
