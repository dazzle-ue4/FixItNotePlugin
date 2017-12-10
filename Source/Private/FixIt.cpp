// Copyright (C) 2015 Skunkwerks Kinetic Industries Inc. All Rights Reserved.

#include "FixItPluginPCH.h"
#include "FixItPlugin.h"
#include "FixIt.h"
#include "FixItWidget.h"

/**************************Added By Linda******************/
#if defined (UNICODE)
#define _T(x)  L ## x
#else
#define _T(x) x
#endif
/***********************************************************/

#define LOCTEXT_NAMESPACE "AFixItNote"


AFixItNote::FConstructorStatics AFixItNote::ConstructorStatics;

// Sets default values
AFixItNote::AFixItNote (const FObjectInitializer &ObjectInitializer) : Super (ObjectInitializer) {
    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----AFixItNote::AFixItNote - Creating a new FixIt note"));

    // Set this actor to NOT call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;
    PrimaryActorTick.bTickEvenWhenPaused = true;
    //PrimaryActorTick.bAllowTickOnDedicatedServer = true;
    this->SetActorTickEnabled (true);
    //bTickInEditor = true;


    // Should only be visible in editor
    SetActorHiddenInGame (true);

	// A sphere component to act as root component of the actor
	RootSphere = CreateDefaultSubobject <USphereComponent> (TEXT("Sphere"));
	RootSphere->SetSphereRadius (0.f);
	RootComponent = RootSphere;

    // The main component is a billboard which always faces the camera
    Billboard = CreateDefaultSubobject <UBillboardComponent> (TEXT ("Billboard"));
	// The billboard should be raised up for visibility when dragged onto the level
	Billboard->SetRelativeLocation(FVector(0.f, 0.f, 70.f));
	Billboard->AttachParent = RootSphere;


    //--------------------------------------------------------------
    // Add text to the sticky note, scaled and rotated appropriately
    //--------------------------------------------------------------
    TextComp = CreateDefaultSubobject <UTextRenderComponent> (TEXT ("Text"));
    //TextComp->SetText (LOCTEXT ("StickyNoteText", "Skunkwerks\nSticky Note"));
    TextComp->HorizontalAlignment = EHTA_Center;
    TextComp->VerticalAlignment = EVRTA_TextCenter;
    TextComp->XScale = 1.0f;
    TextComp->YScale = 1.0f;

    // Need to flip the text around X and Y, so 180, 180, 0.  But that doesn't work (it doesn't set the rotation to that).
    //      TextComp->SetRelativeRotation (FRotator (180, 180, 0));
    // In fact I get seemingly random results with various X, Y, Z values to that.
    // But this works in v4.8:
    TextComp->SetRelativeRotation (FRotator (FQuat (0, 0, 180, 1)));
    // And this works in v4.7:
    // But the FRotator (FQuat... approach works somewhat, although with a bug that limits the Y rotation to 90 degrees (https://answers.unrealengine.com/questions/214801/pitch-rotation-problem.html)
    //TextComp->SetRelativeRotation (FRotator (FQuat (180, 90, 0, 1)));
    // My workaround is to add another 90 degrees (but for some strange reason it needs to be inverted, so -90) to Y:
    //TextComp->AddRelativeRotation (FRotator (FQuat (0, -90, 0, 1)));

    TextComp->AddLocalOffset (FVector (2, 0, 9));           // Put the text slightly in front of the billboard background (otherwise about half of it is obscured)
    TextComp->SetTextRenderColor (FColor::Black);           // Text colour
//    TextComp->SetActorHiddenInGame (true);
    TextComp->AttachParent = Billboard;

    Priority = ConstructorStatics.Priority;
    Recipient = ConstructorStatics.Recipient;
    Text = ConstructorStatics.Text;
    SearchTags = ConstructorStatics.SearchTags;

    Update ();      // Sets background colour of the sticky note and adds the text
}

//AFixItNote::~AFixItNote () {
//    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----AFixItNote::~AFixItNote"));
//}


/**************************Modified By Linda   ******************/
/********************change "char *newLineStr" to "const char *newLineStr"*********/
FText AFixItNote::wrap (FText ftxt, int wrapLength, const char *newLineStr, bool wrapLongWords, FVector2D* size) {

    FString fstr;
    FString wrappedLine;
    int lines;
    int lastLineStart;

    do {
        fstr = ftxt.ToString ();
        int inputLineLength = fstr.Len ();
        lines = 1;
        int offset = 0;
        wrappedLine = FString ();

        int lastSpace = 0;          // Last space we've found, in this line we're now scanning
        lastLineStart = 0;          // Where in fstr, that the line we're now scanning, starts
        while (offset < inputLineLength) {
            if (fstr[offset] == ' ') {
                lastSpace = offset;
            }
            if ((fstr[offset] == '\r') || (fstr[offset] == '\n')) {
                wrappedLine.Append (fstr.Mid (lastLineStart, offset - lastLineStart + 1));
                lastLineStart = offset + 1;
                lastSpace = -1;
                lines++;
            }
            // Wrap here:
            if (offset >= (lastLineStart + wrapLength)) {
                // If there was a space in this line, wrap at the last one found on this line
                if (lastSpace != -1) {
                    wrappedLine.Append (fstr.Mid (lastLineStart, lastSpace - lastLineStart));
                    wrappedLine.Append (newLineStr);
                    offset = lastSpace + 1;
                    lastLineStart = lastSpace + 1;
                    lastSpace = -1;
                    lines++;
                }
                else {
                    // Need to break a word
                    wrappedLine.Append (fstr.Mid (lastLineStart, offset - lastLineStart - 1));
                    wrappedLine.Append ("-");           // Insert a hyphen into the word we're breaking
                    wrappedLine.Append (newLineStr);
                    lastLineStart = offset - 1;
                    lastSpace = -1;
                    lines++;
                }
            }
            offset++;
        }
        // Estimate text width and height
        size->X = wrapLength * 25.0f;
        size->Y = lines * 58.0f;

        wrapLength *= 1.1f;

    // Stop if wider than tall
    } while (size->X / size->Y < 0.85);

    // Whatever is left in line is short enough to just pass through
    wrappedLine.Append (fstr.Mid (lastLineStart, fstr.Len () - lastLineStart));

    if (size->Y < size->X)
        size->Y = size->X;
    if (size->Y < 300.0f)
        size->Y = 300.0f;

    return FText::FromString (wrappedLine);
}


//-------------------------------------------
// Update colour and text on this sticky note
//-------------------------------------------
void AFixItNote::Update () {
    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----AFixItNote::Update"));

    FVector2D size;
    /***********************Changed By Linda**************************/
    const char* newLine="\n";
    FText wrappedText = wrap (Text, 12, newLine, true, &size);
    /*************************************************************/

    // Make a new texture for the background, to colour the sticky note appropriately
    //RenderTexture = CastChecked <UTextureRenderTarget2D> (StaticConstructObject (UTextureRenderTarget2D::StaticClass ()));       // 4.7
    RenderTexture = CastChecked <UTextureRenderTarget2D> (NewObject <UTextureRenderTarget2D> (this, TEXT ("Texture")));              // 4.8
    RenderTexture->ClearColor = FLinearColor (FLinearColor::Red);
    RenderTexture->InitAutoFormat (size.X, size.Y);
    RenderTexture->UpdateResourceImmediate ();

    // And create a canvas so we can draw on it
    FTextureRenderTarget2DResource *TextureResource = (FTextureRenderTarget2DResource *) RenderTexture->Resource;
    FCanvas *Canvas = new FCanvas (TextureResource, NULL, GetWorld (), GMaxRHIFeatureLevel);

    // Pick the colour to paint the texture
    UTexture2D *texture;
    FColor StickyColor;

    switch (Priority) {
    case EFixItNotePriority::Urgent:
        StickyColor = FLinearColor (255.0f / 255.0f, 89.0f / 255.0f, 181.0f / 255.0f).ToFColor (true);
		texture = Cast <UTexture2D> (StaticLoadObject (UTexture2D::StaticClass (), NULL, TEXT ("Texture2D'/FixItNotePlugin/red.red'")));
        break; // Hot pink
    case EFixItNotePriority::Moderate:
        StickyColor = FLinearColor (252.0f / 255.0f, 130.0f / 255.0f, 94.0f / 255.0f).ToFColor (true);
        texture = Cast <UTexture2D> (StaticLoadObject (UTexture2D::StaticClass (), NULL, TEXT ("Texture2D'/FixItNotePlugin/orange.orange'")));
        break; // Darkish orange
    case EFixItNotePriority::Low:
        StickyColor = FLinearColor (190.0f / 255.0f, 215.0f / 255.0f, 233.0f / 255.0f).ToFColor (true);
		texture = Cast <UTexture2D> (StaticLoadObject (UTexture2D::StaticClass (), NULL, TEXT ("Texture2D'/FixItNotePlugin/yellow.yellow'")));
        break; // Light blue
    //case EFixItNotePriority:: :      
    //    StickyColor = FLinearColor (221.0f / 255.0f, 241.0f / 255.0f,  81.0f / 255.0f).ToFColor (true);
    //    texture = Cast <UTexture2D> (StaticLoadObject (UTexture2D::StaticClass (), NULL, TEXT ("Texture2D'/FixItNotePlugin/limegreen.limegreen'")));
    //    break; // Lime green
    //case EFixItNotePriority:: :      
    //    StickyColor = FLinearColor (  1.0f / 255.0f, 160.0f / 255.0f, 216.0f / 255.0f).ToFColor (true);
    //    texture = Cast <UTexture2D> (StaticLoadObject (UTexture2D::StaticClass (), NULL, TEXT ("Texture2D'/FixItNotePlugin/brightblue.brightblue'")));
    //    break; // Bold blue
    //case EFixItNotePriority:: :      
    //    StickyColor = FLinearColor (230.0f / 255.0f, 193.0f / 255.0f, 228.0f / 255.0f).ToFColor (true);
    //    texture = Cast <UTexture2D> (StaticLoadObject (UTexture2D::StaticClass (), NULL, TEXT ("Texture2D'/FixItNotePlugin/purple.purple'")));
    //    break; // Light pink/purple
    default:
    //case EFixItNotePriority::Comment:
        StickyColor = FLinearColor (254.0f / 255.0f, 231.0f / 255.0f, 89.0f / 255.0f).ToFColor (true);
		texture = Cast <UTexture2D> (StaticLoadObject (UTexture2D::StaticClass (), NULL, TEXT ("Texture2D'/FixItNotePlugin/lightblue.lightblue'")));
        break;  // Classic yellow
    }

    Canvas->Clear (StickyColor);        // Clear the canvas to one of the beautiful Post-It colours :)


    // This works if the asset is Imported into the Content Browser first, but won't load it from disk
//Billboard->SetSprite (Cast <UTexture2D> (StaticLoadObject (UTexture2D::StaticClass (), NULL, TEXT ("Texture2D'/FixItNotePlugin/purple.purple'"))));
//Billboard->SetSprite ((UTexture2D *) RenderTexture);
    Billboard->SetSprite (texture);
    SpriteScale = size.Y / 300.0f;

    TextComp->SetText (wrappedText);

    // As of 4.9x, the updates aren't displayed until you move the camera, if only the text has changed.  Changing the texture, does update it though!
    // Sadly none of these have any effect
    //Billboard->MarkRenderDynamicDataDirty ();
    //Billboard->MarkRenderStateDirty ();
    //Billboard->MarkPackageDirty ();
    //TextComp->MarkRenderStateDirty ();
    //TextComp->MarkRenderDynamicDataDirty ();
    //TextComp->MarkPackageDirty ();
    //MarkPackageDirty ();
    //MarkComponentsRenderStateDirty ();
    //
    // Try manually moving the camera a bit.  Still didn't update the screen (or move the camera)
    //if ((GEditor != NULL) && (GEditor->GetActiveViewport () != NULL)) {
    //    FEditorViewportClient *client = (FEditorViewportClient *) GEditor->GetActiveViewport ()->GetClient ();
    //    FRotator editorCameraRotation = client->GetViewRotation ();
    //    editorCameraRotation.Yaw += 1.0;
    //    FVector editorCameraLocation = client->GetViewLocation ();
    //    editorCameraLocation.X += 1.0;
    //    client->SetViewRotation (editorCameraRotation);
    //    client->SetViewLocation (editorCameraLocation);
    //}
}

// Called when the game starts or when spawned
void AFixItNote::BeginPlay () {
	Super::BeginPlay ();
}

// Called every frame
void AFixItNote::Tick (float DeltaTime) {
//    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----AFixItNote::Tick"));

    RotateTextToCamera ();

	Super::Tick (DeltaTime);
}

void AFixItNote::RotateTextToCamera () {
    // Position sticky note text relative to current view location/direction
    if (GEditor == NULL)
        return;
    if (GEditor->GetActiveViewport () == NULL)
        return;

	// A list of all FLevelEditEditorViewportClients
	TArray<FLevelEditorViewportClient *> levelClients = GEditor->LevelViewportClients;

	FLevelEditorViewportClient *client = (FLevelEditorViewportClient *) GEditor->GetActiveViewport()->GetClient();
	// Only proceed if the active viewport client is a FLevelEditorViewportClient
	if (levelClients.Contains(client)) {
		FVector editorCameraDirection = client->GetViewRotation().Vector();
		FVector editorCameraLocation = client->GetViewLocation();

		FVector location = GetActorLocation();          // Where the sticky note text I'm rotating, is

		FRotator rotation = client->GetViewRotation();     // What the camera rotation is

		SetActorRotation(rotation);                     // Set the actor (and hence it's text subcomponent) rotation, to point to the camera

		//UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----FFixItEdMode::RotateTextToCamera   location=%s rotation=%s"), *location.ToString (), *rotation.ToString ());
	}
}

// Called when properties are changed
void AFixItNote::PostEditChangeProperty (struct FPropertyChangedEvent &e) {
    UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----AFixItNote::PostEditChangeProperty"));

    Super::PostEditChangeProperty (e);

    FName PropertyName = (e.Property != NULL) ? e.Property->GetFName () : NAME_None;
    if (!PropertyName.ToString ().Compare (TEXT ("Priority"))) {
        UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----AFixItNote::PostEditChangeProperty: Priority property changed to %d"), (uint8) Priority);
    }
    if (!PropertyName.ToString ().Compare (TEXT ("Recipient"))) {
        UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----AFixItNote::PostEditChangeProperty: Recipient property changed to '%s'"), *Recipient.ToString ());
    }
    if (!PropertyName.ToString ().Compare (TEXT ("Text"))) {
        UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----AFixItNote::PostEditChangeProperty: Text property changed to '%s'"), *Text.ToString ());
        Text = FText::FromString (Text.ToString ().Replace (_T ("<br>"), _T ("\r\n")));    // The wrap method looks for inserted \r\n, but not <br>, so convert here.
    }
    if (!PropertyName.ToString ().Compare (TEXT ("SearchTags"))) {
        UE_LOG (SkunkLog_FixItNote, Log, TEXT ("-----AFixItNote::PostEditChangeProperty: SearchTags property changed to '%s'"), *SearchTags.ToString ());
    }

    Update ();

    RotateTextToCamera ();
}

#undef LOCTEXT_NAMESPACE