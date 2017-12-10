// Copyright (C) 2015 Skunkwerks Kinetic Industries Inc. All Rights Reserved.

#pragma once

/** Sticky Note Priorities */
UENUM()
enum class EFixItNotePriority : uint8 {
    Urgent      UMETA(DisplayName = "High"),
    Moderate    UMETA(DisplayName = "Medium"),
    Low         UMETA(DisplayName = "Low"),
    //Comment     UMETA(DisplayName = "Comment"),
};

/** Sticky Note Filter Priorities */
UENUM()
enum class EFixItNotePriorityFilter : uint8 {
	Urgent      UMETA(DisplayName = "High"),
	Moderate    UMETA(DisplayName = "Medium"),
	Low         UMETA(DisplayName = "Low"),
	All         UMETA(DisplayName = "-- All --"),
};
