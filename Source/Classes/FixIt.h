// Copyright (C) 2015 Skunkwerks Kinetic Industries Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "../Private/FixItCommon.h"       // For EFixItNotePriority
#include "FixIt.generated.h"


#define LOCTEXT_NAMESPACE "AFixItNote"

// Store StickyNote information when actors have been temporarily removed when saving a level
class FixItNoteInfo {
public:
    EFixItNotePriority Priority;
    FText Recipient;
    FText Text;
    FText SearchTags;
    float X;
    float Y;
    float Z;
};

UCLASS(config = Game, BlueprintType, meta = (ShortTooltip = "A development note."))
class AFixItNote : public AActor {
	GENERATED_UCLASS_BODY ()

public:
    //~AFixItNote ();
    /********************Modified By Linda*******************/
    FText wrap (FText ftxt, int wrapLength, const char *newLineStr, bool wrapLongWords, FVector2D* size);
    /***************************************/

	// Called when the game starts or when spawned
	virtual void BeginPlay () override;

	// Called every frame
	virtual void Tick (float DeltaSeconds) override;
    virtual bool ShouldTickIfViewportsOnly () const override { return true; };  // This enables Tick while in the editor

    // Creation Properties
    struct FConstructorStatics {
        EFixItNotePriority Priority;
        FText Recipient;
        FText Text;
        FText SearchTags;
		FConstructorStatics ()
            : Priority (EFixItNotePriority::Low)
            , Recipient (LOCTEXT ("Recipient", ""))
            , Text (LOCTEXT ("Text", ""))
            , SearchTags (LOCTEXT ("SearchTags", ""))
        { }
    };
    static FConstructorStatics ConstructorStatics;


    // Priority of this sticky note
    UPROPERTY (EditAnywhere, BlueprintReadWrite, Category = FixIt)
    EFixItNotePriority Priority;

    // The recipient of this note
    UPROPERTY (EditAnywhere, BlueprintReadWrite, Category = FixIt)
    FText Recipient;

    // Text to display on the sticky note.  You can insert Enter to split lines
    UPROPERTY (EditAnywhere, BlueprintReadWrite, Category = FixIt, meta = (MultiLine = true))
    FText Text;

    // Tags for searching notes
    UPROPERTY (EditAnywhere, BlueprintReadWrite, Category = FixIt, meta = (MultiLine = true))
    FText SearchTags;

//   UPROPERTY (EditAnywhere, BlueprintReadWrite, Category = FixIt)
//   class UFont *Font;                  // Message test                            // NOT USED ATM
//   UPROPERTY (EditAnywhere, BlueprintReadWrite, Category = FixIt)
//   FColor TextRenderColor;             // Colour of the text                      // NOT USED ATM

    virtual void PostEditChangeProperty (struct FPropertyChangedEvent& e) override;

	USphereComponent *RootSphere;

	//UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Rendering)
	UBillboardComponent *Billboard;

    //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rendering)
    UTextRenderComponent *TextComp;

    void RotateTextToCamera ();

    void Update ();

    UTextureRenderTarget2D *RenderTexture;
};

#undef LOCTEXT_NAMESPACE