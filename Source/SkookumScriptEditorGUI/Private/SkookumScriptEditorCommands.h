// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//=======================================================================================

#pragma once

#include "Framework/Commands/Commands.h"
#include "EditorStyleSet.h"

class FSkookumScriptEditorCommands : public TCommands<FSkookumScriptEditorCommands>
  {
  public:

    FSkookumScriptEditorCommands()
      : TCommands<FSkookumScriptEditorCommands>(TEXT("SkookumScript"), FText::FromString(TEXT("SkookumScript")), NAME_None, FEditorStyle::GetStyleSetName())
      {
      }

    virtual void RegisterCommands() override;

    TSharedPtr<FUICommandInfo> m_skookum_button;

  };