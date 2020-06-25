// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//=======================================================================================

#include "SkookumScriptGeneratorBase.h"

#include "UObject/EnumProperty.h"
#include "Misc/FeedbackContext.h"

#if WITH_EDITOR
  #include "Engine/Blueprint.h"
  #include "Engine/UserDefinedStruct.h"
  #include "Engine/UserDefinedEnum.h"
#endif

//=======================================================================================
// FSkookumScriptGeneratorHelper Implementation
//=======================================================================================

//---------------------------------------------------------------------------------------

const FString FSkookumScriptGeneratorHelper::ms_sk_type_id_names[FSkookumScriptGeneratorHelper::SkTypeID__count] =
  {
  TEXT("None"),
  TEXT("Integer"),
  TEXT("Real"),
  TEXT("Boolean"),
  TEXT("String"),
  TEXT("Vector2"),
  TEXT("Vector3"),
  TEXT("Vector4"),
  TEXT("Rotation"),
  TEXT("RotationAngles"),
  TEXT("Transform"),
  TEXT("Color"),
  TEXT("Name"),
  TEXT("Enum"),
  TEXT("UStruct"),
  TEXT("EntityClass"),  // UClass
  TEXT("Entity"),       // UObject
  TEXT("List"),
  };

const FString FSkookumScriptGeneratorHelper::ms_reserved_keywords[] =
  {
  TEXT("branch"),
  TEXT("case"),
  TEXT("change"),
  TEXT("eh"),
  TEXT("else"),
  TEXT("exit"),
  TEXT("false"),
  TEXT("if"),
  TEXT("loop"),
  TEXT("nil"),
  TEXT("race"),
  TEXT("random"),
  TEXT("rush"),
  TEXT("skip"),
  TEXT("sync"),
  TEXT("this"),
  TEXT("this_class"),
  TEXT("this_code"),
  TEXT("this_mind"),
  TEXT("true"),
  TEXT("unless"),
  TEXT("when"),

  // Boolean word operators
  TEXT("and"),
  TEXT("nand"),
  TEXT("nor"),
  TEXT("not"),
  TEXT("nxor"),
  TEXT("or"),
  TEXT("xor"),
  };

#if WITH_EDITOR || HACK_HEADER_GENERATOR
  const FName FSkookumScriptGeneratorHelper::ms_meta_data_key_blueprint_type("BlueprintType");
#endif

const FName FSkookumScriptGeneratorHelper::ms_skookum_script_instance_property_name("SkookumScriptInstanceProperty");

//---------------------------------------------------------------------------------------

FSkookumScriptGeneratorHelper::eSkTypeID FSkookumScriptGeneratorHelper::get_skookum_property_type(FProperty * property_p, bool allow_all)
  {
  // Check for simple types first
  if (property_p->IsA<FNumericProperty>())
    {
    FNumericProperty * numeric_property_p = static_cast<FNumericProperty *>(property_p);
    if (numeric_property_p->IsInteger() && !numeric_property_p->IsEnum())
      {
      return SkTypeID_Integer;
      }
    }
  if (property_p->IsA<FFloatProperty>())       return SkTypeID_Real;
  if (property_p->IsA<FStrProperty>())         return SkTypeID_String;
  if (property_p->IsA<FNameProperty>())        return SkTypeID_Name;
  if (property_p->IsA<FBoolProperty>())        return SkTypeID_Boolean;

  // Any known struct?
  if (property_p->IsA<FStructProperty>())
    {
    FStructProperty * struct_prop_p = CastFieldChecked<FStructProperty>(property_p);
    eSkTypeID type_id = get_skookum_struct_type(struct_prop_p->Struct);
    return (allow_all || type_id != SkTypeID_UStruct || is_struct_type_supported(struct_prop_p->Struct)) ? type_id : SkTypeID_none;
    }

  if (get_enum(property_p))                          return SkTypeID_Enum;
  if (property_p->IsA<FClassProperty>())             return SkTypeID_UClass;
  if (property_p->IsA<FDelegateProperty>())          return SkTypeID_Delegate;
  if (property_p->IsA<FMulticastDelegateProperty>()) return SkTypeID_MulticastDelegate;

  if (property_p->IsA<FObjectPropertyBase>())
    {
    return property_p->IsA<FWeakObjectProperty>() ? SkTypeID_UObjectWeakPtr : SkTypeID_UObject;
    }

  if (FArrayProperty * array_property_p = CastField<FArrayProperty>(property_p))
    {
    // Reject arrays of unknown types and arrays of arrays
    return (allow_all || (is_property_type_supported(array_property_p->Inner) && (get_skookum_property_type(array_property_p->Inner, true) != SkTypeID_List))) ? SkTypeID_List : SkTypeID_none;
    }

  // Didn't find a known type
  return SkTypeID_none;
  }

//---------------------------------------------------------------------------------------

FSkookumScriptGeneratorHelper::eSkTypeID FSkookumScriptGeneratorHelper::get_skookum_struct_type(UStruct * struct_p)
  {
  static FName name_Vector2D("Vector2D");
  static FName name_Vector("Vector");
  static FName name_Vector_NetQuantize("Vector_NetQuantize");
  static FName name_Vector_NetQuantizeNormal("Vector_NetQuantizeNormal");  
  static FName name_Vector4("Vector4");
  static FName name_Quat("Quat");
  static FName name_Rotator("Rotator");
  static FName name_Transform("Transform");
  static FName name_LinearColor("LinearColor");
  static FName name_Color("Color");

  const FName struct_name = struct_p->GetFName();

  if (struct_name == name_Vector2D)                 return SkTypeID_Vector2;
  if (struct_name == name_Vector)                   return SkTypeID_Vector3;
  if (struct_name == name_Vector_NetQuantize)       return SkTypeID_Vector3;
  if (struct_name == name_Vector_NetQuantizeNormal) return SkTypeID_Vector3;
  if (struct_name == name_Vector4)                  return SkTypeID_Vector4;
  if (struct_name == name_Quat)                     return SkTypeID_Rotation;
  if (struct_name == name_Rotator)                  return SkTypeID_RotationAngles;
  if (struct_name == name_Transform)                return SkTypeID_Transform;
  if (struct_name == name_Color)                    return SkTypeID_Color;
  if (struct_name == name_LinearColor)              return SkTypeID_Color;

  return struct_p->IsA<UClass>() ? SkTypeID_UClass : SkTypeID_UStruct;
  }

//---------------------------------------------------------------------------------------

bool FSkookumScriptGeneratorHelper::is_property_type_supported(FProperty * property_p)
  {
  if (property_p->HasAnyPropertyFlags(CPF_EditorOnly)
   || property_p->IsA<FLazyObjectProperty>()
   || property_p->IsA<FSoftObjectProperty>()
   || property_p->IsA<FSoftClassProperty>())
    {
    return false;
    }

  return (get_skookum_property_type(property_p, false) != SkTypeID_none);
  }

//---------------------------------------------------------------------------------------

bool FSkookumScriptGeneratorHelper::is_struct_type_supported(UStruct * struct_p)
  {
  UScriptStruct * script_struct = Cast<UScriptStruct>(struct_p);  
  return (script_struct && (script_struct->HasDefaults() || (script_struct->StructFlags & STRUCT_RequiredAPI)
#if WITH_EDITOR || HACK_HEADER_GENERATOR
    || script_struct->HasMetaData(ms_meta_data_key_blueprint_type)
#endif
    ));
#if 0
  return script_struct
    && (script_struct->StructFlags & (STRUCT_CopyNative|STRUCT_IsPlainOldData|STRUCT_RequiredAPI))
    && !(script_struct->StructFlags & STRUCT_NoExport);
#endif
  }

//---------------------------------------------------------------------------------------

bool FSkookumScriptGeneratorHelper::is_pod(UStruct * struct_p)
  {
  UScriptStruct * script_struct = Cast<UScriptStruct>(struct_p);
  return (script_struct && (script_struct->StructFlags & STRUCT_IsPlainOldData));
  }

//---------------------------------------------------------------------------------------

bool FSkookumScriptGeneratorHelper::does_class_have_static_class(UClass * class_p)
  {
  return class_p->HasAnyClassFlags(CLASS_RequiredAPI | CLASS_MinimalAPI);
  }

//---------------------------------------------------------------------------------------

UEnum * FSkookumScriptGeneratorHelper::get_enum(FField * field_p)
  {
  const FEnumProperty * enum_property_p = CastField<FEnumProperty>(field_p);
  if (enum_property_p)
    {
    return enum_property_p->GetEnum();
    }
  const FByteProperty * byte_property_p = CastField<FByteProperty>(field_p);
  return byte_property_p ? byte_property_p->Enum : nullptr;
  }

//---------------------------------------------------------------------------------------

FString FSkookumScriptGeneratorHelper::skookify_param_name(const FString & name, bool append_question_mark)
  {
  if (name.IsEmpty()) return name;

  // Change title case to lower case with underscores
  FString skookum_name;
  skookum_name.Reserve(name.Len() + 16);
  bool is_boolean = name.Len() > 2 && name[0] == 'b' && isupper(name[1]);
  bool was_upper = true;
  bool was_underscore = false;

  for (int32 i = int32(is_boolean); i < name.Len(); ++i)
    {
    TCHAR c = name[i];

    // Skip special characters
    if (c == TCHAR('?'))
      {
      continue;
      }

    // Is it [A-Za-z0-9]?
    if ((c >= TCHAR('0') && c <= TCHAR('9'))
     || (c >= TCHAR('A') && c <= TCHAR('Z'))
     || (c >= TCHAR('a') && c <= TCHAR('z')))
      {
      // Yes, append it
      bool is_upper = FChar::IsUpper(c) || FChar::IsDigit(c);
      if (is_upper && !was_upper && !was_underscore)
        {
        skookum_name.AppendChar('_');
        }
      skookum_name.AppendChar(FChar::ToLower(c));
      was_upper = is_upper;
      was_underscore = false;
      }
    else
      {
      // No, insert underscore, but only one
      if (!was_underscore)
        {
        skookum_name.AppendChar('_');
        was_underscore = true;
        }
      }
    }

  // Check for initial underscore and digits
  if (FChar::IsDigit(skookum_name[0]) 
   || (skookum_name[0] == '_' && skookum_name.Len() >= 2 && FChar::IsDigit(skookum_name[1])))
    {
    skookum_name = TEXT("p") + skookum_name; // Prepend an alpha character to make it a valid identifier
    }
  else if (skookum_name[0] == '_')
    {
    skookum_name = skookum_name.RightChop(1); // Chop off initial underscore if present
    }

  // Check for reserved keywords and append underscore if found
  if (is_skookum_reserved_word(skookum_name))
    {
    skookum_name.AppendChar('_');
    }

  // For booleans
  if (append_question_mark)
    {
    skookum_name.AppendChar(TCHAR('?'));
    }

  return skookum_name;
  }

//---------------------------------------------------------------------------------------

FString FSkookumScriptGeneratorHelper::skookify_method_name(const FString & name, FProperty * return_property_p)
  {
  FString method_name = skookify_param_name(name, false);
  bool is_boolean = false;

  // Remove K2 (Kismet 2) prefix if present
  if (method_name.Len() > 3 && !method_name.Mid(3, 1).IsNumeric())
    {
    if (method_name.RemoveFromStart(TEXT("k2_"), ESearchCase::CaseSensitive))
      {
      // Check if removing the k2_ turned it into a Sk reserved word
      if (is_skookum_reserved_word(method_name))
        {
        method_name.AppendChar('_');
        }
      }
    }

  if (method_name.Len() > 4 && !method_name.Mid(4, 1).IsNumeric())
    {
    // If name starts with "get_", remove it
    if (method_name.RemoveFromStart(TEXT("get_"), ESearchCase::CaseSensitive))
      {
      // Check if removing the get_ turned it into a Sk reserved word
      if (is_skookum_reserved_word(method_name))
        {
        method_name.AppendChar('_');
        }

      // Allow question mark
      is_boolean = true;
      }
    // If name starts with "set_", remove it and append "_set" instead
    else if (method_name.RemoveFromStart(TEXT("set_"), ESearchCase::CaseSensitive))
      {
      method_name.Append(TEXT("_set"));
      }
    }

  // If name starts with "is_", "has_" or "can_" also append question mark
  if ((name.Len() > 2 && name[0] == 'b' && isupper(name[1]))
   || method_name.Find(TEXT("is_"), ESearchCase::CaseSensitive) == 0
   || method_name.Find(TEXT("has_"), ESearchCase::CaseSensitive) == 0
   || method_name.Find(TEXT("can_"), ESearchCase::CaseSensitive) == 0)
    {
    is_boolean = true;
    }

  // Append question mark if determined to be boolean
  if (is_boolean && return_property_p && return_property_p->IsA<FBoolProperty>())
    {
    method_name += TEXT("?");
    }

  return method_name;
  }



//---------------------------------------------------------------------------------------
// Perform lexical skookification, without name replacement lookup
FString FSkookumScriptGeneratorHelper::skookify_data_name_basic(const FString & name, bool append_question_mark, eDataScope scope)
  {
  FString skookum_name;
  skookum_name.Reserve(name.Len() + 16);

  // Change title case to lower case with underscores
  skookum_name.AppendChars(TEXT("@@"), scope == DataScope_instance ? 1 : 2);
  bool is_boolean = name.Len() > 2 && name[0] == 'b' && isupper(name[1]);
  bool was_upper = true;
  bool was_underscore = false;

  for (int32 i = int32(is_boolean); i < name.Len(); ++i)
    {
    TCHAR c = name[i];

    // Skip special characters
    if (c == TCHAR('?'))
      {
      continue;
      }

    // Is it [A-Za-z0-9]?
    if ((c >= TCHAR('0') && c <= TCHAR('9'))
      || (c >= TCHAR('A') && c <= TCHAR('Z'))
      || (c >= TCHAR('a') && c <= TCHAR('z')))
      {
      // Yes, append it
      bool is_upper = FChar::IsUpper(c) || FChar::IsDigit(c);
      if (is_upper && !was_upper && !was_underscore)
        {
        skookum_name.AppendChar('_');
        }
      skookum_name.AppendChar(FChar::ToLower(c));
      was_upper = is_upper;
      was_underscore = false;
      }
    else
      {
      // No, insert underscore, but only one
      if (!was_underscore)
        {
        skookum_name.AppendChar('_');
        was_underscore = true;
        }
      }
    }

  // Check for reserved keywords and append underscore if found
  if (scope == DataScope_class && (skookum_name == TEXT("@@world") || skookum_name == TEXT("@@random")))
    {
    skookum_name.AppendChar('_');
    }

  // Check if there's an MD5 checksum appended to the name - if so, chop it off
  int32 skookum_name_len = skookum_name.Len();
  if (skookum_name_len > 33)
    {
    const TCHAR * skookum_name_p = &skookum_name[skookum_name_len - 33];
    if (skookum_name_p[0] == TCHAR('_'))
      {
      for (int32 i = 1; i <= 32; ++i)
        {
        uint32_t c = skookum_name_p[i];
        if ((c - '0') > 9u && (c - 'a') > 5u) goto no_md5;
        }
      // We chop off most digits of the MD5 and leave only the first four, 
      // assuming that that's distinctive enough for just a few of them at a time
      skookum_name = skookum_name.Left(skookum_name_len - 28);
    no_md5:;
      }
    }

  if (append_question_mark)
    {
    skookum_name.AppendChar(TCHAR('?'));
    }

  return skookum_name;
  }

//---------------------------------------------------------------------------------------
// Perform lexical skookification, without name replacement lookup
FString FSkookumScriptGeneratorHelper::skookify_class_name_basic(const FString & name)
  {
  FString skookum_name;
  skookum_name.Reserve(name.Len() + 16);

  bool was_underscore = true;
  for (int32 i = 0; i < name.Len(); ++i)
    {
    TCHAR c = name[i];

    // Ensure first character is uppercase
    if (skookum_name.IsEmpty())
      {
      if (islower(c))
        {
        c = toupper(c);
        }
      else if (!isupper(c))
        {
        // If name starts with neither upper nor lowercase letter, prepend "Sk"
        skookum_name.Append(TEXT("Sk"));
        }
      }

    // Is it [A-Za-z0-9]?
    if ((c >= TCHAR('0') && c <= TCHAR('9'))
      || (c >= TCHAR('A') && c <= TCHAR('Z'))
      || (c >= TCHAR('a') && c <= TCHAR('z')))
      {
      // Yes, append it
      skookum_name.AppendChar(c);
      was_underscore = false;
      }
    else
      {
      // No, insert underscore, but only one
      if (!was_underscore)
        {
        skookum_name.AppendChar('_');
        was_underscore = true;
        }
      }
    }

  return skookum_name;
  }

//---------------------------------------------------------------------------------------

bool FSkookumScriptGeneratorHelper::compare_var_name_skookified(const TCHAR * ue_var_name_p, const ANSICHAR * sk_var_name_p)
  {
  uint32_t ue_len = FCString::Strlen(ue_var_name_p);
  uint32_t sk_len = FCStringAnsi::Strlen(sk_var_name_p);

  // Check if there's an MD5 checksum appended to the name - if so, leave only first four digits
  if (ue_len > 33 && ue_var_name_p[ue_len - 33] == '_')
    {
    for (int32 i = 0; i < 32; ++i)
      {
      uint32_t c = ue_var_name_p[ue_len - 32 + i];
      if ((c - '0') > 9u && (c - 'A') > 5u) goto no_md5;
      }
      // We chop off most digits of the MD5 and leave only the first four, 
      // assuming that that's distinctive enough for just a few of them at a time
      ue_len -= 28;
    no_md5:;
    }

  uint32_t ue_i = uint32_t(ue_len >= 2 && ue_var_name_p[0] == 'b' && FChar::IsUpper(ue_var_name_p[1])); // Skip Boolean `b` prefix if present
  uint32_t sk_i = 0;
  do
    {
    // Skip non-alphanumeric characters
    while (ue_i < ue_len && !FChar::IsAlnum(ue_var_name_p[ue_i])) ++ue_i;
    while (sk_i < sk_len && !FChar::IsAlnum(sk_var_name_p[sk_i])) ++sk_i;
    } while (ue_i < ue_len && sk_i < sk_len && FChar::ToLower(ue_var_name_p[ue_i++]) == FChar::ToLower(sk_var_name_p[sk_i++]));
  // Did we find a match?
  return (ue_i == ue_len && sk_i == sk_len);
  }

//---------------------------------------------------------------------------------------

bool FSkookumScriptGeneratorHelper::is_skookum_reserved_word(const FString & name)
  {
  for (uint32 i = 0; i < sizeof(ms_reserved_keywords) / sizeof(ms_reserved_keywords[0]); ++i)
    {
    if (name == ms_reserved_keywords[i]) return true;
    }

  return false;
  }

#if WITH_EDITOR || HACK_HEADER_GENERATOR

//=======================================================================================
// FSkookumScriptGeneratorBase Implementation
//=======================================================================================

const FName         FSkookumScriptGeneratorBase::ms_meta_data_key_function_category ("Category");
const FName         FSkookumScriptGeneratorBase::ms_meta_data_key_display_name      ("DisplayName");

const FString       FSkookumScriptGeneratorBase::ms_asset_name_key          (TEXT("// UE4 Asset Name: "));
const FString       FSkookumScriptGeneratorBase::ms_package_name_key        (TEXT("// UE4 Package Name: \""));
//const FString       FSkookumScriptGeneratorBase::ms_package_path_key        (TEXT("// UE4 Package Path: \""));
TCHAR const * const FSkookumScriptGeneratorBase::ms_editable_ini_settings_p (TEXT("Editable=false\nCanMakeEditable=true\n"));
TCHAR const * const FSkookumScriptGeneratorBase::ms_overlay_name_bp_p       (TEXT("Project-Generated-BP"));
TCHAR const * const FSkookumScriptGeneratorBase::ms_overlay_name_bp_old_p   (TEXT("Project-Generated"));
TCHAR const * const FSkookumScriptGeneratorBase::ms_overlay_name_cpp_p      (TEXT("Project-Generated-C++"));

const FName FSkookumScriptGeneratorBase::ms_name_object ("Object");
const FName FSkookumScriptGeneratorBase::ms_name_class  ("Class");
const FName FSkookumScriptGeneratorBase::ms_name_entity ("Entity");
const FName FSkookumScriptGeneratorBase::ms_name_vector ("Vector");
const FName FSkookumScriptGeneratorBase::ms_name_enum   ("Enum");

const FName FSkookumScriptGeneratorBase::ms_name_data_table_function_library           ("DataTableFunctionLibrary");
const FName FSkookumScriptGeneratorBase::ms_name_gameplay_statics                      ("GameplayStatics");
const FName FSkookumScriptGeneratorBase::ms_name_head_mounted_display_function_library ("HeadMountedDisplayFunctionLibrary");
const FName FSkookumScriptGeneratorBase::ms_name_kismet_array_library                  ("KismetArrayLibrary");
const FName FSkookumScriptGeneratorBase::ms_name_kismet_guid_library                   ("KismetGuidLibrary");
const FName FSkookumScriptGeneratorBase::ms_name_kismet_input_library                  ("KismetInputLibrary");
const FName FSkookumScriptGeneratorBase::ms_name_kismet_material_library               ("KismetMaterialLibrary");
const FName FSkookumScriptGeneratorBase::ms_name_kismet_math_library                   ("KismetMathLibrary");
const FName FSkookumScriptGeneratorBase::ms_name_kismet_node_helper_library            ("KismetNodeHelperLibrary");
const FName FSkookumScriptGeneratorBase::ms_name_kismet_string_library                 ("KismetStringLibrary");
const FName FSkookumScriptGeneratorBase::ms_name_kismet_system_library                 ("KismetSystemLibrary");
const FName FSkookumScriptGeneratorBase::ms_name_kismet_text_library                   ("KismetTextLibrary");
const FName FSkookumScriptGeneratorBase::ms_name_visual_logger_kismet_library          ("VisualLoggerKismetLibrary");

const FFileHelper::EEncodingOptions FSkookumScriptGeneratorBase::ms_script_file_encoding = FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM;

//---------------------------------------------------------------------------------------

FString FSkookumScriptGeneratorBase::get_or_create_project_file(const FString & ue_project_directory_path, const TCHAR * project_name_p, eSkProjectMode * project_mode_p, bool * created_p)
  {
  eSkProjectMode project_mode = SkProjectMode_editable;
  bool created = false;

  // 1) Check permanent location
  FString project_file_path = ue_project_directory_path / TEXT("Scripts/Skookum-project.ini");
  if (!FPaths::FileExists(project_file_path))
    {
    // No project file exists means we are in read only/REPL only mode
    project_mode = SkProjectMode_read_only;

    // 2) Check/create temp location
    // Check temporary location (in `Intermediate` folder)
    FString temp_root_path(ue_project_directory_path / TEXT("Intermediate/SkookumScript"));
    FString temp_scripts_path(temp_root_path / TEXT("Scripts"));
    project_file_path = temp_scripts_path / TEXT("Skookum-project.ini");
    if (!FPaths::FileExists(project_file_path))
      {
      // If in neither folder, create new project in temporary location
      // $Revisit MBreyer - read ini file from default_project_path and patch it up to carry over customizations
      FString proj_ini = FString::Printf(TEXT("[Project]\nProjectName=%s\nStrictParse=true\nUseBuiltinActor=false\nCustomActorClass=Actor\nStartupMind=Master\n%s"), project_name_p, ms_editable_ini_settings_p);
      proj_ini += TEXT("[Output]\nCompileManifest=false\nCompileTo=../Content/SkookumScript/Classes.sk-bin\n");
      proj_ini += TEXT("[Script Overlays]\nOverlay1=*Core|Core\nOverlay2=*Helpers|Helpers\nOverlay3=-*Core-Sandbox|Core-Sandbox\nOverlay4=*VectorMath|VectorMath\nOverlay5=*Engine-Generated|Engine-Generated|A\nOverlay6=*Engine|Engine\nOverlay7=*");
      proj_ini += ms_overlay_name_bp_p;
      proj_ini += TEXT("|");
      proj_ini += ms_overlay_name_bp_p;
      proj_ini += TEXT("|C\nOverlay8=*");
      proj_ini += ms_overlay_name_cpp_p;
      proj_ini += TEXT("|");
      proj_ini += ms_overlay_name_cpp_p;
      proj_ini += TEXT("|A\n");
      if (save_text_file(project_file_path, proj_ini))
        {
        IFileManager::Get().MakeDirectory(*(temp_root_path / TEXT("Content/SkookumScript")), true);
        IFileManager::Get().MakeDirectory(*(temp_scripts_path / ms_overlay_name_bp_p), true);
        FString overlay_sk = TEXT("$$ .\n");
        FString overlay_archive_file_path = temp_scripts_path / ms_overlay_name_cpp_p / TEXT("!Overlay.sk");
        if (save_text_file(overlay_archive_file_path, overlay_sk))
          {
          // Overlay archive file not under source control
          created = true;
          }
        }
      else
        {
        // Silent failure since we don't want to disturb people's workflow
        project_file_path.Empty();
        }
      }

    // Since project does not exist in the permanent location, make sure the binaries don't exist in the permanent location either
    // Otherwise we'd get inconsistencies/malfunction when binaries are loaded
    FString binary_path_stem(ue_project_directory_path / TEXT("Content/SkookumScript/Classes"));
    IFileManager::Get().Delete(*(binary_path_stem + TEXT(".sk-bin")), false, true, true);
    IFileManager::Get().Delete(*(binary_path_stem + TEXT(".sk-sym")), false, true, true);
    }

  if (project_mode_p) *project_mode_p = project_mode;
  if (created_p) *created_p = created;
  return project_file_path;
  }

//---------------------------------------------------------------------------------------

bool FSkookumScriptGeneratorBase::compute_scripts_path_depth(FString project_ini_file_path, const FString & overlay_name)
  {
  // Try to figure the path depth from ini file
  m_overlay_path_depth = 1; // Set to sensible default in case we don't find it in the ini file
  FString ini_file_text;
  if (load_text_file(project_ini_file_path, ini_file_text))
    {
    // Find the substring overlay_name|*|
    FString search_text = overlay_name + TEXT("|");
    int32 pos = 0;
    int32 lf_pos = 0;
    // Skip commented out lines
    do
      {
      pos = ini_file_text.Find(search_text, ESearchCase::CaseSensitive, ESearchDir::FromStart, pos + 1);
      if (pos >= 0)
        {
        // Make sure we are not on a commented out line
        lf_pos = ini_file_text.Find(TEXT("\n"), ESearchCase::CaseSensitive, ESearchDir::FromEnd, pos);
        }
      } while (pos >= 0 && lf_pos >= 0 && ini_file_text[lf_pos + 1] == ';');

    if (pos >= 0)
      {
      pos += search_text.Len();
      while (ini_file_text[pos] != '|')
        {
        if (ini_file_text[pos] == '\n') return false;
        if (++pos >= ini_file_text.Len()) return false;
        }

      // Look what's behind the last bar
      const TCHAR * depth_text_p = &ini_file_text[pos + 1];

      if (*depth_text_p == 'A' || *depth_text_p == 'a')
        {
        m_overlay_path_depth = PathDepth_archived;
        return true;
        }

      if (*depth_text_p == 'C' || *depth_text_p == 'c')
        {
        m_overlay_path_depth = PathDepth_archived_per_class;
        return true;
        }

      int32 path_depth = FCString::Atoi(depth_text_p);
      if (path_depth > 0 || (path_depth == 0 && *depth_text_p == '0'))
        {
        m_overlay_path_depth = path_depth;
        return true;
        }
      }
    }

  return false;
  }

//---------------------------------------------------------------------------------------

bool FSkookumScriptGeneratorBase::load_text_file(const FString & file_path, FString & out_contents) const
  {
  // Load the file body
  if (!FFileHelper::LoadFileToString(out_contents, *file_path))
    {
    return false;
    }

  // Remove any CRs from the loaded file
  out_contents.ReplaceInline(TEXT("\r"), TEXT(""), ESearchCase::CaseSensitive);

  return true;
  }

//---------------------------------------------------------------------------------------

bool FSkookumScriptGeneratorBase::save_text_file(const FString & file_path, const FString & contents) const
  {
  // On Windows, insert CRs before LFs
  #if PLATFORM_WINDOWS
    FString platform_contents = contents.Replace(TEXT("\n"), TEXT("\r\n"));
  #else
    const FString & platform_contents = contents;
  #endif

  if (!FFileHelper::SaveStringToFile(platform_contents, *file_path, ms_script_file_encoding, &IFileManager::Get(), FILEWRITE_EvenIfReadOnly))
    {
    report_error(FString::Printf(TEXT("Could not save file: %s"), *file_path));
    return false;
    }

  return true;
  }

//---------------------------------------------------------------------------------------

bool FSkookumScriptGeneratorBase::save_text_file_if_changed(const FString & file_path, const FString & new_file_contents)
  {
  FString original_file_local;
  load_text_file(file_path, original_file_local);

  const bool has_changed = original_file_local.Len() == 0 || FCString::Strcmp(*original_file_local, *new_file_contents);
  if (has_changed)
    {
    // save the updated version to a tmp file so that the user can see what will be changing
    const FString temp_file_path = file_path + TEXT(".tmp");

    // On Windows, insert CRs before LFs
    #if PLATFORM_WINDOWS
      FString platform_new_file_contents = new_file_contents.Replace(TEXT("\n"), TEXT("\r\n"));
    #else
      const FString & platform_new_file_contents = new_file_contents;
    #endif

    // delete any existing temp file
    IFileManager::Get().Delete(*temp_file_path, false, true);
    if (!FFileHelper::SaveStringToFile(platform_new_file_contents, *temp_file_path, ms_script_file_encoding))
      {
      report_error(FString::Printf(TEXT("Failed to save file: '%s'"), *temp_file_path));
      }
    else
      {
      m_temp_file_paths.AddUnique(temp_file_path);
      }
    }

  return has_changed;
  }

//---------------------------------------------------------------------------------------

void FSkookumScriptGeneratorBase::flush_saved_text_files()
  {
  // Rename temp files
  for (auto & temp_file_path : m_temp_file_paths)
    {
    FString file_path = temp_file_path.Replace(TEXT(".tmp"), TEXT(""));
    IFileManager::Get().Delete(*file_path, false, true, true); // Delete potentially existing version of the file
    if (!IFileManager::Get().Move(*file_path, *temp_file_path, true, true)) // Move new file into its place
      {
      report_error(FString::Printf(TEXT("Couldn't write file '%s'"), *file_path));
      }
    // If source control function provided, make sure file is checked out from source control
    source_control_checkout_or_add(file_path);
    }

  m_temp_file_paths.Reset();
  }

//---------------------------------------------------------------------------------------

FString FSkookumScriptGeneratorBase::skookify_class_name(FName name, FName package_name)
  {
  // A few standard renames we always do
  if (name == ms_name_object) return TEXT("Entity");
  if (name == ms_name_class)  return TEXT("EntityClass");
  if (name == ms_name_entity) return TEXT("GameEntity"); // In case someone defined a class named Entity, make sure it does not collide with SkookumScript's native Entity
  if (name == ms_name_vector) return TEXT("Vector3"); // These are the same class
  if (name == ms_name_enum)   return TEXT("Enum2"); // HACK to avoid collision with Skookum built-in Enum class

  // SkookumScript shortcuts for static function libraries as their names occur very frequently in code
  if (name == ms_name_data_table_function_library)           return TEXT("DataLib");
  if (name == ms_name_gameplay_statics)                      return TEXT("GameLib");
  if (name == ms_name_head_mounted_display_function_library) return TEXT("VRLib");
  if (name == ms_name_kismet_array_library)                  return TEXT("ArrayLib");
  if (name == ms_name_kismet_guid_library)                   return TEXT("GuidLib");
  if (name == ms_name_kismet_input_library)                  return TEXT("InputLib");
  if (name == ms_name_kismet_material_library)               return TEXT("MaterialLib");
  if (name == ms_name_kismet_math_library)                   return TEXT("MathLib");
  if (name == ms_name_kismet_node_helper_library)            return TEXT("NodeLib");
  if (name == ms_name_kismet_string_library)                 return TEXT("StringLib");
  if (name == ms_name_kismet_system_library)                 return TEXT("SystemLib");
  if (name == ms_name_kismet_text_library)                   return TEXT("TextLib");
  if (name == ms_name_visual_logger_kismet_library)          return TEXT("LogLib");

  // Start by checking the rename map for a match
  FString skookum_name = m_current_target_p->find_class_rename_replacement(name, package_name);

  // If none found, make one up that conforms to Sk naming requirements
  if (skookum_name.IsEmpty())
    {
    skookum_name = skookify_class_name_basic(name.ToString());
    }

  return skookum_name;
  }

//---------------------------------------------------------------------------------------

FString FSkookumScriptGeneratorBase::skookify_enum_name(FName name, FName package_name)
  {
  // Start by checking the rename map for a match
  FString skookum_name = m_current_target_p->find_class_rename_replacement(name, package_name);

  // If none found, make one up that conforms to Sk naming requirements
  if (skookum_name.IsEmpty())
    {
    skookum_name = name.ToString();

    #if 0 // MJB Enum name skookification disabled for now
      if (skookum_name.Len() > 0)
        {
        TCHAR ch = skookum_name[0];
        if (FChar::IsUpper(ch))
          {
          // If name starts with upper case letter that's not 'E',
          // or with upper case letter followed by lower case, prefix with 'E'
          if (ch != 'E' || (skookum_name.Len() >= 2 && FChar::IsLower(skookum_name[1])))
            {
            return TEXT("E") + skookum_name;
            }
          }
        else
          {
          // If doesn't start with upper case letter, make first letter uppercase
          skookum_name[0] = FChar::ToUpper(ch);
          }
        }
    #endif
    }

  return skookum_name;
  }

//---------------------------------------------------------------------------------------

FString FSkookumScriptGeneratorBase::skookify_data_name(FName name, FName owner_name, FName package_name, bool append_question_mark, eDataScope scope)
  {
  // Start by checking the rename map for a match
  FString skookum_name = m_current_target_p->find_data_rename_replacement(name, owner_name, package_name);

  // If none found, make one up that conforms to Sk naming requirements
  if (skookum_name.IsEmpty())
    {
    skookum_name = skookify_data_name_basic(name.ToString(), append_question_mark, scope);
    }

  return skookum_name;
  }

//---------------------------------------------------------------------------------------

FString FSkookumScriptGeneratorBase::get_skookified_enum_val_name_by_index(UEnum * enum_p, int32 index)
  {
  return skookify_data_name(FName(*enum_p->GetDisplayNameTextByIndex(index).ToString()), enum_p->GetFName(), enum_p->GetOutermost()->GetFName(), false, DataScope_class);
  }

//---------------------------------------------------------------------------------------

FString FSkookumScriptGeneratorBase::get_skookified_default_enum_val_name_by_id(UEnum * enum_p, const FString & id)
  {
  int32 index = enum_p->GetIndexByNameString(id);
  if (index == INDEX_NONE)
    {
    index = 0;
    }
  return get_skookified_enum_val_name_by_index(enum_p, index);
  }

//---------------------------------------------------------------------------------------
FString FSkookumScriptGeneratorBase::get_skookum_class_name(FField * type_p)
  {
  UObject * obj_p = type_p->GetOwnerStruct();
  #if WITH_EDITOR
    UClass * class_p = type_p->GetOwnerClass();
    if (class_p)
      {
      UBlueprint * blueprint_p = UBlueprint::GetBlueprintFromClass(class_p);
      if (blueprint_p) obj_p = blueprint_p;
      }
  #endif
  FName name = obj_p->GetFName();
  FName package_name = obj_p->GetOutermost()->GetFName();
  return obj_p->IsA<UEnum>() ? skookify_enum_name(name, package_name) : skookify_class_name(name, package_name);
  }

FString FSkookumScriptGeneratorBase::get_skookum_class_name(UObject * type_p)
{
  if (!type_p) { return TEXT(""); }

  UObject * obj_p = type_p;
#if WITH_EDITOR
  UClass * class_p = Cast<UClass>(type_p);
  if (class_p)
  {
    UBlueprint * blueprint_p = UBlueprint::GetBlueprintFromClass(class_p);
    if (blueprint_p) obj_p = blueprint_p;
  }
#endif
  FName name = obj_p->GetFName();
  FName package_name = obj_p->GetOutermost()->GetFName();
  return obj_p->IsA<UEnum>() ? skookify_enum_name(name, package_name) : skookify_class_name(name, package_name);
}

//---------------------------------------------------------------------------------------

FString FSkookumScriptGeneratorBase::get_skookum_data_name(FProperty * property_p)
  {
  UObject * owner_p = property_p->GetOwnerUObject();
  #if WITH_EDITOR
    UClass * owner_class_p = Cast<UClass>(owner_p);
    if (owner_class_p)
      {
      UBlueprint * blueprint_p = UBlueprint::GetBlueprintFromClass(owner_class_p);
      if (blueprint_p) owner_p = blueprint_p;
      }
  #endif
  return skookify_data_name(property_p->GetFName(), owner_p->GetFName(), owner_p->GetOutermost()->GetFName(), property_p->IsA<FBoolProperty>(), DataScope_instance);
  }

//---------------------------------------------------------------------------------------

FString FSkookumScriptGeneratorBase::get_skookum_parent_name(FField * type_p, int32 include_priority, uint32 referenced_flags, UStruct ** out_parent_pp)
  {
  UStruct * struct_or_class_p = type_p->GetOwnerStruct();
  if (struct_or_class_p)
    {
    UStruct * parent_p = struct_or_class_p->GetSuperStruct();
    if (out_parent_pp) *out_parent_pp = parent_p;
    if (!parent_p)
      {
      return get_skookum_struct_type(struct_or_class_p) == SkTypeID_UStruct ? TEXT("UStruct") : TEXT("Object");
      }

    // Mark parent as needed
    on_type_referenced(type_p->GetOwnerUField(), include_priority + 1, referenced_flags);

    return get_skookum_class_name(type_p);
    }  

  #if WITH_EDITOR
  if (FFieldClass* FieldClass = type_p->GetClass())
  {
    if (FFieldClass* SuperField = FieldClass->GetSuperClass())
    {
      UBlueprint * blueprint_p = Cast<UBlueprint>(SuperField->GetDefaultObject()->GetOwnerUObject());
      if (blueprint_p)
      {
        // Mark parent as needed
        FField * parent_field_p = SuperField->GetDefaultObject();
        if (parent_field_p)
        {
          on_type_referenced(parent_field_p->GetOwnerUField(), include_priority + 1, referenced_flags);
        }
        if (out_parent_pp) *out_parent_pp = parent_field_p->GetOwnerStruct();

        return get_skookum_class_name(parent_field_p->GetOwnerUObject());
      }
    }
  }
  #endif

  if (out_parent_pp) *out_parent_pp = nullptr;
  return TEXT("Enum");
  }

//---------------------------------------------------------------------------------------

FString FSkookumScriptGeneratorBase::get_skookum_parent_name(UField * type_p, int32 include_priority, uint32 referenced_flags, UStruct ** out_parent_pp)
{
  UStruct * struct_or_class_p = Cast<UStruct>(type_p);
  if (struct_or_class_p)
  {
    UStruct * parent_p = struct_or_class_p->GetSuperStruct();
    if (out_parent_pp) *out_parent_pp = parent_p;
    if (!parent_p)
    {
      return get_skookum_struct_type(struct_or_class_p) == SkTypeID_UStruct ? TEXT("UStruct") : TEXT("Object");
    }

    // Mark parent as needed
    on_type_referenced(parent_p, include_priority + 1, referenced_flags);

    return get_skookum_class_name(parent_p);
  }

#if WITH_EDITOR
  UBlueprint * blueprint_p = Cast<UBlueprint>(type_p);
  if (blueprint_p)
  {
    UObject * parent_p = blueprint_p->ParentClass;
    // Mark parent as needed
    UField * parent_field_p = get_field(parent_p);
    if (parent_field_p)
    {
      on_type_referenced(parent_field_p, include_priority + 1, referenced_flags);
    }
    if (out_parent_pp) *out_parent_pp = Cast<UStruct>(parent_field_p);

    return get_skookum_class_name(parent_p);

  }

#endif

  if (out_parent_pp) *out_parent_pp = nullptr;
  return TEXT("Enum");
}
//---------------------------------------------------------------------------------------

FString FSkookumScriptGeneratorBase::get_skookum_class_path(UObject * type_p, int32 include_priority, uint32 referenced_flags, FString * out_class_name_p)
  {
  // Remember class name
  FString class_name = get_skookum_class_name(type_p);
  if (out_class_name_p)
    {
    *out_class_name_p = class_name;
    }

  // Make array of the super classes
  TArray<SuperClassEntry> super_class_stack;
  super_class_stack.Reserve(32);
  UStruct * struct_or_class_p = Cast<UStruct>(type_p);
  if (struct_or_class_p)
    {
    bool parent_to_sk_UStruct = (get_skookum_struct_type(struct_or_class_p) == SkTypeID_UStruct);
    UStruct * super_p = struct_or_class_p;
    while ((super_p = super_p->GetSuperStruct()) != nullptr)
      {
      super_class_stack.Push(SuperClassEntry(get_skookum_class_name(super_p), super_p));
      on_type_referenced(super_p, ++include_priority, referenced_flags); // Mark all parents as needed
      // Turn `Vector` into built-in `Vector3`:
      if (get_skookum_struct_type(super_p) != SkTypeID_UStruct)
        {
        parent_to_sk_UStruct = false;
        break;
        }
      }
    // If it's a UStruct, group under virtual parent class "UStruct"
    if (parent_to_sk_UStruct)
      {
      super_class_stack.Push(SuperClassEntry(TEXT("UStruct"), nullptr));
      }
    }
#if WITH_EDITOR
  else if (type_p->IsA<UBlueprint>())
    {
    for (UStruct * super_p = CastChecked<UStruct>(static_cast<UBlueprint *>(type_p)->ParentClass); super_p; super_p = super_p->GetSuperStruct())
      {
      super_class_stack.Push(SuperClassEntry(get_skookum_class_name(super_p), super_p));
      on_type_referenced(super_p, ++include_priority, referenced_flags); // Mark all parents as needed
      }
    }
#endif
  else
    {
    // If not struct must be enum at this point or something is fishy
    UEnum * enum_p = CastChecked<UEnum>(type_p);
    super_class_stack.Push(SuperClassEntry(TEXT("Enum"), nullptr));
    }

  // Build path
  int32 max_super_class_nesting = FMath::Max(m_overlay_path_depth - 1, 0);
  FString class_path = m_overlay_path / TEXT("Object");
  for (int32 i = 0; i < max_super_class_nesting && super_class_stack.Num(); ++i)
    {
    class_path /= super_class_stack.Pop().m_name;
    }
  if (super_class_stack.Num())
    {
    class_name = class_name + TEXT(".") + super_class_stack[0].m_name;
    }
  return class_path / class_name;
  }

//---------------------------------------------------------------------------------------

FString FSkookumScriptGeneratorBase::get_skookum_method_file_name(const FString & script_function_name, bool is_static)
  {
  return script_function_name.Replace(TEXT("?"), TEXT("-Q")) + (is_static ? TEXT("()C.sk") : TEXT("().sk"));
  }

//---------------------------------------------------------------------------------------

FString FSkookumScriptGeneratorBase::get_skookum_property_type_name(FProperty * property_p, bool include_invokable_signature, int32 * out_first_line_length_p, int32 * out_max_line_length_p)
  {
  eSkTypeID type_id = get_skookum_property_type(property_p, true);
  FString type_name;
  bool have_max_line_length = false;

  if (type_id == SkTypeID_UObject || type_id == SkTypeID_UObjectWeakPtr)
    {
    FObjectPropertyBase * object_property_p = CastField<FObjectPropertyBase>(property_p);
    type_name = get_skookum_class_name(object_property_p->PropertyClass);
    }
  else if (type_id == SkTypeID_UStruct)
    {
    FStructProperty * struct_p = CastField<FStructProperty>(property_p);
    type_name = get_skookum_class_name(struct_p->Struct);
    }
  else if (type_id == SkTypeID_Enum)
    {
    UEnum * enum_p = get_enum(property_p);
    type_name = get_skookum_class_name(enum_p);
    }
  else if (type_id == SkTypeID_Delegate || type_id == SkTypeID_MulticastDelegate)
    {
    const TCHAR * type_name_p;
    UFunction * signature_p;
    if (type_id == SkTypeID_Delegate)
      {
      type_name_p = TEXT("Delegate");
      signature_p = static_cast<FDelegateProperty *>(property_p)->SignatureFunction;
      }
    else
      {
      type_name_p = TEXT("MulticastDelegate");
      signature_p = static_cast<FMulticastDelegateProperty *>(property_p)->SignatureFunction;
      }
    FString signature_body;
    if (include_invokable_signature)
      {
      have_max_line_length = true;
      int32 num_inputs;
      signature_body = generate_routine_script_parameters(signature_p, 2, &num_inputs, nullptr, out_max_line_length_p);
      if (num_inputs == 0)
        {
        signature_body = TEXT("()");
        }
      else
        {
        signature_body = TEXT("\n  (\n  ") + signature_body + TEXT("\n  )");
        }
      }
    type_name = type_name_p + signature_body;
    }
  else if (type_id == SkTypeID_List)
    {
    have_max_line_length = true;
    type_name = FString::Printf(TEXT("List{%s}"), *get_skookum_property_type_name(CastField<FArrayProperty>(property_p)->Inner, true, out_first_line_length_p, out_max_line_length_p));
    }
  else
    {
    type_name = ms_sk_type_id_names[type_id];
    }

  // Were there any line feeds?
  if (have_max_line_length)
    {
    if (out_first_line_length_p)
      {
      if (!type_name.FindChar('\n', *out_first_line_length_p))
        {
        *out_first_line_length_p = type_name.Len();
        }
      }
    }
  else
    {
    if (out_first_line_length_p) *out_first_line_length_p = type_name.Len();
    if (out_max_line_length_p)   *out_max_line_length_p = 0;
    }

  return type_name;
  }

//---------------------------------------------------------------------------------------

FString FSkookumScriptGeneratorBase::get_skookum_default_initializer(UFunction * function_p, FProperty * param_p)
  {
  FString default_value;

  // For Blueprintcallable functions, assume all arguments have defaults even if not specified
  bool has_default_value = function_p->HasAnyFunctionFlags(FUNC_BlueprintCallable | FUNC_Exec) // || function_p->HasMetaData(*param_p->GetName());
    && !function_p->HasAllFunctionFlags(FUNC_Delegate) // Delegate signatures have no defaults
    && !param_p->IsA<FDelegateProperty>()              // Delegates have no defaults
    && !param_p->IsA<FMulticastDelegateProperty>();    // Delegates have no defaults
  if (has_default_value)
    {
    default_value = function_p->GetMetaData(*param_p->GetName());
    }
  if (default_value.IsEmpty())
    {
    FName cpp_default_value_key(*(TEXT("CPP_Default_") + param_p->GetName()));
    if (function_p->HasMetaData(cpp_default_value_key))
      {
      has_default_value = true;
      default_value = function_p->GetMetaData(cpp_default_value_key);

      // In 4.20 somewhere, default values of nullptr started getting meta data set to none.
      // See HeaderParser.cpp FHeaderParser::DefaultValueStringCppFormatToInnerFormat
      if (param_p->IsA(FClassProperty::StaticClass()) || param_p->IsA(FObjectPropertyBase::StaticClass()))
        {
        if (default_value.Equals(TEXT("None")))
          {
          default_value = TEXT("");
          }
        }
      }
    }
  if (has_default_value)
    {
    // Trivial default?
    if (default_value.IsEmpty())
      {
      eSkTypeID type_id = get_skookum_property_type(param_p, true);
      switch (type_id)
        {
        case SkTypeID_Integer:         default_value = TEXT("0"); break;
        case SkTypeID_Real:            default_value = TEXT("0.0"); break;
        case SkTypeID_Boolean:         default_value = TEXT("false"); break;
        case SkTypeID_String:          default_value = TEXT("\"\""); break;
        case SkTypeID_Enum:            default_value = get_skookum_class_name(get_enum(param_p)) + TEXT(".") + get_skookified_enum_val_name_by_index(get_enum(param_p), 0); break;
        case SkTypeID_Name:            default_value = TEXT("Name!none"); break;
        case SkTypeID_Vector2:
        case SkTypeID_Vector3:
        case SkTypeID_Vector4:
        case SkTypeID_Rotation:
        case SkTypeID_RotationAngles:
        case SkTypeID_Transform:
        case SkTypeID_Color:
        case SkTypeID_List:
        case SkTypeID_Delegate:
        case SkTypeID_MulticastDelegate:
        case SkTypeID_UStruct:         default_value = get_skookum_property_type_name(param_p) + TEXT("!"); break;
        case SkTypeID_UClass:
        case SkTypeID_UObject:
        case SkTypeID_UObjectWeakPtr:  default_value = (param_p->GetName() == TEXT("WorldContextObject")) ? TEXT("@@world") : get_skookum_class_name(CastField<FObjectPropertyBase>(param_p)->PropertyClass) + TEXT("!null"); break;
        }
      }
    else
      {
      // Remove variable assignments from default_value (e.g. "X=")
      for (int32 pos = 0; pos < default_value.Len() - 1; ++pos)
        {
        if (FChar::IsAlpha(default_value[pos]) && default_value[pos + 1] == '=')
          {
          default_value.RemoveAt(pos, 2);
          }
        }

      // Trim trailing zeros off floating point numbers
      for (int32 pos = 0; pos < default_value.Len(); ++pos)
        {
        if (FChar::IsDigit(default_value[pos]))
          {
          int32 npos = pos;
          while (npos < default_value.Len() && FChar::IsDigit(default_value[npos])) ++npos;
          if (npos < default_value.Len() && default_value[npos] == '.')
            {
            ++npos;
            while (npos < default_value.Len() && FChar::IsDigit(default_value[npos])) ++npos;
            int32 zpos = npos - 1;
            while (default_value[zpos] == '0') --zpos;
            if (default_value[zpos] == '.') ++zpos;
            ++zpos;
            if (npos > zpos) default_value.RemoveAt(zpos, npos - zpos);
            npos = zpos;
            }
          pos = npos;
          }
        }

      // Skookify the default argument
      eSkTypeID type_id = get_skookum_property_type(param_p, true);
      switch (type_id)
        {
        case SkTypeID_Integer:         break; // Leave as-is
        case SkTypeID_Real:            if (default_value[default_value.Len() - 1] == 'f') default_value[default_value.Len() - 1] = '0'; break; // If last character is f, replace with 0
        case SkTypeID_Boolean:         default_value = FChar::IsDigit(default_value[0]) ? (default_value[0] == '1' ? TEXT("true") : TEXT("false")) : *default_value.ToLower(); break;
        case SkTypeID_String:          default_value = TEXT("\"") + default_value + TEXT("\""); break;
        case SkTypeID_Name:            default_value = (default_value == TEXT("None") ? TEXT("Name!none") : TEXT("Name!(\"") + default_value + TEXT("\")")); break;
        case SkTypeID_Enum:            default_value = get_skookum_class_name(get_enum(param_p)) + TEXT(".") + get_skookified_default_enum_val_name_by_id(get_enum(param_p), default_value); break;
        case SkTypeID_Vector2:         default_value = TEXT("Vector2!xy") + default_value; break;
        case SkTypeID_Vector3:         default_value = TEXT("Vector3!xyz(") + default_value + TEXT(")"); break;
        case SkTypeID_Vector4:         default_value = TEXT("Vector4!xyzw") + default_value; break;
        case SkTypeID_Rotation:        break; // Not implemented yet - leave as-is for now
        case SkTypeID_RotationAngles:  default_value = TEXT("RotationAngles!yaw_pitch_roll(") + default_value + TEXT(")"); break;
        case SkTypeID_Transform:       break; // Not implemented yet - leave as-is for now
        case SkTypeID_Color:           default_value = TEXT("Color!rgba") + default_value; break;
        case SkTypeID_UStruct:         if (default_value == TEXT("LatentInfo")) default_value = get_skookum_class_name(CastField<FStructProperty>(param_p)->Struct) + TEXT("!"); break;
        case SkTypeID_UClass:          default_value = skookify_class_name(FName(*default_value), NAME_None) + TEXT(".static_class"); break;
        case SkTypeID_UObject:
        case SkTypeID_UObjectWeakPtr:  if (default_value == TEXT("WorldContext") || default_value == TEXT("WorldContextObject") || param_p->GetName() == TEXT("WorldContextObject")) default_value = TEXT("@@world"); break;
        }
      }

    check(!default_value.IsEmpty()); // Default value must be non-empty at this point

    default_value = TEXT(" : ") + default_value;
    }

  return default_value;
  }

//---------------------------------------------------------------------------------------

uint32 FSkookumScriptGeneratorBase::get_skookum_symbol_id(const FString & string)
  {
  char buffer[256];
  char * end_p = FPlatformString::Convert(buffer, sizeof(buffer), *string, string.Len());
  return FCrc::MemCrc32(buffer, end_p - buffer);
  }

//---------------------------------------------------------------------------------------

void FSkookumScriptGeneratorBase::build_comment_block(FString& comment_block, const FString& ue_name_comment)
{
  // Convert to comment block
  if (!comment_block.IsEmpty())
    {
    // "Comment out" the comment block
    comment_block = TEXT("// ") + comment_block;
    comment_block.ReplaceInline(TEXT("\n"), TEXT("\n// "));
    comment_block += TEXT("\n");
    // Replace parameter names with their skookified versions
    for (int32 pos = 0;;)
      {
      pos = comment_block.Find(TEXT("@param"), ESearchCase::IgnoreCase, ESearchDir::FromStart, pos);
      if (pos < 0) break;

      pos += 6; // Skip "@param"
      while (pos < comment_block.Len() && FChar::IsWhitespace(comment_block[pos])) ++pos; // Skip white space
      int32 identifier_begin = pos;
      while (pos < comment_block.Len() && FChar::IsIdentifier(comment_block[pos])) ++pos; // Skip identifier
      int32 identifier_length = pos - identifier_begin;
      if (identifier_length > 0)
        {
        // Replace parameter name with skookified version
        FString param_name = skookify_param_name(comment_block.Mid(identifier_begin, identifier_length), false);
        comment_block.RemoveAt(identifier_begin, identifier_length, false);
        comment_block.InsertAt(identifier_begin, param_name);
        pos += param_name.Len() - identifier_length;
        }
      }
    }

  // Add UE4 name of this object
  comment_block += TEXT("//\n") + ue_name_comment;
}

//---------------------------------------------------------------------------------------

FString FSkookumScriptGeneratorBase::get_comment_block(UObject * type_p)
  {
  FString comment_block;

  // Comment specifying the original name of this object
  FString this_kind =
    type_p->IsA<UFunction>() ? TEXT("method") :
    (type_p->IsA<UClass>() ? TEXT("class") :
    (type_p->IsA<UStruct>() ? TEXT("struct") :
    (type_p->IsA<UEnum>() ? TEXT("enum") :
    TEXT("type"))));
  FString ue_name_comment = FString::Printf(TEXT("// UE4 name of this %s: %s\n"), *this_kind, *type_p->GetName());

  #if WITH_EDITOR || HACK_HEADER_GENERATOR
    UField * field_p = get_field(type_p);
    if (field_p)
      {
      // Get tool tip from meta data
      comment_block = field_p->GetToolTipText().ToString();
      
      build_comment_block(comment_block, ue_name_comment);

      // Add display name of this object
      if (field_p->HasMetaData(ms_meta_data_key_display_name))
      {
        FString display_name = field_p->GetMetaData(ms_meta_data_key_display_name);
        comment_block += FString::Printf(TEXT("// Blueprint display name: %s\n"), *display_name);
      }

      // Add Blueprint category
      if (field_p->HasMetaData(ms_meta_data_key_function_category))
      {
        FString category_name = field_p->GetMetaData(ms_meta_data_key_function_category);
        comment_block += FString::Printf(TEXT("// Blueprint category: %s\n"), *category_name);
      }
    }
    else
  #endif
      {
      // Add UE4 name of this object
      comment_block += ue_name_comment;
      }

  return comment_block + TEXT("\n");
  }

//---------------------------------------------------------------------------------------

FString FSkookumScriptGeneratorBase::get_comment_block(FField * type_p)
{
  FString comment_block;

  // Comment specifying the original name of this object
  FString this_kind =
    type_p->IsA<FStructProperty>() ? TEXT("struct") :
      (type_p->IsA<FProperty>() ? TEXT("property") :
      (type_p->IsA<FEnumProperty>() ? TEXT("enum") :
        TEXT("type")));
  FString ue_name_comment = FString::Printf(TEXT("// UE4 name of this %s: %s\n"), *this_kind, *type_p->GetName());

#if WITH_EDITOR || HACK_HEADER_GENERATOR
  // Get tool tip from meta data
  comment_block = type_p->GetToolTipText().ToString();

  build_comment_block(comment_block, ue_name_comment);

  // Add display name of this object
  if (type_p->HasMetaData(ms_meta_data_key_display_name))
  {
    FString display_name = type_p->GetMetaData(ms_meta_data_key_display_name);
    comment_block += FString::Printf(TEXT("// Blueprint display name: %s\n"), *display_name);
  }

  // Add Blueprint category
  if (type_p->HasMetaData(ms_meta_data_key_function_category))
  {
    FString category_name = type_p->GetMetaData(ms_meta_data_key_function_category);
    comment_block += FString::Printf(TEXT("// Blueprint category: %s\n"), *category_name);
  }
#endif

  return comment_block + TEXT("\n");
}

//---------------------------------------------------------------------------------------
// Find its UField if type is not a UField
UField * FSkookumScriptGeneratorBase::get_field(UObject * type_p)
  {
  #if WITH_EDITOR
    UBlueprint * blueprint_p = Cast<UBlueprint>(type_p);
    if (blueprint_p)
      {
      return blueprint_p->GeneratedClass;
      }    
  #endif
  return Cast<UField>(type_p);
  }

//---------------------------------------------------------------------------------------

FString FSkookumScriptGeneratorBase::multiline_right_pad(const FString & text, int32 desired_width)
  {
  int32 lf_pos;
  bool has_lf = text.FindLastChar('\n', lf_pos);
  return text.RightPad(has_lf ? lf_pos + 1 + desired_width : desired_width);
  }

//---------------------------------------------------------------------------------------

FString FSkookumScriptGeneratorBase::generate_routine_script_parameters(UFunction * function_p, int32 indent_spaces, int32 * out_num_inputs_p, FString * out_return_type_name_p, int32 * out_max_line_length_p)
  {
  if (out_return_type_name_p) out_return_type_name_p->Empty();
  if (out_num_inputs_p) *out_num_inputs_p = 0;
  if (out_max_line_length_p) *out_max_line_length_p = 0;

  FString parameter_body;

  bool has_params_or_return_value = (function_p->ChildProperties != NULL);
  if (has_params_or_return_value)
    {
    // Figure out column width of variable types & names
    int32 max_type_length = 0;
    int32 max_name_length = 0;
    int32 inputs_count = 0;
    for (TFieldIterator<FProperty> param_it(function_p); param_it; ++param_it)
      {
      FProperty * param_p = *param_it;
      if ((param_p->GetPropertyFlags() & (CPF_ReturnParm | CPF_Parm)) == CPF_Parm)
        {
        int32 first_type_line_length;
        int32 max_type_line_length;
        FString type_name = get_skookum_property_type_name(param_p, true, &first_type_line_length, &max_type_line_length);
        FString var_name = skookify_param_name(param_p->GetName(), param_p->IsA<FBoolProperty>());
        max_type_length = FMath::Max(max_type_length, max_type_line_length ? max_type_line_length : first_type_line_length);
        max_name_length = FMath::Max(max_name_length, var_name.Len());
        ++inputs_count;
        }
      }
    if (out_num_inputs_p) *out_num_inputs_p = inputs_count;
    if (out_max_line_length_p) *out_max_line_length_p = indent_spaces + max_type_length + 1 + max_name_length;

    // Format nicely
    FString separator;
    FString separator_indent = TEXT("\n") + FString::ChrN(indent_spaces, ' ');
    for (TFieldIterator<FProperty> param_it(function_p); param_it; ++param_it)
      {
      FProperty * param_p = *param_it;
      if (param_p->HasAllPropertyFlags(CPF_Parm))
        {
        if (param_p->HasAllPropertyFlags(CPF_ReturnParm))
          {
          if (out_return_type_name_p) *out_return_type_name_p = get_skookum_property_type_name(param_p);
          }
        else
          {
          FString type_name = get_skookum_property_type_name(param_p, true);
          FString var_name = skookify_param_name(param_p->GetName(), param_p->IsA<FBoolProperty>());
          FString default_initializer = get_skookum_default_initializer(function_p, param_p);
          parameter_body += separator + multiline_right_pad(type_name, max_type_length) + TEXT(" ");
          if (default_initializer.IsEmpty())
            {
            parameter_body += var_name;
            }
          else
            {
            parameter_body += var_name.RightPad(max_name_length) + get_skookum_default_initializer(function_p, param_p);
            }
          }
        separator = separator_indent;
        }
      }
    }

  return parameter_body;
  }

//---------------------------------------------------------------------------------------

FString FSkookumScriptGeneratorBase::generate_class_meta_file_body(UObject * type_p)
  {
  // Begin with comment bock explaining the class
  FString meta_body = get_comment_block(type_p);

  // Add name and file name of package where it came from if applicable
  bool is_reflected_data = false;

  #if WITH_EDITOR
    UObject * asset_p = nullptr;

    if (UBlueprint * blueprint_p = Cast<UBlueprint>(type_p))
      {
      is_reflected_data = true;
      asset_p = blueprint_p;
      }

    if (UClass * class_p = Cast<UClass>(type_p))
      {
      if (UBlueprint * blueprint_p = UBlueprint::GetBlueprintFromClass(class_p))
        {
        is_reflected_data = true;
        asset_p = blueprint_p;
        }
      }

    if (UUserDefinedStruct * struct_p = Cast<UUserDefinedStruct>(type_p))
      {
      is_reflected_data = true;
      asset_p = struct_p;
      }

    if (UUserDefinedEnum * enum_p = Cast<UUserDefinedEnum>(type_p))
      {
      is_reflected_data = true;
      asset_p = enum_p;
      }

    if (asset_p)
      {
      meta_body += ms_asset_name_key + asset_p->GetName() + TEXT("\n");
      UPackage * package_p = Cast<UPackage>(asset_p->GetOutermost());
      if (package_p)
        {
        meta_body += ms_package_name_key + package_p->GetName() + TEXT("\"\n");
        // The package path has inconsistent content and can cause bogus file changes to the generated script files, therefore disabled here for now 
        //meta_body += ms_package_path_key + package_p->FileName.ToString() + TEXT("\"\n");
        }
      meta_body += TEXT("\n");
      }
  #endif

  // Also add annotations
  meta_body += FString::Printf(TEXT("annotations: &reflected_%s &name(\"%s\")\n"), is_reflected_data ? TEXT("data") : TEXT("cpp"), *type_p->GetName());

  return meta_body;
  }

//---------------------------------------------------------------------------------------

FString FSkookumScriptGeneratorBase::generate_class_instance_data_file_body(UStruct * struct_or_class_p, int32 include_priority, uint32 referenced_flags)
  {
  FString data_body;

  // Figure out column width of variable types & names
  int32 max_first_type_line_length = 0;
  int32 max_type_line_length = 0;
  int32 max_name_length = 0;
  int32 max_raw_name_length = 0;
  for (TFieldIterator<FProperty> property_it(struct_or_class_p, EFieldIteratorFlags::ExcludeSuper); property_it; ++property_it)
    {
    FProperty * var_p = *property_it;
    if (can_export_property(var_p, include_priority, referenced_flags))
      {
      int32 first_line_length;
      int32 max_line_length;
      FString type_name = get_skookum_property_type_name(var_p, true, &first_line_length, &max_line_length);
      FString var_name = get_skookum_data_name(var_p);
      FString raw_name = var_p->GetName();
      max_first_type_line_length = FMath::Max(max_first_type_line_length, first_line_length);
      max_type_line_length = FMath::Max(max_type_line_length, max_line_length);
      max_name_length = FMath::Max(max_name_length, var_name.Len());
      max_raw_name_length = FMath::Max(max_raw_name_length, raw_name.Len());
      }
    }

  // Format nicely
  int32 max_type_length = FMath::Max(max_type_line_length, max_first_type_line_length + max_raw_name_length + 9);
  for (TFieldIterator<FProperty> property_it(struct_or_class_p, EFieldIteratorFlags::ExcludeSuper); property_it; ++property_it)
    {
    FProperty * var_p = *property_it;
    FString type_name = get_skookum_property_type_name(var_p, true);
    FString var_name = get_skookum_data_name(var_p);
    FString raw_name = var_p->GetName();
    FString data_definition = multiline_right_pad(FString::Printf(TEXT("&raw(\"%s\")"), *raw_name).RightPad(max_raw_name_length + 9) + type_name, max_type_length) + TEXT(" !") + var_name.RightPad(max_name_length);
    if (can_export_property(var_p, include_priority, referenced_flags))
      {
      FString comment;
      #if WITH_EDITOR || HACK_HEADER_GENERATOR
        comment = var_p->GetToolTipText().ToString().Replace(TEXT("\n"), TEXT(" "));
      #endif
      data_body += FString::Printf(TEXT("%s // %s\n"), *data_definition, *comment);
      }
    else if (var_p->GetFName() != ms_skookum_script_instance_property_name) // Completely hide SkookumScriptInstanceProperties
      {
      data_body += FString::Printf(TEXT("/* %s // Unsupported, Skipped in config or marked EDITOR_ONLY */\n"), *data_definition);
      }
    }

  // Prepend empty line if anything there
  if (!data_body.IsEmpty())
    {
    data_body = TEXT("\n") + data_body;
    }

  return data_body;
  }

//---------------------------------------------------------------------------------------

FString FSkookumScriptGeneratorBase::generate_enum_class_data_body(UEnum * enum_p)
  {
  FString data_body = TEXT("\n");
  FString enum_type_name = get_skookum_class_name(enum_p);

  // Class data members
  for (int32 enum_index = 0; enum_index < enum_p->NumEnums(); ++enum_index)
    {
    FString skookified_val_name = get_skookified_enum_val_name_by_index(enum_p, enum_index);
    data_body += FString::Printf(TEXT("%s !%s\n"), *enum_type_name, *skookified_val_name);
    }

  return data_body;
  }

//---------------------------------------------------------------------------------------

FString FSkookumScriptGeneratorBase::generate_enum_class_constructor_body(UEnum * enum_p)
  {
  FString enum_type_name = get_skookum_class_name(enum_p);
  FString constructor_body = FString::Printf(TEXT("// %s\n// EnumPath: %s\n\n()\n\n  [\n"), *enum_type_name, *enum_p->GetPathName());

  // Class constructor
  int32 max_name_length = 0;
  // Pass 0: Compute max_name_length
  // Pass 1: Generate the data
  for (uint32_t pass = 0; pass < 2; ++pass)
    {
    for (int32 enum_index = 0; enum_index < enum_p->NumEnums(); ++enum_index)
      {
      FString skookified_val_name = get_skookified_enum_val_name_by_index(enum_p, enum_index);
      if (pass == 0)
        {
        max_name_length = FMath::Max(max_name_length, skookified_val_name.Len());
        }
      else
        {
        int32 enum_val = enum_p->GetValueByIndex(enum_index);
        constructor_body += FString::Printf(TEXT("  %s %s!int(%d)\n"), *(skookified_val_name + TEXT(":")).RightPad(max_name_length + 1), *enum_type_name, enum_val);
        }
      }
    }

  constructor_body += TEXT("  ]\n");

  return constructor_body;
  }

//---------------------------------------------------------------------------------------

FString FSkookumScriptGeneratorBase::generate_method_script_file_body(UFunction * function_p, const FString & script_function_name)
  {
  // Generate method content
  FString method_body = get_comment_block(function_p);

  // Generate aka annotations
  method_body += FString::Printf(TEXT("&aka(\"%s\")\n"), *function_p->GetName());
  if (function_p->HasMetaData(ms_meta_data_key_display_name))
    {
    FString display_name = function_p->GetMetaData(ms_meta_data_key_display_name);
    if (display_name != function_p->GetName())
      {
      method_body += FString::Printf(TEXT("&aka(\"%s\")\n"), *display_name);
      }
    }
  method_body += TEXT("\n");

  // Generate parameter list
  FString return_type_name;
  int32 num_inputs;
  method_body += TEXT("(") + generate_routine_script_parameters(function_p, 1, &num_inputs, &return_type_name);

  // Place return type on new line if present and more than one parameter
  if (num_inputs > 1 && !return_type_name.IsEmpty())
    {
    method_body += TEXT("\n");
    }
  method_body += TEXT(") ") + return_type_name + TEXT("\n");

  return method_body;
  }

//---------------------------------------------------------------------------------------

void FSkookumScriptGeneratorBase::generate_class_meta_file(UField * type_p, const FString & class_path, const FString & skookum_class_name)
  {
  const FString meta_file_path = class_path / TEXT("!Class.sk-meta");
  FString body = type_p ? generate_class_meta_file_body(type_p) : TEXT("// ") + skookum_class_name + TEXT("\n");
  save_text_file_if_changed(*meta_file_path, body);
  }

//---------------------------------------------------------------------------------------

void FSkookumScriptGeneratorBase::generate_root_meta_file(const TCHAR * root_class_name_p)
  {
  generate_class_meta_file(nullptr, m_overlay_path / TEXT("Object") / root_class_name_p, root_class_name_p);
  }

//=======================================================================================
// GenerationTargetBase implementation
//=======================================================================================

FSkookumScriptGeneratorBase::GenerationTargetBase::eState 
FSkookumScriptGeneratorBase::GenerationTargetBase::initialize(const FString & root_directory_path, const FString & project_name, const GenerationTargetBase * inherit_from_p)
  {
  // Check if root path actually exists
  if (!IFileManager::Get().DirectoryExists(*root_directory_path))
    {
    return State_invalid;
    }

  // Yes, only then set it
  m_root_directory_path = root_directory_path;
  m_project_name = project_name;

  TArray<FString> skip_classes;
  TArray<FString> rename_classes;
  TArray<FString> rename_properties;

  // Do we have a SkookumScript.ini file?
  FString ini_file_path = get_ini_file_path();
  bool ini_file_exists = IFileManager::Get().FileExists(*ini_file_path);
  if (ini_file_exists)
    {
    // Check if file changed since we last parsed it
    FDateTime ini_file_stamp = IFileManager::Get().GetTimeStamp(*ini_file_path);
    if (ini_file_stamp == m_ini_file_stamp)
      {
      return State_valid_unchanged;
      }
    m_ini_file_stamp = ini_file_stamp;

    // Load settings from SkookumScript.ini
    FConfigCacheIni skookumscript_ini(EConfigCacheType::Temporary);
    skookumscript_ini.GetArray(TEXT("CommonSettings"), TEXT("+SkipClasses"), skip_classes, ini_file_path);
    skookumscript_ini.GetArray(TEXT("CommonSettings"), TEXT("+RenameClasses"), rename_classes, ini_file_path);
    skookumscript_ini.GetArray(TEXT("CommonSettings"), TEXT("+RenameProperties"), rename_properties, ini_file_path);
    }
  else if (!inherit_from_p)
    {
    // No SkookumScript.ini found, load legacy specifications from UHT's DefaultEngine.ini

    // Check if file changed since we last parsed it
    FDateTime ini_file_stamp = IFileManager::Get().GetTimeStamp(*GEngineIni);
    if (ini_file_stamp == m_ini_file_stamp)
      {
      return State_valid_unchanged;
      }
    m_ini_file_stamp = ini_file_stamp;

    GConfig->GetArray(TEXT("SkookumScriptGenerator"), TEXT("SkipClasses"), skip_classes, GEngineIni);
    }
  else
    {
    // No ini file found at all
    if (!m_ini_file_stamp.GetTicks())
      {
      return State_valid_unchanged;
      }
    m_ini_file_stamp = FDateTime(0);
    }

  // Found ini data - parse it

  // Inherit rename rules so renaming is consistent
  if (inherit_from_p)
    {
    m_class_rename_map = inherit_from_p->m_class_rename_map;
    m_data_rename_map = inherit_from_p->m_data_rename_map;
    }

  // Remember skip classes and convert to FName
  for (const FString & class_name : skip_classes)
    {
    m_skip_classes.Add(FName(*class_name));
    }

  // Remember and translate rename expressions
  FString separator(TEXT("->"));
  FString dot(TEXT("."));
  FString filter, replacement;
  FString package_name, owner_name, key;
  for (const FString & expr : rename_classes)
    {
    // expr = filter->replacement
    if (expr.Split(separator, &filter, &replacement, ESearchCase::CaseSensitive))
      {
      // Trim whitespace surrounding the separator if any
      filter.TrimStartAndEndInline();
      replacement.TrimStartAndEndInline();
      // Do some sanity checking
      if (replacement.Len() > 0 && FChar::IsUpper(replacement[0]))
        {
        // filter = [package_name.]class_name
        if (!filter.Split(dot, &package_name, &key, ESearchCase::CaseSensitive, ESearchDir::FromEnd))
          {
          package_name.Empty();
          key = filter;
          }
        // key -> {package_name, replacement}
        ClassRenameEntry & new_entry = m_class_rename_map.Add(FName(*key));
        new_entry.m_package_name = *package_name;
        new_entry.m_replacement = replacement;
        }
      else
        {
        GWarn->Log(ELogVerbosity::Warning, FString::Printf(TEXT("Replacement '%s' in SkookumScript class rename rule '%s' must start with an uppercase letter. Rule will be ignored."), *replacement, *expr));
        }
      }
    else
      {
      GWarn->Log(ELogVerbosity::Warning, FString::Printf(TEXT("SkookumScript class rename rule '%s' does not contain a separator (%s) and will be ignored."), *expr, *separator));
      }
    }
  for (const FString & expr : rename_properties)
    {
    // expr = filter|replacement
    if (expr.Split(separator, &filter, &replacement, ESearchCase::CaseSensitive))
      {
      // Trim whitespace surrounding the separator if any
      filter.TrimStartAndEndInline();
      replacement.TrimStartAndEndInline();
      // Do some sanity checking
      if (replacement.Len() >= 2 && replacement[0] == '@' && replacement[1] != '@')
        {
        // filter = [[package_name.]owner_name.]property_name
        if (filter.Split(dot, &package_name, &key, ESearchCase::CaseSensitive, ESearchDir::FromEnd))
          {
          // Check if package_name is specified - if so, split it off
          if (package_name.Split(dot, &filter, &owner_name, ESearchCase::CaseSensitive, ESearchDir::FromEnd))
            {
            package_name = filter;
            }
          else
            {
            owner_name = package_name;
            package_name.Empty();
            }
          }
        else
          {
          package_name.Empty();
          owner_name.Empty();
          key = filter;
          }
        // key -> {package_name, owner_name, replacement}
        DataRenameEntry & new_entry = m_data_rename_map.Add(FName(*key));
        new_entry.m_package_name = *package_name;
        new_entry.m_owner_name = *owner_name;
        new_entry.m_replacement = replacement;
        }
      else
        {
        GWarn->Log(ELogVerbosity::Warning, FString::Printf(TEXT("Replacement '%s' in SkookumScript property rename rule '%s' must start with a single '@'. Rule will be ignored."), *replacement, *expr));
        }
      }
    else
      {
      GWarn->Log(ELogVerbosity::Warning, FString::Printf(TEXT("SkookumScript property rename rule '%s' does not contain a separator (%s) and will be ignored."), *expr, *separator));
      }
    }

  return State_valid_changed;
  }

//---------------------------------------------------------------------------------------

bool FSkookumScriptGeneratorBase::GenerationTargetBase::is_valid() const
  {
  return !m_root_directory_path.IsEmpty();
  }

//---------------------------------------------------------------------------------------

FString FSkookumScriptGeneratorBase::GenerationTargetBase::get_ini_file_path() const
  {
  return m_root_directory_path.IsEmpty() ? m_root_directory_path : m_root_directory_path / TEXT("Config/SkookumScript.ini");
  }

//---------------------------------------------------------------------------------------

bool FSkookumScriptGeneratorBase::GenerationTargetBase::is_type_skipped(FName type_name) const
  {
  // Don't export any classes that are set to skip in the config file. Some BP types will be passed to this
  // from SkookumScriptRuntimeGenerator with names like MyClass_C. Remove the _C before checking the class.
  FString type_string = *type_name.ToString();
  type_string.RemoveFromEnd(TEXT("_C"));
  return m_skip_classes.Contains(FName(*type_string));
  }


//---------------------------------------------------------------------------------------

FString FSkookumScriptGeneratorBase::GenerationTargetBase::find_class_rename_replacement(FName name, FName package_name) const
  {
  // Quickly bail if there is nothing to rename
  if (m_class_rename_map.Num())
    {
    // First, look up the name of this object in the map
    for (tClassRenameMap::TConstKeyIterator it(m_class_rename_map, name); it; ++it)
      {
      // Then check the match candidates for an actual match
      const FName & key = it.Key();
      const ClassRenameEntry & match = it.Value();
      if (key.GetDisplayIndex() == name.GetDisplayIndex()
       && (package_name.IsNone() || match.m_package_name.IsNone() || match.m_package_name == package_name))
        { 
        return match.m_replacement;
        }
      }
    }

  // Nothing found, return empty string
  return FString();
  }

//---------------------------------------------------------------------------------------

FString FSkookumScriptGeneratorBase::GenerationTargetBase::find_data_rename_replacement(FName name, FName owner_name, FName package_name) const
  {
  // Quickly bail if there is nothing to rename
  if (m_data_rename_map.Num())
    {
    // First, look up the name of this object in the map
    for (tDataRenameMap::TConstKeyIterator it(m_data_rename_map, name); it; ++it)
      {
      // Then check the match candidates for an actual match
      const FName & key = it.Key();
      const DataRenameEntry & match = it.Value();
      if (key.GetDisplayIndex() == name.GetDisplayIndex()
       && (package_name.IsNone() || match.m_package_name.IsNone() || match.m_package_name == package_name)
       && (owner_name.IsNone() || match.m_owner_name.IsNone() || match.m_owner_name == owner_name))
        { 
        return match.m_replacement;
        }
      }
    }

  // Nothing found, return empty string
  return FString();
  }

#endif // WITH_EDITOR || HACK_HEADER_GENERATOR
