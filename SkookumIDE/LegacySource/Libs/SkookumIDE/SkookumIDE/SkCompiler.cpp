// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumIDE Application
//
// SkookumScript Compiler / Evaluator & supporting classes
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumIDE/SkCompiler.hpp>
#include <SkookumIDE/SkConsole.hpp>

#include <AgogCore/AChecksum.hpp>
#include <AgogCore/ACompareMethod.hpp>
#include <AgogCore/ADatum.hpp>
#include <AgogCore/AMethod.hpp>
#include <AgogCore/AStringRef.hpp>
#include <AgogCore/ASymbolTable.hpp>

#include <AgogIO/AApplication.hpp>
#include <AgogIO/ADirectory.hpp>

#include <AgogGUI_OS/ADialogOS.hpp>

#include <SkookumScript/SkActor.hpp>
#include <SkookumScript/SkActorClass.hpp>
#include <SkookumScript/SkBrain.hpp>
#include <SkookumScript/SkClass.hpp>
#include <SkookumScript/SkDebug.hpp>
#include <SkookumScript/SkExpressionBase.hpp>
#include <SkookumScript/SkInvokableClass.hpp>
#include <SkookumScript/SkInvokedMethod.hpp>
#include <SkookumScript/SkLiteral.hpp>
#include <SkookumScript/SkMemberInfo.hpp>
#include <SkookumScript/SkNone.hpp>
#include <SkookumScript/SkObjectID.hpp>
#include <SkookumScript/SkParameters.hpp>
#include <SkookumScript/SkSymbolDefs.hpp>
#include <SkookumScript/SkTypedClass.hpp>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


//=======================================================================================
// Local Global Structures
//=======================================================================================

namespace
{

  // Enumerated constants
  enum
    {
    SkCompiler_dots_per_line_max      = 50,
    SkCompiler_internal_method_count  = 0,
    SkCompiler_file_ext_ids_length    = 7,  // Length of .sk-ids
    SkCompiler_overlay_str_length     = MAX_PATH + 100,
    SkCompiler_progress_yield_freq    = 16,
    SkCompiler_parse_class_count_init = 1280  // $Revisit - CReis This should be config file driven
    };

  // Custom Agog Binary Handle Structure
  struct SkBinaryHandleCompiler : public SkBinaryHandle
    {
    // Public Data

      // File handle
      AFile m_file;

    // Public Methods

      SkBinaryHandleCompiler(const AFile & file)
        : m_file(file)
        { m_binary_p = NULL; m_size = 0u; }
    };

  const bool SkCompiler_save_compiled_def      = true;
  const bool SkCompiler_ensure_atomics_def     = true;
  const bool SkCompiler_evaluate_def           = true;

  // *Main* config (.ini) file
  const char * g_ini_file_name_main_p          = "Skookum.ini";
  const char * g_ini_section_settings_p        = "Settings";
  const char * g_ini_key_project_default_p     = "ProjectDefault";
  const char * g_ini_key_user_p                = "User";

  // *Project* config (.ini) file
  const char * g_ini_section_project_p         = "Project";
  const char * g_ini_key_strict_p              = "StrictParse";
  const char * g_ini_key_startup_mind_p        = "StartupMind";

  const char * g_ini_section_overlays_p        = "Script Overlays";
  const char * g_ini_key_overlay_p             = "Overlay%u";

  const char * g_ini_section_output_p          = "Output";
  const char * g_ini_key_compiled_manifest_p   = "CompileManifest";
  const char * g_ini_key_compile_to_p          = "CompileTo";
  const char * g_ini_key_copy_to_p             = "CopyTo%u";

  // *User/IDE* config (.ini) file
  const char * g_ini_section_compiler_p        = "Compiler";
  const char * g_ini_key_code_load_p           = "CodeLoad";
  const char * g_ini_key_compiled_save_p       = "CompiledCodeSave";
  const char * g_ini_key_ensure_atomics_p      = "EnsureAtomics";
  const char * g_ini_key_evaluate_p            = "EvaluateScripts";
  
  AString  g_ini_code_load_def;
  AString  g_ini_overlay_core_name_def;
  AString  g_ini_overlay_core_path_def;
  AString  g_ini_overlay_actor_name_def;
  AString  g_ini_overlay_actor_path_def;
  AString  g_overlay_object_root;
  AString  g_file_ext_skookum;
  AString  g_file_ext_compiled;
  AString  g_file_ext_symbols;

} // End unnamed namespace


//---------------------------------------------------------------------------------------
struct SkScriptEntry : public ADirEntry
  {
  // Data Members

    // Modification time of the compiled code binary.
    ATime m_bin_mod_time;

    // Checksums for tracking changes on the source folders and files that define the
    // class hierarchy.  Used for quick dependency checking when determining whether the
    // compiled code binary is up-to-date with the source scripts.
    uint32_t m_checksum_folders;
    uint32_t m_checksum_files;

    // Set to true if a member is found that is newer than 'm_bin_mod_time'
    bool m_member_newer_b;

    SkScriptEntry() : ADirEntry(), m_checksum_folders(UINT32_MAX), m_checksum_files(UINT32_MAX), m_member_newer_b(false) {}
  };


//=======================================================================================
// SkOverlay Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Gets directory path equivalent for specified class in this overlay
AString SkOverlay::get_path_class(const SkClass & cls) const
  {
  AString  path(NULL, MAX_PATH, 0u);
  uint32_t path_depth    = 0u;
  uint32_t class_depth   = cls.get_superclass_depth();
  uint32_t overlay_depth = (m_path_depth == SkOverlay::Path_depth_any)
    ? class_depth
    : ((uint32_t(m_path_depth) > class_depth) ? class_depth : uint32_t(m_path_depth));

  path.append(m_path_qual);

  while (path_depth < overlay_depth)
    {
    // Append classes onto path.
    path.append(cls.get_class_depth_at(path_depth)->get_name_str());
    path.append('\\');
    path_depth++;
    }

  if (class_depth > overlay_depth)
    {
    // Append flattened name
    path.append(cls.get_superclass()->get_name_str());
    path.append('.');
    }

  // Add `cls` name to end of path
  path.append(cls.get_name_str());
  path.append('\\');

  return path;
  }

//---------------------------------------------------------------------------------------
// Gets directory path equivalent for specified member in this overlay
AString SkOverlay::get_path_member(const SkMemberInfo & member) const
  {
  AString path(get_path_class(*member.get_class()));

  path.append(member.as_file_title(SkMemberInfo::PathFlag__file));

  return path;
  }


//=======================================================================================
// SkCompiler Class Data Members
//=======================================================================================

SkCompiler * SkCompiler::ms_compiler_p = nullptr;


//=======================================================================================
// SkCompiler Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------
void SkCompiler::initialize()
  {
  // Initialize static variables
  const_cast<AString&>(g_ini_code_load_def)          = "newest";
  const_cast<AString&>(g_ini_overlay_core_name_def)  = "Core";
  const_cast<AString&>(g_ini_overlay_core_path_def)  = "ClassLibs\\Core\\";
  const_cast<AString&>(g_ini_overlay_actor_name_def) = "Actor";
  const_cast<AString&>(g_ini_overlay_actor_path_def) = "ClassLibs\\Actor\\";
  const_cast<AString&>(g_overlay_object_root)        = "Object\\";
  const_cast<AString&>(g_file_ext_skookum)           = ".sk";
  const_cast<AString&>(g_file_ext_compiled)          = "sk-bin";
  const_cast<AString&>(g_file_ext_symbols)           = "sk-sym";
  }

//---------------------------------------------------------------------------------------
void SkCompiler::deinitialize()
  {
  const_cast<AString&>(g_ini_code_load_def)          = AString::ms_empty;
  const_cast<AString&>(g_ini_overlay_core_name_def)  = AString::ms_empty;
  const_cast<AString&>(g_ini_overlay_core_path_def)  = AString::ms_empty;
  const_cast<AString&>(g_ini_overlay_actor_name_def) = AString::ms_empty;
  const_cast<AString&>(g_ini_overlay_actor_path_def) = AString::ms_empty;
  const_cast<AString&>(g_overlay_object_root)        = AString::ms_empty;
  const_cast<AString&>(g_file_ext_skookum)           = AString::ms_empty;
  const_cast<AString&>(g_file_ext_compiled)          = AString::ms_empty;
  const_cast<AString&>(g_file_ext_symbols)           = AString::ms_empty;
  }

//---------------------------------------------------------------------------------------
bool SkCompiler::is_initialized()
  {
  return !g_ini_code_load_def.is_empty();
  }

//---------------------------------------------------------------------------------------
// Constructor
SkCompiler::SkCompiler(
  SkConsoleBase * console_p // = nullptr
  ) :
  m_flags(Flag__default | (console_p == nullptr) ? Flag_compile_only : Flag_object_ids_validate),
  m_ini_main(get_ini_file_main()),
  m_ini_ide(get_ini_file_user()),
  m_classes(0u),
  m_methods(SkCompiler_internal_method_count),
  m_coroutines(0u),
  m_data_members(0u),
  m_pre_errors(0u),
  m_errors(0u),
  m_warnings(0u),
  m_progress_count(0u),
  m_init_type(Init_phased),
  m_phase(Phase_idle),
  m_phase_start(Phase_determine_newest),
  m_current_overlay_p(nullptr),
  m_console_p(console_p),
  // $Revisit - CReis The project file could be passed as a exec argument to ensure it
  // doesn't redundantly load a different project before the desired project.
  m_ini_proj_default(get_ini_file_proj_default()),
  m_ini_proj(console_p ? console_p->get_ini_file_proj_startup() : m_ini_proj_default),
  m_use_builtin_actor(true),
  m_bind_name_class_p(nullptr)
  {
  SK_ASSERTX(is_initialized(), "SkCompiler must be initialized before use!");

  m_timer.set_idle_func_p(new AMethod<SkCompiler>(this, &SkCompiler::on_idle));

  load_settings();
  }

//---------------------------------------------------------------------------------------
//  Destructor
// Author(s):    Conan Reis
SkCompiler::~SkCompiler()
  {
  // Unloads SkookumScript and cleans-up
  if (is_flags(Flag_bindings_bound))
    {
    SkookumScript::deinitialize_gameplay();
    SkookumScript::deinitialize_sim();
    SkookumScript::deinitialize_program();
    }

  SkookumScript::deinitialize();

  ms_compiler_p = nullptr;
  }

//---------------------------------------------------------------------------------------
// Start-up compiler phases
// Arg         init_type - start compilation using background phases or do all compilation
//             immediately before returning.  See SkCompiler::eInit
// Author(s):   Conan Reis
void SkCompiler::phases_init(
  eInit init_type // = Init_phased
  )
  {
  m_init_type = init_type;

  // Begin start-up process
  phase_start(m_phase_start);

  if (init_type == Init_immediate)
    {
    // Complete all phases in one go without waiting for on_idle() to be called via
    // multiple iterations in the timer update loop.
    while (m_phase != Phase_idle)
      {
      on_idle();
      }
    }
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkCompiler::phase_start(ePhase phase)
  {
  m_phase = phase;

  m_timer.enable_idle_processing(m_phase != Phase_idle);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkCompiler::phase_next()
  {
  bool last_phase = false;

  if (m_console_p)
    {
    m_console_p->status_update();
    }

  switch (m_phase)
    {
    case Phase_preparse:
      m_phase = Phase_parse;
      break;

    case Phase_parse:
      if (is_flags(Flag_save_compiled) && (m_errors == 0u))
        {
        m_phase = Phase_save_compiled;
        }
      else
        {
        last_phase = true;
        }
      break;

    case Phase_save_compiled:
      last_phase = true;
      break;

    case Phase_load_compiled:
      last_phase = true;
      break;

    case Phase_bind_atomics:
      phases_completed();
      break;
    }

  if (last_phase)
    {
    if (is_flags(Flag_evaluate_scripts))
      {
      m_phase = Phase_bind_atomics;
      }
    else
      {
      phases_completed();
      }
    }
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkCompiler::phases_completed()
  {
  if (!is_flags(Flag_compile_only))
    {
    calc_member_count();
    }

  phase_start(Phase_idle);

  if (m_console_p)
    {
    m_console_p->on_compile_complete();
    }

  if (is_flags(Flag_compile_only))
    {
    AApplication::shut_down(m_errors ? AExitCode_error : AExitCode_ok);
    }
  }

//---------------------------------------------------------------------------------------
// Updates progress on the console if one exists and occasionally yields to
//             other applications
// Arg         completed_pass - set to true if a compile pass was completed
// Author(s):   Conan Reis
void SkCompiler::progress(
  bool completed_pass // = false
  )
  {
  if (m_console_p)
    {
    m_console_p->progress_dot(completed_pass);
    }

  m_progress_count++;

  if (completed_pass || (m_progress_count >= SkCompiler_progress_yield_freq))
    {
    m_progress_count = 0u;
    AMessageTarget::yield();
    }
  }

//---------------------------------------------------------------------------------------
// Allow log and other aspects of UI to get a chance to update
// Author(s):   Conan Reis
void SkCompiler::update_msg_queue()
  {
  if (m_init_type == Init_phased)
    {
    AMessageTarget::process_messages(AAsyncFilter__none);
    }
  }

//---------------------------------------------------------------------------------------
// Maps an expected text file in ASCII/ANSI or UTF-8 to a memory block and skips past any
// Byte Order Mark (BOM). If the file is UTF-16/UCS-2 an error is printed and nullptr is
// returned with `*length_p != 0u`.
// 
// This efficiently streams in the file as needed as the memory is accessed using the same
// tech as an OS swap file.
// 
// Returns: file as memory block or nullptr if empty or UTF-16/UCS-2 file.
// 
// Params:
//   length_p - address to store length of file in bytes
const char * SkCompiler::map_text_file(AFile * file_p, uint32_t * length_p)
  {
  const char * file_cstr_p = static_cast<const char *>(
    file_p->map_memory(AFile::Access_read, length_p));

  if (*length_p <= 1u)
    {
    return file_cstr_p;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Check for Unicode Byte Order Mark (BOM)

  uint8_t ch1 = uint8_t(file_cstr_p[0]);
  uint8_t ch2 = uint8_t(file_cstr_p[1]);

  // UTF-16 (Big Endian)     FE FF
  // UTF-16 (Little Endian)  FF FE
  if (((ch1 == 0xfe) && (ch2 == 0xff)) || ((ch1 == 0xff) && (ch2 == 0xfe)))
    {
    bool show_error = true;
    
    if (m_phase == Phase_preparse)
      {
      m_pre_errors++;
      show_error = is_flags(Flag_show_preparse_errors);
      }
    else
      {
      m_errors++;
      }

    if (show_error)
      {
      SkDebug::print(
        a_str_format(
          "\nUnsupported UTF-16/UCS-2 text file '%s' found!\n"
          "Please resave to ASCII/ANSI or UTF-8 (limited display support and non-ASCII characters in comments okay).\n",
          file_p->as_cstr()
          ),
        SkLocale_all,
        SkDPrintType_error);
      }

    return nullptr;
    }

  // UTF-8 (optional BOM)    EF BB BF
  if ((ch1 == 0xef) && (ch2 == 0xbb) && (*length_p >= 3u) && (uint8_t(file_cstr_p[2]) == 0xbf))
    {
    // UTF-8 with BOM
    // Skip BOM
    *length_p -= 3u;
    return file_cstr_p + 3;
    }

  // ASCII or ANSI
  return file_cstr_p;
  }

//---------------------------------------------------------------------------------------
// Arg          file_p - file to parse for invokables
// Author(s):    Conan Reis
void SkCompiler::parse_file(AFile * file_p)
  {
  SkMemberInfo info;

  if (parse_file_member(*file_p, &info))
    {
    // Note, data members are parsed at preparse stage
    if (info.m_type < SkMember_data)
      {
      #if (SKOOKUM & SK_DEBUG)

        // $HACK - Stop at a specific file
        //if ((info.m_member_id.get_scope()->get_name_id() == 0x15854c57)  // remove_eqv
        //  && (info.m_member_id.get_name_id() == 0x21c4152f))             // List
        //  {
        //  // Put breakpoint on line below
        //  ADebug::print_format("\n" A_SOURCE_STR "Found file: %s\n", file_p->as_cstr());
        //  }

        SkExpressionBase::set_new_expr_debug_info(SkDebugInfo::Flag__default_source);
      #endif

      SkClassUnaryBase * scope_p = info.get_class_scope();
      SkClass *          class_p = scope_p->get_key_class();

      if (!class_p->is_loaded())
        {
        // Update once per class
        progress();
        class_p->set_loaded();
        }

      switch (info.m_type)
        {
        case SkMember_method:
          parse_file_method(file_p, info.m_member_id.get_name(), scope_p);
          break;

        case SkMember_coroutine:
          parse_file_coroutine(file_p, info.m_member_id.get_name(), scope_p);
          break;
        }

      #if (SKOOKUM & SK_DEBUG)
        SkExpressionBase::set_new_expr_debug_info(SkDebugInfo::Flag__default);
      #endif
      }
    }
  }

//---------------------------------------------------------------------------------------
// Parses file path and returns a pointer to a SkClass or SkActorClass or nullptr if
// class path is not valid.
// 
// Returns: Class or nullptr
// 
// Params:
//   file: file to parse
//   
// Modifiers: static
// Author(s): Conan Reis
SkClass * SkCompiler::parse_file_class(const AFile & file)
  {
  const ADirectory & dir = file.get_directory();
  return SkBrain::get_class(dir.is_extensioned() ? dir.get_extension(false) : dir.get_title());
  }

//---------------------------------------------------------------------------------------
// Parses file for class meta information.
//
// Author(s): Conan Reis
void SkCompiler::parse_file_class_meta(
  // file to parse
  AFile * file_p,
  // Class scope to modify
  SkClass * class_p
  )
  {
  uint32_t     length;
  const char * file_cstr_p = map_text_file(file_p, &length);

  if (file_cstr_p == nullptr)
    {
    // File is empty or invalid
    return;
    }

  SkParser::Args args;
  SkParser parser(file_cstr_p, length);

  if (!parser.parse_class_meta_source(class_p, args))
    {
    m_errors++;
    SkDebug::print_parse_error(args, file_p->get_file_str(), &parser);
    }

  progress();
  }

//---------------------------------------------------------------------------------------
// Parses file for class object ID validation information.
//
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // true if parsed successfully, false if not
  bool
SkCompiler::parse_file_object_ids(
  // file to parse
  AFile * file_p,
  // Class scope to modify
  SkClass * class_p
  )
  {
  ASymbolTable * object_ids_p = class_p->get_object_id_valid_list_merge();

  if ((object_ids_p == nullptr)
    || ((class_p->get_flags() & SkClass::Flag_object_id_parse_list) == 0u))
    {
    // Object IDs are not being validated - ignore.
    // $Revisit - CReis Could warn about presence of unused/ignored object ID files
    return true;
    }

  uint32_t     length;
  const char * file_cstr_p = map_text_file(file_p, &length);

  if (file_cstr_p == nullptr)
    {
    // File is empty or invalid
    return length == 0u;
    }

  SkParser::Args args;
  SkParser parser(file_cstr_p, length);

  parser.parse_symbol_ids_source(object_ids_p, args);

  if (!args.is_ok())
    {
    m_errors++;
    SkDebug::print_parse_error(args, file_p->get_file_str(), &parser);
    return false;
    }

  progress();

  return true;
  }

//---------------------------------------------------------------------------------------
// Parses file for a series of data members.
//
// #Notes
//   Duplicate data member code files from successive overlays have the individual data
//   members interleaved - i.e. all unique data members are added regardless of which
//   overlay it came from even if different overlays have the same data member file names.
//
// #Author(s) Conan Reis
void SkCompiler::parse_file_data_members(
  // file to parse
  AFile * file_p,
  // Class scope to modify
  SkClassUnaryBase * scope_p
  )
  {
  uint32_t     length;
  const char * file_cstr_p = map_text_file(file_p, &length);

  if (file_cstr_p == nullptr)
    {
    // File is empty or invalid
    return;
    }

  SkParser::Args args;
  SkParser       parser(file_cstr_p, length);
  uint32_t       num_data_members = 0;
  parser.parse_data_members_source(scope_p, args, true, &num_data_members);

  if (args.m_result == SkParser::Result_ok)
    {
    m_data_members += num_data_members;
    }
  else
    {
    m_errors++;
    SkDebug::print_parse_error(args.m_result, file_p->get_file_str(), &parser, args.m_end_pos);
    }

  progress();
  }

//---------------------------------------------------------------------------------------
// Parses file for a method
// Arg         file_p - file to parse for method
// Arg         name - name of method
// Arg         scope_p - scope for the method
// See:        preparse_file_method()
// Author(s):   Conan Reis
void SkCompiler::parse_file_method(
  AFile *            file_p,
  const ASymbol &    name,
  SkClassUnaryBase * scope_p
  )
  {
  // Ignore method if it's body has been fully registered by a higher sequence overlay
  if (scope_p->is_method_registered(name, false))
    {
    return;
    }

  uint32_t     length;
  const char * file_cstr_p = map_text_file(file_p, &length);

  if (file_cstr_p == nullptr)
    {
    // File is empty or invalid
    return;
    }

  SkParser::Args args;
  SkParser       parser(file_cstr_p, length);

  if (length >= UINT16_MAX)
    {
    SkDebug::print(
      a_str_format(
        "\n\nWarning: Source file is too large - limit is 65535 bytes!  It is %u bytes.\n"
        "[Split it into multiple methods.]\n"
        "File: %s\n",
        length,
        file_p->as_cstr()),
      SkLocale_all,
      SkDPrintType_warning);
    }

  parser.parse_method_source(name, scope_p, args);

  if (args.is_ok())
    {
    m_methods++;

    //progress();
    }
  else
    {
    m_errors++;
    SkDebug::print_parse_error(args.m_result, file_p->get_file_str(), &parser, args.m_end_pos, args.m_start_pos);
    }
  }

//---------------------------------------------------------------------------------------
// Parses file for a coroutine
// Arg         file_p - file to parse for coroutine
// Arg         name - name of coroutine
// Arg         scope_p - class scope for the coroutine
// Author(s):   Conan Reis
void SkCompiler::parse_file_coroutine(
  AFile *            file_p,
  const ASymbol &    name,
  SkClassUnaryBase * scope_p
  )
  {
  // Ignore coroutine if it's body has been fully registered by a higher sequence overlay
  if (scope_p->is_coroutine_registered(name))
    {
    return;
    }

  uint32_t     length;
  const char * file_cstr_p = map_text_file(file_p, &length);

  if (file_cstr_p == nullptr)
    {
    // File is empty or invalid
    return;
    }

  SkParser       parser(file_cstr_p, length, false);
  SkParser::Args args;

  if (length >= UINT16_MAX)
    {
    SkDebug::print(
      a_str_format(
        "\n\nWarning: Source file is too large - limit is 65535 bytes!  It is %u bytes.\n"
        "[Split it into multiple coroutines/methods.]\n"
        "File: %s\n",
        length,
        file_p->as_cstr()),
      SkLocale_all,
      SkDPrintType_warning);
    }

  parser.parse_coroutine_source(name, scope_p, args);

  if (args.is_ok())
    {
    m_coroutines++;

    //progress();
    }
  else
    {
    m_errors++;
    SkDebug::print_parse_error(args.m_result, file_p->get_file_str(), &parser, args.m_end_pos, args.m_start_pos);
    }
  }

//---------------------------------------------------------------------------------------
// Reparses source files
// Author(s):   Conan Reis
void SkCompiler::reparse()
  {
  reparse_warn();

  m_classes      = 0u;
  m_methods      = SkCompiler_internal_method_count;
  m_coroutines   = 0u;
  m_data_members = 0u;
  m_pre_errors   = 0u;
  m_errors       = 0u;
  m_warnings     = 0u;

  if (m_console_p)
    {
    m_console_p->on_reparse();
    }

  SkDebug::print_agog("Clearing up previous SkookumScript session...\n");
  update_msg_queue();

  // Free up previous classes/etc.
  if (is_flags(Flag_bindings_bound))
    {
    SkookumScript::deinitialize_gameplay();
    SkookumScript::deinitialize_sim();
    SkookumScript::deinitialize_program();
    }

  SkookumScript::deinitialize();

  SkDebug::print_agog("  ...done!\n\n");
  update_msg_queue();

  // Reregister/register stuff
  // Clear flags
  enable_flags(Flag_pre_load_init | Flag_bindings_bound, false);

  phase_start(Phase_preparse);
  }

//---------------------------------------------------------------------------------------
// Reparse only if compiled binary is stale (out-of-date) compared to source
//             scripts.
// Returns:    true if reparsed false if not
// Author(s):   Conan Reis
bool SkCompiler::reparse_if_stale()
  {
  if (!is_compiled_fresh())
    {
    reparse();

    return true;
    }

  return false;
  }

//---------------------------------------------------------------------------------------
// Warn about recompiling if using Remote IDE
// Modifiers:   static
// Author(s):   Conan Reis
void SkCompiler::reparse_warn()
  {
  if (SkRemoteBase::ms_default_p->is_remote_ide() && SkRemoteBase::ms_default_p->is_connected()
    && ((SkConsole::ms_console_p == nullptr) || !SkConsole::ms_console_p->get_remote_ide()->is_remote_compile_req()))
    {
    SkDebug::print(
      "\nRecompiling all the scripts or a single member script in the Remote IDE just\n"
      "checks the code for correctness - it is not transferred to the runtime!\n\n"
      "Recompiling a class (& optional subclasses) *will* be transfered to the remote\n"
      "runtime.\n",
      SkLocale_all,
      SkDPrintType_warning);
    }
  }

//---------------------------------------------------------------------------------------
// Reparse the specified class (and optionally all its subclasses).
//             This can be called while the scripts are running.  It will also add/remove
//             members though it will not add/remove classes.
// Returns:    Number of classes that were prepped
// Arg         class_p - class to reparse
// Arg         recurse - indicates whether subclasses should be reparsed as well (usually
//             a good idea).
// See:        reparse(), reparse_method(), reparse_coroutine()
// Author(s):   Conan Reis
uint32_t SkCompiler::reparse_class_prep(
  SkClass * class_p,
  AString * class_strs_p,
  uint32_t  flags // = Reparse__default
  )
  {
  uint32_t prep_count = 1u;

  // $Revisit - If the code is remote - rather than saving previous working class members
  // just don't send any code binary to the runtime.

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // If class_p in a demand loaded group ensure loaded and lock changes in memory
  SkClass * demand_root_p = class_p->get_demand_loaded_root();

  if (demand_root_p)
    {
    if (!demand_root_p->is_loaded())
      {
      SkDebug::print(
        a_str_format("\nAuto loading Skookum class group '%s' into memory before reparse.\n", demand_root_p->get_name_cstr_dbg()),
        SkLocale_all,
        SkDPrintType_warning);

      load_compiled_class_group(demand_root_p);
      }

    if (!demand_root_p->is_load_locked())
      {
      demand_root_p->lock_load();

      SkDebug::print(
        a_str_format("\nDemand loaded Skookum class group '%s' now locked in memory (changes will not auto-unload).\n", demand_root_p->get_name_cstr_dbg()),
        SkLocale_all,
        SkDPrintType_warning);
      }
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Save members for reintegration and clear the rest

  // Clear data members
  class_p->remove_instance_data_all();

  // Save class data members to seed new members with current data values
  // $Revisit - CReis Save class data members to seed new members with current data values
   
  // Clear class data members
  class_p->remove_class_data_all();
   
  // Save method members to reuse data-structures which may have C++ bindings and might be
  // referenced and then clear them to get ready for reparsing.
  SkClass::ms_reparse_info.m_methods.append_all(class_p->m_methods);
  class_p->m_methods.remove_all();
  SkClass::ms_reparse_info.m_class_methods.append_all(class_p->m_class_methods);
  class_p->m_class_methods.remove_all();
  class_p->m_destructor_p = nullptr;

  // Save coroutines to reuse data-structures which may have C++ bindings and might be referenced
  SkClass::ms_reparse_info.m_coroutines.append_all(class_p->m_coroutines);
  class_p->m_coroutines.remove_all();

  // Add class name to reparse list
  class_strs_p->append(class_p->get_name_str_dbg());

  // If built-in actor class add instance count to name if 1 or more
  if (class_p->is_builtin_actor_class())
    {
    uint32_t instance_count = static_cast<SkActorClass *>(class_p)->get_instances().get_length();

    if (instance_count)
      {
      class_strs_p->ensure_size(class_strs_p->get_length() + 8u);
      class_strs_p->append_format("(%u)", instance_count);
      }
    }

  class_strs_p->append(", ", 2u);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Recurse
  if (flags & Reparse_recurse)
    {
    uint32_t subclass_count = class_p->get_subclasses().get_length();

    if (subclass_count)
      {
      SkClass ** classes_pp      = class_p->get_subclasses().get_array();
      SkClass ** classes_end_pp  = classes_pp + subclass_count;

      for (; classes_pp < classes_end_pp; classes_pp++)
        {
        prep_count += reparse_class_prep(*classes_pp, class_strs_p, flags);
        }
      }
    }

  return prep_count;
  }

//---------------------------------------------------------------------------------------
// Reparse the specified class (and optionally all its subclasses).
// This can be called while the scripts are running.  It will also add/remove members
// though it will not add/remove classes.
// 
// Returns:  Number of errors encountered during parse.
// 
// Params:
//   class_p: class to reparse
//   recurse: indicates whether subclasses should be reparsed as well (usually a good idea).
//   
// See:       reparse(), reparse_method(), reparse_coroutine()
// Author(s): Conan Reis
uint32_t SkCompiler::reparse_class(
  SkClass * class_p,
  uint32_t  flags // = Reparse__default
  )
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Prepare IDE for reparse
  SkClass::ms_reparse_info.m_is_active = true;

  m_classes      = 0u;
  m_methods      = 0u;
  m_coroutines   = 0u;
  m_data_members = 0u;
  m_pre_errors   = 0u;
  m_errors       = 0u;
  m_warnings     = 0u;

  if (m_console_p)
    {
    m_console_p->on_reparse(class_p);
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Prepare class(es) for reparse
  AString class_list_str;

  m_classes = reparse_class_prep(class_p, &class_list_str, flags);

  // Remove last ", "
  class_list_str.remove_end(2u);

  SkDebug::print_agog(a_str_format("\n\nReparsing:\n  %s\n\n", class_list_str.as_cstr()));



  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Preparse script files
  bool recurse = (flags & Reparse_recurse) != 0u;

  preparse_files_overlays(*class_p, recurse);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Parse Script Files
  parse_files_overlays(*class_p, recurse);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Clean-up
  
  // Get rid of any methods that weren't reused
  SkClass::ms_reparse_info.m_methods.free_all();
  SkClass::ms_reparse_info.m_class_methods.free_all();
  SkClass::ms_reparse_info.m_coroutines.free_all();


  // Clean-up IDE, note any errors, etc.
  SkClass::ms_reparse_info.m_is_active = false;

  if (m_console_p)
    {
    m_console_p->on_compile_complete();
    }

  parse_complete();

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Optionally call class constructor(s) if no errors and not a remote IDE (called later on runtime side)
  if ((m_errors == 0u) && (flags & Reparse_class_ctor) && SkRemoteBase::ms_default_p->should_class_ctors_be_called())
    {
    SkDebug::print_agog(a_str_format("\n\nCalling class constructor%s for:\n  %s\n", (m_classes > 1u) ? "s" : "", class_list_str.as_cstr()));
    update_msg_queue();

    if (recurse)
      {
      // Run class ctor recursively
      class_p->invoke_class_ctor_recurse();
      }
    else
      {
      class_p->invoke_class_ctor();
      }

    SkDebug::print_agog("  ...done!\n\n");
    update_msg_queue();
    }


  // $Revisit - CReis Should evaluation be temporarily disabled if there were errors?

  return m_errors;
  }

//---------------------------------------------------------------------------------------
// Reparses file for a method
// Arg         file_p - file to parse for method
// Arg         name - name of method
// Arg         scope_p - scope for the method
// Author(s):   Conan Reis
SkParser::eResult SkCompiler::reparse_file_method(
  AFile *            file_p,
  const ASymbol &    name,
  SkClassUnaryBase * scope_p
  )
  {
  reparse_warn();

  SkMethodBase * existing_method_p = scope_p->find_method(name);

  SkDebug::print_agog(
    a_str_format(
      "\nFile: %s\n"
      "  Recompiling... ",
      file_p->as_cstr()));

  if (existing_method_p == nullptr)
    {
    m_errors++;
    SkDebug::print_error(
      "Cannot recompile!\n"
      "  Can only recompile existing methods.");

    return SkParser::Result_err_unimplemented;
    }

  uint32_t     length;
  const char * file_cstr_p = map_text_file(file_p, &length);

  if (file_cstr_p == nullptr)
    {
    // File is empty or invalid
    return length ? SkParser::Result_err_unexpected_char : SkParser::Result_err_unexpected_eof;
    }

  #if (SKOOKUM & SK_DEBUG)
    SkExpressionBase::set_new_expr_debug_info(SkDebugInfo::Flag__default_source);
  #endif

  SkParser::Args args;
  SkParser       parser(file_cstr_p, length);
  SkMethodBase * new_method_p = parser.parse_method_source(name, scope_p, args, false);

  #if (SKOOKUM & SK_DEBUG)
    SkExpressionBase::set_new_expr_debug_info(SkDebugInfo::Flag__default);
  #endif

  if (args.is_ok())
    {
    eSkInvokable type = new_method_p->get_invoke_type();

    if (type != existing_method_p->get_invoke_type())
      {
      // Replace with new
      scope_p->append_method(new_method_p);
      }
    else
      {
      // Replace in-place
      if (type == SkMember_method)
        {
        static_cast<SkMethod *>(existing_method_p)->assign_take(static_cast<SkMethod *>(new_method_p));
        }
      else
        {
        existing_method_p->assign(*new_method_p);
        }

      delete new_method_p;
      }

    // Lock changes in memory
    SkClass * demand_root_p = scope_p->get_key_class()->get_demand_loaded_root();

    if (demand_root_p && !demand_root_p->is_load_locked())
      {
      demand_root_p->lock_load();
      SkDebug::print(a_str_format("\n  ...Demand loaded group '%s' now locked in memory (will not auto-unload).\n", demand_root_p->get_name_cstr()));
      }

    SkDebug::print_agog("\n  ...done! Method now recombobulated.\n");
    }
  else
    {
    SkDebug::print_parse_error(args, file_p->get_file_str(), &parser);
    SkDebug::print_agog("\n[The original method remains unchanged.]\n", SkLocale_all, SkDPrintType_warning);
    }

  return args.m_result;
  }

//---------------------------------------------------------------------------------------
// Reparses file for a coroutine
// Arg         file_p - file to parse for coroutine
// Arg         name - name of coroutine
// Arg         scope_p - scope for the coroutine
// Author(s):   Conan Reis
SkParser::eResult SkCompiler::reparse_file_coroutine(
  AFile *         file_p,
  const ASymbol & name,
  SkClass *       scope_p
  )
  {
  reparse_warn();

  SkCoroutineBase * existing_coroutine_p = scope_p->find_coroutine(name);

  SkDebug::print_agog(
    a_str_format(
      "\nFile: %s\n"
      "  Recompiling... ",
      file_p->as_cstr()));

  if (existing_coroutine_p == nullptr)
    {
    SkDebug::print_error(
      "Cannot recompile!\n"
      "  Can only recompile existing coroutines.");

    return SkParser::Result_err_unimplemented;
    }

  uint32_t     length;
  const char * file_cstr_p = map_text_file(file_p, &length);

  if (file_cstr_p == nullptr)
    {
    // File is empty or invalid
    return length ? SkParser::Result_err_unexpected_char : SkParser::Result_err_unexpected_eof;
    }

  #if (SKOOKUM & SK_DEBUG)
    SkExpressionBase::set_new_expr_debug_info(SkDebugInfo::Flag__default_source);
  #endif

  SkParser::Args   args;
  SkParser         parser(file_cstr_p, length);
  SkCoroutineBase * new_coroutine_p = parser.parse_coroutine_source(name, scope_p, args, false);

  #if (SKOOKUM & SK_DEBUG)
    SkExpressionBase::set_new_expr_debug_info(SkDebugInfo::Flag__default);
  #endif

  if (args.is_ok())
    {
    eSkInvokable type = new_coroutine_p->get_invoke_type();

    if (type != existing_coroutine_p->get_invoke_type())
      {
      // Replace with new
      scope_p->append_coroutine(new_coroutine_p);
      }
    else
      {
      // Replace in-place
      if (type == SkMember_coroutine)
        {
        static_cast<SkCoroutine *>(existing_coroutine_p)->assign_take(static_cast<SkCoroutine *>(new_coroutine_p));
        }
      else
        {
        existing_coroutine_p->assign(*new_coroutine_p);
        }

      delete new_coroutine_p;
      }

    // Lock changes in memory
    SkClass * demand_root_p = scope_p->get_key_class()->get_demand_loaded_root();

    if (demand_root_p && !demand_root_p->is_load_locked())
      {
      demand_root_p->lock_load();
      SkDebug::print(a_str_format("\n  ...Demand loaded group '%s' now locked in memory (will not auto-unload).\n", demand_root_p->get_name_cstr()));
      }

    SkDebug::print_agog("\n  ...done! Coroutine now recombobulated.\n");
    }
  else
    {
    SkDebug::print_parse_error(args, file_p->get_file_str(), &parser);
    SkDebug::print_agog("\n[The original coroutine remains unchanged.]\n", SkLocale_all, SkDPrintType_warning);
    }

  return args.m_result;
  }

//---------------------------------------------------------------------------------------
// (Re)load any object ID validation list for the specified class.
// Use SkBrain::ms_object_class_p->object_id_validate_recurse() to (re)validate all object
// id expressions that may use the object IDs.
//
// See Also  SkCompiler: :load_object_ids_recurse(), SkClass::object_id_validate_recurse()
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // # of errors encountered during reparse  
  uint32_t
SkCompiler::load_object_ids(
  SkClass * class_p,
  eAVerbosity print_level // = AVerbosity_full
  )
  {
  uint32_t validation_flags = class_p->get_object_id_validate();

  if ((validation_flags & SkClass::Flag_object_id_parse_list) == 0u)
    {
    // No list to reparse - exit
    return 0u;
    }

  // Reset validation ids for the class
  class_p->clear_object_id_valid_list();

  struct NestedReparseObjIdsFile
    {
    SkClass * m_class_p;
    uint32_t m_error_count;
    eAVerbosity m_print_level;

    NestedReparseObjIdsFile(SkClass * class_p, eAVerbosity print_level)
      : m_class_p(class_p), m_error_count(0u), m_print_level(print_level) {}

    void reparse_file_object_ids(AFile * file_p)
      {
      switch (parse_file_member_type(*file_p))
        {
        case SkMember_object_ids_defer:
        case SkMember_object_ids:
          if (m_print_level == AVerbosity_full)
            {
            SkDebug::print(a_str_format("\n  %s", file_p->as_cstr()));
            }

          if (!SkCompiler::ms_compiler_p->parse_file_object_ids(file_p, m_class_p))
            {
            m_error_count++;
            }
          break;

        // Skip other types of files
        }
      }
    };

  NestedReparseObjIdsFile reparse_obj(class_p, print_level);

  AMethodArg<NestedReparseObjIdsFile, AFile *> parse_file_func(
    &reparse_obj, &NestedReparseObjIdsFile::reparse_file_object_ids);

  apply_overlay_files(&parse_file_func, *class_p, false, false);

  return reparse_obj.m_error_count;
  }

//---------------------------------------------------------------------------------------
// (Re)load any object ID validation list for the specified class and its subclasses -
// depending on the hierarchy argument.
// 
// Use SkBrain::ms_object_class_p->object_id_validate_recurse() to (re)validate all object
// id expressions that may use the object IDs.
//
// See Also  SkCompiler: :load_object_ids(), SkClass::object_id_validate_recurse()
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // # of errors encountered during reparse  
  uint32_t
SkCompiler::load_object_ids_recurse(
  SkClass * class_p,
  eAHierarchy hierarchy, // = AHierarchy__all
  eAVerbosity print_level // = AVerbosity_full
  )
  {
  struct NestedReparseObjIdsClass
    {
    uint32_t    m_error_count;
    eAVerbosity m_print_level;


    void load_object_ids(SkClass * class_p)
      {
      m_error_count += SkCompiler::ms_compiler_p->load_object_ids(class_p, m_print_level);
      }
    };

  NestedReparseObjIdsClass reparse_obj;

  reparse_obj.m_error_count = 0u;
  reparse_obj.m_print_level = print_level;

  AMethodArg<NestedReparseObjIdsClass, SkClass *> class_func(
    &reparse_obj, &NestedReparseObjIdsClass::load_object_ids);


  if (print_level >= AVerbosity_brief)
    {
    if ((class_p == SkBrain::ms_object_class_p) && (hierarchy & AHierarchy_recurse))
      {
      SkDebug::print_agog("\nLoading all object ID validation lists...");
      }
    else
      {
      SkDebug::print_agog(a_str_format(
        "\n\nLoading object ID validation lists for '%s'%s...",
        class_p->get_name_cstr_dbg(),
        (hierarchy & AHierarchy_recurse) ? " (and its subclasses)" : ""));
      }
    }

  class_p->iterate_recurse(&class_func, hierarchy);

  if (print_level >= AVerbosity_brief)
    {
    SkDebug::print_agog("\n  ...done!\n\n");
    }

  if (reparse_obj.m_error_count)
    {
    if (print_level >= AVerbosity_critical)
      {
      SkDebug::print_agog(
        a_str_format("  ######## %u %s ########\n\n", reparse_obj.m_error_count, (reparse_obj.m_error_count == 1u) ? "error" : "errors"),
        SkLocale_all,
        SkDPrintType_error);
      }
    }

  return reparse_obj.m_error_count;
  }

//---------------------------------------------------------------------------------------
// (Re)load any object ID validation list for the specified class.
// Use SkBrain::ms_object_class_p->object_id_validate_recurse() to (re)validate all object
// id expressions that may use the object IDs.
//
// See Also  SkCompiler: :load_object_ids_recurse(), SkClass::object_id_validate_recurse()
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // # of errors encountered during reparse  
  uint32_t
SkCompiler::load_object_ids_generated(
  eAVerbosity print_level // = AVerbosity_full
  )
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Nested structures & code
  struct NestedGeneratedObjIdsFile
    {
    uint32_t m_error_count;
    eAVerbosity m_print_level;

    NestedGeneratedObjIdsFile(eAVerbosity print_level)
      : m_error_count(0u), m_print_level(print_level) {}

    void reparse_file_object_ids(AFile * file_p)
      {
      SkQualifier class_info;

      switch (parse_file_member_type(*file_p, &class_info))
        {
        case SkMember_object_ids_defer:
        case SkMember_object_ids:
          if (m_print_level == AVerbosity_full)
            {
            SkDebug::print(a_str_format("\n  %s", file_p->as_cstr()));
            }

          if (!SkCompiler::ms_compiler_p->parse_file_object_ids(file_p, class_info.get_scope()))
            {
            m_error_count++;
            }
          break;

        // Skip other types of files
        }
      }
    };

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure generated object ID directory exists
  ADirectory bin_dir(get_ini_compiled_file().get_path() + "ObjectId\\");

  if (!bin_dir.is_existing())
    {
    return 0u;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Iterate through generated object ID files
  NestedGeneratedObjIdsFile reparse_obj(print_level);

  AMethodArg<NestedGeneratedObjIdsFile, AFile *> parse_file_func(
    &reparse_obj, &NestedGeneratedObjIdsFile::reparse_file_object_ids);

  if (print_level >= AVerbosity_brief)
    {
    SkDebug::print_agog("Loading all generated object ID validation lists...");
    }

  bin_dir.apply_files(&parse_file_func);

  if (print_level >= AVerbosity_brief)
    {
    SkDebug::print_agog("\n  ...done!\n\n");
    }

  if (reparse_obj.m_error_count)
    {
    if (print_level >= AVerbosity_critical)
      {
      SkDebug::print_agog(
        a_str_format("  ######## %u %s ########\n\n", reparse_obj.m_error_count, (reparse_obj.m_error_count == 1u) ? "error" : "errors"),
        SkLocale_all,
        SkDPrintType_error);
      }
    }

  return reparse_obj.m_error_count;
  }

//---------------------------------------------------------------------------------------
// (Re)load any object ID validation list for the specified class and its subclasses -
// depending on the hierarchy argument.  Then (re)validate all object ID expressions in
// the complete class hierarchy.
//
// See Also  SkCompiler: :load_object_ids_recurse(), SkClass::object_id_validate_recurse()
// #Author(s) Conan Reis
void SkCompiler::load_and_validate_object_ids(
  SkClass * class_p,
  eAHierarchy hierarchy, // = AHierarchy__all
  eAVerbosity print_level // = AVerbosity_full
  )
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Load object ID validation lists
  if (class_p == nullptr)
    {
    class_p = SkBrain::ms_object_class_p;
    }

  uint32_t error_count = load_object_ids_recurse(class_p, hierarchy, print_level);

  if (error_count)
    {
    m_errors += error_count;
    return;
    }

  error_count = load_object_ids_generated(print_level);

  if (error_count)
    {
    m_errors += error_count;
    return;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Validate object ID expressions
  if (print_level >= AVerbosity_brief)
    {
    SkDebug::print_agog("Validating all object ID expressions...\n");
    }

  uint32_t expr_count = 0u;

  error_count = SkBrain::ms_object_class_p->object_id_validate_recurse(
    (m_flags & Flag_object_ids_validate) != 0u, &expr_count);

  if (print_level >= AVerbosity_brief)
    {
    if (print_level == AVerbosity_full)
      {
      SkDebug::print_agog(a_str_format(
        "  %u object ID expressions validated.\n",
        expr_count));
      }

    SkDebug::print_agog("  ...done!\n\n");
    }

  if (error_count)
    {
    m_errors += error_count;
    if (print_level >= AVerbosity_critical)
      {
      SkDebug::print_agog(
        a_str_format("  ######## %u %s ########\n\n", error_count, (error_count == 1u) ? "error" : "errors"),
        SkLocale_all,
        SkDPrintType_error);
      }
    }
  }

//---------------------------------------------------------------------------------------

SkClass * SkCompiler::get_bind_name_class() const
  {
  if (m_bind_name_class_p == nullptr)
    {
    // $CReis - CReis The "Name" default should be read from main ini file rather than hardcoded.
    ASymbol name_class_sym(ASymbol::create(
      m_ini_proj.get_value_default("Name", "ObjectIDNameClass", g_ini_section_project_p), ATerm_short));

    m_bind_name_class_p = SkBrain::get_class(name_class_sym);
    }

  return m_bind_name_class_p;
  }

//---------------------------------------------------------------------------------------
// Counts classes and their members
// Author(s):   Conan Reis
void SkCompiler::calc_member_count()
  {
  m_methods      = 0u;
  m_coroutines   = 0u;
  m_data_members = 0u;

  m_classes = SkBrain::get_classes().get_length();

  SkClass *  class_p;
  SkClass ** classes_pp     = SkBrain::get_classes().get_array();
  SkClass ** classes_end_pp = classes_pp + m_classes;

  for (; classes_pp < classes_end_pp; classes_pp++)
    {
    class_p = *classes_pp;
    m_methods      += class_p->get_instance_methods().get_length() + class_p->get_class_methods().get_length();
    m_coroutines   += class_p->get_coroutines().get_length();
    m_data_members += class_p->get_instance_data().get_length() + class_p->get_instance_data_raw().get_length() + class_p->get_class_data().get_length();
    }
  }

//---------------------------------------------------------------------------------------
// Sets the load type used by the compiler
// Arg         load_type - see eSkLoad
// Examples:   Usually called by the settings dialog from the Skookum console.
// Author(s):   Conan Reis
void SkCompiler::set_load_type(eSkLoad load_type)
  {
  const char * load_cstr_p = "newest";  // Load whichever is newest - binary or scripts

  switch (load_type)
    {
    case SkLoad_script:  // Load script text files
      load_cstr_p = "scripts";
      break;

    case SkLoad_binary:  // Load compiled code binary
      load_cstr_p = "binary";
      break;
    }

  get_ini_ide().set_value(
    load_cstr_p,
    g_ini_key_code_load_p,
    g_ini_section_compiler_p);
  }

//---------------------------------------------------------------------------------------
// Loads the console settings
// See:        save_settings()
// Author(s):   Conan Reis
void SkCompiler::load_settings()
  {
  // Reset flags
  m_flags = m_flags & (Flag__default | Flag_compile_only | Flag_object_ids_validate);

  SkParser::enable_strict(get_ini_strict());

  enable_flags(
    Flag_save_manifest,
    m_ini_proj.get_value_bool_default(
      true, g_ini_key_compiled_manifest_p, g_ini_section_output_p));

  AIni & ide_ini = get_ini_ide();
    

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Code scripts visual indent/tab settings
  

  // Get / store indent size
  SkDebug::ms_indent_size = uint32_t(ide_ini.get_value_int_default(
    AString_indent_spaces_def,
    "IndentSize",
    g_ini_section_compiler_p));

  // Get / store tab stop size
  SkDebug::ms_tab_stops = uint32_t(ide_ini.get_value_int_default(
    AString_tab_stop_def,
    "TabStopSize",
    g_ini_section_compiler_p));


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (is_flags(Flag_compile_only))
    {
    // Ignore some ini settings if only updating code binary
    // Set flags
    enable_flags(Flag_save_compiled);

    // Clear flags
    enable_flags(Flag_ensure_bindings | Flag_evaluate_scripts, false);
    }
  else
    {
    enable_flags(
      Flag_show_preparse_errors,
      ide_ini.get_value_bool_default(
        false, "ShowPreparseErrors", g_ini_section_compiler_p));
      
    enable_flags(
      Flag_save_compiled,
      ide_ini.get_value_bool_default(
        SkCompiler_save_compiled_def, g_ini_key_compiled_save_p, g_ini_section_compiler_p));

    enable_flags(
      Flag_ensure_bindings,
      ide_ini.get_value_bool_default(
        SkCompiler_ensure_atomics_def, g_ini_key_ensure_atomics_p, g_ini_section_compiler_p));

    enable_flags(
      Flag_evaluate_scripts,
      ide_ini.get_value_bool_default(
        SkCompiler_evaluate_def, g_ini_key_evaluate_p, g_ini_section_compiler_p));
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Initialize actor parsing settings

  m_use_builtin_actor       = m_ini_proj.get_value_bool_default(true, "UseBuiltinActor", g_ini_section_project_p);
  m_custom_actor_class_name = ASymbol::create(m_ini_proj.get_value_default("", "CustomActorClass", g_ini_section_project_p), ATerm_short);
  m_bind_name_class_p  = nullptr;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Execute any pre-compilation command-line arguments
  if (is_flags(Flag_compile_only))
    {
    AString command_args(AApplication::ms_this_app_p->get_command_line_args());

    cmd_args_execute(command_args);
    // Args not cleared since the compiler just exits when tasks complete
    }
  

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  load_overlays();


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Determine start phase - load compiled code on start-up?
  m_phase_start = Phase_determine_newest;  // Load whichever is newest - binary or scripts

  if (!is_flags(Flag_compile_only))
    {
    switch (get_ini_code_load())
      {
      case SkLoad_script:  // Load script text files
        m_phase_start = Phase_preparse;
        break;

      case SkLoad_binary:  // Load compiled code binary
        m_phase_start = Phase_load_compiled;
        break;
      }
    }

  // Note, all settings are currently saved whenever they are modified
  }

//---------------------------------------------------------------------------------------
// Enables or disables the checking of the atomic C++ bindings - i.e. ensure
//             that all atomic invokables that were declared have a C++ call bound to
//             them whenever the class hierarchy is loaded/parsed.
// Author(s):   Conan Reis
void SkCompiler::enable_ensure_atomics(
  bool enable_b // = true
  )
  {
  if (is_flags(Flag_ensure_bindings) != enable_b)
    {
    enable_flags(Flag_ensure_bindings, enable_b);

    SkDebug::print(enable_b
      ? "\nWhenever the class hierarchy is loaded/parsed all C++ SkookumScript invokables that\n"
        "were declared will be checked to ensure they have a C++ call bound to them.\n"
      : "\nWhenever the class hierarchy is loaded/parsed all C++ SkookumScript invokables that\n"
        "were declared will *not* be checked to ensure they have a C++ call bound to them.\n");
    }

  // Always save latest setting to ini file
  get_ini_ide().set_value_bool(
    enable_b, g_ini_key_ensure_atomics_p, g_ini_section_compiler_p);
  }

//---------------------------------------------------------------------------------------
// Enables or disables the saving of compiled binary after a parse
// Arg         enable_b - if true compiled binary is saved after parse, if false it is not
// Author(s):   Conan Reis
void SkCompiler::enable_compiled_save(
  bool enable_b // = true
  )
  {
  if (is_flags(Flag_save_compiled) != enable_b)
    {
    enable_flags(Flag_save_compiled, enable_b);
    SkDebug::print(enable_b
      ? "\nA Compiled Binary will be saved after any reparse.\n"
      : "\nA Compiled Binary will *not* be saved after any reparse.\n");
    }

  // Always save latest setting to ini file
  get_ini_ide().set_value_bool(
    enable_b,
    g_ini_key_compiled_save_p,
    g_ini_section_compiler_p);
  }

//---------------------------------------------------------------------------------------
// Enables or disables the evaluation of scripts
// Arg         enable_b -
// Author(s):   Conan Reis
void SkCompiler::enable_evaluation(
  bool enable_b // = true
  )
  {
  if (is_flags(Flag_evaluate_scripts) != enable_b)
    {
    enable_flags(Flag_evaluate_scripts, enable_b);

    // Save latest setting to ini file
    get_ini_ide().set_value_bool(enable_b, g_ini_key_evaluate_p, g_ini_section_compiler_p);

    if (enable_b == SkookumScript::is_flag_set(SkookumScript::Flag_paused))
      {
      if (enable_b)
        {
        if (is_flags(Flag_bindings_bound))
          {
          SkookumScript::enable_flag(SkookumScript::Flag_paused, false);
          }
        else
          {
          if (m_phase == Phase_idle)
            {
            // Atomics not bound yet
            SkDebug::print_agog("\nScript evaluation: Starting...\n");

            phase_start(Phase_bind_atomics);
            }
          }
        }
      else
        {
        SkookumScript::enable_flag(SkookumScript::Flag_paused, true);

        SkDebug::print_agog("\nScript evaluation: disabled.\n");
        }
      }
    }
  }

//---------------------------------------------------------------------------------------
// Execute contents of specified file as a script running on the master mind object.
// Arg         file_p - script file address
// Arg         locale - indicates where to run script SkLocale_runtime or SkLocale_ide
// Author(s):   Conan Reis
bool SkCompiler::execute_file(
  AFile *   file_p,
  eSkLocale locale // = SkLocale_runtime
  )
  {
  SkDebug::print_agog(a_str_format("\nExecuting file (on %s):\n  %s ...\n", (locale == SkLocale_runtime) ? "runtime" : "IDE", file_p->as_cstr()), SkLocale_ide);

  uint         length;
  bool         success     = false;
  const char * file_cstr_p = map_text_file(file_p, &length);

  if (file_cstr_p == nullptr)
    {
    // File is empty or invalid
    return false;
    }

  AString       code(file_cstr_p, length);
  SkRemoteIDE * remote_p     = SkConsole::ms_console_p->get_remote_ide();
  eSkLocale     actual_local = SkRemoteBase::locale_as_actual(locale);

  if ((actual_local == SkLocale_runtime) && remote_p->is_remote_ide())
    {
    remote_p->cmd_invoke(code);

    // $Revisit - CReis Should get feedback from remote side that indicates whether it
    // executed successfully or not.
    success = true;
    }
  else
    {
    success = (SkParser::invoke_script(code) >= SkParser::Result_ok_deferred);
    }

  return success;
  }

//---------------------------------------------------------------------------------------
// Called during initialization every idle loop
// Author(s):   Conan Reis
void SkCompiler::on_idle()
  {
  switch (m_phase)
    {
    case Phase_determine_newest:
      determine_compiled_fresh();
      break;

    case Phase_preparse:
      preparse_files();
      break;

    case Phase_parse:
      parse_files();
      break;

    case Phase_save_compiled:
      compiled_save();
      break;

    case Phase_load_compiled:
      compiled_load();
      break;

    case Phase_bind_atomics:
      bind_atomics();
      break;

    default:  // Phase_idle
      if (m_console_p)
        {
        m_console_p->status_update();
        }
    }
  }

//---------------------------------------------------------------------------------------
// Apply supplied function to files in enabled overlays for specified class (and subclasses
// if recurse is true).
void SkCompiler::apply_overlay_files(
  AFunctionArgBase<AFile *> * file_func_p,
  const SkClass & cls,
  bool recurse, // = true
  bool print_info // = true
  )
  {
  char         buffer_p[300];
  AString      str(buffer_p, 300u, 0u);
  ADirectory   dir;
  uint32_t     length      = m_overlays.get_length();
  uint32_t     overlay_idx = length;
  SkOverlay ** overlays_pp = m_overlays.get_array();

  update_msg_queue();

  do
    {
    overlay_idx--;
    m_current_overlay_p = overlays_pp[overlay_idx];

    // Skip unapplied overlays
    if (!m_current_overlay_p->m_apply_b)
      {
      continue;
      }

    if (print_info)
      {
      str.format("\n  '%s': %s", m_current_overlay_p->m_name.as_cstr(), m_current_overlay_p->m_path.as_cstr());

      if (!recurse)
        {
        str.append(m_current_overlay_p->get_path_class(cls));
        }

      SkDebug::print_agog(str);
      }

    // Recurse through classes
    apply_overlay_files_recurse(file_func_p, cls, recurse);

    if (print_info)
      {
      progress(true);
      }
    }
  while (overlay_idx > 0u);
  }

//---------------------------------------------------------------------------------------
// Apply supplied function to files in given overlay for specified class 
// (and subclasses if recurse is true)
void SkCompiler::apply_overlay_files_recurse(AFunctionArgBase<AFile *> * file_func_p, const SkClass & cls, bool recurse)
  {
  ADirectory dir(m_current_overlay_p->get_path_class(cls));

  if (dir.is_existing()) // Might not exist in this particular overlay
    {
    dir.apply_files(file_func_p, false); // Do not recurse here as hierarchy might be flattened - instead, recurse below
    }

  // Recurse into class hierarchy here
  if (recurse)
    {
    for (auto sub_class_p : cls.get_subclasses())
      {
      apply_overlay_files_recurse(file_func_p, *sub_class_p, true);
      }
    }
  }

//---------------------------------------------------------------------------------------
// Determines if compiled binary is fresh/up-to-date(true) or stale/out-of-date(false)
//             - i.e. which is newer the compiled code binary or script text and optionally
//             sets the next phase accordingly.
// Returns:    true if up to date and false if not
// Arg         auto_progress - if true it will go to the next phase (compile, load or quit)
//             after determining if things are up-to-date.  If false it justs determines
//             if things are up-to-date.
// Notes:      $Note - CReis Currently if a script file that is older than the current
//             binary file is copied over a script file with the same name but different
//             code text then the change is not detected.
//             Could also have a checksum stored based on script file sizes if old files
//             with the same name are copied to script source directories.  The only thing
//             absolutely correct would be a checksum made from the text of all script
//             files, but that would be fairly slow.
// Author(s):   Conan Reis
bool SkCompiler::determine_compiled_fresh(
  bool auto_progress // = true
  )
  {
  if (auto_progress)
    {
    SkDebug::print_agog("\nChecking SkookumScript " A_BITS_STR " code dependencies for compiled binary...\n");
    update_msg_queue();
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure compiled binary file exists
  AFile bin_file(get_ini_compiled_file());

  if (!bin_file.is_titled() || !bin_file.is_existing())
    {
    if (auto_progress)
      {
      SkDebug::print_agog("  Compiled binary symbol file does not exist!\n  Switching to source file parsing...\n\n");
      phase_start(Phase_preparse);
      }

    return false;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Load compiled code binary id and checksums
  SkBrain::BinHeader bin_header;

  bin_file.read(&bin_header, sizeof(SkBrain::BinHeader));


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Determine if binary id is valid
  eAEquate version = SkBrain::is_binary_id_valid(bin_header.m_id);

  if (version != AEquate_equal)
    {
    if (auto_progress)
      {
      SkDebug::print_agog(a_str_format(
        "  Compiled binary file is %s!\n  Switching to source file parsing...\n\n",
        (version == AEquate_less)
          ? "stale, compiler/runtime is newer"
          : "newer, compiler/runtime is older"));

      phase_start(Phase_preparse);
      }

    return false;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Determine if checksums are valid - if they are zero then there was a problem when
  // the binary was written and it was not completed.
  if ((bin_header.m_checksum_folders == 0u) || (bin_header.m_checksum_files == 0u))
    {
    if (auto_progress)
      {
      SkDebug::print_agog("  Compiled binary file is incomplete or corrupted!\n  Switching to source file parsing...\n\n");
      phase_start(Phase_preparse);
      }

    return false;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Get the bin file's modification time.  [Assumes the symbol file is the same.]
  SkScriptEntry script_entry;

  bin_file.get_time(&script_entry.m_bin_mod_time);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Create class hierarchy & member checksum and determine if any member file is newer
  // than the compiled code binary file.
  // [Iterate through overlays in reverse order]

  char         buffer_p[300];
  AString      str(buffer_p, 300u, 0u);
  ADirectory   dir;
  uint32_t     length      = m_overlays.get_length();
  uint32_t     overlay_idx = length;
  SkOverlay ** overlays_pp = m_overlays.get_array();

  AMethodArg<SkCompiler, ADirEntry *> determine_newest_func(this, &SkCompiler::parse_entry_newest);

  do
    {
    overlay_idx--;
    m_current_overlay_p = overlays_pp[overlay_idx];

    // Skip unapplied overlays
    if (m_current_overlay_p->m_apply_b)
      {
      str.empty();
      str.append(m_current_overlay_p->m_path_qual);
      str.append(g_overlay_object_root);
      dir.set_path(str);
      dir.apply_subentries(&determine_newest_func, &script_entry);
      }
    }
  while ((overlay_idx > 0u) && !script_entry.m_member_newer_b);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Was a member file found that was newer than the compiled code binary?
  bool bin_stale_b = script_entry.m_member_newer_b;


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // If the binary seems up-to-date according to the file timestamps, now compare
  // checksums which take into account addition and removal of script files and folders.
  if (!bin_stale_b)
    {
    bin_stale_b = (script_entry.m_checksum_folders != bin_header.m_checksum_folders)
      || (script_entry.m_checksum_files != bin_header.m_checksum_files);
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Notify Remote IDE if needed
  if (SkConsole::ms_console_p)
    {
    SkConsole::ms_console_p->get_remote_ide()->cmd_compiled_state_reply(
      bin_stale_b ? SkRemoteBase::CompiledState_stale : SkRemoteBase::CompiledState_fresh);
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Return early if not auto-progressing
  if (!auto_progress)
    {
    return !bin_stale_b;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // If stale - recompile
  if (bin_stale_b)
    {
    SkDebug::print_agog("  Compiled binary is stale - script code is newer.\n  Switching to source file parsing...\n\n");

    phase_start(Phase_preparse);

    return false;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // $Revisit - CReis Could say nothing if only compiling (Flag_compile_only) and it is up-to-date
  SkDebug::print_agog("  ...Skookum compiled binary is up-to-date.\n\n");

  // "Primary" Binary is up-to-date, but the copies may be missing or different.
  // Make copies of binaries (for different projects/platforms etc.) if desired and the
  // copies are not already up-to-date.
  compiled_copy();

  if (!is_flags(Flag_compile_only) || is_flags(Flag_object_ids_validate))
    {
    phase_start(Phase_load_compiled);
    }
  else
    {
    phases_completed();
    }

  return true;
  }

//---------------------------------------------------------------------------------------
// Called for each folder or file in a script overlay - accumulates checksums
//             and determines if any script files are newer than the current binary file.
// Arg         entry_p - folder or file info
// Examples:   called by determine_compiled_fresh()
// Author(s):   Conan Reis
void SkCompiler::parse_entry_newest(ADirEntry * entry_p)
  {
  SkScriptEntry * script_entry_p = static_cast<SkScriptEntry *>(entry_p);

  if (entry_p->m_data_p->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // It is a folder - check for valid class name
    uint32_t end_pos;
    uint32_t start_pos = 0u;
    SkParser name(script_entry_p->m_data_p->cFileName);

    if ((m_current_overlay_p->m_path_depth != SkOverlay::Path_depth_any) && name.find('.', 1u, &start_pos))
      {
      start_pos++;
      }

    // Ensures that directories that do not have a valid class name are skipped
    if ((name.parse_name_class(start_pos, &end_pos, nullptr, SkParser::ClassCheck_no_validate) == SkParser::Result_ok)
      && (end_pos == name.get_length()))
      {
      // Accrue folder checksum
      script_entry_p->m_checksum_folders =
        AChecksum::generate_crc32_cstr(name.as_cstr() + start_pos, end_pos - start_pos, script_entry_p->m_checksum_folders);
      script_entry_p->m_recurse_b = true;
      }
    }
  else
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // It is a file - check for valid member file name
    AFile file(*script_entry_p->m_entry_str_p);

    switch (parse_file_member_type(file))
      {
      case SkMember_object_ids_defer:
        // Ignored - not a compiler dependency

      case SkMember__invalid:
      case SkMember__error:
        // Ignore
        break;

      default:  // Valid Members
        {
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Determine if the modification time of this member file is newer than the binary file
        ATime file_time(script_entry_p->m_data_p->ftLastWriteTime);
          
        if (file_time > script_entry_p->m_bin_mod_time)
          {
          // Member file is newer than the compiled binary file - no need to continue file scan
          script_entry_p->m_member_newer_b = true;
          script_entry_p->m_continue_b     = false;

          return;
          }

        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Accrue file checksum
        script_entry_p->m_checksum_files += AChecksum::generate_crc32(file.get_title());
        }
      }
    }
  }

//---------------------------------------------------------------------------------------
// Preparse script files using overlay info so that there is enough context info for the
// full parse later.
// 
// Arg         cls - class to look for script files
// Arg         recurse - indicates whether the class directories should be recursed or not
// Author(s):   Conan Reis
void SkCompiler::preparse_files_overlays(
  const SkClass & cls,
  bool            recurse // = true
  )
  {
  // Get names and parameters (without defaults and code bodies)
  // Iterate through overlays in reverse order - if an item already exists from a
  // previous lower sequence overlay then skip it.

  SkDebug::print_agog("Pre-parsing code & data members from overlays:");

  AMethodArg<SkCompiler, AFile *> file_func(this, &SkCompiler::preparse_file);

  uint32_t flags_checking = SkParser::Flag_type_check;

  if (is_flags(Flag_object_ids_validate))
    {
    flags_checking |= SkParser::Flag_obj_id_validate;
    }

  // Turn on preparse flag and turn off type-checking flag
  SkParser::get_default_flags().set_and_clear(
    SkParser::Flag_preparse,
    flags_checking);

  apply_overlay_files(&file_func, cls, recurse);

  // Now that pre-parsing is done, turn on type-checking flag and turn off preparse flag.
  SkParser::get_default_flags().set_and_clear(
    flags_checking,
    SkParser::Flag_preparse);

  SkDebug::print_agog("\n  ...done!\n\n");
  }

//---------------------------------------------------------------------------------------
// [Parse Pass #1] Preparses the class hierarchy via the class directories so that there
// is enough context info for the full parse later.
void SkCompiler::preparse_files()
  {
  if (!is_flags(Flag_pre_load_init))
    {
    // Starts up SkookumScript - creates/registers atomic classes, coroutines, etc.
    // that do not require code or compiled binary code to be loaded.  It also sets the
    // shared symbol table.
    SkookumScript::initialize();
    enable_flags(Flag_pre_load_init);

    // Make initial built-in classes
    SkBrain::initialize_core_classes(SkCompiler_parse_class_count_init);
    }

  // Initialize checksums
  SkBrain::ms_checksum_folders = UINT32_MAX;
  SkBrain::ms_checksum_files   = UINT32_MAX;


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Notify Remote IDE if needed
  if (SkConsole::ms_console_p)
    {
    SkConsole::ms_console_p->get_remote_ide()->cmd_compiled_state_reply(SkRemoteBase::CompiledState_compiling);
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Create class hierarchy - iterate through overlays in reverse order

  char         buffer_p[300];
  AString      str(buffer_p, 300u, 0u);
  ADirectory   dir;
  uint32_t     length      = m_overlays.get_length();
  uint32_t     overlay_idx = length;
  SkOverlay ** overlays_pp = m_overlays.get_array();

  AMethodArg<SkCompiler, ADirectory *> parse_dir_func(this, &SkCompiler::parse_directory_class);

  SkDebug::print_agog("Pre-parsing classes from overlays (highest to lowest #):");
  update_msg_queue();

  do
    {
    overlay_idx--;
    m_current_overlay_p = overlays_pp[overlay_idx];

    // Skip unapplied overlays
    if (m_current_overlay_p->m_apply_b)
      {
      str.format("\n  '%s': %s ...", m_current_overlay_p->m_name.as_cstr(), m_current_overlay_p->m_path.as_cstr());
      SkDebug::print(str);

      str.empty();
      str.append(m_current_overlay_p->m_path_qual);
      str.append(g_overlay_object_root);
      dir.set_path(str);
      dir.apply_subdirs(&parse_dir_func, true);
      parse_directory_classes_flattened();
      }
    }
  while (overlay_idx > 0u);

  SkDebug::print_agog("\n  ...done!\n\n");

  // Set up more classes (this can be called repeatedly)
  SkBrain::initialize_after_classes_known(ASymbol::create(get_ini_startup_mind()));


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Load generated object IDs if doing validation
  if (m_flags & Flag_object_ids_validate)
    {
    load_object_ids_generated(AVerbosity_brief);
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Preparse code files
  preparse_files_overlays(*SkBrain::ms_object_class_p, true);

  if (m_pre_errors)
    {
    SkDebug::print_agog(
      a_str_format("\n  ######## %u pre-parse %s ########\n\n\n", m_pre_errors, (m_pre_errors == 1u) ? "error" : "errors"),
      SkLocale_all,
      SkDPrintType_error);
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Build vtables after pre-parsing
  SkBrain::ms_object_class_p->build_vtables_recurse(false);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Resolve raw data if a callback function is given
  SkBrain::ms_object_class_p->resolve_raw_data_recurse();

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Notify Remote IDE if needed
  if (m_errors && SkConsole::ms_console_p)
    {
    SkConsole::ms_console_p->get_remote_ide()->cmd_compiled_state_reply(SkRemoteBase::CompiledState_errors);
    }

  phase_next();
  }

//---------------------------------------------------------------------------------------
// Parses a directory into a class using either:
//   SuperClass/Class - parent dir as superclass and title as class
//   SuperClass.Class - (flattened) title as superclass and extension as class
// 
// Params:
//   dir_p: directory to parse
// 
// See: SkOverlay::m_path_depth
void SkCompiler::parse_directory_class(ADirectory * dir_p)
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ASymbol  class_name;
  uint32_t end_pos;
  bool     super_path_b = !dir_p->is_extensioned();

  SkParser class_parse(super_path_b ? dir_p->get_name() : dir_p->get_extension(false));

  // Skip directories that do not have a valid class name
  if ((class_parse.parse_name_class(0u, &end_pos, &class_name, SkParser::ClassCheck_no_validate) != SkParser::Result_ok)
    || (end_pos != class_parse.get_length()))
    {
    return;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Accrue folder checksum
  // - only need to do once - skip for successive attempts to parse
  if (!is_flags(Flag_misordered_flattened))
    {
    SkBrain::ms_checksum_folders = AChecksum::generate_crc32(class_parse, SkBrain::ms_checksum_folders);
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Determine superclass and create class
  ASymbol   super_name;
  SkClass * class_p      = nullptr;
  SkClass * superclass_p = nullptr;

  // Note that superclasses that come from the path are never flattened - they are just
  // the class name - and they have already been validated.
  SkParser super_parse(super_path_b ? dir_p->get_parent_dir().get_name() : dir_p->get_title());

  if (class_name.get_id() == ASymbolId_Object)
    {
    class_p = SkBrain::ms_object_class_p;
    }
  else
    {
    if ((super_parse.parse_name_class(0u, &end_pos, &super_name, SkParser::ClassCheck_no_validate) != SkParser::Result_ok)
      || (end_pos != super_parse.get_length()))
      {
      // Invalid superclass - skip
      if (super_parse.as_cstr()[end_pos] == '.')
        {
        m_errors++;
        SkDebug::print_error(
          a_str_format(
            "Flattened class directories may not have class subdirectories!\n"
            "  - skipped %s",
            dir_p->get_path().as_cstr()),
          AErrLevel_error);
        }
      return;
      }

    superclass_p = SkBrain::get_class(super_name);

    if (superclass_p == nullptr)
      {
      // Superclass not set up yet - should only be for some flattened classes.
      // Store for later creation - copy if first time through otherwise reuse.
      m_flattened_dirs.append(is_flags(Flag_misordered_flattened) ? *dir_p : *new ADirectory(*dir_p));

      return;
      }

    // Disallow classes to be derived from "Class" and "None" unless created internally by C++
    if ((superclass_p->is_class(*SkBrain::ms_class_class_p))
      || (superclass_p->is_class(*SkNone::get_class())))
      {
      if (SkBrain::get_class(class_name) == nullptr)
        {
        m_errors++;
        SkDebug::print_error(
          a_str_format(
            "The classes 'Class' and 'None' may not have additional subclasses!\n"
            "  - The class '%s\\%s' has been skipped.",
            super_parse.as_cstr(),
            class_parse.as_cstr()),
          AErrLevel_error);
        }

      return;
      }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Register class
    class_p = SkBrain::create_class(class_name, superclass_p);

    // Ensure at proper directory depth
    uint32_t path_depth = m_current_overlay_p->m_path_depth;

    if (super_path_b)
      {
      // Superclass based on parent directory - Superclass/Subclass
      if ((path_depth != SkOverlay::Path_depth_any)
        && (class_p->get_superclass_depth() > path_depth))
        {
        // Too deep should be flattened
        m_errors++;
        SkDebug::print_agog(
          a_str_format(
            "\nClass directory in overlay '%s' too deep and should be flattened. Depth %u / Desired %u\n"
              "  - Use path   '%s' so IDE can find its files\n"
              "    instead of '%s'.",
            m_current_overlay_p->m_name.as_cstr(),
            class_p->get_superclass_depth(),
            path_depth,
            m_current_overlay_p->get_path_class(*class_p).as_cstr(),
            dir_p->get_path().as_cstr()),
          SkLocale_all,
          SkDPrintType_warning);
        }
      }
    else
      {
      // Flattened classes Superclass.Subclass
      if (path_depth == SkOverlay::Path_depth_any)
        {
        m_errors++;
        SkDebug::print_agog(
          a_str_format(
            "\nFlattened class directories not enabled in overlay '%s'!\n"
              "  - Use path   '%s' so IDE can find its files\n"
              "    instead of '%s'.",
            m_current_overlay_p->m_name.as_cstr(),
            m_current_overlay_p->get_path_class(*class_p).as_cstr(),
            dir_p->get_path().as_cstr()),
          SkLocale_all,
          SkDPrintType_warning);
        }
      else
        {
        SkClass * parent_path_class_p = SkBrain::get_class(dir_p->get_parent_dir().get_name());
        int32_t   current_depth       = parent_path_class_p ? (int32_t(parent_path_class_p->get_superclass_depth()) + 1) : -1;

        if (current_depth != int32_t(path_depth))
          {
          m_errors++;
          SkDebug::print_agog(
            a_str_format(
              "\nOverlay '%s' specifies flattened directories at level %u, but found at level %i!\n"
              "  - Use path   '%s' so IDE can find its files\n"
              "    instead of '%s'.",
              m_current_overlay_p->m_name.as_cstr(),
              path_depth,
              current_depth,
              m_current_overlay_p->get_path_class(*class_p).as_cstr(),
              dir_p->get_path().as_cstr()),
            SkLocale_all,
            SkDPrintType_warning);
          }
        }
      }

    m_classes = SkBrain::get_classes().get_length();;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Parse class meta file if it exists - this is done before any other script files are
  // parsed to ensure that the class is setup properly.
  AString meta_title("!Class.sk-meta", 14u);
  AFile   meta_file(dir_p->get_path(), meta_title);

  if (!meta_file.is_existing())
    {
    // Class doesn't have a meta file
    return;
    }

  parse_file_class_meta(&meta_file, class_p);
  }

//---------------------------------------------------------------------------------------
// Parses any flattened class directories that were unable to be processed since their
// superclasses did not yet exist.
// 
// See: parse_directory_class(), SkOverlay::m_path_depth
void SkCompiler::parse_directory_classes_flattened()
  {
  uint32_t flattened_count = m_flattened_dirs.get_length();

  if (flattened_count)
    {
    // Parse any flattened class directories
    ADirectory * dir_p;
    APArray<ADirectory> dirs;
    uint32_t flattened_count_prev = 0u;

    enable_flags(Flag_misordered_flattened);

    do
      {
      flattened_count_prev = flattened_count;
      flattened_count = 0u;
      dirs = m_flattened_dirs;
      m_flattened_dirs.empty();
      dir_p = dirs.pop();

      do
        {
        parse_directory_class(dir_p);

        if (flattened_count == m_flattened_dirs.get_length())
          {
          // Directory was processed.
          delete dir_p;
          }

        dir_p = dirs.pop();
        flattened_count = m_flattened_dirs.get_length();
        }
      while (dir_p);
      }
    while (flattened_count && (flattened_count != flattened_count_prev));

    enable_flags(Flag_misordered_flattened, false);

    if (flattened_count)
      {
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Superclasses were specified that did not exist
      AString error_str;

      m_errors += flattened_count;
      error_str.ensure_size(512u);
      error_str.format(
        "%u %s with a non-existent superclass in '%s' overlay!\n",
        flattened_count,
        (flattened_count == 1) ? "class was using a flattened directory" : "classes were using flattened directories",
        m_current_overlay_p->m_name.as_cstr()
        );

      do 
        {
        dir_p = m_flattened_dirs.pop();
        error_str.append("  ", 2u);
        error_str.append(dir_p->get_name());
        error_str.append('\n');
        delete dir_p;
        }
      while (m_flattened_dirs.is_filled());

      error_str.append(
        (flattened_count == 1) ? "  - the class has been skipped" : "  - the classes have been skipped");
      SkDebug::print_error(error_str, AErrLevel_error);
      }
    }
  }

//---------------------------------------------------------------------------------------
// Arg          file_p - file to preparse for invokables
// Author(s):    Conan Reis
void SkCompiler::preparse_file(AFile * file_p)
  {
  SkMemberInfo info;

  if (!parse_file_member(*file_p, &info))
    {
    if (info.m_type == SkMember__error)
      {
      SkDebug::print_error(a_str_format("Invalid script filename:\n  %s\n  - File skipped!", file_p->as_cstr()));
      }

    // File skipped
    return;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Continue parsing member

  //#if (SKOOKUM & SK_DEBUG)
  //  // $HACK - Stop at a specific file
  //  if ((info.m_member_id.get_scope()->get_name_id() == 0x15854c57)  // remove_eqv
  //    && (info.m_member_id.get_name_id() == 0x21c4152f))             // List
  //    {
  //    // Put breakpoint on line below
  //    ADebug::print_format("\n" A_SOURCE_STR "Found file: %s\n", file_p->as_cstr());
  //    }
  //#endif

  bool compile_dependency = true;
  SkClassUnaryBase * scope_p = info.get_class_scope();

  switch (info.m_type)
    {
    case SkMember_method:
      preparse_file_method(file_p, info.m_member_id.get_name(), scope_p);
      break;

    case SkMember_coroutine:
      preparse_file_coroutine(file_p, info.m_member_id.get_name(), scope_p);
      break;

    case SkMember_data:
      parse_file_data_members(file_p, scope_p);
      break;

    case SkMember_object_ids_defer:
      compile_dependency = false;
      // \/ \/ Allow fall through \/ \/

    case SkMember_object_ids:
      parse_file_object_ids(file_p, static_cast<SkClass *>(scope_p));
      break;

    // Some file types are skipped at preparse stage
      //SkMember_class_meta - parsed during class creation stage
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Accrue file checksum
  if (compile_dependency)
    {
    SkBrain::ms_checksum_files += AChecksum::generate_crc32(file_p->get_title());
    }
  }

//---------------------------------------------------------------------------------------
// Called at the end of a parse to clean-up, note any errors, etc.
// Author(s):   Conan Reis
void SkCompiler::parse_complete()
  {
  // Note any parse errors
  if (m_errors)
    {
    if (m_console_p)
      {
      m_console_p->on_error(m_errors);
      }

    SkDebug::print_agog(
      a_str_format("\n\n  ######## %u %s ########\n", m_errors, (m_errors == 1u) ? "error" : "errors"),
      SkLocale_all,
      SkDPrintType_error);
    }

  // Remove unreferenced data
  // $Revisit - CReis Might need to call these more than once since one round may free up
  // references for the next round.
  SkInvokableClass::shared_ensure_references();
  SkParameters::shared_ensure_references();
  SkTypedClass::shared_ensure_references();
  SkClassUnion::shared_ensure_references();
  }

//---------------------------------------------------------------------------------------
// Fully parses all code files - methods, data, coroutines, etc.
// Arg         cls - class to look for script files
// Arg         recurse - indicates whether the class directories should be recursed or not
// Author(s):   Conan Reis
void SkCompiler::parse_files_overlays(
  const SkClass & cls,
  bool            recurse // = true
  )
  {
  AMethodArg<SkCompiler, AFile *> parse_file_func(this, &SkCompiler::parse_file);

  SkDebug::print_agog("Parsing code from overlays:");

  apply_overlay_files(&parse_file_func, cls, recurse);

  SkDebug::print_agog("...done!\n\n");
  }

//---------------------------------------------------------------------------------------
// [Parse Pass #2]  Fully parses all code files - methods, data, coroutines,
//             etc.
// Author(s):   Conan Reis
void SkCompiler::parse_files()
  {
  SkParser::clear_stats();

  parse_files_overlays(*SkBrain::ms_object_class_p, true);

  // Clean-up, note any errors, etc.
  parse_complete();


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Notify Remote IDE if needed
  if (m_errors && SkConsole::ms_console_p)
    {
    SkConsole::ms_console_p->get_remote_ide()->cmd_compiled_state_reply(SkRemoteBase::CompiledState_errors);
    }

  #ifdef A_SHOW_NOTES
    SkParser::print_stats();
  #endif

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Post parse tasks

  // Set all classes as loaded
  SkBrain::get_classes().apply_method(&SkClass::set_loaded);

  // Track string literal dupes - for string pooling
  #ifdef SKLITERAL_STRING_TRACK_DUPES
    uint32_t extra_bytes;
    uint32_t extra_refs;

    SkLiteral::get_pooled_string_info(&extra_bytes, &extra_refs);
    ADebug::print_format(
      "Redundant string literal memory -\n"
      "  redundant string count: %8u\n"
      "  buffer bytes:           %8u\n"
      "  AStringRef bytes:       %8u\n"
      "  Total:                  %8u\n\n",
      extra_refs,
      extra_bytes,
      extra_refs * sizeof(AStringRef),
      extra_bytes + (extra_refs * sizeof(AStringRef)));
  #endif // SKLITERAL_STRING_TRACK_DUPES

  phase_next();
  }

//---------------------------------------------------------------------------------------
// Preparses file for a method - i.e. just notes the parameters and ignores
//             the rest of the code if any.
// Arg         file_p - file to parse for method
// Arg         name - name of method
// Arg         scope_p - scope for the method
// Arg         class_method - true if it is a class method, false if it is an instance
//             method.
// See:        parse_file_method()
// Notes:      method-file = ws parameters [ws code-block] ws
// Author(s):   Conan Reis
void SkCompiler::preparse_file_method(
  AFile *            file_p,
  const ASymbol &    name,
  SkClassUnaryBase * scope_p
  )
  {
  // Ignore duplicate methods (likely overridden by higher sequence overlays)
  if (scope_p->is_method_registered(name, true))
    {
    return;
    }

  uint32_t     length;
  const char * file_cstr_p = map_text_file(file_p, &length);

  if (file_cstr_p == nullptr)
    {
    // File is empty or invalid
    return;
    }

  SkParser::Args args;
  SkParser       parser(file_cstr_p, length);
      
  parser.preparse_method_source(name, scope_p, args);

  if (args.m_result != SkParser::Result_ok)
    {
    m_pre_errors++;

    if (is_flags(Flag_show_preparse_errors))
      {
      SkDebug::print_parse_error(
        args.m_result, file_p->get_file_str(), &parser, args.m_end_pos);
      }
    }
  }

//---------------------------------------------------------------------------------------
// Preparses file for a coroutine - i.e. just notes the parameters and ignores the rest
// of the code if any.
// 
// #Params
//   file_p:  file to parse for coroutine
//   name:    name of coroutine
//   scope_p: class scope for the coroutine
//   
// #Notes
//   ``` EBNF
//   coroutine-file-name = script-name '().sk'
//   coroutine-file      = ws coroutine ws
//   coroutine           = parameters [ws code-block]
//   ```
//
// Author(s):  Conan Reis
void SkCompiler::preparse_file_coroutine(
  AFile *            file_p,
  const ASymbol &    name,
  SkClassUnaryBase * scope_p
  )
  {
  // Ignore duplicate coroutines (likely overridden by higher sequence overlays)
  if (scope_p->is_coroutine_valid(name))
    {
    return;
    }

  uint32_t     length;
  const char * file_cstr_p = map_text_file(file_p, &length);

  if (file_cstr_p == nullptr)
    {
    // File is empty or invalid
    return;
    }

  SkParser::Args args;
  SkParser       parser(file_cstr_p, length, false);
      
  parser.preparse_coroutine_source(name, scope_p, args);

  if (args.m_result != SkParser::Result_ok)
    {
    m_pre_errors++;

    if (is_flags(Flag_show_preparse_errors))
      {
      SkDebug::print_parse_error(
        args.m_result, file_p->get_file_str(), &parser, args.m_end_pos);
      }
    }
  }


//---------------------------------------------------------------------------------------
// Determines if binary for class hierarchy and associated info exists.
// 
// This check is done before the symbol file check since the symbol file is only needed
// when debugging and giving an error about missing the compiled binary is more intuitive
// to the end user than a missing symbol file.
// 
// #Returns
//   file to parse for coroutine
// 
// See Also:    get_binary_hierarchy()
// Modifiers:   virtual - overridden from SkookumRuntimeBase
// Author(s):   Conan Reis
bool SkCompiler::is_binary_hierarchy_existing()
  {
  AFile compiled_file(get_ini_compiled_file_query());

  if (!compiled_file.is_titled() || !compiled_file.is_existing())
    {
    SkDebug::print_agog(a_str_format("  Could not find compiled binary file '%s'!\n\n", compiled_file.as_cstr()), SkLocale_all, SkDPrintType_error);
    update_msg_queue();

    return false;
    }

  return true;
  }

//---------------------------------------------------------------------------------------

void SkCompiler::on_binary_hierarchy_path_changed()
  {
  // Nothing to be done here for now
  }

//---------------------------------------------------------------------------------------
// Gets memory representing binary for class hierarchy and associated info.
// 
// See Also:    load_compiled_scripts(), SkCompiler::get_binary_class_group()
// Modifiers:   virtual - overridden from SkookumRuntimeBase
// Author(s):   Conan Reis
SkBinaryHandle * SkCompiler::get_binary_hierarchy()
  {
  SkBinaryHandleCompiler * handle_p = new SkBinaryHandleCompiler(get_ini_compiled_file_query());

  SkDebug::print_agog(a_str_format("  Loading compiled binary file '%s'...\n", handle_p->m_file.get_file_str().as_cstr()));
  update_msg_queue();

  handle_p->m_binary_p = handle_p->m_file.map_memory(
    AFile::Access_read, &handle_p->m_size);

  return handle_p;
  }

//---------------------------------------------------------------------------------------
// Loads group of classes with specified class as root.  This can be used as
//             a mechanism to "demand load" scripts.
// See:        load_compiled_scripts(), SkookumRuntime::get_binary_class_group()
// Modifiers:   virtual - overridden from SkookumRuntimeBase
// Author(s):   Conan Reis
SkBinaryHandle * SkCompiler::get_binary_class_group(const SkClass & cls)
  {
  SkBinaryHandleCompiler * handle_p =
    new SkBinaryHandleCompiler(get_ini_compiled_file_query(true));

  // $Revisit - CReis Should use fast custom uint32_t to hex string function.
  handle_p->m_file.set_title(a_str_format("Class[%x]", cls.get_name_id()));
    
  handle_p->m_binary_p = handle_p->m_file.map_memory(
    AFile::Access_read, &handle_p->m_size);

  return handle_p;
  }


#if defined(A_SYMBOL_STR_DB_AGOG)  
  
//---------------------------------------------------------------------------------------
// Gets memory representing binary for class hierarchy and associated info.
// 
// See Also:    load_compiled_scripts(), SkCompiler::get_binary_class_group()
// Modifiers:   virtual - overridden from SkookumRuntimeBase
// Author(s):   Conan Reis
SkBinaryHandle * SkCompiler::get_binary_symbol_table()
  {
  AFile sym_file(get_ini_compiled_file_query());

  sym_file.set_extension(g_file_ext_symbols);

  // Ensure symbol table binary exists
  if (!sym_file.is_existing())
    {
    SkDebug::print_agog(a_str_format("  Compiled binary symbol file '%s' does not exist!\n\n", sym_file.as_cstr()));
    update_msg_queue();

    return nullptr;
    }


  SkDebug::print_agog(a_str_format("  Loading compiled binary symbol file '%s'...\n", sym_file.as_cstr()));
  update_msg_queue();

  SkBinaryHandleCompiler * handle_p = new SkBinaryHandleCompiler(sym_file);

  // $Revisit - CReis Determine if memory mapping is the most efficient way to stream in
  // and operate on a file as fast as possible.
  handle_p->m_binary_p = handle_p->m_file.map_memory(
    AFile::Access_read, &handle_p->m_size);

  return handle_p;
  }

#endif


//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkCompiler::release_binary(SkBinaryHandle * handle_p)
  {
  delete static_cast<SkBinaryHandleCompiler *>(handle_p);
  }

//---------------------------------------------------------------------------------------
// Loads binary representation of SkookumScript data structures (compiled
//             binary code)
// Author(s):   Conan Reis
void SkCompiler::compiled_load()
  {
  SkDebug::print_agog("Loading previously parsed compiled binary...\n");

  if (load_compiled_hierarchy() != SkLoadStatus_ok)
    {
    SkDebug::print_agog("  Switching to source file parsing...\n\n");
    phase_start(Phase_preparse);
    return;
    }

  enable_flags(Flag_pre_load_init);
  SkDebug::print_agog("  ...done!\n");


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // 5) Load demand load classes during stand-alone mode so all class info is available
  // for the IDE.
  // $Revisit - CReis This could be a user setting
  if (!SkDebug::is_engine_present())
    {
    SkDebug::print_agog("\nLoading all demand load class groups...\n");
    load_compiled_class_group_all();
    SkDebug::print_agog("  ...done!\n");
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Load object ID validation files
  if (m_flags & Flag_object_ids_validate)
    {
    load_and_validate_object_ids(SkBrain::ms_object_class_p, AHierarchy__all, AVerbosity_brief);
    }
  else
    {
    load_object_ids_recurse(SkBrain::ms_object_class_p, AHierarchy__all, AVerbosity_brief);
    load_object_ids_generated(AVerbosity_brief);
    }

  phase_next();
  }

//---------------------------------------------------------------------------------------
// Saves out compiled binary representation of SkookumScript data structures
// Author(s):   Conan Reis
bool SkCompiler::compiled_save()
  {
  bool  saved_b = false;
  AFile file_bin(get_ini_compiled_file_query(false));
  AFile file_sym(file_bin);

  file_sym.set_extension(g_file_ext_symbols);

  SkDebug::print_agog("\nGenerating compiled binaries from class hierarchy...\n");

  if (file_bin.is_titled()
    && file_bin.ensure_writable_query()
    && file_sym.ensure_writable_query())
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Erase stale binaries
    ADirectory bin_dir(file_bin.get_path());
    AString    wild_bins("*.", 2u);

    wild_bins.append(g_file_ext_compiled);

    if (bin_dir.is_existing())
      {
      // $Vital - CReis Delete all files for now and change to just bin files after a bit
      bin_dir.remove_files(false, wild_bins);
      }


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Save compiled binary

    ASymbolTable saved_syms;

    // Track all symbols that are saved in the compiled binary.
    ASymbol::track_serialized(&saved_syms);

    SkDebug::print_agog(a_str_format("  Saving class hierarchy compiled binary...\n    %s\n", file_bin.as_cstr()));
    update_msg_queue();

    // Remove unused data before saving
    SkBrain::compact(!m_errors);

    uint32_t binary_length = SkBrain::as_binary_length();

    void * binary_mem_p = file_bin.map_memory_create(AFile::Access_write, binary_length);

    void * binary_p = binary_mem_p;
    SkBrain::as_binary(&binary_p);
    // NOTE: When this fails, set A_SANITY_CHECK_BINARY_SIZE=1 to find the culprit
    SK_ASSERTX((uint8_t *)binary_p - (uint8_t *)binary_mem_p == binary_length, a_str_format("Inconsistent binary length of compiled binary (expected: %d, actual: %d)!", binary_length, (uint8_t *)binary_p - (uint8_t *)binary_mem_p));

    file_bin.close();

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Save Demand Load Class Groups
    // The binaries for load on demand classes are saved after (rather than during) the
    // main class hierarchy binary save since it is assumed to be faster than saving back
    // and fourth between different files.
    AString bin_manifest_str;
    bool    save_manifest_b = is_flags(Flag_save_manifest);

    if (save_manifest_b)
      {
      bin_manifest_str.append(file_bin.get_name());
      }

    SkDebug::print_agog("  Saving compiled binaries for load on demand classes...\n");
    update_msg_queue();
    compiled_save_demand_loaded(
      SkBrain::ms_object_class_p,
      &file_bin,
      save_manifest_b ? &bin_manifest_str : nullptr);
    SkDebug::print_agog("    ...done! Scripts now combobulated.\n");

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Save optional compiled binary manifest file
    if (save_manifest_b)
      {
      AFile file_bin_manifest(file_bin.get_path(), "SkookumBinaries.txt");

      SkDebug::print_agog(a_str_format("  Saving compiled binaries manifest...\n    %s\n", file_bin_manifest.as_cstr()));
      file_bin_manifest.write_text(bin_manifest_str);
      }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Save Symbol Table
    SkDebug::print_agog(a_str_format("  Saving symbol table...\n    %s\n", file_sym.as_cstr()));

    void * sym_file_mem_p = file_sym.map_memory_create(AFile::Access_write, saved_syms.as_binary_length());

    saved_syms.as_binary(&sym_file_mem_p);
    file_sym.close();
    ASymbol::track_serialized(nullptr);

    saved_b = true;

    SkDebug::print_agog("  ...done!\n\n");


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Make copies of binaries (for different projects/platforms etc.) if desired.
    compiled_copy();


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Notify Remote IDE if needed
    if (SkConsole::ms_console_p)
      {
      SkConsole::ms_console_p->get_remote_ide()->cmd_compiled_state_reply(SkRemoteBase::CompiledState_fresh);
      }
    }
  else
    {
    SkDebug::print_agog(a_str_format("  Compiled binary '%s' not saved\n", file_bin.as_cstr()), SkLocale_all, SkDPrintType_warning);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Notify Remote IDE if needed
    if (SkConsole::ms_console_p)
      {
      SkConsole::ms_console_p->get_remote_ide()->cmd_compiled_state_reply(SkRemoteBase::CompiledState_stale);
      }
    }

  phase_next();

  return saved_b;
  }

//---------------------------------------------------------------------------------------
// Saves out any demand loaded class and all its subclasses recursively.
// Author(s):   Conan Reis
void SkCompiler::compiled_save_demand_loaded(
  SkClass * class_p,
  AFile *   file_p,
  AString * bin_manifest_str_p // = nullptr
  )
  {
  if (class_p->is_demand_loaded_root())
    {
    // Save out class and its subclasses as an individual compiled binary file.
    // $Revisit - CReis Should use fast custom uint32_t to hex string function.
    file_p->set_title(a_str_format("Class[%x]", class_p->get_name_id()));

    //ADebug::print_format("    %s  %s\n", class_p->get_name_cstr(), file_p->as_cstr());

    // Add to binary manifest if desired
    if (bin_manifest_str_p)
      {
      AString file_name(file_p->get_name());

      bin_manifest_str_p->ensure_size_extra(2u + file_name.get_length());
      bin_manifest_str_p->append(", ", 2u);
      bin_manifest_str_p->append(file_name);
      }

    void * binary_p = file_p->map_memory_create(AFile::Access_write, class_p->as_binary_group_length(false));

    class_p->as_binary_group(&binary_p, false);
    file_p->close();
    }
  else
    {
    // Iterate over subclasses looking for demand loaded classes.
    uint32_t length = class_p->get_subclasses().get_length();

    if (length)
      {
      SkClass ** classes_pp     = class_p->get_subclasses().get_array();
      SkClass ** classes_end_pp = classes_pp + length;

      for (; classes_pp < classes_end_pp; classes_pp++)
        {
        compiled_save_demand_loaded(*classes_pp, file_p, bin_manifest_str_p);
        }
      }
    }
  }

//---------------------------------------------------------------------------------------
// Make copies of binaries (for different projects/platforms etc.) if desired
//             and the copies are not already up-to-date.
// Author(s):   Conan Reis
void SkCompiler::compiled_copy()
  {
  AFile file_bin(get_ini_compiled_file_query(false));

  if (file_bin.is_named())
    {
    AString copy_key(nullptr, 20u, 0u);
    ATime   file_bin_time = file_bin.get_time_modified();
    uint    count         = 1u;
    bool    found_entry_b = false;
    bool    copied_b      = false;
    AFile   dest_file;
    AFile   file_sym(file_bin);

    file_sym.set_extension(g_file_ext_symbols);

    ADirectory source_dir(file_bin.get_directory());
    ADirectory dest_dir;

    //AString    wild_bins("*.", 2u);
    //wild_bins.append(g_file_ext_compiled);
    
    // $Revisit - CReis Copy everything for now
    AString wild_bins('*');


    do
      {
      // Load copy entry
      found_entry_b = false;
      copy_key.format(g_ini_key_copy_to_p, count);
      dest_file = m_ini_proj.get_value_file(copy_key, g_ini_section_output_p);

      if (dest_file.is_named())
        {
        found_entry_b = true;

        // Make copy if destination does not exist or is different mod time
        if (!dest_file.is_existing()
          || (file_bin_time != dest_file.get_time_modified()))
          {
          if (!copied_b)
            {
            SkDebug::print_agog("  Making copy of compiled binaries...\n");
            update_msg_queue();
            copied_b = true;
            }

          SkDebug::print_agog(a_str_format("    %s*.sk-bin + *.sk-sym\n", dest_file.get_path().as_cstr()));

          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          // Erase stale binaries
          dest_dir.set_path(dest_file.get_path());

          if (dest_dir.is_existing())
            {
            dest_dir.remove_files(false);

            // $Vital - CReis Delete all files for now and change to just bin files after a bit
            //dest_dir.remove_files(false, wild_bins);
            }
          else
            {
            dest_dir.create();
            }


          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          // Copy binaries
          source_dir.copy_files(dest_dir, wild_bins, AFile::Overwrite_read_only);

          // $Revisit - CReis Copy everything for now
          //dest_file.set_extension(g_file_ext_symbols);
          //file_sym.copy(dest_file);
          }

        count++;
        }
      }
    while (found_entry_b);

    if (copied_b)
      {
      SkDebug::print_agog("\n");
      }
    }
  }

//---------------------------------------------------------------------------------------
// Binds C++ methods (atomics) to Skookum
// Author(s):   Conan Reis
void SkCompiler::bind_atomics()
  {
  if (!is_flags(Flag_bindings_bound))
    {
    SkDebug::print_agog("Binding with C++ routines...\n");

    // Registers/connects Generic SkookumScript atomic classes, coroutines, etc. with the
    // compiled binary that was just loaded.
    SkookumScript::initialize_program();

    #if (SKOOKUM & SK_DEBUG)
      if (is_flags(Flag_ensure_bindings) && SkDebug::is_engine_present())
        {
        SkBrain::ensure_atomics_registered();
        }
    #endif

    enable_flags(Flag_bindings_bound);
    SkDebug::print_agog("  ...done!\n\n");


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Enable SkookumScript evaluation
    if (is_flags(Flag_evaluate_scripts))
      {
      SkookumScript::enable_flag(SkookumScript::Flag_paused, false);
      }

    SkookumScript::initialize_sim();
    SkookumScript::initialize_gameplay();

    phase_next();
    }
  }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Finds the file for the described member and returns the overlay where it
//             was found.
// Returns:    Overlay where member file was found or nullptr if not found in any overlay.
// Arg         member_info - member to find the file for
// Arg         file_p - address to store file or partial file (relative path without
//             overlay info) if not found.  Ignored if nullptr.
// Arg         match - flags that overlay must have in order to match find            
// See:        get_member_name()
// Author(s):   Conan Reis
SkOverlay * SkCompiler::find_member_file(
  const SkMemberInfo & member_info,
  AFile *              file_p, // = nullptr
  SkOverlay::eFlag     match   // = SkOverlay::Flag_applied
  )
  {
  AFile        file;
  SkOverlay *  overlay_p   = nullptr;
  SkOverlay ** overlays_pp = m_overlays.get_array();
  uint32_t     overlay_idx = m_overlays.get_length();

  // Iterate through overlays from highest to lowest sequence until file is found
  do
    {
    overlay_idx--;
    overlay_p = overlays_pp[overlay_idx];

    // If requested, skip overlays that are not currently applied
    if (((match & SkOverlay::Flag_applied) == 0u) || overlay_p->m_apply_b)
      {
      // Get path from overlay
      file.set_file_str(overlay_p->get_path_member(member_info));

      // Determine if the file exists
      if (file.is_existing())
        {
        // Bingo.

        // $Vital - CReis Data members are interleaved and may not be located in the first overlay
        // that has a data file.  In order to be correct, the data files must be searched for
        // the specific data member.

        if (file_p)
          {
          *file_p = file;
          }

        return overlay_p;
        }
      }
    }
  while (overlay_idx != 0u);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // File not found in overlays

  if (file_p)
    {
    // Set partial file (relative path without overlay info)
    file_p->set_file_str(member_info.as_file_title(SkMemberInfo::PathFlag__file));
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Determines member info for supplied file and finds the overlay where it
//             was found.
// Returns:    Overlay where member file was found or nullptr if not found in any overlay.
//             Note that the file may be in an overlay that is not currently applied 
//             (check return value m_apply_b) or it may be overridden by a file in an
//             overriding overlay (check topmost_p).
// Arg         file_p - address to store file or partial file (relative path without
//             overlay info) if not found.  Ignored if nullptr.
// Arg         info_p - address to store member info
// Arg         topmost_p - true if the file is being used and not overridden - i.e. in
//             overlay and not overridden by file in another overlay.
// Author(s):   Conan Reis
SkOverlay * SkCompiler::find_file_overlay(
  const AFile &  file,
  SkMemberInfo * info_p,   // = nullptr
  bool *         topmost_p // = nullptr
  )
  {
  SkMemberInfo info;
  SkOverlay *  overlay_p = nullptr;
  bool         topmost   = false;

  if (parse_file_member(file, &info))
    {
    // Determine overlay path
    AString overlay_path(file.as_qualified().get_path());
    uint32_t    end_overlay_idx = overlay_path.get_length();
    
    overlay_path.find_reverse('\\', info.get_class()->get_superclass_depth() + 2u, &end_overlay_idx);
    overlay_path.remove_end(overlay_path.get_length() - (end_overlay_idx + 1u));

    // Determine if such an overlay exists
    SkOverlay ** overlays_pp = m_overlays.get_array();
    uint32_t     overlay_idx = m_overlays.get_length();

    do
      {
      overlay_idx--;

      if (overlay_path.is_iequal(overlays_pp[overlay_idx]->m_path_qual))
        {
        // Bingo.
        overlay_p = overlays_pp[overlay_idx];

        // Exit while
        break;
        }
      }
    while (overlay_idx != 0u);

    // If need to know if it was topmost - test if topmost is same overlay
    if (topmost_p && overlay_p && overlay_p->m_apply_b)
      {
      topmost = overlay_p == find_member_file(info);
      }
    }

  if (info_p)
    {
    *info_p = info;
    }

  if (topmost_p)
    {
    *topmost_p = topmost;
    }

  return overlay_p;
  }

//---------------------------------------------------------------------------------------
// Determines member info for supplied file
// Returns:    true if file represents valid member, false if not
// Arg         file - file to determine member
// Modifiers:   static
// Author(s):   Conan Reis
eSkMember SkCompiler::parse_file_member_type(
  const AFile & file,
  SkQualifier * ident_p, // = nullptr
  bool * class_member_p // = nullptr
  )
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // All recognized files should end with a '.sk*' extension
  const AString & file_str     = file.get_file_str();
  uint32_t        file_ext_idx = file.get_extension_index();

  if (file_str.icompare_sub(g_file_ext_skookum, file_ext_idx) != AEquate_equal)
    {
    // Non .sk* files are ignored
    return SkMember__invalid;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Look for .sk-ids or .sk~ids
  SkParser file_name(file.get_title());

  if ((file_str.get_length() - file_ext_idx) == SkCompiler_file_ext_ids_length)
    {
    const char * file_cstr_a = file_str.as_cstr();

    if ((file_cstr_a[file_ext_idx + 4u] == 'i')
      && (file_cstr_a[file_ext_idx + 5u] == 'd')
      && (file_cstr_a[file_ext_idx + 6u] == 's'))
      {
      // Must identify an object ID list
      if (!AString::ms_is_uppercase[uint8_t(file_name.get_first())])
        {
        return SkMember__invalid;
        }

      // It is an object ID validation list
      eSkMember type = (file_cstr_a[file_ext_idx + 5u] == '-')
        ? SkMember_object_ids : SkMember_object_ids_defer;

      if (ident_p)
        {
        // Determine class
        SkClass * class_p = nullptr;
        
        if (file_name.parse_class(0u, nullptr, &class_p) != SkParser::Result_ok)
          {
          return SkMember__error;
          }

        ident_p->set_scope(class_p);
        }

      return type;
      }
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Parse member types other than object ID lists
  return file_name.identify_member_filename(ident_p, class_member_p, false);
  }

//---------------------------------------------------------------------------------------
// Determines member info for supplied file
// Returns:    true if file represents valid member, false if not
// Arg         file - file to determine member
// Arg         info_p - address to store member info
// Modifiers:   static
// Author(s):   Conan Reis
bool SkCompiler::parse_file_member(const AFile & file, SkMemberInfo * info_p)
  {
  info_p->m_type = parse_file_member_type(file, &info_p->m_member_id, &info_p->m_class_scope);

  if (info_p->m_type >= SkMember__invalid)
    {
    return false;
    }

  SkClass * class_p = parse_file_class(file);
      
  info_p->m_member_id.set_scope(class_p);

  if (class_p == nullptr)
    {
    info_p->m_type = SkMember__error;
    }

  return true;
  }

//---------------------------------------------------------------------------------------
// Loads the Class Hierarchy Overlay info.
// Author(s):   Conan Reis
void SkCompiler::load_overlays()
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Load overlays from ini file

  AString     overlay_key(nullptr, 12u, 0u);
  AString     overlay_str;
  SkOverlay * overlay_p;
  uint32_t    name_pos;
  uint32_t    bar_pos;
  uint        count         = 1u;
  bool        found_entry_b = false;

  m_overlays.free_all();

  do
    {
    // Load overlay entry
    found_entry_b = false;
    overlay_key.format(g_ini_key_overlay_p, count);
    overlay_str = m_ini_proj.get_value(overlay_key, g_ini_section_overlays_p);

    if (overlay_str.is_filled()
      && (overlay_str != AIni::ms_not_present_str)
      && overlay_str.find('|', 1u, &bar_pos))
      {
      found_entry_b = true;
      overlay_p = new SkOverlay;
      overlay_p->m_sequence = count;
      overlay_p->m_apply_b = true;
      overlay_p->m_editable_b = true;
      name_pos = 0;
      if (overlay_str[name_pos] == '-')
        {
        overlay_p->m_apply_b = false;
        ++name_pos;
        }
      if (overlay_str[name_pos] == '*')
        {
        overlay_p->m_editable_b = false;
        ++name_pos;
        }
      overlay_p->m_name = overlay_str.get(name_pos, bar_pos - name_pos);

      // Look for path flatten depth
      uint32_t bar2_pos = overlay_str.get_length();

      overlay_p->m_path_depth = SkOverlay::Path_depth_any;

      if (overlay_str.find('|', 1u, &bar2_pos, bar_pos + 1u))
        {
        overlay_p->m_path_depth = overlay_str.as_int(bar2_pos + 1u);
        }

      overlay_p->m_path = overlay_str.get(bar_pos + 1u, bar2_pos - bar_pos - 1u);

      // Clean overlay path
      overlay_p->m_path.replace_all('/', '\\');

      if (overlay_p->m_path.get_last() != '\\')
        {
        overlay_p->m_path += '\\';
        }

      // Make sure the path is fully qualified in relation to the project ini file
      overlay_p->m_path_qual = m_ini_proj.make_qualified(ADirectory(overlay_p->m_path));
      // If not found, try qualifying it relative to the default proj ini file
      if (!ADirectory::is_existing_path(overlay_p->m_path_qual))
        {
        overlay_p->m_path_qual = m_ini_proj_default.make_qualified(ADirectory(overlay_p->m_path));
        }

      m_overlays.append(*overlay_p);

      count++;
      }
    }
  while (found_entry_b);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure that there is at least one overlay entry

  bool resave_b = false;

  count = m_overlays.get_length();

  if (!count)
    {
    m_overlays.append(*new SkOverlay(
      1u,
      g_ini_overlay_core_name_def,
      g_ini_overlay_core_path_def,
      m_ini_proj.make_qualified(ADirectory(g_ini_overlay_core_path_def)),
      true));
    count++;

    if (SkookumScript::get_app_info()->use_builtin_actor())
      {
      m_overlays.append(*new SkOverlay(
        2u,
        g_ini_overlay_actor_name_def,
        g_ini_overlay_actor_path_def,
        m_ini_proj.make_qualified(ADirectory(g_ini_overlay_actor_path_def)),
        true));
      count++;
      }

    resave_b = true;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Validate overlay entries

  bool         path_valid_b;
  AString      path;
  SkOverlay ** overlay_pp         = m_overlays.get_array();
  SkOverlay ** overlay_end_pp     = overlay_pp + count;
  AString      missing_path_guess = m_ini_proj.get_file().get_path();

  for (; overlay_pp < overlay_end_pp; overlay_pp++)
    {
    overlay_p = *overlay_pp;

    if (overlay_p->m_apply_b && !ADirectory::is_existing_path(overlay_p->m_path_qual))
      {
      path_valid_b = m_console_p
        && m_console_p->browse_folder(
          &path,
          a_cstr_format("Please choose SkookumScript class hierarchy overlay '%s' scripts directory.  '%s' loaded from '%s' does not exist!", overlay_p->m_name.as_cstr(), overlay_p->m_path_qual.as_cstr(), m_ini_proj.get_file().get_name().as_cstr()),
          missing_path_guess)
        && ADirectory::is_existing_path(path);

      A_VERIFY(path_valid_b, a_cstr_format("The class hierarchy overlay '%s' scripts directory '%s' does not exist!\n[Does the '%s' configuration file exist and is it properly set up?]", overlay_p->m_name.as_cstr(), path.as_cstr(), m_ini_proj.get_file().get_name().as_cstr()), ErrId_nonexistent_dir, SkCompiler);

      ADirectory overlay_dir(path);

      overlay_p->m_path_qual = m_ini_proj.make_qualified(overlay_dir);
      overlay_p->m_path      = m_ini_proj.make_relative(overlay_dir);
      missing_path_guess     = overlay_dir.get_path(); // Last selected folder is a good guess for the next missing overlay folder
      resave_b               = true;
      }
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Resave overlays if necessary
  if (resave_b)
    {
    save_overlays();
    }
  }

//---------------------------------------------------------------------------------------
// Saves the Class Hierarchy Overlay info.
// Author(s):   Conan Reis
void SkCompiler::save_overlays()
  {
  char         buffer_a[SkCompiler_overlay_str_length];
  AString      entry_str(buffer_a, SkCompiler_overlay_str_length, 0u);
  AString      overlay_key(nullptr, 12u, 0u);
  SkOverlay ** overlay_pp     = m_overlays.get_array();
  SkOverlay ** overlay_end_pp = overlay_pp + m_overlays.get_length();
  SkOverlay *  overlay_p;

  // Remove previous overlays
  m_ini_proj.remove_section(g_ini_section_overlays_p);

  for (; overlay_pp < overlay_end_pp; overlay_pp++)
    {
    overlay_p = *overlay_pp;
    overlay_key.format(g_ini_key_overlay_p, overlay_p->m_sequence);

    if (overlay_p->m_path_depth == SkOverlay::Path_depth_any)
      {
      entry_str.format("%s%s%s|%s", overlay_p->m_apply_b ? "" : "-", overlay_p->m_editable_b ? "" : "*", overlay_p->m_name.as_cstr(), overlay_p->m_path.as_cstr());
      }
    else
      {
      entry_str.format("%s%s%s|%s|%i", overlay_p->m_apply_b ? "" : "-", overlay_p->m_editable_b ? "" : "*", overlay_p->m_name.as_cstr(), overlay_p->m_path.as_cstr(), overlay_p->m_path_depth);
      }

    m_ini_proj.set_value(entry_str, overlay_key, g_ini_section_overlays_p);
    }
  }

//---------------------------------------------------------------------------------------
// Gets rid of the previous overlays, changes to the new ones supplied, saves
//             them to the configuration (ini) file, and reparses the code using the new
//             overlays.
// Arg         overlays - new overlays to copy
// Author(s):   Conan Reis
void SkCompiler::set_overlays(APArray<SkOverlay> & overlays)
  {
  SkOverlay ** overlay_pp     = overlays.get_array();
  SkOverlay ** overlay_end_pp = overlay_pp + overlays.get_length();

  // Remove previous overlays
  m_overlays.free_all();

  for (; overlay_pp < overlay_end_pp; overlay_pp++)
    {
    m_overlays.append(*new SkOverlay(**overlay_pp));
    }

  save_overlays();

  // Reparse using new overlays
  reparse();
  }

//---------------------------------------------------------------------------------------
// Check if any overlay has its editable property set
bool SkCompiler::have_any_editable_overlays() const
  {
  for (auto overlay_p : m_overlays)
    {
    if (overlay_p->m_editable_b)
      {
      return true;
      }
    }

  return false;
  }

//---------------------------------------------------------------------------------------
// Gets the file to save and load the SkookumScript compiled binary from the ini file.
// 
// Returns: File name to use for the compiled binary
AFile SkCompiler::get_ini_compiled_file()
  {
  AFile empty;

  return m_ini_proj.get_value_file_default(
    empty,
    g_ini_key_compile_to_p,
    g_ini_section_output_p);
  }

//---------------------------------------------------------------------------------------
// Gets the file to save and load the SkookumScript compiled binary from the
//             ini file.
// Returns:    File name to use for the compiled binary
// Arg         loading - true if loading the file, false if saving the file
// Modifiers:   virtual - Overridden from SkConsoleBase
// Author(s):   Conan Reis
AFile SkCompiler::get_ini_compiled_file_query(
  bool loading_b // = true
  )
  {
  return (m_console_p)
    ? m_console_p->get_ini_compiled_file_query(loading_b)
    : get_ini_compiled_file();
  }

//---------------------------------------------------------------------------------------
// Sets the file to save and load the SkookumScript compiled binary from the
//             ini file.
// Returns:    File name to use for the compiled binary
// Arg         loading - true if loading the file, false if saving the file
// Modifiers:   static
// Author(s):   Conan Reis
void SkCompiler::set_ini_compiled_file(const AFile & file)
  {
  AFile comp_file(file);

  // Ensure correct extension
  comp_file.set_extension(g_file_ext_compiled);

  m_ini_proj.set_value_file_rel(comp_file, g_ini_key_compile_to_p, g_ini_section_output_p);
  }

//---------------------------------------------------------------------------------------
// Determines (by looking at the ini file) whether compiled binary should be
//             loaded at start-up rather than parsing the code files.
// Returns:    true if compiled binary should be loaded on start-up, false if code should
//             be used at start up.
// Modifiers:   static
// Author(s):   Conan Reis
eSkLoad SkCompiler::get_ini_code_load()
  {
  AString load_str(get_ini_ide().get_value_default(
    g_ini_code_load_def,
    g_ini_key_code_load_p,
    g_ini_section_compiler_p));

  switch (load_str.get_first())
    {
    // binary
    case 'b':
    case 'B':
      return SkLoad_binary;

    // script
    case 's':
    case 'S':
      return SkLoad_script;
    }

  return SkLoad_newer;
  }

//---------------------------------------------------------------------------------------
// Gets the current project name
AString SkCompiler::get_ini_project_name()
  {
  return m_ini_proj.get_value_default(
    AString::ms_empty,
    "ProjectName",
    g_ini_section_project_p);
  }

//---------------------------------------------------------------------------------------
// Checks if the current project is editable by looking for the "Editable" entry
// If no Editable entry is present, assume it's editable
bool SkCompiler::get_ini_project_editable()
  {
  return m_ini_proj.get_value_bool_default(
    true, 
    "Editable", 
    g_ini_section_project_p);
  }

//---------------------------------------------------------------------------------------
// Checks if the current project can be made editable by looking for the "CanMakeEditable" entry
// If no CanMakeEditable entry is present, assume it can not be made editable
bool SkCompiler::get_ini_project_can_make_editable()
  {
  return m_ini_proj.get_value_bool_default(
    false,
    "CanMakeEditable",
    g_ini_section_project_p);
  }

//---------------------------------------------------------------------------------------
// Gets class for the startup `Mind` object. Default is the `Master` `Mind` class.
AString SkCompiler::get_ini_startup_mind()
  {
  return m_ini_proj.get_value_default(
    "Master",
    g_ini_key_startup_mind_p,
    g_ini_section_project_p);
  }

//---------------------------------------------------------------------------------------
// Sets class for the startup `Mind` object.
void SkCompiler::set_ini_startup_mind(const AString & mind_class_str)
  {
  m_ini_proj.set_value(
    mind_class_str,
    g_ini_key_startup_mind_p,
    g_ini_section_project_p);
  }

//---------------------------------------------------------------------------------------
// Display command-line help
// See:        cmd_arg*() methods
// Modifiers:   static
// Author(s):   Conan Reis
void SkCompiler::cmd_args_help()
  {
  SkDebug::print_agog(
    "\n----------------------------------\n"
    "SkookumScript Compiler Command-line Arguments\n"
    "----------------------------------\n\n"
    "  -id[~]    Object ID validation.\n"
    "              '-id' indicates validation performed even for deferred object IDs - i.e.\n"
    "                 object_id_validate in meta class info set to 'exist' or 'defer'\n"
    "              '-id~' indicates *no* validation performed for deferred object IDs - i.e.\n"
    "                 object_id_validate in meta class info set to 'exist' or 'defer'\n"
    "                 [This is the default setting for the compiler.]"
    "  -? or -h  Displays this help blurb\n\n"
    "  [Switches may use forward slash (/) rather than dash (-) if desired.]\n\n");
  }

//---------------------------------------------------------------------------------------
// Parses single command-line argument
// Returns:    true if arg parsed successfully and false if not
// Arg         cmd_str - full command-line string
// Arg         idx_begin - character index position to start parse
// Arg         idx_end_p - address to store character index position where parse ended
// See:        cmd_arg*() methods
// Modifiers:   static
// Author(s):   Conan Reis
bool SkCompiler::cmd_arg_parse(
  const AString & cmd_str,
  uint32_t        idx_begin,
  uint32_t *          idx_end_p
  )
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Scan past any whitespace
  uint32_t     length     = cmd_str.get_length();
  const char * cmd_cstr_p = cmd_str.as_cstr();

  cmd_str.find(ACharMatch_not_white_space, 1u, &idx_begin, idx_begin);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure a reasonable number of chars left to parse
  if (idx_begin >= length)
    {
    return false;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure arg starts with - or /
  char ch = cmd_cstr_p[idx_begin];

  if ((ch != '-') && (ch != '/'))
    {
    return false;
    }

  uint32_t arg_start = idx_begin;

  idx_begin++;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Determine next argument switch type
  ch = cmd_cstr_p[idx_begin];

  bool success = true;

  switch (ch)
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case '?':
    case 'h':
    case 'H':
      // Help
      cmd_args_help();
      // Skip rest of args
      idx_begin = length;
      break;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case 'i':
      if (cmd_cstr_p[idx_begin + 1u] == 'd')
        {
        // Validate object IDs
        idx_begin += 2u;

        bool validate_ids = true;

        if (cmd_cstr_p[idx_begin] == '~')
          {
          idx_begin++;
          validate_ids = false;
          }
        else
          {
          if (is_flags(Flag_compile_only))
            {
            SkDebug::print_agog("\nSkookumScript Object ID validation pass\n");
            }
          }

        enable_flags(Flag_object_ids_validate, validate_ids);
        break;
        }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    default:
      success = false;
    }

  if (idx_end_p)
    {
    *idx_end_p = success ? idx_begin : arg_start;
    }

  return success;
  }

//---------------------------------------------------------------------------------------
// Run command-line arguments
// Arg         cmd_str - full command-line string
// See:        cmd_args_execute_auto(), on_drag_drop(), cmd_arg*() methods
// Modifiers:   static
// Author(s):   Conan Reis
void SkCompiler::cmd_args_execute(const AString & cmd_str)
  {
  // Note that whitespace should already be cropped off.
  if (cmd_str.is_empty())
    {
    return;
    }

  //SkDebug::print_agog(a_cstr_format("\nCommand-line arguments:\n  %s\n\n", cmd_str.as_cstr()));

  bool arg_error  = false;
  uint32_t cmd_length = cmd_str.get_length();
  uint32_t idx        = 0u;

  // $Revisit - CReis Check all command-line arguments before any are executed?
  while ((idx < cmd_length) && cmd_arg_parse(cmd_str, idx, &idx))
    {
    }

  arg_error = (idx != cmd_length);

  if (arg_error)
    {
    SkDebug::print_agog(
      a_cstr_format("\nError in Skookum Compiler command-line arguments!\n  %s\n\n", cmd_str.as_cstr() + idx),
      SkLocale_ide,
      SkDPrintType_error);

    cmd_args_help();
    }
  }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Internal Class Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
// Gets the path for the main configuration file.
// [Also sets ms_compiler_p since it is called early on.]
// 
// Returns: project configuration file path
AFile SkCompiler::get_ini_file_main()
  {
  // $Revisit - CReis This could be passed as a exec argument to make it easy to use the
  // same executable for different projects.
   
  ms_compiler_p = this;

  // Make default main ini file name.  It will be "Skookum32.ini" or "Skookum64.ini".
  AFile ini_file(AApplication::ms_this_app_p->get_file());

  // Make common main ini file - unique for each bit platform
  ini_file.set_name(g_ini_file_name_main_p);

  // If ini does not exist in same directory as app look for it in parent directory.
  if (!ini_file.is_existing())
    {
    AString parent_path(ini_file.get_directory().get_parent_path());

    if (parent_path.is_filled())
      {
      AFile parent_file(parent_path, ini_file.get_name());

      if (parent_file.is_existing())
        {
        // Switch to ini file in parent directory
        ini_file = parent_file;
        }
      }
    }

  AIni alias_test(ini_file);

  if (alias_test.attempt_alias())
    {
    ini_file = alias_test.get_file();
    }

  return ini_file;
  }

//---------------------------------------------------------------------------------------
AFile SkCompiler::get_ini_file_proj_default()
  {
  // Make default project file name for case when it is not in the config file.
  AFile ini_file("Skookum-default-proj.ini");

  // Get project file from config file
  ini_file = m_ini_main.get_value_file_default(
      ini_file,
      g_ini_key_project_default_p,
      g_ini_section_settings_p);

  A_VERIFYX(
    ini_file.is_existing(),
    AErrMsg(
      a_str_format(
        "Could not find the default SkookumScript project file!\n\n"
        "The '%s' configuration file has a '[%s] %s' key that references the file '%s' which does not exist!\n\n"
        "[Please ensure file is present and up-to-date or file referenced in the configuration file is correct.]",
        m_ini_main.as_cstr(),
        g_ini_section_settings_p,
        g_ini_key_project_default_p,
        ini_file.as_cstr()),
      AErrLevel_fatal,
      "SkookumScript Default Project File Missing!"))

  return ini_file;
  }

//---------------------------------------------------------------------------------------
// Gets the path for the IDE user preferences configuration file.
// Returns:    IDE user preferences configuration file path
// Author(s):   Conan Reis
AFile SkCompiler::get_ini_file_user()
  {
  // Make default project file name for case when it is not in the config file.
  AFile ini_file("Skookum-user.ini");

  // Get project file from config file
  ini_file = m_ini_main.get_value_file_default(
      ini_file,
      g_ini_key_user_p,
      g_ini_section_settings_p);

  return ini_file;
  }

//---------------------------------------------------------------------------------------
// Gets the "strict" compile setting from the project configuration file.
// Returns:    "strict" compile setting
// Author(s):   Conan Reis
bool SkCompiler::get_ini_strict()
  {
  return m_ini_proj.get_value_bool_default(
    true, g_ini_key_strict_p, g_ini_section_project_p);
  }

//---------------------------------------------------------------------------------------
// Sets the "strict" compile setting in the project configuration file.
// Author(s):   Conan Reis
void SkCompiler::set_ini_strict(bool strict)
  {
  m_ini_proj.set_value_bool(
    strict, g_ini_key_strict_p, g_ini_section_project_p);
  }
