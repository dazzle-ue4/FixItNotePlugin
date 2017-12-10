// Copyright (C) 2015 Skunkwerks Kinetic Industries Inc. All Rights Reserved.

#pragma once

#include "FixItPluginPCH.h"
#include "Toolkits/BaseToolkit.h"

/**
* Public interface to StickyNote Edit mode.
*/
class FFixItEdModeToolkit : public FModeToolkit {
public:
    virtual void RegisterTabSpawners (const TSharedRef <class FTabManager> &TabManager) override;
    virtual void UnregisterTabSpawners (const TSharedRef <class FTabManager> &TabManager) override;

    /** Initializes the StickyNote mode toolkit */
    virtual void Init(const TSharedPtr <class IToolkitHost> &InitToolkitHost);

    /** IToolkit interface */
    virtual FName GetToolkitFName () const override;
    virtual FText GetBaseToolkitName () const override;
    virtual class FEdMode* GetEditorMode () const override;
    virtual TSharedPtr <class SWidget> GetInlineContent () const override;

    void PostUndo ();

private:
    TSharedPtr <class SFixItWidget> FixItEdWidget;
};