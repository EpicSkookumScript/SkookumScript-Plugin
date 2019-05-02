// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//=======================================================================================

#include "SkookumScriptEditorCommands.h"

PRAGMA_DISABLE_OPTIMIZATION
void FSkookumScriptEditorCommands::RegisterCommands()
  {
  #define LOCTEXT_NAMESPACE "SkookumScript"

    UI_COMMAND(m_skookum_button, "SkookumIDE", "Show in SkookumIDE", EUserInterfaceActionType::Button, FInputChord(EModifierKey::Alt, EKeys::Tilde));

  #undef LOCTEXT_NAMESPACE
  }
PRAGMA_ENABLE_OPTIMIZATION
