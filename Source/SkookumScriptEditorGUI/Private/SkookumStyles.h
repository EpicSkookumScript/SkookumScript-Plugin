// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//=======================================================================================

#pragma once

#include "UObject/NameTypes.h"
#include "Templates/SharedPointer.h"

//---------------------------------------------------------------------------------------

class FSkookumStyles
  {
  public:
    // Initializes the value of MenuStyleInstance and registers it with the Slate Style Registry.
    static void Initialize();

    // Unregisters the Slate Style Set and then resets the MenuStyleInstance pointer.
    static void Shutdown();

    // Retrieves a reference to the Slate Style pointed to by MenuStyleInstance.
    static const class ISlateStyle& Get();

    // Retrieves the name of the Style Set.
    static FName GetStyleSetName();

  private:

    // Singleton instance used for our Style Set.
    static TSharedPtr<class FSlateStyleSet> ms_singleton_p;

  };

