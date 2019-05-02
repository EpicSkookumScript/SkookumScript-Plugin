// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumIDE Application
//
// SkookumScript IDE Console (Workbench window) & supporting classes
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumIDE/SkConsole.hpp>
#include <SkookumIDE/SkClassBrowser.hpp>
#ifndef SK_NO_RESOURCES
  #include <SkookumIDE/SkookumIDE_Res.h>
#endif
#include <SkookumIDE/SkVersionText.hpp>
#include <AgogCore/AMethod.hpp>
#include <AgogCore/ASymbolTable.hpp>
#include <SkookumScript/SkExpressionBase.hpp>
#include <SkookumScript/SkInvokedMethod.hpp>
#include <SkookumScript/SkMind.hpp>
#include <AgogIO/AClipboard.hpp>
#include <AgogIO/ADirectory.hpp>
#include <AgogGUI/ATrueTypeFont.hpp>
#include <AgogGUI_OS/ADialogOS.hpp>
#include <AgogGUI_OS/APopMenuOS.hpp>
#include <Mmsystem.h>  // uses: PlaySound()
#include <shellapi.h>  // Uses ShellExecute()
#include <RichEdit.h>

#pragma warning( push )
 #pragma warning( disable : 4458 ) // hidden class member
 #include <gdiplus.h>
#pragma warning( pop )


//=======================================================================================
// Local Global Structures
//=======================================================================================

namespace
{

  // Enumerated constants
  enum
    {
    SkConsole_status_inset             = 2,
    SkConsole_status_inset2            = SkConsole_status_inset * 2,
    SkConsole_status_offset            = 2,
    SkConsole_border_offset            = 2,   // Sunken border offset
    SkIncrementalSearchEditBox_spacing = 6,
    SkConsole_image_width              = 16,
    SkConsole_dots_per_line_max        = 75,
    SkConsole_classes_width_spacing    = 75,  // Spacing unit = 1/2 average character height
    SkConsole_classes_height_spacing   = 69,
    SkConsole_about_width_spacing      = 50
    };

  enum eSkCmdFlag
    {
    SkCmdFlag_foreground    = 1 << 0,
    SkCmdFlag_log           = 1 << 1,
    SkCmdFlag_class         = 1 << 2,
    SkCmdFlag_member        = 1 << 3,
    SkCmdFlag_suppres_help  = 1 << 4,
    SkCmdFlag__view_mask = SkCmdFlag_log | SkCmdFlag_class | SkCmdFlag_member
    };

  const bool SkConsole_show_browser_def      = true;
  const bool SkConsole_play_sounds_def       = true;

  // *IDE* config (.ini) file

  const char * g_ini_section_project_p          = "Project";
  const char * g_ini_key_load_last_project_p    = "LoadLastProject";
  const char * g_ini_key_last_project_p         = "LastProject";
  const char * g_ini_key_last_default_project_p = "LastDefaultProject";

  const char * g_ini_section_console_p       = "Script Console";
  const char * g_ini_key_split_ratio_p       = "SplitRatio";
  const char * g_ini_key_split_orient_p      = "SplitOrient";
  const char * g_ini_key_edit_font_p         = "EditFont";
  const char * g_ini_key_edit_font_size_p    = "EditFontSize";
  const char * g_ini_key_code_list_font_p    = "CodeListFont";
  const char * g_ini_key_code_list_size_p    = "CodeListFontSize";
  const char * g_ini_key_locale_right_alt    = "KeyboardLocaleUsesRightAltChars";
  const char * g_ini_key_play_sounds_p       = "PlaySounds";
  const char * g_ini_key_sound_open_p        = "SoundOpen";
  const char * g_ini_key_sound_close_p       = "SoundClose";
  const char * g_ini_key_sound_error_p       = "SoundError";
  const char * g_ini_key_sound_breakpoint_p  = "SoundBreakpoint";
  const char * g_ini_key_scheme_file_p       = "SyntaxScheme";
  const char * g_ini_key_workspace_file_p    = "WorkspaceFile";
  const char * g_ini_key_online_mode_p       = "OnlineMode";
  const char * g_ini_key_online_menu_p       = "OnlineMenu";
  const char * g_ini_key_remote_update_p     = "RemoteUpdate";
  const char * g_ini_key_error_dialog_p      = "CompileErrorDialog";

  const char * g_ini_section_search_p             = "Incremental Search";
  const char * g_ini_search_key_width_p           = "Width";
  const char * g_ini_search_key_case_sensitive_p  = "CaseSensitive";
  const char * g_ini_search_key_fuzzy_p           = "FuzzySearch";

  const char * g_ini_section_tooltip_p              = "ToolTip";                  //  Note: Same as in SkNavigationView.cpp   $Revisit: JStenersen - INI settings should be in a global INI namespace) since the .ini file is a global thingy.
  const char * g_ini_tooltip_enable_on_startup_p    = "ToolTipEnableOnStartup";   //  Note: Same as in SkNavigationView.cpp   $Revisit: JStenersen - INI settings should be in a global INI namespace) since the .ini file is a global thingy.
  const char * g_ini_tooltip_enable_log_p           = "ToolTipEnableLog";
  const char * g_ini_tooltip_enable_workspace_p     = "ToolTipEnableWorkSpace";

  const char * g_ini_section_auto_complete_p        = "AutoComplete";
  const char * g_ini_auto_complete_active_p         = "AutoCompleteActive";
  const char * g_ini_auto_complete_key_length_min_p = "AutoCompleteKeyLenghtMin";

  const char * g_ini_section_sbrowser_p      = "Script Browser";
  const char * g_ini_key_show_browser_p      = "ShowBrowser";
  const char * g_ini_key_version_control_p   = "VersionControl";

  const char * g_compiled_ext_filter_p       = "Skookum Compiled Binary (*.sk-bin)\0*.sk-bin\0";
  const char * g_project_ext_filter_p        = "Skookum Project (*.ini)\0*.ini\0";

  const char * g_ini_edit_font_def_p         = "Consolas";
  const int    g_ini_edit_font_size_def      = 11;  // Point Size (not pixel size)
  const char * g_ini_code_list_font_def_p    = "Lucida Console";
  const int    g_ini_code_list_font_size_def = 10;  // Point Size (not pixel size)

  const char * g_ini_section_view_settings_p      = "View Settings";
  const char * g_ini_key_disassembly_p            = "Disassembly";
  const char * g_ini_key_expression_guide_p       = "ExpressionGuide";
  const char * g_ini_key_auto_parse_p             = "AutoParse";
  const char * g_ini_key_auto_parse_sel_p         = "AutoParseSelection";
  const char * g_ini_key_syntax_highlight_p       = "SyntaxHighlight";
  const char * g_ini_key_current_line_highlight_p = "CurrentLineHighlight";



  // *Project* config (.ini) file
  const char * g_ini_key_startup_mind_p        = "StartupMind";


  AString  g_ini_workspace_file_def;  // Default workspace text file
  AString  g_ini_compiled_file_def;
  AString  g_ide_title;
  AString  g_ini_sound_open_def;
  AString  g_ini_sound_error_def;
  AString  g_ini_sound_close_def;
  AString  g_ini_sound_breakpoint_def;
  AString  g_ini_scheme_def;

  AString  g_overlay_text;
  AString  g_compiled_path_text;
  AString  g_eval_mind_text;

  static const AColor g_color_bg(               0.18f, 0.18f, 0.22f);      // Pro
  static const AColor g_color_text_bg(          0.15f, 0.15f, 0.19f);      // Pro Edit partial
  static const AColor g_color_text_edit_bg(     0.13f, 0.13f, 0.16f);      // Pro Edit
  static const AColor g_color_text_current_line(1.0f,  1.0f,  1.0f, 0.1f); // Current line colour

  const AColor SkLog_colour_bground(g_color_bg);
  const AColor SkLog_colour_title(  1.0f,  0.87f, 0.0f,  1.0f);  // #ffdd00 yellow
  const AColor SkLog_colour_note(   0.1f,  1.0f,  0.2f,  1.0f);  //         keen green
  const AColor SkLog_colour_system( 0.53f, 0.73f, 1.0f,  1.0f);  // #88bbff electric blue
  const AColor SkLog_colour_error(  1.0f,  0.0f,  0.5f,  1.0f);  //         red
  const AColor SkLog_colour_warning(1.0f,  0.61f, 0.2f,  1.0f);  // #ff9b32 orange
  const AColor SkLog_colour_result( 1.0f,  1.0f,  0.25f, 1.0f);  //         light yellow
  const AColor SkLog_colour_trace(  0.75f, 0.75f, 1.0f,  1.0f);  //         lavender

} // End unnamed namespace


typedef AMethodArg<SkOverlayList, tSkOverlaySubText *> tSkOverlayTextCall;


//=======================================================================================
// SkContextInfo Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// true if the info objects would refer to the same file, false if not
// Author(s):   Conan Reis
bool SkContextInfo::compare_files(const SkMemberInfo & info) const
  {
  // return as_file_title() == info.as_file_title();

  return ((m_type == SkMember_data) && (m_type == info.m_type))
    ? (m_class_scope == info.m_class_scope) && (m_member_id.get_scope() == info.m_member_id.get_scope())
    : operator==(info);
  }

//---------------------------------------------------------------------------------------
// Converts this identifier ref to the file that it represents by searching
//             through overlays to find it.
// Returns:    A file that this identifier ref is based on.  If is_titled() called on the
//             result returns false then a file could not be found - as_file_create() can
//             be used instead.
// Author(s):   Conan Reis
AFile SkContextInfo::as_file_existing() const
  {
  AFile ident_file;

  if (SkCompiler::ms_compiler_p->find_member_file(*this, &ident_file) == nullptr)
    {
    ident_file.empty_file_str();
    }

  return ident_file;
  }

//---------------------------------------------------------------------------------------
// Converts this identifier ref to the file that it would represents using
//             the specified overlay set in the class browser.
// Returns:    A file that this identifier ref would be based on for the working overlay.
// Author(s):   Conan Reis
AFile SkContextInfo::as_file_create(const SkOverlay & overlay) const
  {
  // Start with working overlay path
  AString path(overlay.get_path_class(*get_class()));

  // Add member filename
  path.append(as_file_title(PathFlag__file));

  return path;
  }

//---------------------------------------------------------------------------------------
// Action: focus on this identifier in the class browser
// Author(s):   Conan Reis
void SkContextInfo::action_goto_browser() const
  {
  SkClassBrowser * browser_p = SkConsole::ms_console_p->display_browser();

  switch (m_type)
    {
    case SkMember_class_meta:
      browser_p->set_class(m_member_id.get_scope());
      browser_p->get_class_tree().set_focus();
      break;

    default:
      browser_p->set_member(*this);
      browser_p->focus_editor();
    }
  }

//---------------------------------------------------------------------------------------
// Opens the directory of the file this identifier ref represents with the
//             Window's Explorer and selects the file.
// See:        Other  action_* commands.
// Author(s):   Conan Reis
void SkContextInfo::action_goto_file_explorer() const
  {
  AFile source_file(as_file_existing());

  if (source_file.is_titled())
    {
    SkDebug::print(a_str_format("\nFile: %s\n  - opening in Windows Explorer...\n", source_file.as_cstr()));

    AFile explorer("explorer.exe");

    explorer.execute(a_str_format("/e, /select,\"%s\"", source_file.as_cstr()));
    }
  else
    {
    SkDebug::print(a_str_format("\nCannot explore file: %s\n  - it does not exist!\n\n", source_file.as_cstr()));
    }
  }

//---------------------------------------------------------------------------------------
// Opens the file this identifier ref represents with editor that is
//             associated with its extension. 
// See:        Other  action_* commands.
// Author(s):   Conan Reis
void SkContextInfo::action_edit_external() const
  {
  if (is_valid())
    {
    AFile source_file(as_file_existing());

    if (source_file.is_titled())
      {
      SkDebug::print(a_str_format("\nFile: '%s'\n  - opening in editor associated with '%s' extension...\n", source_file.as_cstr(), source_file.get_extension().as_cstr()));
      source_file.execute();
      }
    else
      {
      SkDebug::print(a_str_format("\nCannot open file: '%s'\n  - it does not exist!\n\n", source_file.as_cstr()));
      }
    }
  }

//---------------------------------------------------------------------------------------
// Copies the name of this member/class to the clipboard.
// See:        Other  action_* commands.
// Author(s):   Conan Reis
void SkContextInfo::action_copy_name() const
  {
  AString name_str((m_type == SkMember_class_meta)
    ? m_member_id.get_scope()->get_name_str()
    : m_member_id.get_name_str());

    {
    AClipboard clip(SkConsole::ms_console_p);

    clip.set_text(name_str);
    }

  SkDebug::print(a_str_format("\nCopied \"%s\" to the clipboard\n", name_str.as_cstr()));
  }

//---------------------------------------------------------------------------------------
// Copies the file string that represents this member/class to the clipboard
//             associated with its extension. 
// See:        Other  action_* commands.
// Author(s):   Conan Reis
void SkContextInfo::action_copy_path() const
  {
  AString file_str(as_file_existing().get_file_str());

    {
    AClipboard clip(SkConsole::ms_console_p);

    clip.set_text(file_str);
    }

  SkDebug::print(a_str_format("\nCopied \"%s\" to the clipboard\n", file_str.as_cstr()));
  }

//---------------------------------------------------------------------------------------
// Inserts the name of this member/class in the selection point of the Class
//             Browser edit window.
// See:        Other  action_* commands.
// Author(s):   Conan Reis
void SkContextInfo::action_insert_name_editor() const
  {
  AString name_str((m_type == SkMember_class_meta)
    ? m_member_id.get_scope()->get_name_str()
    : m_member_id.get_name_str());

  SkClassBrowser * browser_p = SkConsole::ms_console_p->display_browser();

  browser_p->get_edit_view().get_editor().replace_selection(name_str, true);
  browser_p->focus_editor();
  }

//---------------------------------------------------------------------------------------
// Inserts the name of this member/class in the selection point of the 
//             Console workspace window.
// See:        Other  action_* commands.
// Author(s):   Conan Reis
void SkContextInfo::action_insert_name_workspace() const
  {
  AString name_str((m_type == SkMember_class_meta)
    ? m_member_id.get_scope()->get_name_str()
    : m_member_id.get_name_str());

  SkConsole::ms_console_p->get_workspace().replace_selection(name_str);
  SkConsole::ms_console_p->show();
  SkConsole::ms_console_p->make_foreground();
  SkConsole::ms_console_p->get_workspace().set_focus();
  }

//---------------------------------------------------------------------------------------
// Opens the file this identifier ref represents with editor that is
//             associated with its extension. 
// See:        Other  action_* commands.
// Author(s):   Conan Reis
void SkContextInfo::action_p4_checkout() const
  {
  if (!is_valid())
    {
    return;
    }

  AFile source_file(as_file_existing());

  if (source_file.is_titled())
    {
    if (source_file.is_read_only())
      {
      SkDebug::print(a_str_format("\nFile: %s\n  - checking out from Perforce...", source_file.as_cstr()));

      if (source_file.p4_checkout(true))
        {
        SkDebug::print(" done!\n\n");
        }
      else
        {
        SkDebug::print(" failed or still working on it!\n\n");
        }
      }
    else
      {
      SkDebug::print(a_str_format("\nFile: %s\n  - is readable and probably already checked out from Perforce.\n\n", source_file.as_cstr()));
      }
    }
  else
    {
    SkDebug::print(a_str_format("\nCannot checkout file: %s\n  - it does not exist!\n\n", source_file.as_cstr()));
    }
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkContextInfo::action_p4_revert() const
  {
  // p4 revert

  A_DPRINT(A_SOURCE_FUNC_STR " - not written yet!\n", this);

  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkContextInfo::action_p4_diff_prev() const
  {
  // Method 1:
  //   p4 print -o "C:\temp\path" -q "C:\local\path"
  //   p4merge -text -C none -nl "//depot/path#2" -nr "C:\local\path (workspace file)" "C:\temp\path" "C:\local\path"
  //
  // Method 2:
  //   p4v -p 172.16.0.2:1666 -c ClientName -u UserName -cmd "prevdiff C:\local\path"

  A_DPRINT(A_SOURCE_FUNC_STR " - not written yet!\n", this);

  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkContextInfo::action_p4_properties() const
  {
  // Properties - can be used to find out who else has the file checked out
  //
  // Method 1:
  //   Parse "p4 fstats"
  //
  // Method 2:
  //   p4v -p 172.16.0.2:1666 -c ClientName -u UserName -cmd "properties C:\local\path"

  A_DPRINT(A_SOURCE_FUNC_STR " - not written yet!\n", this);

  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkContextInfo::action_p4_timelapse() const
  {
  // Timelapse View of revisions
  //   p4v -p 172.16.0.2:1666 -c ClientName -u UserName -cmd "annotate C:\local\path"

  A_DPRINT(A_SOURCE_FUNC_STR " - not written yet!\n", this);

  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkContextInfo::action_p4_history() const
  {
  // History - lists all revisions
  //   p4v -p 172.16.0.2:1666 -c ClientName -u UserName -cmd "history C:\local\path"

  A_DPRINT(A_SOURCE_FUNC_STR " - not written yet!\n", this);

  }

//=======================================================================================
// SkEditBox Class Data
//=======================================================================================

AString SkEditBox::ms_result;


//=======================================================================================
// SkEditBox Methods
//=======================================================================================

//---------------------------------------------------------------------------------------

void SkEditBox::initialize()
  {
  }

//---------------------------------------------------------------------------------------

void SkEditBox::deinitialize()
  {
  ms_result = AString::ms_empty;
  }

//---------------------------------------------------------------------------------------
// Constructor
// Author(s):   Conan Reis
SkEditBox::SkEditBox(
  AWindow *       parent_p,
  const AString & initial,  // = AString::ms_empty
  const AFont &   font,     // = AFont::ms_fixed
  uint32_t        flags     // = RichFlag_show_selection_always
  ) :
  ARichEditOS(parent_p, initial, flags, font)
  {
  // Set custom word-break callback.
  ::SendMessage(m_os_handle, EM_SETWORDBREAKPROC, 0u, reinterpret_cast<LPARAM>(word_break_callback));

  enable_subclass_messages();
  set_tabs_fixed_spaces(SkDebug::ms_tab_stops);

  }

//---------------------------------------------------------------------------------------
// Get identifier/string/selection context at caret position.
//
// See Also  word_break_callback(), SkParser: :identify_member_name()
// #Author(s) Conan Reis
eSkMatchKind SkEditBox::caret_context(
  // optional match criteria to fill (ignored if nullptr)
  SkMatchCriteria * match_info_p, // = nullptr
  // optional beginning character index address (ignored if nullptr)
  uint32_t * begin_idx_p, // = nullptr
  // optional ending character index address (ignored if nullptr)
  uint32_t * end_idx_p // = nullptr
  ) const
  {
  SkMatchCriteria match_info;

  match_info.m_class_match_type  = AStrMatch_subpart;
  match_info.m_member_match_type = AStrMatch_subpart;

  uint32_t caret_idx = get_caret_index();
  uint32_t caret_row = get_row_from_index(caret_idx);
  AString  row_str(get_row(caret_row));
  uint32_t row_length = row_str.get_length();

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ignore if empty
  if (row_length == 0u)
    {
    if (match_info_p)
      {
      *match_info_p = match_info;
      }

    if (begin_idx_p)
      {
      *begin_idx_p = caret_idx;
      }

    if (end_idx_p)
      {
      *end_idx_p = caret_idx;
      }

    return SkMatchKind__invalid;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Find identifiers to look up
  uint32_t     begin_idx;
  uint32_t     end_idx;
  uint32_t     caret_row_idx = a_min(get_row_index_from_index(caret_idx), row_length - 1u);
  const char * row_cstr_p    = row_str.as_cstr();
  char         ch            = row_cstr_p[caret_row_idx];
  eSkMatchKind match_kind    = SkMatchKind__invalid;


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Get on identifier
  if (ch == '!')
    {
    caret_row_idx++;

    if (row_cstr_p[caret_row_idx] == '!')
      {
      caret_row_idx++;
      }
    }
  else
    {
    if ((ch != '?') || AString::ms_char_match_table[ACharMatch_not_identifier][uint8_t(ch)])
      {
      row_str.find(ACharMatch_identifier, 1u, &caret_row_idx, caret_row_idx);
      }
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Find end of identifier
  begin_idx = 0u;
  end_idx   = row_str.get_length();
  row_str.find(ACharMatch_not_identifier, 1u, &end_idx, caret_row_idx);

  // extra check for predicate symbol '?'
  if (row_cstr_p[end_idx] == '?')
    {
    end_idx++;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Find beginning of identifier
  if (row_str.find_reverse(
    ACharMatch_not_identifier, 1u, &begin_idx, 0u, (caret_row_idx == row_length) ? row_length - 1u : caret_row_idx))
    {
    begin_idx++;
    }

  bool ctor_b   = false;  // or dtor - any method starting with !
  bool data_b   = false;
  bool scoped_b = false;
  uint32_t sym_id   = ASymbol_id_null;

  ch = begin_idx ? row_cstr_p[begin_idx - 1u] : '\0';

  switch (ch)
    {
    case '@':
      begin_idx--;
      data_b = true;

      if (begin_idx)
        {
        ch = row_cstr_p[begin_idx - 1u];

        if (AString::ms_char_match_table[ACharMatch_identifier][uint8_t(ch)])
          {
          // Scope operator
          begin_idx++;
          scoped_b = true;
          data_b   = false;
          }
        else
          {
          if (ch == '@')
            {
            // Class member
            begin_idx--;
            }
          }
        }
      break;

    case '!':
      // Could be create temporary, instantiate or constructor/destructor
      if (begin_idx)
        {
        ch = (begin_idx >= 2u) ? row_cstr_p[begin_idx - 2u] : ' ';

        if (ch == '!')
          {
          // Destructor
          begin_idx -= 2u;
          ctor_b = true;
          }
        else
          {
          if (!AString::ms_is_space[uint8_t(ch)])
            {
            // *Try* to determine if it is a constructor or method call on an instantiate.
            sym_id = ASYMBOL_CSTR_TO_ID(row_cstr_p + begin_idx - 1u, end_idx - begin_idx + 1u);

            if (ASYMBOL_IS_REFFED(sym_id))
              {
              // It is a bit of a guess - however a constructor exists with that name in
              // the symbol table, so seems to be a constructor method.
              begin_idx--;
              ctor_b = true;
              }
            }
          }
        }
      break;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Determine type of context
  uint32_t idx;
  char prior_ch1 = (begin_idx > 0u) ? row_cstr_p[begin_idx - 1u] : '\0';
  char prior_ch2 = (begin_idx > 1u) ? row_cstr_p[begin_idx - 2u] : '\0';

  ch = row_cstr_p[begin_idx];

  if (AString::ms_is_uppercase[uint8_t(ch)])
    {
    if ((prior_ch1 == '.') || ((prior_ch1 == '>') && (prior_ch2 == '>')))
      {
      // Looks like a conversion method
      match_kind = SkMatchKind_routines;
      }
    else
      {
      match_kind = SkMatchKind_classes;
      row_str.get(&match_info.m_class_match_str, begin_idx, end_idx - begin_idx);
      }
    }

  if (match_kind == SkMatchKind__invalid)
    {
    match_kind = ctor_b
      ? SkMatchKind_routines
      : (data_b ? SkMatchKind_data : SkMatchKind_members);

    // Cannot use '(' at the end of identifier to determine if it is a routine since
    // routines with zero arguments can omit brackets and invoke () operator can be used
    // on variables.
     
    // Check for scope
    if (scoped_b)
      {
      uint32_t class_idx = 0u;

      idx = begin_idx - 1u;

      if (row_str.find_reverse(ACharMatch_not_identifier, 1u, &class_idx, 0u, idx - 1u))
        {
        class_idx++;
        }

      // Store class identifier
      row_str.get(&match_info.m_class_match_str, class_idx, idx - class_idx);
      }
    }

  if (match_kind != SkMatchKind_classes)
    {
    row_str.get(&match_info.m_member_match_str, begin_idx, end_idx - begin_idx);
    }

  if (match_info.m_class_match_str.is_empty())
    {
    // Check for class member context
    if ((begin_idx > 2u)
      && (ctor_b || (prior_ch1 == '.')))
      {
      uint32_t class_idx = 0u;

      idx = begin_idx - (ctor_b ? 1u : 2u);

      if (row_str.find_reverse(ACharMatch_not_identifier, 1u, &class_idx, 0u, idx))
        {
        class_idx++;
        }

      if (AString::ms_is_uppercase[uint8_t(row_cstr_p[class_idx])])
        {
        begin_idx = class_idx;
        row_str.get(&match_info.m_class_match_str, begin_idx, (idx + 1u - begin_idx));
        }
      }
    }

  if (match_info_p)
    {
    match_info_p->m_class_match_str = match_info.m_class_match_str;
    match_info_p->m_class_match_type  =
      match_info.m_class_match_str.is_filled() ? AStrMatch_exact : AStrMatch_subpart;
    match_info_p->m_member_match_str = match_info.m_member_match_str;
    match_info_p->m_member_match_type =
      match_info.m_member_match_str.is_filled() ? AStrMatch_exact : AStrMatch_subpart;
    }

  if (begin_idx_p)
    {
    *begin_idx_p = begin_idx;
    }

  if (end_idx_p)
    {
    *end_idx_p = end_idx;
    }

  return match_kind;
  }

//---------------------------------------------------------------------------------------
// Sets the default font for this window.
// Also ensures that correct tab stops are set whenever font is adjusted.
// 
// Examples:   `window.set_font(AFont("Arial", 11.0f));`
// Modifiers:   Virtual - override for custom behaviour
// See:         constructor, get_font(), reset_font()
// Author(s):   Conan Reis
void SkEditBox::set_font(const AFont & font)
  {
  if (font.m_sys_font_p != m_font.m_sys_font_p)
    {
    ARichEditOS::set_font(font);
    set_tabs_fixed_spaces(SkDebug::ms_tab_stops);
    refresh();
    }
  }

//---------------------------------------------------------------------------------------
// Executes selected lines or single line that caret is on if no lines selected.
// Arg         locale - indicates where to run the command
// Author(s):   Conan Reis
void SkEditBox::action_evaluate_selected(
  eSkLocale locale // = SkLocale_runtime
  )
  {
  save_settings(SkLog_silent);

  AString code(get_selection_or_row());

  // Add extra space at end to ensure debug printing can reference a bit beyond selection
  code.append(' ');

  // Prep flags
  SkRemoteIDE * remote_p     = SkConsole::ms_console_p->get_remote_ide();
  uint32_t      locale_flags = locale;

  if (locale_flags & SkLocale_runtime)
    {
    locale_flags |= (remote_p->is_remote_runtime() ? SkLocale_local : SkLocale_remote);
    }

  if (locale_flags & SkLocale_ide)
    {
    locale_flags |= (remote_p->is_remote_ide() ? SkLocale_local : SkLocale_remote);
    }

  if (locale_flags & SkLocale_embedded)
    {
    locale_flags = SkLocale_local;
    }

  // Run remotely?
  if (locale_flags & SkLocale_remote)
    {
    remote_p->cmd_invoke(code);
    }

  // Run locally?
  if (locale_flags & SkLocale_local)
    {
    if (SkParser::invoke_script(code, &ms_result) == SkParser::Result_ok)
      {
      AString log_str(nullptr, ms_result.get_length() + 3u, 0u);

      log_str.append('\n');
      log_str.append(ms_result);
      log_str.append('\n');
      SkDebug::print(log_str, SkLocale_ide, SkDPrintType_result);
      }
    }
  }

//---------------------------------------------------------------------------------------
// Custom word-break callback - in particular ensures that underscore '_' is treated as
// part of a "word".
//
// #Notes
//   http://msdn.microsoft.com/en-us/library/windows/desktop/hh270412(v=vs.85).aspx
//   http://msdn.microsoft.com/en-us/library/windows/desktop/bb761665(v=vs.85).aspx
//   http://msdn.microsoft.com/en-us/library/windows/desktop/bb761709(v=vs.85).aspx
//   http://support.microsoft.com/kb/109551
//   http://wiredplane-wintools.googlecode.com/svn/trunk/PROJECTS_ROOT/WireNote/NoteEditCtrl.cpp
//
// #Modifiers static
// #See Also  caret_context(), EM_SETWORDBREAKPROC Windows message 
// #Author(s) Conan Reis
int CALLBACK SkEditBox::word_break_callback(
  LPTSTR lpch,
  int ichCurrent,
  int cch,
  int code
  )
  {
  // [It would be nice if you could just get the existing callback and only override what
  // you want and call it for all the default stuff - unfortunately EM_GETWORDBREAKPROC
  // only returns the custom callback.  If you call it when the default callback is in
  // place it just returns nullptr.]

  // $Revisit - CReis This was quickly thrown together and may have some problems.
  // In particular there were dire warnings about making a custom word-break and having it
  // work properly with languages other than English.

  char buffer_p[1024];
  int length = ::WideCharToMultiByte(
    CP_ACP, 0u, reinterpret_cast<LPCWSTR>(lpch), cch, buffer_p, 1024, nullptr, nullptr);
   
  char ch  = buffer_p[ichCurrent];
  char ch0 = ichCurrent ? buffer_p[ichCurrent - 1u] : '\0';
  
  switch (code)
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case WB_CLASSIFY:
      // Retrieves the character class and word break flags of the character at the
      // specified position. This value is for use with rich edit controls.
      enum
        {
        ClassFlag_whitespace  = 0,
        ClassFlag_identifier  = 1,
        ClassFlag_token       = 2
        };

      switch (ch)
        {
        case '?':
          // Object ID or predicate variable
          if (ch0 != '@')
            {
            // predicate variable
            return ClassFlag_identifier;
            }
          break;

        case '@':
          {
          // Object ID, scope operator or data member
          char ch2 = buffer_p[ichCurrent + 1u];

          if ((ch0 == '@')
            || (ch2 == '@')
            || AString::ms_char_match_table[ACharMatch_not_identifier][uint8_t(ch0)])
            {
            // data member
            return ClassFlag_identifier;
            }
          }
          break;

        case '!':
          // create temporary, instantiate or constructor/destructor
          if ((ch0 != '!') || !AString::ms_is_space[uint8_t(ch0)])
            {
            // constructor/destructor
            // [or instantiate with method which isn't an identifier but expensive to detect]
            return ClassFlag_identifier;
            }
          break;

       default:
          if (AString::ms_is_space[uint8_t(ch)])
            {
            return ClassFlag_whitespace | WBF_ISWHITE;
            }

          if (AString::ms_char_match_table[ACharMatch_identifier][uint8_t(ch)])
            {
            return ClassFlag_identifier;
            }
        }
      
      // Else assume it is a token character
      return ClassFlag_token;
 
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case WB_ISDELIMITER:
      // Checks whether the character at the specified position is a delimiter.
      switch (ch)
        {
        case '?':
          // Object ID or predicate variable
          if (ch != '@')
            {
            // predicate variable
            return FALSE;
            }
          break;

        case '@':
          {
          // Object ID, scope operator or data member
          char ch2 = buffer_p[ichCurrent + 1u];

          if ((ch0 == '@')
            || (ch2 == '@')
            || AString::ms_char_match_table[ACharMatch_not_identifier][uint8_t(ch0)])
            {
            // data member
            return FALSE;
            }
          }
          break;

        case '!':
          // create temporary, instantiate or constructor/destructor
          if ((ch0 != '!') || !AString::ms_is_space[uint8_t(ch0)])
            {
            // constructor/destructor
            // [or instantiate with method which isn't an identifier but expensive to detect]
            return ClassFlag_identifier;
            }
          break;

        default:
          return AString::ms_char_match_table[ACharMatch_identifier][uint8_t(ch)]
            ? FALSE : TRUE;
        }

      return FALSE;
 
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case WB_LEFT:
      // Finds the beginning of a word to the left of the specified position.
      // Allow fall-through
      // \/ \/ \/ \/ \/ \/
    case WB_LEFTBREAK:
      // Finds the end-of-word delimiter to the left of the specified position.
      // This value is for use with rich edit controls.
      // Allow fall-through
      // \/ \/ \/ \/ \/ \/
    case WB_MOVEWORDLEFT:
      {
      // Finds the beginning of a word to the left of the specified position.
      // This value is used during CTRL+LEFT key processing.
      // This value is for use with rich edit controls.
      AString str(buffer_p, length, true);
      uint32_t    idx_end = 0u;

      if (ichCurrent)
        {
        ichCurrent--;
        ch  = ch0;
        ch0 = ichCurrent ? buffer_p[ichCurrent - 1u] : '\0';
        }

      bool data_ident_b = false;
      
      if ((ch == '?')
        && AString::ms_char_match_table[ACharMatch_identifier][uint8_t(ch0)])
        {
        data_ident_b = true;
        ichCurrent--; 
        }

      if (ch == '@')
        {
        char ch2 = buffer_p[idx_end + 1u];

        if ((ch2 == '@') || AString::ms_char_match_table[ACharMatch_identifier][uint8_t(ch2)])
          {
          data_ident_b = true;
          }
        }

      if (ch == '!')
        {
        if (ch0 == '!')
          {
          // Destructor
          ichCurrent--;
          }

        return ichCurrent;
        }

      if (data_ident_b
        || AString::ms_char_match_table[ACharMatch_identifier][uint8_t(ch)])
        {
        if (str.find_reverse(ACharMatch_not_identifier, 1u, &idx_end, 0u, ichCurrent))
          {
          idx_end++;
          }

        ch0 = idx_end ? buffer_p[idx_end - 1u] : '\0';

        switch (ch0)
          {
          case '@':
            idx_end--;

            if (idx_end)
              {
              ch0 = buffer_p[idx_end - 1u];

              if (AString::ms_char_match_table[ACharMatch_identifier][uint8_t(ch0)])
                {
                // Scope operator
                idx_end++;
                }
              else
                {
                if (ch0 == '@')
                  {
                  idx_end--;
                  }
                }
              }
            break;

          case '!':
            // Could be create temporary, instantiate or constructor/destructor
            if (idx_end)
              {
              ch0 = (idx_end >= 2u) ? buffer_p[idx_end - 2u] : ' ';

              if (ch0 == '!')
                {
                // Destructor
                return idx_end - 2;
                }

              if (!AString::ms_is_space[uint8_t(ch0)])
                {
                // Constructor
                // [or instantiate with method which isn't an identifier but expensive to detect]
                idx_end--;
                }
              }
            break;
          }

        return idx_end;
        }

      if (AString::ms_char_match_table[ACharMatch_white_space][uint8_t(ch)])
        {
        if (str.find_reverse(ACharMatch_not_white_space, 1u, &idx_end, 0u, ichCurrent))
          {
          idx_end++;
          }

        return idx_end;
        }

      if (str.find_reverse(ACharMatch_not_token, 1u, &idx_end, 0u, ichCurrent))
        {
        idx_end++;

        if ((buffer_p[idx_end] =='?') && (buffer_p[idx_end - 1u] != '@'))
          {
          idx_end++;
          }
        }

      return idx_end;
      }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case WB_RIGHT:
      // Finds the beginning of a word to the right of the specified position.
      // This is useful in right-aligned edit controls.
      // Allow fall-through
      // \/ \/ \/ \/ \/ \/
    case WB_RIGHTBREAK:
      // Finds the end-of-word delimiter to the right of the specified position.
      // This is useful in right-aligned edit controls.
      // This value is for use with rich edit controls.
      // Allow fall-through
      // \/ \/ \/ \/ \/ \/
    case WB_MOVEWORDRIGHT:
      {
      // Finds the beginning of a word to the right of the specified position.
      // This value is used during CTRL+RIGHT key processing.
      // This value is for use with rich edit controls.
      AString str(buffer_p, length, true);
      uint32_t    idx_end      = length;
      bool    data_ident_b = false;
      char    ch2          = buffer_p[idx_end + 1u];
      
      if (ch == '@')
        {
        // Could be data member, object ID or scope operator
        switch (ch2)
          {
          // Object IDs
          case '\'':
          case '#':
          case '?':
            str.find(ACharMatch_not_token, 1u, &idx_end, ichCurrent + 1u);
            return idx_end;

          case '@':
            data_ident_b = true;
            ichCurrent += 2u;
            break;

          default:
            if (AString::ms_char_match_table[ACharMatch_identifier][uint8_t(ch0)])
              {
              // Scope operator
              return ichCurrent + 1u;
              }

            data_ident_b = true;
            ichCurrent++;
          }
        }

      if (ch == '?')
        {
        // Could be end of predicate identifier or object ID
        if (ch2 != '\'')
          {
          // predicate identifier
          return ichCurrent + 1u;
          }
        }

      if (ch == '!')
        {
        // Could be create temporary, instantiate or constructor/destructor
        if (ch2 == '!')
          {
          // Destructor
          return ichCurrent + 2u;
          }

        if ((ch0 == '\0') || AString::ms_is_space[uint8_t(ch0)])
          {
          // Create temporary
          return ichCurrent + 1u;
          }

        // Constructor
        // [or instantiate with method which isn't an identifier but expensive to detect]
        data_ident_b = true;
        ichCurrent++;
        }

      if (data_ident_b
        || AString::ms_char_match_table[ACharMatch_identifier][uint8_t(ch)])
        {
        str.find(ACharMatch_not_identifier, 1u, &idx_end, ichCurrent);

        // extra check for predicate symbol '?'
        return (buffer_p[idx_end] == '?') ? idx_end + 1u : idx_end;
        }

      if (AString::ms_char_match_table[ACharMatch_white_space][uint8_t(ch)])
        {
        str.find(ACharMatch_not_white_space, 1u, &idx_end, ichCurrent);

        return idx_end;
        }

      str.find(ACharMatch_not_token, 1u, &idx_end, ichCurrent);

      if (idx_end)
        {
        switch (buffer_p[idx_end - 1u])
          {
          case '@':
            idx_end--;

            if (idx_end && (buffer_p[idx_end - 1u] == '@'))
              {
              idx_end--;
              }
            break;

          case '!':
            idx_end--;

            if (idx_end && (buffer_p[idx_end - 1u] == '!'))
              {
              idx_end--;
              }
            break;
          }
        }

      return idx_end;
      }
    }

  return ichCurrent;
  }


//=======================================================================================
// SkEditSyntax Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Constructor
// Author(s):   Conan Reis
SkEditSyntax::SkEditSyntax(
  eType             type,
  AWindow *         parent_p,
  SkIncrementalSearchEditBox::eParentContext  parent_context,
  const AString &   initial, // = AString::ms_empty
  const AFont &     font     // = AFont::ms_fixed
  ) :
  SkEditBox(parent_p, initial, font, (type == Type_single_line) ? RichFlag_single_line : RichFlag_show_selection_always),
  m_type(type),
  m_identify_flags(SkParser::IdentifyFlag__default),
  m_scheme(AColor::Scheme_default),
  m_start_idx_prev(0u),
  m_start_group_prev(m_start_group_prev),
  m_group(m_start_group_prev),
  m_invert_b(false),
  m_incremental_search_editbox(this, parent_context)
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Get syntax style from ini file
  AString style_str = SkCompiler::ms_compiler_p->get_ini_ide().get_value_default(
    g_ini_scheme_def, g_ini_key_scheme_file_p, g_ini_section_console_p);

  if (style_str.get_length() >= 2u)
    {
    style_str.lowercase();

    switch (style_str(0u))
      {
      case 'l':  // assume "light"
        m_scheme = AColor::Scheme_light;
        break;

      case 'd':
        if (style_str(1u) == 'a')  // assume "dark"
          {
          m_scheme = AColor::Scheme_dark;
          }
      }
    }
  }

//---------------------------------------------------------------------------------------
// Return source file currently being edited. Default is no file.
// 
// #Author(s) Conan Reis
const AFile & SkEditSyntax::get_source_file() const
  {
  static AFile s_file;

  return s_file;
  }

//---------------------------------------------------------------------------------------
// Loads file associated with this edit box in an associated external editor.
// Author(s):   Conan Reis
void SkEditSyntax::action_edit_externally()
  {
  const AFile & source_file = get_source_file();
  bool          existing    = source_file.is_titled();

  if (existing)
    {
    // Save settings which may also save the file - possibly creating the file in the process.
    save_settings();
    existing = source_file.is_existing();

    if (existing)
      {
      SkDebug::print(a_str_format("\nFile: '%s'\n  - opening in editor associated with '%s' extension...\n", source_file.as_cstr(), source_file.get_extension().as_cstr()));
      source_file.execute();
      }
    }

  if (!existing)
    {
    SkDebug::print(a_str_format("\nCannot open file: '%s'\n  - it does not exist!\n\n", source_file.as_cstr()), SkLocale_ide, SkDPrintType_warning);
    }
  }

//---------------------------------------------------------------------------------------
// Opens the directory of the file this identifier ref represents with the
//             Window's Explorer and selects the file.
// See:        Other  action_* commands.
// Author(s):   Conan Reis
void SkEditSyntax::action_goto_file_explorer()
  {
  const AFile & source_file = get_source_file();

  save_settings();

  if (source_file.is_titled())
    {
    SkDebug::print(a_str_format("\nFile: %s\n  - opening in Windows Explorer...\n", source_file.as_cstr()));

    AFile explorer("explorer.exe");

    explorer.execute(a_str_format("/e, /select,\"%s\"", source_file.as_cstr()));
    }
  else
    {
    const SkContextInfo * member_p = &SkConsole::ms_console_p->get_browser()->get_member_info();

    // $Revisit - CReis Not all classes have a !Class.sk-meta file yet.
    if (member_p && (member_p->m_type == SkMember_class_meta))
      {
      ADirectory class_dir(source_file.get_path());

      if (class_dir.is_existing())
        {
        ::ShellExecute(nullptr, "explore", class_dir.as_cstr(), nullptr, nullptr, SW_SHOWNORMAL);
        }
      else
        {
        SkDebug::print(a_str_format("\nClass does not have a !Class.sk-meta file and cannot open folder: %s\n  - it does not exist in the working overlay!\n\n", class_dir.as_cstr()));
        }
      }
    else
      {
      SkDebug::print(a_str_format("\nCannot explore file: %s\n  - it does not exist!\n\n", source_file.as_cstr()));
      }
    }
  }

//---------------------------------------------------------------------------------------
// Copies the file string that represents this member/class to the clipboard
//             associated with its extension. 
// See:        Other  action_* commands.
// Author(s):   Conan Reis
void SkEditSyntax::action_copy_path() const
  {
  AString file_str(get_source_file().get_file_str());

    {
    AClipboard clip(SkConsole::ms_console_p);

    clip.set_text(file_str);
    }

  SkDebug::print(a_str_format("\nCopied \"%s\" to the clipboard\n", file_str.as_cstr()));
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkEditSyntax::set_source(
  const AString & str,
  bool            read_only_b // = false
  )
  {
  freeze();
  enable_on_modified(false);

  set_text(str);
  if (SkConsole::ms_console_p->is_syntax_highlight())
    {
    syntax_highlight(Coverage_all);
    }
  else
    {
    syntax_highlight(Coverage_none);
    }
  enable_read_only(read_only_b);

  enable_on_modified();
  unfreeze();
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkEditSyntax::syntax_highlight(
  eCoverage coverage // = Coverage_all
  )
  {
  //  No syntax highlight changes if a compilation is underway.
  if (SkCompiler::ms_compiler_p->get_phase() != SkCompiler::Phase_idle)
    {
    return;
    }

  SkParser parser(get_text());
  uint32_t length = parser.get_length();

  if (length == 0u)
    {
    return;
    }

  enable_on_modified(false);
  freeze();

  // Preserve previous selection
  uint32_t old_start;
  uint32_t old_end;

  get_selection(&old_start, &old_end);

  // Iterate through text setting style
  ATextStyle style;
  uint32_t   start = 0u;
  uint32_t   end   = 0u;

  if (coverage == Coverage_visible)
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Only do syntax highlight for visible area
    uint32_t idx_first = get_index_visible_first();
    uint32_t idx_last  = a_min(get_index_visible_last(), length);

    if (m_start_idx_prev > idx_first)
      {
      m_start_idx_prev   = 0u;
      m_start_group_prev = Group_1;
      }

    start   = m_start_idx_prev;
    m_group = m_start_group_prev;

    do
      {
      get_syntax_style(&style, parser.identify_text(start, &end, m_identify_flags));

      if (end > idx_first)
        {
        set_text_style(start, end, style, false);
        }
      else
        {
        m_start_idx_prev   = end;
        m_start_group_prev = m_group;
        }

      start = end;
      } while(end < idx_last);
    }
  else if (coverage == Coverage_all)
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Do syntax highlight for all text regardless whether visible or not
    reset_syntax_style(m_scheme);

    do
      {
      get_syntax_style(&style, parser.identify_text(start, &end, m_identify_flags));

      set_text_style(start, end, style, false);
      start = end;
      } while(end < length);
    }
  else  // coverage == Coverage_none
    {
    reset_syntax_style(m_scheme);
    get_syntax_style(&style, SkParser::Identify_normal_text);
    set_text_style(start, length - 1, style, false);
    }

  // Restore previous selection
  select(old_start, old_end);

  unfreeze();
  enable_on_modified();
  }

//---------------------------------------------------------------------------------------
// Set up edit box for new syntax style.
// Arg         scheme - colour scheme to use
// Author(s):   Conan Reis
void SkEditSyntax::reset_syntax_style(
  AColor::eScheme scheme // = AColor::Scheme_default
  )
  {
  AColor colour_bg;

  static AColor s_colour_bg(      0.13f, 0.13f, 0.16f, 1.0f);
  static AColor s_colour_bg_light(0.85f, 0.9f,  0.95f, 1.0f);

  // Reset group style counter
  m_group = Group_1;

  m_scheme           = scheme;
  m_invert_b         = false;
  m_start_idx_prev   = 0u;
  m_start_group_prev = m_group;

  switch (m_scheme)
    {
    case AColor::Scheme_default:  // Default colour scheme based on system user preferences
      m_default_colour = AColor::ms_default;
      colour_bg        = AColor::ms_default;
      m_invert_b       = !AColor::get_element_os(COLOR_WINDOW).is_dark();
      break;

    case AColor::Scheme_light:    // Built-in scheme with light background (& usually dark text)
      m_default_colour = AColor::ms_black;
      colour_bg        = s_colour_bg_light;
      m_invert_b       = true;
      break;

    case AColor::Scheme_custom:   // User specified custom settings from configuration file or some other source
    case AColor::Scheme_dark:     // Built-in scheme with dark background (& usually light text)
      m_default_colour = AColor::ms_white;
      colour_bg        = s_colour_bg;
    }

  // Set all text to common style
  set_text_background(colour_bg);
  set_text_style(ATextStyle(m_default_colour, AText__all, AText__none));
  }


//---------------------------------------------------------------------------------------
// Gets colour for parsed identifier/element based on scheme
// Returns:    colour requested based on scheme 
// Arg         ident - parsed identifier/element to get a colour for
// See:        syntax highlight
// Author(s):   Conan Reis
void SkEditSyntax::get_syntax_style(
  ATextStyle *        style_p,
  SkParser::eIdentify element
  )
  {
  static AColor s_group1       ( 0.0f,  1.0f,  0.0f  );
  static AColor s_group2       ( 0.4f,  0.75f, 0.0f  );
  static AColor s_group3       ( 0.0f,  0.5f,  0.2f  );
  static AColor s_number       ( 0.13f, 0.6f,  1.0f  );  // #2299ff
  static AColor s_lighter_grey ( 0.8f,  0.8f,  0.8f  );
  static AColor s_pale_green   ( 0.77f, 1.0f,  0.78f );
  static AColor s_annotation   ( 0.0f,  0.75f, 0.0f  );
  static AColor s_light_yellow ( 1.0f,  1.0f,  0.5f  );
  static AColor s_comment      ( 0.5f,  0.77f, 1.0f  );
  static AColor s_light_violet ( 1.0f,  0.69f, 0.88f );  // #ffb0e0
  static AColor s_med_violet   ( 1.0f,  0.55f, 0.80f );
  static AColor s_clr_object_id( 0.67f, 0.6f,  1.0f  );  // #aa99ff

  style_p->m_effect_flags = AText__none;
  style_p->m_effect_mask  = AText__all;

  AColor * font_color_p = nullptr;

  // Select dark scheme colours (change to light scheme later if desired)
  switch (element)
    {
    case SkParser::Identify_reserved_word:
      font_color_p = &s_pale_green;
      break;

    case SkParser::Identify_class:
      font_color_p = &s_light_yellow;
      break;

    case SkParser::Identify_class_like:
      font_color_p = &AColor::ms_orange;
      break;

    case SkParser::Identify_operator:
      font_color_p = &AColor::ms_green;
      break;

    case SkParser::Identify_op_group_open:
      // $Revisit - CReis The colouring would be even more informative if it differentiated between group type
      font_color_p = (m_group == Group_1) ? &s_group1 : ((m_group == Group_2) ? &s_group2 : &s_group3);
      m_group = (m_group == Group_3) ? Group_1 : eGroup(m_group + 1);
      break;

    case SkParser::Identify_op_group_close:
      m_group = (m_group == Group_1) ? Group_3 : eGroup(m_group - 1);
      font_color_p = (m_group == Group_1) ? &s_group1 : ((m_group == Group_2) ? &s_group2 : &s_group3);
      break;

    case SkParser::Identify_comment:
      font_color_p = &s_comment;
      style_p->m_effect_flags = AText_italics;
      break;

    case SkParser::Identify_string:
      font_color_p = &s_light_violet;
      break;

    case SkParser::Identify_symbol:
      font_color_p = &s_med_violet;
      break;

    case SkParser::Identify_object_id:
      font_color_p = &s_clr_object_id;
      break;

    case SkParser::Identify_number:
      font_color_p = &s_number;
      break;

    case SkParser::Identify_annotation:
      font_color_p = &s_annotation;
      break;

    case SkParser::Identify_lexical_error:
      font_color_p = &AColor::ms_red;
      break;

    default: // SkParser::Identify_normal_text
      font_color_p = &AColor::ms_white;
    }

  // Since dark colours are used, invert if background is light (invert_b == true)
  if (m_invert_b && font_color_p && !font_color_p->is_default())
    {
    m_color_font = font_color_p->as_invert_luminance();
    font_color_p = &m_color_font;
    }

  style_p->m_font_color_p = font_color_p;
  }

//---------------------------------------------------------------------------------------
// Called whenever the right mouse button is released or when the user types
//             Shift+F10, or presses and releases the context menu key (which usually
//             looks like a small menu with a mouse pointer on it, it is usually between
//             the right hand Start menu key and the right hand Control key - its scan
//             scan code is VK_APPS).
// Returns:    a boolean indicating whether the default MS Windows API process with
//             respect to the given message should be called (true) or not (false).
//             The default behaviour is to call this window's parent's on_contect_menu().
// Arg         screen_pos - screen co-ordinates of the mouse cursor when the right button
//             is released or (-1, -1) if this event is caused by a keystroke.
// Examples:   called by the system
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
bool SkEditSyntax::on_context_menu(const AVec2i & screen_pos)
  {
  enum eLogPop
    {
    // File [Not sub-menu]
    EditPop_save,
    EditPop_copy_path,
    EditPop_open_explorer,
    EditPop_edit_external,

    // Formatting
    EditPop_indent,
    EditPop_unindent,

    EditPop_tabs_to_spaces,
    EditPop_trim_trailing_spaces,
    EditPop_lowercase,
    EditPop_uppercase,
    EditPop_capitalize,

    EditPop_sort,

    EditPop_comment_block,
    EditPop_comment_lines,
    EditPop_block_wrap,

    // Version Control
    EditPop_p4_checkout,
    EditPop_p4_revert,
    EditPop_p4_diff,
    EditPop_p4_history,
    EditPop_p4_timelapse,
    EditPop_p4_properties,

    // Navigation
    EditPop_goto_local,
    EditPop_goto_context,
    EditPop_goto_class,
    EditPop_goto_invokable,
    EditPop_goto_data,
    EditPop_goto_all,

    EditPop_history_next,
    EditPop_history_prev,

    EditPop_bookmarks,

    // Compile & Debug
    EditPop_execute_selected,
    EditPop_execute_selected_ide,
    EditPop_watch_selected,
    EditPop_break_toggle,
    EditPop_recompile_member,

    EditPop_arrange_panes
    };


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  APopMenuOS   pop_menu;
  bool         selected        = is_selected();
  const char * sel_type_cstr_p = selected ? "selection" : "line";

  //  Setup the Edit Menu
  APopMenuOS edit_menu;
  SkMainWindowBase::append_menubar_edit(edit_menu, this, SkMainWindowBase::get_focused_last_type());
  pop_menu.append_submenu(&edit_menu, "Edit");

  //  Setup the Format Menu
  APopMenuOS format_menu;

  if (m_type != Type_single_line)
    {
    pop_menu.append_submenu(&format_menu, "Formatting");
    format_menu.append_item("Indent lines\t[Tab]",          EditPop_indent);
    format_menu.append_item("Unindent lines\t[Shift+Tab]",  EditPop_unindent);

    //--------------------
    format_menu.append_separator();
    format_menu.append_item("Convert tabs to spaces",       EditPop_tabs_to_spaces, false);
    format_menu.append_item("Trim trailing spaces",         EditPop_trim_trailing_spaces, false);
    format_menu.append_item("lowercase",                    EditPop_lowercase, false);
    format_menu.append_item("UPPERCASE\t[Ctrl+Shift+A]",    EditPop_uppercase, false);
    format_menu.append_item("Capitalize",                   EditPop_capitalize, false);

    //--------------------
    format_menu.append_separator();
    format_menu.append_item("Sort lines",                   EditPop_sort, false);

    //--------------------
    // Code Formatting
    format_menu.append_separator();
    format_menu.append_item("Comment /* */",                EditPop_comment_block, false);
    format_menu.append_item("Comment Lines //",             EditPop_comment_lines, false);
    format_menu.append_item("Wrap in code block []",        EditPop_block_wrap, false);

    if ((m_type == Type_editor)
      && (SkConsole::ms_console_p->get_version_control_system() == SkVersionControl_p4))
      {
      // Version Control Commands
      //APopMenuOS p4_menu;

        //pop_menu.append_submenu(&p4_menu, "Version Control");
        pop_menu.append_item("Perforce - Checkout\t[Alt+P]",          EditPop_p4_checkout);
        //p4_menu.append_item("Perforce - Revert...",                  EditPop_p4_revert, false);
        //p4_menu.append_item("Perforce - Diff Previous...\t[Ctrl+P]", EditPop_p4_diff, false);
        //p4_menu.append_item("Perforce - History...\t[Ctrl+Shift+P]", EditPop_p4_history, false);
        //p4_menu.append_item("Perforce - Timelapse View...",          EditPop_p4_timelapse, false);
        //p4_menu.append_item("Perforce - Properties...",              EditPop_p4_properties, false);
      }
    }

  APopMenuOS nav_menu;

    pop_menu.append_submenu(&nav_menu, "Navigate/View");
    nav_menu.append_item("Goto current/focus type...\t[Ctrl+G]",     EditPop_goto_local);
    nav_menu.append_item("Goto Context...\t[Alt+G]",                 EditPop_goto_context);
    nav_menu.append_item("Goto Class...\t[Alt+C]",                   EditPop_goto_class);
    nav_menu.append_item("Goto Routine...\t[Alt+Ctrl+G]",            EditPop_goto_invokable);
    nav_menu.append_item("Goto Data...\t[Alt+Shift+G]",              EditPop_goto_data);
    nav_menu.append_item("Goto...\t[Ctrl+Shift+G]",                  EditPop_goto_all);

    //--------------------
    nav_menu.append_separator();
    nav_menu.append_item("History Next\t[Alt+-> or Mouse Next]",     EditPop_history_next);
    nav_menu.append_item("History Previous\t[Alt+<- or Mouse Prev]", EditPop_history_prev);

    //--------------------
    //nav_menu.append_separator();
    //nav_menu.append_item("Bookmarks...",                             EditPop_bookmarks, false);

  APopMenuOS compile_menu;

  if (m_type != Type_single_line)
    {
    pop_menu.append_submenu(&compile_menu, "Compile && Debug");
    compile_menu.append_item(a_cstr_format("Execute %s\t[F4, Ctrl+Enter]", sel_type_cstr_p), EditPop_execute_selected);
    compile_menu.append_item(a_cstr_format("Execute %s on IDE\t[Shift+F4, Shift+Enter]", sel_type_cstr_p), EditPop_execute_selected_ide);
    compile_menu.append_item(a_cstr_format("Add %s to Watch", sel_type_cstr_p),  EditPop_watch_selected, false);
    compile_menu.append_item("Toggle Breakpoint\t[F9]",                          EditPop_break_toggle, false);

    SkClassBrowser *      browser_p  = SkConsole::ms_console_p->get_browser();
    const SkContextInfo * member_p   = browser_p ? &SkConsole::ms_console_p->get_browser()->get_member_info() : nullptr;
    bool                  compilable = member_p && (member_p->m_type < SkMember_data);

    compile_menu.append_item(
      a_cstr_format("Recompile %s\t[Ctrl+F7]", compilable ? member_p->as_file_title().as_cstr() : "*N/A*"),
      EditPop_recompile_member,
      compilable);
    }

  if (m_type != Type_single_line)
    {
    //--------------------
    // File [no sub-menu]
    pop_menu.append_separator();
    pop_menu.append_item("Save\t[Ctrl+S]",   EditPop_save);

    //--------------------
    pop_menu.append_separator();
    pop_menu.append_item("Copy file path",                            EditPop_copy_path);
    pop_menu.append_item("Open in external editor...\t[Ctrl+E]",      EditPop_edit_external);
    pop_menu.append_item("Open in file explorer...\t[Ctrl+Shift+E]",  EditPop_open_explorer);

    //--------------------
    // Other [no sub-menu]
    pop_menu.append_separator();
    pop_menu.append_item("Arrange panes...",   EditPop_arrange_panes);
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  bool      call_parent  = false;
  uint32_t  item_id      = 0u;

  // Using the parent's handle since it will use the editor I-bar cursor otherwise
  if (pop_menu.show(screen_pos, m_parent_handle, & item_id))
    {
    const SkContextInfo & context = SkClassBrowser::ms_browser_p->get_member_info();

    switch (eLogPop(item_id))
      {
      // $Vital - CReis Ensure that the edit commands ensure that the source is editable.
      case EditPop_copy_path:
        action_copy_path();
        break;

      case EditPop_indent:
        indent_selection(SkDebug::ms_indent_size);
        break;

      case EditPop_unindent:
        unindent_selection(
          SkDebug::ms_indent_size, SkDebug::ms_tab_stops);
        break;

      case EditPop_edit_external:
        action_edit_externally();
        break;

      case EditPop_open_explorer:
        action_goto_file_explorer();
        break;

      case EditPop_p4_checkout:
        context.action_p4_checkout();
        break;

      case EditPop_goto_local:
        SkConsole::ms_console_p->display_goto_context_focus();
        break;

      case EditPop_goto_context:
        SkConsole::ms_console_p->display_goto_context(*this);
        break;

      case EditPop_goto_class:
        SkConsole::ms_console_p->display_goto_dialog(SkMatchKind_classes);
        break;

      case EditPop_goto_invokable:
        SkConsole::ms_console_p->display_goto_dialog(SkMatchKind_routines);
        break;

      case EditPop_goto_data:
        SkConsole::ms_console_p->display_goto_dialog(SkMatchKind_data);
        break;

      case EditPop_goto_all:
        SkConsole::ms_console_p->display_goto_dialog(SkMatchKind_all);
        break;

      case EditPop_history_next:
        SkConsole::ms_console_p->display_browser();
        SkConsole::ms_console_p->get_browser()->get_edit_view().history_next();
        break;

      case EditPop_history_prev:
        SkConsole::ms_console_p->display_browser();
        SkConsole::ms_console_p->get_browser()->get_edit_view().history_prev();
        break;

      case EditPop_execute_selected:
        action_evaluate_selected();
        break;

      case EditPop_execute_selected_ide:
        action_evaluate_selected(SkLocale_ide);
        break;

      case EditPop_recompile_member:
        SkConsole::ms_console_p->compile_member_browser();
        break;

      case EditPop_save:
        save_settings();
        break;

      case EditPop_arrange_panes:
        call_parent = true;
        break;
      }
    }

  //if (SkConsole::ms_console_p->on_menubar(item_id))
  //  {
  //  return false;   //  Processed the message, so any parent context (??) should process
  //  }

  // Call parent's on_context_menu()?
  return call_parent;
  }

//---------------------------------------------------------------------------------------
// Called whenever a key is pressed.
// Arg         key - key code corresponding to a physical key on the keyboard.
//             If Shift-'2' is pressed, AKey_shift is sent first and then AKey_2, not '@'.
//             Defines for codes are prefixed with "AKey_" and are in AgogIO/AKeyboard.hpp
//             AKey_0 thru AKey_9 are the same as ANSI '0' thru '9' (0x30 - 0x39)
//             AKey_A thru AKey_Z are the same as ANSI 'A' thru 'Z' (0x41 - 0x5A)
//             Special characters like AKey_control are also possible.
// Arg         repeated - true if this is a repeated send (holding down the key), false
//             if this is the first time key has been pressed.
// See:        on_character(), on_key_release()
// Notes:      AKeyboard::get_mod_keys() can be called to determine if any modifier keys
//             are in effect.
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
bool SkEditSyntax::on_key_press(
  eAKey key,
  bool  repeated
  )
  {
  if (m_type != Type_single_line)
    {
    eAKeyMod mod_keys = AKeyboard::get_mod_keys();

    if (!m_incremental_search_editbox.on_key_press_bidirectional(key, repeated, mod_keys, true))
      {
      return false;
      }
      
      switch (key)
      {
      case AKey_tab:
        {
        switch (mod_keys)
          {
          case AKeyMod_none:
            {
            uint32_t sel_begin;
            uint32_t sel_end;

            get_selection(&sel_begin, &sel_end);

            if (sel_begin == sel_end)
              {
              // Simple tab
              replace_selection(
                AString(' ', SkDebug::ms_indent_size),
                true);
              }
            else
              {
              indent_selection(SkDebug::ms_indent_size);
              }

            return false;
            }

          case AKeyMod_shift:
            unindent_selection(
              SkDebug::ms_indent_size,
              SkDebug::ms_tab_stops);
            return false;
          }

        break;
        }

      case AKey_return:
      case AKey_num_enter:
        switch (mod_keys)
          {
          case AKeyMod_none:
            {
            // Auto-indent based on previous indentation
            uint32_t current_row = get_row_caret();
            AString  row_str(get_row(current_row));

            if (row_str.is_filled())
              {
              uint prev_row_indent =
                row_str.find_indent_column(SkDebug::ms_tab_stops);

              if (prev_row_indent)
                {
                row_str.empty();
                row_str.append('\n');
                row_str.append(' ', prev_row_indent);

                // $Revisit - CReis Consider: Some editors like Visual Studio do this as 2 actions - newline, indent - so auto-indent you can undo indent.
                replace_selection(row_str, true);

                return false;
                }
              }

            // $ToDo - CReis Also auto-continue comment
            break;
            }

          case AKeyMod_ctrl:
            if (!repeated)
              {
              action_evaluate_selected();
              }
            return false;

          case AKeyMod_shift:
            if (!repeated)
              {
              action_evaluate_selected(SkLocale_ide);
              }
            return false;
          }
        break;

      case AKey_home:
        switch (mod_keys)
          {
          case AKeyMod_none:
            toggle_caret_home_nonspace();
            return false;

          // $Revisit - CReis Should work though select() doesn't handle the "anchor"
          // properly.  It always uses the minimum rather than 'start' as the anchor.
          // [The anchor/active index *can* be determined via TOM in ARichEditOS objects.]
          //case AKeyMod_shift:
          //  {
          //  uint32_t anchor_idx;
          //  get_selection(&anchor_idx, nullptr);
          //  select(anchor_idx, get_index_home_nonspace(anchor_idx));
          //  return false;
          //  }
          }
        break;

      case AKey_insert:
        switch (mod_keys)
          {
          case AKeyMod_shift:
            clipboard_paste_plain();
            return false;

          case AKeyMod_ctrl:
            if (!repeated)
              {
              clipboard_copy_plain_sel_or_row();
              }
            return false;

          case AKeyMod_alt:
            replace_selection(ms_result, true);
            return false;

          case AKeyMod_alt_ctrl:
            {
            AClipboard clip(SkConsole::ms_console_p);

            clip.set_text(ms_result);
            return false;
            }
          }
        break;

      case AKey_f4:
        switch (mod_keys)
          {
          case AKeyMod_none:
            if (!repeated)
              {
              action_evaluate_selected();
              }

            return false;

          case AKeyMod_shift:
            if (!repeated)
              {
              action_evaluate_selected(SkLocale_ide);
              }

            return false;
          }
        break;

      case 'C':     //  Copy
        switch (mod_keys)
          {
          case AKeyMod_ctrl:
            if (!repeated)
              {
              clipboard_copy_plain_sel_or_row();
              }
            return false;
          }
        break;

      case 'E':     //  Open external editor
        if (mod_keys == AKeyMod_ctrl)
          {
          if (!repeated)
            {
            action_edit_externally();
            }

          return false;
          }
        break;

      case 'S':     //  Save settings
        if (mod_keys == AKeyMod_ctrl)
          {
          if (!repeated)
            {
            save_settings();
            }

          return false;
          }
        break;

      case 'V':     //  Paste
        if (mod_keys == AKeyMod_ctrl)
          {
          clipboard_paste_plain();
          return false;
          }
        break;

      case AKey_delete:     //  Cut the current line if nothing selected.
        if (mod_keys == AKeyMod_shift)
          {
          if (!is_selected())
            {
            clipboard_copy_plain_sel_or_row();
            remove_row(get_row_caret(), true);
            return false;
            }
          }
        break;

      case 'X':     //  Cut the current line if nothing selected.
        if (mod_keys == AKeyMod_ctrl)
          {
          if (!is_selected())
            {
            clipboard_copy_plain_sel_or_row();
            remove_row(get_row_caret(), true);
            return false;
            }
          }
        break;

      case 'Z':     //  Redo
        if (mod_keys == AKeyMod_ctrl_shift)
          {
          redo();
          return false;
          }
        break;
      }
    }

  if (m_parent_p)
    {
    // Parents of AWindow objects should only be other AWindow objects.
    return static_cast<AWindow *>(m_parent_p)->on_key_press(key, repeated);
    }

  // If key not used - call default procedure
  return true;
  } //  SkEditSyntax::on_key_press()


//---------------------------------------------------------------------------------------
//  # Modifiers: virtual - Override for custom behavior.
//  # Author(s): John Stenersen
bool SkEditSyntax::on_focus()
  {
//  A_DPRINT(A_SOURCE_FUNC_STR "\n");

  SkMainWindowBase::on_focus(this, SkMainWindowBase::FocusType_editsyntax);

  return true;
  }


//---------------------------------------------------------------------------------------
//  Invalidate the editbox so any graphics elements will be properly redrawn.
//
//  Author(s):  John Stenersen
bool SkEditSyntax::on_scrollbar_horiz()
  {
  invalidate();
  return true;
  }


//---------------------------------------------------------------------------------------
//  Invalidate the editbox so any graphics elements will be properly redrawn.
//
//  Author(s):  John Stenersen
bool SkEditSyntax::on_scrollbar_vert()
  {
  invalidate();
  return true;
  }

//---------------------------------------------------------------------------------------
// Called whenever a window's client size is changing.  Usually this is associated with
// a user dragging a window's sizing border.
// 
// Examples: called by the system
// 
// Notes:
//   `on_size()` is called first, followed with repeated calles to `on_sizing()` and then
//   one final call to `on_sized()`.
//   This applies to the window's client area size changing and not necessarily the outer
//   edge of the window.
//   
// See:       on_size(), on_sized()
// Author(s): Conan Reis
void SkEditSyntax::on_sizing()
  {
  m_incremental_search_editbox.reposition();
  }

//---------------------------------------------------------------------------------------
// Called whenever the window moves in its client space.
// Can also be *custom* called by a parent when it moves in screen space.
// 
// Params:
//   space: Space_client or Space_screen
//
// Notes:
//   `on_move()` is called first, followed with repeated calles to `on_moving()` and then
//   one final call to `on_moved()`.
//   This applies to the window's client area size changing and not necessarily the outer
//   edge of the window.
// 
// See:       on_move(), on_moved()
// Author(s): Conan Reis
void SkEditSyntax::on_moving(eSpace space)
  {
  m_incremental_search_editbox.reposition();
  }

//---------------------------------------------------------------------------------------
// Converts a dos based (\r\n) index for the same text in rich edit (\r) form
// Author(s):   Conan Reis
uint SkEditSyntax::file_to_index(uint file_index) const
  {
  if (file_index == 0u)
    {
    return 0;
    }

  // $Revisit - CReis This feels kind of hacky.  Is there a better mechanism to do this?
  uint approx_index = file_index - get_row_from_index(file_index);
  uint approx_lines = get_row_from_index(approx_index);

  if ((approx_index + approx_lines) == file_index)
    {
    return approx_index;
    }

  approx_index = file_index - approx_lines;
  approx_lines = get_row_from_index(approx_index);

  if ((approx_index + approx_lines) == file_index)
    {
    return approx_index;
    }

  approx_index = file_index - approx_lines;
  approx_lines = get_row_from_index(approx_index);

  if ((approx_index + approx_lines) == file_index)
    {
    return approx_index;
    }

  return file_index - approx_lines;
  } //  SkEditSyntax::file_to_index()


//---------------------------------------------------------------------------------------
void SkEditSyntax::idx_to_expr_span(
  uint32_t   idx,
  uint32_t * idx_begin_p,
  uint32_t * idx_end_p
  )
  {
  const SkContextInfo * info_p = &SkClassBrowser::ms_browser_p->get_member_info();
  static SkParser s_parser(AString::ms_empty);
  SkParser::Args  args;

  get_text(&s_parser);
  args.set_idx_probe(idx);
  s_parser.parse_coroutine_source(info_p->m_member_id.get_name(), info_p->get_class(), args, false);
  
  if (args.m_end_pos)
    {
    args.m_end_pos--;
    }
  
  if (args.m_end_pos < args.m_start_pos)
    {
    args.m_end_pos = args.m_start_pos;
    }

  *idx_begin_p = args.m_start_pos;
  *idx_end_p   = args.m_end_pos;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // $HACK - CReis Show any change of desired and parsed type at current index
  static SkClassDescBase * s_desired_type_p = nullptr;
  static SkClassDescBase * s_type_p = nullptr;
  
  if ((args.m_desired_type_p != s_desired_type_p) || (args.m_expr_type != s_type_p))
    {
    if (args.m_desired_type_p)
      {
      args.m_desired_type_p->reference();
      }

    if (s_desired_type_p)
      {
      s_desired_type_p->dereference();
      }

    s_desired_type_p = args.m_desired_type_p;

    if (args.m_expr_type)
      {
      args.m_expr_type->reference();
      }

    if (s_type_p)
      {
      s_type_p->dereference();
      }

    s_type_p = args.m_expr_type;

    SkDebug::print_ide(
      a_str_format(
        "Desired: %s  Current: %s\n",
        s_desired_type_p ? s_desired_type_p->as_code().as_cstr() : "n/a",
        s_type_p ? s_type_p->as_code().as_cstr() : "n/a"),
      SkLocale_ide,
      SkDPrintType_trace);
    }
  }

//---------------------------------------------------------------------------------------
// Draw span range/selection
void SkEditSyntax::draw_mark(
  uint32_t idx,
  const AColor & color,
  Gdiplus::Graphics & graphics
  )
  {
  int  x;
  int  y;

  get_index_position(idx, &x, &y);

  Gdiplus::Pen pen(color.as_argb(), 3.0f);

  int fwidth  = m_font.get_avg_width();
  int fheight = m_font.get_height();

  // Draw start underline
  graphics.DrawLine(&pen, x, y + fheight, x + fwidth, y + fheight);

  // Draw "book end" lines
  graphics.DrawLine(&pen, x, y, x, y + fheight);
  }

//---------------------------------------------------------------------------------------
// Draw span range/selection
void SkEditSyntax::draw_span(
  uint32_t idx_begin,
  uint32_t idx_end,
  const AColor & color,
  Gdiplus::Graphics & graphics
  )
  {
  int  x;
  int  y;
  int  x2;
  int  y2;

  get_index_position(idx_begin, &x, &y);
  get_index_position(idx_end, &x2, &y2);

  Gdiplus::Pen pen(color.as_argb(), 3.0f);
  Gdiplus::Pen pen_fine(color.as_argb(), 1.0f);

  // Draw start underline
  int  fwidth  = m_font.get_avg_width();
  int  fheight = m_font.get_height();

  graphics.DrawLine(&pen, x, y + fheight, x + fwidth, y + fheight);

  // Draw end underline
  graphics.DrawLine(&pen, x2, y2 + fheight, x2 + fwidth, y2 + fheight);

  // Draw "book end" lines
  graphics.DrawLine(&pen_fine, x, y, x, y + fheight);
  graphics.DrawLine(&pen_fine, x2 + fwidth, y2, x2 + fwidth, y2 + fheight);

  // Draw connector lines
  pen_fine.SetDashStyle(Gdiplus::DashStyleDash);
  graphics.DrawLine(&pen_fine, x + fwidth, y + fheight, x2, y2 + fheight);
  }

//---------------------------------------------------------------------------------------
// Draw span range/selection with pivot
void SkEditSyntax::draw_span_pivot(
  uint32_t idx_begin,
  uint32_t idx_end,
  uint32_t idx_pivot,
  const AColor & color,
  Gdiplus::Graphics & graphics
  )
  {
  int  x;
  int  y;
  int  x2;
  int  y2;
  int  pivot_x;
  int  pivot_y;

  get_index_position(idx_begin, &x, &y);
  get_index_position(idx_end, &x2, &y2);
  get_index_position(idx_pivot, &pivot_x, &pivot_y);

  Gdiplus::Pen pen(color.as_argb(), 3.0f);
  Gdiplus::Pen pen_fine(color.as_argb(), 1.0f);

  // Draw start underline
  int  fwidth  = m_font.get_avg_width();
  int  fheight = m_font.get_height();

  graphics.DrawLine(&pen, x, y + fheight, x + fwidth, y + fheight);

  // Draw end underline
  graphics.DrawLine(&pen, x2, y2 + fheight, x2 + fwidth, y2 + fheight);

  // Draw "book end" lines
  graphics.DrawLine(&pen_fine, x, y, x, y + fheight);
  graphics.DrawLine(&pen_fine, x2 + fwidth, y2, x2 + fwidth, y2 + fheight);

  // Draw pivot point for expression
  AColor draw_colour(color);
  draw_colour.m_alpha = 0.25f;

  Gdiplus::SolidBrush brush_mark(draw_colour.as_argb());

  graphics.FillRectangle(&brush_mark, pivot_x, pivot_y, fwidth, fheight);

  if (color.is_opaque())
    {
    graphics.DrawRectangle(&pen_fine, pivot_x, pivot_y, fwidth, fheight);
    }

  // Draw pivot connector lines
  pen_fine.SetDashStyle(Gdiplus::DashStyleDash);
  graphics.DrawLine(&pen_fine, x + fwidth, y + fheight, pivot_x, pivot_y + fheight);
  graphics.DrawLine(&pen_fine, x2, y2 + fheight, pivot_x + fwidth, pivot_y + fheight);
  }

//---------------------------------------------------------------------------------------
//  Draws a horizontal squiggly line.
//  
//  Author(s):  John Stenersen
void draw_line_squiggly(Gdiplus::Graphics * graphics, IN const Gdiplus::Pen* pen, IN INT x1, IN INT y1, IN INT x2)
  {
  int  increment = 4;
  int  ascend    = 1;
  int  descend   = 2;
  bool up        = true;

  for (int x = x1; x < x2; x = x + increment)
    {
    if (up)
      {
      graphics->DrawLine(pen, x, y1 + ascend, x + increment, y1 - descend);
      }
    else
      {
      graphics->DrawLine(pen, x, y1 - descend, x + increment, y1 + ascend);
      }

    up = !up;
    }
  } //  draw_line_squiggly()


//---------------------------------------------------------------------------------------
//  Whenever the mouse wheel is spun, invalidate the rich editbox.
//  
//  Author(s):  John Stenersen
void SkEditSyntax::on_mouse_spinning(f32 wheel_delta, eAMouse buttons, const AVec2i & client_pos)
  {
  if (m_type != Type_single_line)
    {
    invalidate(true, true);   //  Note: This doesn't seem to be working - not the entire client space is cleared.
    }
  }


//---------------------------------------------------------------------------------------
//  Whenever the rich editbox selection changes this routine is called to auto-parse the selection.
//  
//  Author(s):  John Stenersen
void SkEditSyntax::on_selecting(uint32_t start, uint32_t end)
  {
  if (m_type != Type_log)
    {
    //A_DPRINT(A_SOURCE_FUNC_STR "start = %ld, end = %ld\n", start, end);
    }

  switch (m_type)
    {
    case Type_single_line:
      return;

    case Type_editor:
      if (SkClassBrowser::ms_browser_p)
        {
        SkClassBrowser::ms_browser_p->status_update();
        }
      //  Intentionally fall thru.

    case Type_workspace:
    case Type_log:
      {
      uint32_t caret_index = get_caret_index();
      if (SkConsole::ms_console_p->is_current_line_highlight() && (caret_index != m_caret_index_last))
        {
        invalidate();
        m_caret_index_last = caret_index;
        }
      }
      break;

    default:
      A_DPRINT(A_SOURCE_FUNC_STR "Unknown m_type = %ld\n", m_type);
      return;
    }

  //*****************************
  //  Auto-parse for selections (workspace pane only) and if not compiling.
  if ((m_type != Type_workspace) || (SkCompiler::ms_compiler_p->get_phase() != SkCompiler::Phase_idle))
    {
    invalidate();
    return;
    }

  //  Copy the last auto-parse info
  bool      save_auto_parse_ok    = m_auto_parse_ok;
  uint32_t  save_auto_parse_start = m_auto_parse_start;
  uint32_t  save_auto_parse_end   = m_auto_parse_end;

  //  Get the current selection state.
  uint32_t sel_len = end - start;

  SkParser::Args  args;
  if ((start != end) && SkConsole::ms_console_p->is_auto_parse_sel())
    {
    SkParser parser(AString(nullptr, sel_len + 10u, 0u));
    parser.append("()[", 3u);
    parser.append(get_text(start, end), sel_len);
    parser.append(" \n]", 3u);

    SkInstance * instance_p = SkookumScript::get_master_mind_or_meta_class();
    if (!instance_p)
      {
      //A_DPRINT(A_SOURCE_FUNC_STR "SkookumScript::get_master_mind() returned nullptr\n");
      return;
      }

    SkClass * class_p = instance_p->get_class();
    parser.reset_scope(class_p, ASymbol_auto_parse_);

    ASymbolTable sym_tab;
    uint32_t     sym_tab_len_before = ASymbolTable::ms_main_p->get_length();
    sym_tab.track_auto_parse_init();

    SkMethod * method_p = static_cast<SkMethod *>(parser.parse_method(args, ASymbol_auto_parse_, SkInvokeTime_any, false));

    if (method_p)
      {
      delete method_p;
      }

    sym_tab.track_auto_parse_term();

    uint32_t sym_tab_len_after = ASymbolTable::ms_main_p->get_length();
    uint32_t sym_tab_len_delta = sym_tab_len_after - sym_tab_len_before;

    if (sym_tab_len_delta != 0)
      {
      //A_DPRINT(A_SOURCE_FUNC_STR "Symbol table delta = %ld, before = %ld, after = %ld\n", sym_tab_len_delta, sym_tab_len_before, sym_tab_len_after);
      }
    }

  if (args.m_result != SkParser::Result_ok)
    {
    m_auto_parse_ok     = false;
    m_auto_parse_start  = min(max(args.m_start_pos + start - 3u, start), end - 1);
    m_auto_parse_end    = min(args.m_end_pos   + start - 3u, end);
    }
  else
    {
    m_auto_parse_ok     = true;
    m_auto_parse_start  = 0;
    m_auto_parse_end    = 0;
    }

  //  If the auto-parse info has changed, need to invalidate the rect so the graphics get redrawn.
  if ((save_auto_parse_ok != m_auto_parse_ok) || (save_auto_parse_start != m_auto_parse_start) || (save_auto_parse_end != m_auto_parse_end))
    {
    invalidate();
    }

  /*
  if (start != end)
    {
    A_DPRINT(A_SOURCE_FUNC_STR "ok = %ld, len = %ld, start = %ld, end = %ld, m_type = %ld\n", m_auto_parse_ok, sel_len, start, end, m_type);
    A_DPRINT(A_SOURCE_FUNC_STR "auto_start = %ld, auto_end = %ld, arg_start = %ld, arg_end = %ld\n", m_auto_parse_start, m_auto_parse_end, args.m_start_pos, args.m_end_pos);
    }
  */
  } //  SkEditSyntax::on_selecting()


//---------------------------------------------------------------------------------------
// Called when the window client area is to be drawn.
// Returns:    Boolean indicating whether the default Windows procedure with respect to
//             the given message should be called (true) or not (false)
// Modifiers:   virtual - Override for custom behavior.
// Author(s):   Conan Reis
bool SkEditSyntax::on_draw()
  {
  // Single lines don't have special draw overlays
  if (m_type == Type_single_line)
    {
    return true;
    }

  // $Revisit - CReis As much work as possible here should be cached rather than done each redraw.

  bool draw_current_line_highlight  = SkConsole::ms_console_p->is_current_line_highlight();
  bool draw_auto_parse              =
    !m_auto_parse_ok && (
      (SkConsole::ms_console_p->is_auto_parse()     && (get_selection_length() <= 0)) ||
      (SkConsole::ms_console_p->is_auto_parse_sel() && (get_selection_length() >  0)));

  // $Revisit - CReis Could query subclasses to see if they need any drawing.

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Call original draw window procedure at start
  call_subclass_proc(WM_PAINT, 0u, 0u);

  HDC hdc = ::GetDC(m_os_handle);  // GetDC() is used rather than BeginPaint() since this is actually in the middle of the control's draw

  Gdiplus::Graphics  graphics(hdc);

  int font_width  = m_font.get_avg_width();
  int font_height = m_font.get_height();
  graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  //  Draw the current line
  if (draw_current_line_highlight)
    {
    int current_x;
    int current_y;

    Gdiplus::Pen current_pen(g_color_text_current_line.as_argb(), 1.0f);
    get_index_position(get_caret_index(), &current_x, &current_y);
    graphics.DrawRectangle(&current_pen, 0, current_y, get_width_client() - 1, font_height);
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Draw auto-parse elements.
  if (draw_auto_parse)
    {
    int start_x;
    int start_y;
    int end_x;
    int end_y;
    int middle_x;
    int middle_y;
    uint32_t start_row = get_row_from_index(m_auto_parse_start);
    uint32_t end_row   = get_row_from_index(m_auto_parse_end);

    get_index_position(m_auto_parse_start, &start_x, &start_y);
    get_index_position(m_auto_parse_end,   &end_x,   &end_y  );

    AColor color(1.0f, 0.0f, 0.0f, 1.0f);
    Gdiplus::Pen pen(color.as_argb(), 2.0f);
    Gdiplus::Pen pen_bar(color.as_argb(), 2.0f);

    //  Draw the gutter bar.
    graphics.DrawLine(&pen_bar, 1, start_y, 1, end_y + font_height);

    if (start_row == end_row)
      {
      //  Underline all on the same row.
      if (start_x == end_x)
        {
        draw_line_squiggly(&graphics, &pen, start_x, start_y + font_height, end_x + font_width);
        }
      else
        {
        draw_line_squiggly(&graphics, &pen, start_x, start_y + font_height, end_x);
        }
      }
    else
      {
      //  Partially underline the start row.
      AString text(get_row(start_row));
      text.truncate();
      get_index_position(get_index_from_row(start_row) + text.get_length(), &middle_x, &middle_y);
      draw_line_squiggly(&graphics, &pen, start_x, start_y + font_height, middle_x);

      //  Partially underline the end row.
      text = get_row(end_row);
      text.truncate();
      uint32_t non_space;
      if (text.find(ACharMatch_not_white_space, 1, &non_space))
        {
        get_index_position(get_index_from_row(end_row) + non_space, &start_x, &start_y);
        get_index_position(get_index_from_row(end_row), &middle_x, &middle_y);
        draw_line_squiggly(&graphics, &pen, start_x, start_y + font_height, end_x);
        }

      //  Underline entire lines.
      for(uint32_t row = start_row + 1; row < end_row; row++)
        {
        text = get_row(row);
        text.truncate();
        if (text.find(ACharMatch_not_white_space, 1, &non_space))
          {
          get_index_position(get_index_from_row(row) + non_space, &start_x, &start_y);
          get_index_position(get_index_from_row(row) + text.get_length(), &middle_x, &middle_y);
          draw_line_squiggly(&graphics, &pen, start_x, middle_y + font_height, middle_x);
          }
        }
      }
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  on_draw_subpart(graphics);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Clean up
  ::ReleaseDC(m_os_handle, hdc);

  // Original draw window procedure was already called at start of this method
  return false;
  } //  SkEditSyntax::on_draw()


//=======================================================================================
// SkWorkspace Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Constructor
// Author(s):   Conan Reis
SkWorkspace::SkWorkspace(
  AWindow *   parent_p,
  SkConsole * console_p
  ) :
  SkEditSyntax(Type_workspace, parent_p, SkIncrementalSearchEditBox::ParentContext_workspace, AString::ms_empty, console_p->get_ini_font()),
  m_console_p(console_p),
  m_tooltip(this,
    "Select text and press F4 to run it remotely on the App.\n"
    "Select text and Shift+F4 to run it locally on the IDE.\n"
    "Drag and drop a script file to run it locally.\n"
    "Alt+G to Goto a class, method, coroutine, etc.\n"
    "<A>See Website Docs</A> or Right-Click for more options.\n"
    "<A>Disable</A> this ToolTip, Ctrl+T to re-enable.",
    SkConsole::ms_console_p->get_ini_ide().get_value_bool_default(false, g_ini_tooltip_enable_on_startup_p, g_ini_section_tooltip_p) ||
      SkConsole::ms_console_p->get_ini_ide().get_value_bool_default(true, g_ini_tooltip_enable_workspace_p, g_ini_section_tooltip_p),
    -1, //  delay_reshow default
    *AFont::ms_narrow_p,
    reinterpret_cast<ARichEditOS *>(this)),
    m_auto_complete(this),
    m_workspace_file(SkCompiler::ms_compiler_p->get_ini_ide().get_value_file_default(
      g_ini_workspace_file_def, g_ini_key_workspace_file_p, g_ini_section_console_p))
  {
  m_tooltip.set_header("Workbench Window (REPL)", IDI_SKOOKUM);
  m_tooltip.set_link("http://www.skookumscript.com/docs/");
  m_tooltip.set_link_funct(5, AToolTipOS::on_link_disable);

  //  Get the auto-complete defaults.
  SkAutoComplete::ms_active         = SkConsole::ms_console_p->get_ini_ide().get_value_bool_default(true, g_ini_auto_complete_active_p,         g_ini_section_auto_complete_p);
  SkAutoComplete::ms_key_length_min = SkConsole::ms_console_p->get_ini_ide().get_value_int_default( 1,    g_ini_auto_complete_key_length_min_p, g_ini_section_auto_complete_p);
  } //  SkWorkspace::SkWorkspace()


//---------------------------------------------------------------------------------------
// Get the workspace file used by the workspace window.
// Modifiers:   static
// Author(s):   Conan Reis
const AFile & SkWorkspace::get_ini_workspace_file() const
  {
  return m_workspace_file;
  }

//---------------------------------------------------------------------------------------
// Saves workspace info
// See:        save_settings()
// Author(s):   Conan Reis
void SkWorkspace::load_settings()
  {
  // Load workspace text from previous session if it exists
  AFile work_file(get_ini_workspace_file());

  if (work_file.is_existing())
    {
    AString source(work_file.read_text());

    source.line_break_dos2unix();
    set_source(source);
    }
  else
    {
    set_text(
      "// Select code & press F4 to execute it (or Shift+F4 to execute locally on IDE).\n"
      "// Or drag & drop a .sk script file here to execute it.\n\n");
    }
  }

//---------------------------------------------------------------------------------------
// Saves workspace info
// 
// See: load_settings()
void SkWorkspace::save_settings(
  eSkLog log // = SkLog_ide_print
  )
  {
  AFile file(get_ini_workspace_file());

  if (log == SkLog_ide_print)
    {
    SkDebug::print(a_str_format("\nSaving '%s'.\n", file.as_cstr()));
    }

  set_break_conversion(ALineBreak_dos);
  AString work_str(get_text());
  set_break_conversion(ALineBreak_unix);
  
  file.write_text(work_str);

  SkConsole::ms_console_p->get_ini_ide().set_value_bool(
    m_tooltip.is_enabled(), g_ini_tooltip_enable_workspace_p, g_ini_section_tooltip_p);
  }

//---------------------------------------------------------------------------------------
// The user has taken an action that may have altered text in an edit control.
//             Unlike the on_predraw() event, this event is sent after the system updates
//             the screen.
// See:        enable_on_modified, on_predraw()
// Notes:      This event is not called when it is a multi-line control and the text is
//             set via WM_SETTEXT
// Modifiers:   virtual
// Author(s):   Conan Reis
void SkWorkspace::on_modified()
  {
  if (SkConsole::ms_console_p->is_syntax_highlight())
    {
    syntax_highlight(Coverage_visible);
    }
  SkEditSyntax::on_modified();
  }


//---------------------------------------------------------------------------------------
//  # Modifiers: virtual - Override for custom behavior.
//  # Author(s): John Stenersen
bool SkWorkspace::on_focus()
  {
//  A_DPRINT(A_SOURCE_FUNC_STR "\n");

  SkConsole::ms_console_p->set_focus_splitter(reinterpret_cast<ASplitterOS *>(this->get_parent()));
  SkMainWindowBase::on_focus(this, SkMainWindowBase::FocusType_workspace);

  return true;
  }


//---------------------------------------------------------------------------------------
//  Whenever the workspace pane loses focus, cancel the auto-complete listbox.
//  
//  # Modifiers: virtual - Override for custom behavior.
//  # Author(s): John Stenersen
void SkWorkspace::on_focus_lost(HWND focus_window)
  {
//  A_DPRINT(A_SOURCE_FUNC_STR "\n");
//  A_DPRINT(A_SOURCE_STR "focus_window = %ld, auto = %ld, parent = %ld\n", focus_window, m_auto_complete.get_os_handle(), get_os_handle());

  if ((focus_window != m_auto_complete.get_os_handle()) && (focus_window != get_os_handle()))
    {
    m_auto_complete.cancel();
    }
  }


//---------------------------------------------------------------------------------------
//  Passes any printable character to the auto-complete first and, if not "consumed" then
//  the character is processed by the default proc.
//  
//  Author(s):  John Stenersen
bool SkWorkspace::on_character(char ch, bool repeated)
  {
  if (!m_auto_complete.on_parent_character(ch, repeated, AKeyboard::get_mod_keys()))
    {
    return false;
    }

  return true;  //  Not processed, so pass to default proc.
  }


//---------------------------------------------------------------------------------------
//  Passes any key pressed to the auto-complete first and, if not "consumed" then the
//  character is processed by the default proc.
//  
//  Author(s):  John Stenersen
bool SkWorkspace::on_key_press(eAKey key, bool repeated)
  {
  if (!m_auto_complete.on_parent_key_press(key, repeated, AKeyboard::get_mod_keys()))
    {
    return false;
    }

  return SkEditSyntax::on_key_press(key, repeated);
  }


//=======================================================================================
// SkLog Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Constructor
SkLog::SkLog(
  AWindow *   parent_p,
  SkConsole * console_p
  ) :
  SkEditSyntax(Type_log, parent_p, SkIncrementalSearchEditBox::ParentContext_log, "", console_p->get_ini_font()),
  m_console_p(console_p),
  m_tooltip(this,
    "<A>Try:</A> Output from the local IDE are *italicized*.\n"
    "<A>Try:</A> Output from the remote App are not italicized.\n"
    "<A>See Website Docs</A> or Right-Click for more options.\n"
    "<A>Disable</A> this ToolTip, Ctrl+T to re-enable.",
    SkConsole::ms_console_p->get_ini_ide().get_value_bool_default(false, g_ini_tooltip_enable_on_startup_p, g_ini_section_tooltip_p) ||
      SkConsole::ms_console_p->get_ini_ide().get_value_bool_default(true, g_ini_tooltip_enable_log_p, g_ini_section_tooltip_p),
    -1, //  delay_reshow default
    *AFont::ms_narrow_p,
    reinterpret_cast<ARichEditOS *>(this))
  {
  m_tooltip.set_header("Print Log", IDI_SKOOKUM);
  m_tooltip.set_link("http://www.skookumscript.com/docs/");
  m_tooltip.set_link_funct(0, AToolTipOS::on_link_local_message_example);
  m_tooltip.set_link_funct(1, AToolTipOS::on_link_remote_message_example);
  m_tooltip.set_link_funct(3, AToolTipOS::on_link_disable);

  // Set all text to common style
  set_text_background(SkLog_colour_bground);
  set_text_style(ATextStyle(AColor::ms_white, AText__all, AText__none));

  append_style(
    "SkookumScript IDE\n"
    "Version: " SK_VERSION_TEXT "\n"
    A_PLAT_STR_DESC"\n"
    A_COPYRIGHT_TEXT"\n\n",
    ATextStyle(SkLog_colour_title, AText_bold, AText_bold));
  } //  SkLog::SkLog()


//---------------------------------------------------------------------------------------
// Saves log info
void SkLog::save_settings(
  eSkLog log // = SkLog_ide_print
  )
  {
  SkConsole::ms_console_p->get_ini_ide().set_value_bool(
    m_tooltip.is_enabled(), g_ini_tooltip_enable_log_p, g_ini_section_tooltip_p);
  }

//---------------------------------------------------------------------------------------
// Called whenever a mouse button is pressed in the client region of this
//             window (or anywhere if the mouse is 'captured').
// Returns:    Boolean indicating whether the default Windows procedure with respect to
//             the given message should be called (true) or not (false)
// Arg         button - the button that was just pressed - see eAMouse
// Arg         buttons - the press state of all the mouse buttons - see eAMouse
// Arg         client_pos - client position of mouse cursor
// Arg         double_click - true if this was a double click
// See:        on_mouse_release(), on_mouse_moving(), on_mouse_spinning(),
//             enable_mouse_capture(), AMouse
// Notes:      AKeyboard::get_mod_keys() can be called to determine if any modifier keys
//             are in effect.
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
bool SkLog::on_mouse_press(
  eAMouse        button,
  eAMouse        buttons,
  const AVec2i & client_pos,
  bool           double_click
  )
  {
  if (double_click && (button == AMouse_left))
    {
    uint32_t caret_row = get_row_caret();
    AString  click_line(get_row(caret_row));

    click_line.crop();
    
    uint32_t line_length = click_line.get_length();

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Does double clicked line start with "File:"
    if ((line_length > 6u) && click_line.find("File: ", 1u, nullptr, 0u, 5u))
      {
      select_row(caret_row);

      // $Revisit - CReis This should happen as the "File:" is detected - before the double click
      set_text_style_selection(AUnderline_solid);

      // Clean line
      click_line.remove_all(0u, 6u);

      uint32_t idx_desc;
      uint32_t idx_begin = ADef_uint32;
      uint32_t idx_end   = 0u;

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Get optional index range
      if (click_line.find_reverse('[', 1u, &idx_desc))
        {
        uint32_t idx;

        idx_begin = click_line.as_uint(idx_desc + 1u, &idx);
        idx_end   = (click_line.get_at(idx) == '-')
          ? click_line.as_uint(idx + 1u)
          : idx_begin;

        click_line.set_length(idx_desc);
        }

      SkContextInfo info;
      AFile         source(click_line);

      if (SkCompiler::parse_file_member(source, &info))
        {
        m_console_p->browse_member(info, idx_begin, idx_end);
        }

      return false;
      }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Does double clicked line start with "Member:"
    if ((line_length > 8u) && click_line.find("Member:", 1u, nullptr, 0u, 6u))
      {
      // Select line
      // $Revisit - CReis This should happen as the "Member:" is detected - before the double click
      select_row(caret_row);
      set_text_style_selection(AUnderline_solid);


      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Determine member
      SkContextInfo info;
      uint32_t      pos = 7u;
      SkParser      member_ident(click_line);

      member_ident.find(ACharMatch_not_white_space, 1u, &pos, pos);

      if (member_ident.identify_member_name(&info, pos, &pos) != SkParser::Result_ok)
        {
        return false;
        }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Determine member file index for caret/selection
      uint32_t idx_range;
      uint32_t idx_begin = 0u;
      uint32_t idx_end   = ADef_uint32;

      click_line.find(ACharMatch_not_white_space, 1u, &pos, pos);

      // Get optional index range
      if ((pos < line_length) && (click_line.find('[', 1u, &idx_range, pos)))
        {
        uint32_t idx;

        idx_begin = click_line.as_uint(idx_range + 1u, &idx);
        idx_end   = (click_line.get_at(idx) == '-')
          ? click_line.as_uint(idx + 1u)
          : idx_begin;
        }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      if (info.m_type < SkMember__invalid)
        {
        m_console_p->browse_member(info, idx_begin, idx_end);
        }

      return false;
      }
    }

  return true;
  }


//---------------------------------------------------------------------------------------
// Called whenever the right mouse button is released or when the user types
//             Shift+F10, or presses and releases the context menu key (which usually
//             looks like a small menu with a mouse pointer on it, it is usually between
//             the right hand Start menu key and the right hand Control key - its scan
//             scan code is VK_APPS).
// Returns:    a boolean indicating whether the default MS Windows API process with
//             respect to the given message should be called (true) or not (false).
//             The default behaviour is to call this window's parent's on_contect_menu().
// Arg         screen_pos - screen co-ordinates of the mouse cursor when the right button
//             is released or (-1, -1) if this event is caused by a keystroke.
// Examples:   called by the system
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   John Stenersen
bool SkLog::on_context_menu(const AVec2i & screen_pos)
  {
  APopMenuOS    edit_menu;
  SkMainWindowBase::append_menubar_edit(edit_menu, this, FocusType_log);

  uint32_t  item_id   = 0u;

  // Using the parent's handle since it will use the editor I-bar cursor otherwise
  if (edit_menu.show(screen_pos, m_os_handle, & item_id))
    {
    return !SkConsole::ms_console_p->on_menubar(item_id);
    }

  return false;
  } //  SkLog::on_context_menu()


//---------------------------------------------------------------------------------------
//  Called whenever a key is pressed.
//  Arg       key - key code corresponding to a physical key on the keyboard.
//  Arg       repeated - true if this is a repeated send (holding down the key), false
//            if this is the first time key has been pressed.
//  See:      on_character(), on_key_release()
//  Notes:    AKeyboard::get_mod_keys() can be called to determine if any modifier keys
//            are in effect.
//  Modifiers:   virtual - Override for custom behaviour.
//  Author(s):   John Stenersen
bool SkLog::on_key_press(
  eAKey key,
  bool  repeated
  )
  {
  eAKeyMod mod_keys = AKeyboard::get_mod_keys();
  
  if (!m_incremental_search_editbox.on_key_press_bidirectional(key, repeated, mod_keys, true))
    {
    return false;
    }

  // Ignore repeated keys
  if (!repeated)
    {
    switch (key)
      {
      case 'T':
        if (mod_keys == AKeyMod_ctrl)
          {
          if (m_tooltip)
            {
            m_tooltip.enable_activate();
            m_tooltip.show_at_mouse();
            set_focus();
            return false;
            }
          }
        break;
      }
    }

  if (m_parent_p)
    {
    // Parents of AWindow objects should only be other AWindow objects.
    return static_cast<AWindow *>(m_parent_p)->on_key_press(key, repeated);
    }

  // If key not used - call default procedure
  return true;
  } //  SkLog::on_key_press()


//---------------------------------------------------------------------------------------
//  # Modifiers: virtual - Override for custom behaviour.
//  # Author(s): John Stenersen
bool SkLog::on_focus()
  {
//  A_DPRINT(A_SOURCE_FUNC_STR "\n");

  SkConsole::ms_console_p->set_focus_splitter(reinterpret_cast<ASplitterOS *>(this->get_parent()));
  SkMainWindowBase::on_focus(this, SkMainWindowBase::FocusType_log);

  return true;
  }


//=======================================================================================
// SkOverlayList Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
SkOverlayList::SkOverlayList(
  SkClassSettings * settings_p,
  SkConsole * console_p
  ) :
  AListOS<SkOverlay>(settings_p),
  m_settings_p(settings_p),
  m_console_p(console_p)
  {
  enable_gridlines();
  enable_remove_events();
  enable_header_swapping(false);

  column_append(new AColumnOS<SkOverlay>(
    "Sequence",
    new tSkOverlayTextCall(this, &SkOverlayList::on_text_sequence),
    nullptr,
    false,
    AListOS_column_width_title,
    AColumnAlign_centered));

  column_append(new AColumnOS<SkOverlay>(
    "Name",
    new tSkOverlayTextCall(this, &SkOverlayList::on_text_name)));

  column_append(new AColumnOS<SkOverlay>(
    "Directory",
    new tSkOverlayTextCall(this, &SkOverlayList::on_text_dir)));
  }

//---------------------------------------------------------------------------------------
// Destructor
// Author(s):   Conan Reis
SkOverlayList::~SkOverlayList()
  {
  // Some windows/controls need to call destroy() in their own destructor
  // rather than letting the AMessageTarget destructor call it since destroy()
  // will end up sending windows messages and the windows/controls need to have
  // their virtual table still intact.
  destroy();
  }

//---------------------------------------------------------------------------------------
// Arg         row - index of row - can call row2info to get associated info
// Notes:      Override for custom behavior
// Modifiers:   virtual
// Author(s):   Conan Reis
void SkOverlayList::on_subitem_activated(
  uint row,
  uint rank
  )
  {
  //A_DPRINT(A_SOURCE_FUNC_STR "(row: %u, column: %u) 0x%p\n", row, rank, this);

  SkOverlay * overlay_p = row2item(row);

  if (overlay_p)
    {
    AFile overlay_file(overlay_p->m_path_qual);

    overlay_file.execute();
    }
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkOverlayList::on_item_focused(SkOverlay * item_p, uint row)
  {
  m_settings_p->on_overlay_focus(row);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkOverlayList::on_item_selected(SkOverlay * item_p, uint row, bool selected)
  {
  // [Incomplete]
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkOverlayList::on_text_sequence(tSkOverlaySubText * info_p)
  {
  info_p->m_subitem_str_p->format(
    info_p->m_item_p->m_apply_b ? "%u" : "[%u]",
    info_p->m_item_p->m_sequence);
  info_p->m_save_text = false;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkOverlayList::on_text_name(tSkOverlaySubText * info_p)
  {
  // Note that append() is used rather than = so that the buffer in m_subitem_str_p is used.
  info_p->m_subitem_str_p->append(info_p->m_item_p->m_name);
  info_p->m_save_text = false;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkOverlayList::on_text_dir(tSkOverlaySubText * info_p)
  {
  // Note that append() is used rather than = so that the buffer in m_subitem_str_p is used.
  info_p->m_subitem_str_p->append(info_p->m_item_p->m_path);
  info_p->m_save_text = false;
  }


//=======================================================================================
// SkClassSettings Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Constructor
// Author(s):   Conan Reis
SkClassSettings::SkClassSettings(SkConsole * console_p) :
  m_console_p(console_p),
  m_overlay_add_btn(this, "+"),
  m_overlay_remove_btn(this, "-"),
  m_overlay_toggle_btn(this, "[ ]"),
  m_overlay_remap_btn(this, "..."),
  m_overlay_up_btn(this, "/\\"),
  m_overlay_down_btn(this, "\\/"),
  m_overlay_list(this, console_p),
  m_overlays_changed_b(false),
  m_compiled_load_tgl(this, "Load:", ACheckType_3_state),
  m_compiled_save_tgl(this, "Save Compiled Binary after parse"),
  m_compiled_path(this),
  m_compiled_remap_btn(this, "..."),
  m_compiled_path_text_y(0),
  m_eval_atomics_tgl(this, "Ensure bindings for C++ atomics"),
  m_eval_mind_class(SkEditSyntax::Type_single_line, this, SkIncrementalSearchEditBox::ParentContext_class_settings, "", console_p->get_ini_font()),
  m_eval_mind_btn(this, "..."),
  m_eval_mind_text_y(0),
  m_ok_btn(this, "OK"),
  m_cancel_btn(this, "Cancel"),
  m_apply_btn(this, "Apply")
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Setup Class Settings Window
  int spacing = get_spacing();
  int width   = spacing * SkConsole_classes_width_spacing;
  int height  = spacing * SkConsole_classes_height_spacing;

  enable_title_bar();
  #ifndef SK_NO_RESOURCES
    set_icon(IDI_SKOOKUM);
  #else
    set_icon_file(make_qualified(AFile("Media\\SkookumScript.ico")).get_file_str().as_cstr());
  #endif
  set_title_buttons(TitleButton_close);
  update_title();
  enable_sizing();
  set_area(width, height);

  // Setup device context (DC) drawing properties - info is retained since it has its own
  // private DC.
  HDC hdc = ::GetDC(m_os_handle);

  ::SelectObject(hdc, ((ATrueTypeFont *)m_font.m_sys_font_p)->m_font_handle_p);
  ::SetTextColor(hdc, ::GetSysColor(COLOR_WINDOWTEXT));
  ::SetBkColor(hdc, ::GetSysColor(COLOR_3DFACE));
  ::SetBkMode(hdc, OPAQUE);  // TRANSPARENT

  ::ReleaseDC(m_os_handle, hdc);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Setup OK & Cancel buttons [UI built from bottom to top]
  AVec2i area(m_cancel_btn.get_area());

  area *= 1.15f;

  m_apply_btn.set_region(area);
  m_apply_btn.set_on_pressed_func(new AMethod<SkClassSettings>(this, &SkClassSettings::apply_changes));
  m_apply_btn.show();
  m_cancel_btn.set_region(area);
  m_cancel_btn.set_on_pressed_func(new AMethod<SkClassSettings>(this, &SkClassSettings::close_default));
  m_cancel_btn.show();
  m_ok_btn.set_region(area);
  m_ok_btn.enable_default_border();
  m_ok_btn.set_on_pressed_func(new AMethod<SkClassSettings>(this, &SkClassSettings::on_ok));
  m_ok_btn.show();

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Setup Evaluation Settings UI

  // Disable Mind class selector for now
  m_eval_mind_class.set_text(SkCompiler::ms_compiler_p->get_ini_startup_mind());
  m_eval_mind_class.set_on_modified_func(new AMethod<SkClassSettings>(this, &SkClassSettings::on_startup_mind_modified));
  m_eval_mind_class.syntax_highlight();
  m_eval_mind_btn.enable_input(false);

  area.m_x         = area.m_y;
  m_eval_rect.left = spacing;
  m_eval_mind_btn.set_area(area);
  m_eval_mind_btn.show();
  m_eval_mind_class.set_border(Border_sunken);
  m_eval_mind_class.show();
  m_eval_atomics_tgl.show();

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Setup Compiled Binary Settings UI
  m_compiled_rect.left = spacing;
  m_compiled_remap_btn.set_area(area);
  m_compiled_remap_btn.set_on_pressed_func(new AMethod<SkClassSettings>(this, &SkClassSettings::on_compiled_remap));
  m_compiled_remap_btn.show();
  m_compiled_path.set_border(Border_sunken);
  m_compiled_path.show();
  m_compiled_save_tgl.show();
  m_compiled_load_tgl.set_on_toggled_func(new AMethodArg<SkClassSettings, eAFlag>(this, &SkClassSettings::on_toggle_load));
  m_compiled_load_tgl.show();


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Setup Object Hierarchy Overlays UI
  m_overlay_rect.left = spacing;
  m_overlay_down_btn.set_area(area);
  m_overlay_down_btn.set_on_pressed_func(new AMethod<SkClassSettings>(this, &SkClassSettings::on_overlay_down));
  m_overlay_down_btn.show();
  m_overlay_up_btn.set_area(area);
  m_overlay_up_btn.set_on_pressed_func(new AMethod<SkClassSettings>(this, &SkClassSettings::on_overlay_up));
  m_overlay_up_btn.show();
  m_overlay_remap_btn.set_area(area);
  m_overlay_remap_btn.set_on_pressed_func(new AMethod<SkClassSettings>(this, &SkClassSettings::on_overlay_remap));
  m_overlay_remap_btn.show();
  m_overlay_toggle_btn.set_area(area);
  m_overlay_toggle_btn.set_on_pressed_func(new AMethod<SkClassSettings>(this, &SkClassSettings::on_overlay_toggle));
  m_overlay_toggle_btn.show();
  m_overlay_remove_btn.set_area(area);
  m_overlay_remove_btn.set_on_pressed_func(new AMethod<SkClassSettings>(this, &SkClassSettings::on_overlay_remove));
  m_overlay_remove_btn.show();
  m_overlay_add_btn.set_area(area);
  m_overlay_add_btn.set_on_pressed_func(new AMethod<SkClassSettings>(this, &SkClassSettings::on_overlay_add));
  m_overlay_add_btn.show();
  m_overlay_list.set_border(Border_sunken);
  m_overlay_list.show();
  }

//---------------------------------------------------------------------------------------
void SkClassSettings::update_title()
  {
  AString title(g_ide_title);

  title += ": Project Settings (";
  title += SkCompiler::ms_compiler_p->get_ini_project_name();
  title += ")";
  set_title(title);
  }

//---------------------------------------------------------------------------------------
// Applies changed settings if any.
void SkClassSettings::apply_changes()
  {
  // Check for changes to startup mind class
  if (m_eval_mind_class.is_modified())
    {
    AString mind_class_str = m_eval_mind_class.get_text();
    AString old_mind_str   = SkCompiler::ms_compiler_p->get_ini_startup_mind();

    mind_class_str.crop();

    if (mind_class_str != old_mind_str)
      {
      SkClass * mind_class_p = SkBrain::get_class(mind_class_str);

      if ((mind_class_p == nullptr) || (!mind_class_p->is_class(*SkBrain::ms_master_class_p)))
        {
        SkDebug::print_error(
          a_str_format("'%s' is not a subclass of the Master Mind class - defaulting to 'Master' class!", mind_class_str.as_cstr()),
          AErrLevel_error);
        mind_class_str = "Master";
        mind_class_p   = SkBrain::ms_master_class_p;
        }

      m_eval_mind_class.set_text(mind_class_str);
      SkCompiler::ms_compiler_p->set_ini_startup_mind(mind_class_str);

      // $Revisit - CReis Change the startup Mind class while the IDE/runtime are running.
      SkDebug::print(
        "\nChanges to the startup master mind class only take effect once the IDE and runtime are restarted!\n",
        SkLocale_all,
        SkDPrintType_warning);
      }
    }

  SkCompiler::ms_compiler_p->set_load_type(eSkLoad(m_compiled_load_tgl.get_toggle_state()));

  SkCompiler::ms_compiler_p->enable_compiled_save(m_compiled_save_tgl.is_toggled());

  if (m_compiled_path.is_modified())
    {
    SkCompiler::ms_compiler_p->set_ini_compiled_file(m_compiled_path.get_text().crop());
    m_compiled_path.set_modified(false);
    }

  SkCompiler::ms_compiler_p->enable_ensure_atomics(m_eval_atomics_tgl.is_toggled());

  if (m_overlays_changed_b)
    {
    APArray<SkOverlay> overlays;

    m_overlay_list.get_items(&overlays);
    SkCompiler::ms_compiler_p->set_overlays(overlays);
    m_overlays_changed_b = false;
    }
  }

//---------------------------------------------------------------------------------------
// Loads class hierarchy settings and displays dialog.
// Author(s):   Conan Reis
void SkClassSettings::display()
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Load existing settings
  eAFlag load_state = eAFlag(SkCompiler::ms_compiler_p->get_ini_code_load());

  m_compiled_load_tgl.set_toggle_state(load_state);
  on_toggle_load(load_state);
  update_title();

  m_compiled_save_tgl.enable_toggle(SkCompiler::ms_compiler_p->is_flags(SkCompiler::Flag_save_compiled));
  m_compiled_path.set_text(SkCompiler::ms_compiler_p->get_ini_compiled_file());
  m_compiled_path.set_modified(false);
  m_eval_atomics_tgl.enable_toggle(SkCompiler::ms_compiler_p->is_flags(SkCompiler::Flag_ensure_bindings));

  m_overlay_list.remove_all();

  // Copy current overlays
  uint         length         = SkCompiler::ms_compiler_p->m_overlays.get_length();
  SkOverlay ** overlay_pp     = SkCompiler::ms_compiler_p->m_overlays.get_array();
  SkOverlay ** overlay_end_pp = overlay_pp + length;

  for (; overlay_pp < overlay_end_pp; overlay_pp++)
    {
    m_overlay_list.append(*new SkOverlay(**overlay_pp));
    }

  m_overlay_list.column_set_width_rank(0u, LVSCW_AUTOSIZE_USEHEADER);
  m_overlay_list.column_set_width_rank(1u, LVSCW_AUTOSIZE);
  m_overlay_list.column_set_width_rank(2u, LVSCW_AUTOSIZE);
  m_overlay_list.focus_row(0u);
  m_overlay_list.select_row(0u);
  m_overlays_changed_b = false;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Display dialog
  set_pos_centered_display();
  on_sizing();
  show();
  }

//---------------------------------------------------------------------------------------
// Called whenever the overlay list is modified
// Author(s):   Conan Reis
void SkClassSettings::set_overlays_changed()
  {
  if (!m_overlays_changed_b)
    {
    SkDebug::print("\nOnce changes are applied, the code will be reparsed to account for the changes to the overlays.\n");
    }

  m_overlays_changed_b = true;
  }

//---------------------------------------------------------------------------------------
void SkClassSettings::on_startup_mind_modified()
  {
  m_eval_mind_class.syntax_highlight(SkEditSyntax::Coverage_visible);
  }

//---------------------------------------------------------------------------------------
// Ensures that the proper buttons are enabled/disabled when a particular
//             overlay is focused.
// Arg         row - row index of focused overlay
// Notes:      This event can be called when removing the currently focused item but
//             before it is actually removed.  This is due to a new focus being set, but
//             the list length will decrease immediately after this call.
//             focus
// Author(s):   Conan Reis
void SkClassSettings::on_overlay_focus(uint row)
  {
  //A_DPRINT(A_SOURCE_FUNC_STR " on_overlay_focus(row:%u) - len:%u\n", row, m_overlay_list.get_length());

  // Ensure Core library at position 0 is locked
  m_overlay_remove_btn.enable_input(row != 0u);
  m_overlay_toggle_btn.enable_input(row != 0u);
  m_overlay_up_btn.enable_input(row > 1u);
  m_overlay_down_btn.enable_input((row != 0u) && (row != (m_overlay_list.get_length() - 1u)));
  }

//---------------------------------------------------------------------------------------
// Called whenever the toggle state of the 'Evaluate Scripts' checkbox is changed.
// Author(s):   Conan Reis
void SkClassSettings::on_toggle_load(eAFlag new_state)
  {
  const char * text_p = nullptr;

  switch (new_state)
    {
    case AFlag_off:
      text_p = "Load: scripts";
      break;

    case AFlag_on:
      text_p = "Load: compiled code binary";
      break;

    case AFlag__toggle:
      text_p = "Load: newest of binary or scripts";
      break;
    }

  m_compiled_load_tgl.set_text(text_p, true, AHorizAlign_left);
  }

//---------------------------------------------------------------------------------------
// Called when the window client area is to be drawn.
// Returns:    Boolean indicating whether the default Windows procedure with respect to
//             the given message should be called (true) or not (false)
// Modifiers:   virtual - Overridden from AgogGUI\AWindow.
// Author(s):   Conan Reis
bool SkClassSettings::on_draw()
  {
  PAINTSTRUCT ps;
  HDC         hdc     = ::BeginPaint(m_os_handle, &ps);
  int         spacing = get_spacing();

  // Set header1 font
  ::SelectObject(hdc, ((ATrueTypeFont *)AFont::ms_header1_p->m_sys_font_p)->m_font_handle_p);

  ::DrawEdge(hdc, &m_overlay_rect, EDGE_ETCHED, BF_RECT);
  ::ExtTextOut(hdc, m_overlay_rect.left + spacing, m_overlay_rect.top - spacing, 0u, nullptr, g_overlay_text.as_cstr(), g_overlay_text.get_length(), nullptr);
  ::DrawEdge(hdc, &m_compiled_rect, EDGE_ETCHED, BF_RECT);
  ::ExtTextOut(hdc, m_compiled_rect.left + spacing, m_compiled_rect.top - spacing, 0u, nullptr, "Compiled Binary Settings", 24, nullptr);
  ::DrawEdge(hdc, &m_eval_rect, EDGE_ETCHED, BF_RECT);
  ::ExtTextOut(hdc, m_eval_rect.left + spacing, m_eval_rect.top - spacing, 0u, nullptr, "Evaluation Settings", 19, nullptr);

  // Put default font back
  ::SelectObject(hdc, ((ATrueTypeFont *)m_font.m_sys_font_p)->m_font_handle_p);

  ::ExtTextOut(hdc, m_compiled_rect.left + spacing, m_compiled_path_text_y, 0u, nullptr, g_compiled_path_text.as_cstr(), g_compiled_path_text.get_length(), nullptr);
  ::ExtTextOut(hdc, m_eval_rect.left + spacing, m_eval_mind_text_y, 0u, nullptr, g_eval_mind_text.as_cstr(), g_eval_mind_text.get_length(), nullptr);

  ::EndPaint(m_os_handle, &ps);

  return true;
  }

//---------------------------------------------------------------------------------------
// Called when input (keyboard) focus is gained.
// Returns:    In non-OS windows returning true indicates that changing the focus to this
//             window is allowed.  In OS windows (AEditOS, AListOS, ATreeOS, etc.) the
//             return value is ignored.
// See:        on_focus_lost()
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
bool SkClassSettings::on_focus()
  {
//  A_DPRINT(A_SOURCE_FUNC_STR "\n");

  ADialogOS::set_common_parent(this);

  return true;  // Allow focus
  }

//---------------------------------------------------------------------------------------
// Called whenever a window's client size is changing.  Usually this is
//             associated with a user dragging a window's sizing border.
// Examples:   called by the system
// See:        on_size(), on_sized()
// Notes:      This applies to the window's client area size changing and not
//             necessarily the outer edge of the window.
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
void SkClassSettings::on_sizing()
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Setup OK & Cancel buttons [UI built from bottom to top]
  AVec2i cancel_area(m_cancel_btn.get_area());
  AVec2i client_area(get_area_client());
  int    spacing  = get_spacing();
  int    spacing2 = spacing * 2;
  int    x        = client_area.m_x - cancel_area.m_x - spacing;
  int    y        = client_area.m_y - cancel_area.m_y - spacing;

  m_apply_btn.set_position(x, y);
  x -= cancel_area.m_x + spacing;
  m_cancel_btn.set_position(x, y);
  x -= cancel_area.m_x + spacing;
  m_ok_btn.set_position(x, y);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Setup Evaluation Settings UI
  int height           = cancel_area.m_y;
  m_eval_rect.right   = client_area.m_x - spacing;
  m_eval_rect.bottom  = y - spacing2;
  x                   = m_eval_rect.right - height - spacing;
  y                   = m_eval_rect.bottom - height - spacing;
  m_eval_mind_text_y = y + SkConsole_border_offset;
  m_eval_mind_btn.set_position(x, y);
  int width = x;  // Hold on to old x
  x        = m_eval_rect.left + spacing2 + m_font.get_width(g_eval_mind_text);
  width   -= x + spacing;
  m_eval_mind_class.set_region(x, y, width, height);
  x  = m_eval_rect.left + spacing;
  y -= height + spacing;
  m_eval_atomics_tgl.set_position(x, y);
  y -= spacing2;
  m_eval_rect.top = y;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Setup Compiled Binary Settings UI
  m_compiled_rect.right  = client_area.m_x - spacing;
  m_compiled_rect.bottom = y - (spacing * 3);
  x                      = m_compiled_rect.right - height - spacing;
  y                      = m_compiled_rect.bottom - height - spacing;
  m_compiled_path_text_y = y + SkConsole_border_offset;
  m_compiled_remap_btn.set_position(x, y);
  width  = x;  // Hold on to old x
  x      = m_compiled_rect.left + spacing2 + m_font.get_width(g_compiled_path_text);
  width -= x + spacing;
  m_compiled_path.set_region(x, y, width, height);
  x  = m_compiled_rect.left + spacing;
  y -= height + spacing;
  m_compiled_save_tgl.set_position(x, y);
  y -= m_compiled_save_tgl.get_height();
  m_compiled_load_tgl.set_position(x, y);
  y -= spacing2;
  m_compiled_rect.top = y;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Setup Object Hierarchy Overlays UI
  m_overlay_rect.right  = client_area.m_x - spacing;
  m_overlay_rect.top    = spacing2;
  m_overlay_rect.bottom = y - (spacing * 3);
  x                     = m_overlay_rect.right - height - spacing;
  y                     = m_overlay_rect.top + spacing;
  m_overlay_down_btn.set_position(x, y);
  x -= height;
  m_overlay_up_btn.set_position(x, y);
  x -= height;
  m_overlay_remap_btn.set_position(x, y);
  x -= height;
  m_overlay_toggle_btn.set_position(x, y);
  x -= height;
  m_overlay_remove_btn.set_position(x, y);
  x -= height;
  m_overlay_add_btn.set_position(x, y);

  x  = m_overlay_rect.left + spacing;
  y += height + spacing;
  m_overlay_list.set_region(
    x,
    y,
    m_overlay_rect.right - x - spacing,
    m_overlay_rect.bottom - y - spacing);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure that controls are redrawn
  refresh();
  }

//---------------------------------------------------------------------------------------
// Called when overlay add button is pressed
// Author(s):   Conan Reis
void SkClassSettings::on_overlay_add()
  {
  AString path;

  // $Revisit - CReis Should ask for a overlay name rather than just using the name of the dir.

  if (ADialogOS::browse_folder(
    &path,
    "Select class hierarchy overlay code directory.\n"
    "It will have/has a 'Object' directory for the 'Object' class as a subfolder.\n"
    "[The directory title will be used as the overlay name.]\n\n"
    "By default it will allow any class hierarchy folder depth.\n"
    "To specify a different depth and turn on diretory flattening to get around MAX_PATH\n"
    "limitations add '|#' at the end of the overlay line where # is the depth and Object=0"
    ))
    {
    ADirectory  dir(path.crop());
    SkOverlay * overlay_p = new SkOverlay(
      m_overlay_list.get_length() + 1u,
      dir.get_name(),
      SkCompiler::ms_compiler_p->m_ini_proj.make_relative(dir),
      SkCompiler::ms_compiler_p->m_ini_proj.make_qualified(dir),
      true);

    m_overlay_list.append(*overlay_p);
    m_overlay_list.columns_set_width();
    m_overlay_remove_btn.enable_input();
    m_overlay_toggle_btn.enable_input();
    on_overlay_focus(m_overlay_list.get_focus_row());

    set_overlays_changed();
    }
  }

//---------------------------------------------------------------------------------------
// Called when overlay remove button is pressed
// Author(s):   Conan Reis
void SkClassSettings::on_overlay_remove()
  {
  uint focus_row = m_overlay_list.get_focus_row();

  m_overlay_list.remove_row(focus_row);

  uint row    = focus_row;
  uint length = m_overlay_list.get_length();

  while (row < length)
    {
    m_overlay_list.row2item(row)->m_sequence = row + 1u;
    row++;
    }

  bool buttons_b = (length > 1u);

  m_overlay_remove_btn.enable_input(buttons_b);
  m_overlay_toggle_btn.enable_input(buttons_b);

  row = (focus_row < length) ? focus_row : (focus_row - 1u);
  m_overlay_list.focus_row(row);
  m_overlay_list.select_row(row);
  on_overlay_focus(row);  // If the row was already focused it will not call on_overlay_focus() again

  set_overlays_changed();
  }

//---------------------------------------------------------------------------------------
// Called when overlay toggle button is pressed
// Author(s):   Conan Reis
void SkClassSettings::on_overlay_toggle()
  {
  SkOverlay * overlay_p = m_overlay_list.get_focus();

  overlay_p->m_apply_b = !overlay_p->m_apply_b;
  m_overlay_list.update(*overlay_p);
  set_overlays_changed();
  }

//---------------------------------------------------------------------------------------
// Called when overlay remap button is pressed
// Author(s):   Conan Reis
void SkClassSettings::on_overlay_remap()
  {
  SkOverlay * overlay_p = m_overlay_list.get_focus();
  AString     path;

  if (ADialogOS::browse_folder(
      &path,
      a_cstr_format("Please choose SkookumScript class hierarchy overlay '%s' scripts directory.", overlay_p->m_name.as_cstr()),
      overlay_p->m_path_qual))
    {
    ADirectory overlay_dir(path);

    overlay_p->m_path_qual = SkCompiler::ms_compiler_p->m_ini_proj.make_qualified(overlay_dir);
    overlay_p->m_path      = SkCompiler::ms_compiler_p->m_ini_proj.make_relative(overlay_dir);
    m_overlay_list.update(*overlay_p);
    set_overlays_changed();
    }
  }

//---------------------------------------------------------------------------------------
// Called when overlay move up button is pressed
// Author(s):   Conan Reis
void SkClassSettings::on_overlay_up()
  {
  uint row = m_overlay_list.get_focus_row();

  m_overlay_list.row2item(row)->m_sequence--;
  row--;
  m_overlay_list.row2item(row)->m_sequence++;

  ACmpLogical<SkOverlay> cmp_logical;

  m_overlay_list.sort(&cmp_logical);

  on_overlay_focus(row);
  set_overlays_changed();
  }

//---------------------------------------------------------------------------------------
// Called when overlay move down button is pressed
// Author(s):   Conan Reis
void SkClassSettings::on_overlay_down()
  {
  uint row = m_overlay_list.get_focus_row();

  m_overlay_list.row2item(row)->m_sequence++;
  row++;
  m_overlay_list.row2item(row)->m_sequence--;

  ACmpLogical<SkOverlay> cmp_logical;

  m_overlay_list.sort(&cmp_logical);

  on_overlay_focus(row);
  set_overlays_changed();
  }

//---------------------------------------------------------------------------------------
// Called when Compiled Binary remap button "..." is pressed
// Author(s):   Conan Reis
void SkClassSettings::on_compiled_remap()
  {
  AFile compiled_file(m_compiled_path.get_text().crop());
  
  if (ADialogOS::save_file(&compiled_file, "Specify compiled runtime structures binary file to load/save", g_compiled_ext_filter_p, false))
    {
    m_compiled_path.set_text(compiled_file.get_file_str());
    m_compiled_path.set_modified();
    }
  }

//---------------------------------------------------------------------------------------
// Called when "OK" button is pressed
// Author(s):   Conan Reis
void SkClassSettings::on_ok()
  {
  hide();
  apply_changes();
  }


//=======================================================================================
// SkAbout Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Constructor
// Author(s):   Conan Reis
SkAbout::SkAbout() :
  m_ok_btn(this, "Close")
  {
  int spacing = get_spacing();
  int width   = spacing * SkConsole_about_width_spacing;
  int height  = int(width / A_golden_ratio);

  set_area(width, height);
  enable_title_bar();
  #ifndef SK_NO_RESOURCES
    set_icon(IDI_SKOOKUM);
  #else
    set_icon_file(make_qualified(AFile("Media\\SkookumScript.ico")).get_file_str().as_cstr());
  #endif
  set_title_buttons(TitleButton_close);
  set_title(g_ide_title + ": About");

  // Setup close button
  AVec2i ok_area(m_ok_btn.get_area());
  AVec2i client_area(get_area_client());

  ok_area *= 1.15f;
  m_ok_btn.enable_default_border();
  m_ok_btn.set_region(
    (client_area.m_x - ok_area.m_x) / 2,
    client_area.m_y - ok_area.m_y - spacing,
    ok_area.m_x,
    ok_area.m_y);
  m_ok_btn.set_on_pressed_func(new AMethod<SkAbout>(this, &SkAbout::close_default));
  m_ok_btn.show();

  // Setup device context (DC) drawing properties - info is retained since it has its own
  // private DC.
  HDC hdc = ::GetDC(m_os_handle);

  ::SelectObject(hdc, ((ATrueTypeFont *)m_font.m_sys_font_p)->m_font_handle_p);
  ::SetTextColor(hdc, ::GetSysColor(COLOR_WINDOWTEXT));
  ::SetBkMode(hdc, TRANSPARENT);

  ::ReleaseDC(m_os_handle, hdc);
  }

//---------------------------------------------------------------------------------------
// Called when the window client area is to be drawn.
// Returns:    Boolean indicating whether the default Windows procedure with respect to
//             the given message should be called (true) or not (false)
// Modifiers:   virtual - Overridden from AgogGUI\AWindow.
// Author(s):   Conan Reis
bool SkAbout::on_draw()
  {
  PAINTSTRUCT ps;
  HDC         hdc  = ::BeginPaint(m_os_handle, &ps);
  AVec2i      area(get_area_client());
  RECT        tarea = {0, m_font.get_height(), area.m_x, area.m_y};
  AString     text(
    "SkookumScript Programming Language\n"
    "Integrated Development Environment\n\n"
    "Version: " SK_VERSION_TEXT "\n"
    A_PLAT_STR_DESC"\n\n"
    A_COPYRIGHT_TEXT);

  ::DrawText(hdc, text.as_cstr(), text.get_length(), &tarea, DT_CENTER | DT_TOP | DT_NOPREFIX | DT_NOCLIP);

  ::EndPaint(m_os_handle, &ps);

  return true;
  }

//---------------------------------------------------------------------------------------
// Called whenever a key is pressed.
// Arg         key - key code corresponding to a physical key on the keyboard.
//             If Shift-'2' is pressed, AKey_shift is sent first and then AKey_2, not '@'.
//             Defines for codes are prefixed with "AKey_" and are in AgogIO/AKeyboard.hpp
//             AKey_0 thru AKey_9 are the same as ANSI '0' thru '9' (0x30 - 0x39)
//             AKey_A thru AKey_Z are the same as ANSI 'A' thru 'Z' (0x41 - 0x5A)
//             Special characters like AKey_control are also possible.
// Arg         repeated - true if this is a repeated send (holding down the key), false
//             if this is the first time key has been pressed.
// See:        on_character(), on_key_release()
// Notes:      AKeyboard::get_mod_keys() can be called to determine if any modifier keys
//             are in effect.
// Modifiers:   virtual - overridden from AWindow
// Author(s):   Conan Reis
bool SkAbout::on_key_press(
  eAKey key,
  bool  repeated
  )
  {
  switch (key)
    {
    case AKey_escape:
    case AKey_return:
    case AKey_num_enter:
      if (AKeyboard::get_mod_keys() == AKeyMod_none)
        {
        close_default();
        return false;
        }
      break;
    }

  return SkConsole::ms_console_p->on_key_press(key, repeated);
  }


//=======================================================================================
// SkErrorDialog Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Constructor
// Author(s):   Conan Reis
SkErrorDialog::SkErrorDialog() :
  m_recompile_btn(this, "Recompile"),
  m_continue_btn(this, "Continue"),
  m_ok_btn(this, "Close")
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Setup device context (DC) drawing properties - info is retained since it has its own
  // private DC.
  HDC hdc = ::GetDC(m_os_handle);

  ::SelectObject(hdc, ((ATrueTypeFont *)m_font.m_sys_font_p)->m_font_handle_p);
  ::SetTextColor(hdc, ::GetSysColor(COLOR_WINDOWTEXT));
  ::SetBkMode(hdc, TRANSPARENT);
  ::ReleaseDC(m_os_handle, hdc);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Setup
  enable_title_bar();
  #ifndef SK_NO_RESOURCES
    set_icon(IDI_SKOOKUM);
  #else
    set_icon_file(make_qualified(AFile("Media\\SkookumScript.ico")).get_file_str().as_cstr());
  #endif
  set_title_buttons(TitleButton_close);
  set_title(g_ide_title + ": Compile Error!");


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Setup Close button
  m_ok_btn.set_on_pressed_func(new AMethod<SkErrorDialog>(this, &SkErrorDialog::close_default));
  m_ok_btn.enable_default_border();
  m_ok_btn.show();


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Setup Continue button
  m_continue_btn.set_on_pressed_func(new AMethod<SkErrorDialog>(this, &SkErrorDialog::on_btn_continue));
  m_continue_btn.show();


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Setup Recompile button
  AVec2i button_area(m_recompile_btn.get_area());
  
  button_area *= 1.25f;
  m_recompile_btn.set_area(button_area);
  m_recompile_btn.set_on_pressed_func(new AMethod<SkErrorDialog>(this, &SkErrorDialog::on_btn_recompile));
  m_recompile_btn.show();
  }

//---------------------------------------------------------------------------------------
// Set message text and resize error dialog accordingly
// Author(s):   Conan Reis
void SkErrorDialog::set_message(const AString & msg)
  {
  m_text.empty();
  m_text.append(msg);
  m_text.append("\n  Close [Esc/Enter] - close this dialog & recompile or continue by hand");

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Setup dialog window dimensions

  int  spacing = get_spacing();
  int  width   = spacing * 64;
  RECT tarea   = {0, 0, width, 0};
  HDC  hdc     = ::GetDC(m_os_handle);

  // Determine text area
  ::DrawText(hdc, m_text.as_cstr(), m_text.get_length(), &tarea, DT_CALCRECT | DT_LEFT | DT_TOP | DT_NOPREFIX | DT_NOCLIP);

  ::ReleaseDC(m_os_handle, hdc);

  AVec2i button_area(m_recompile_btn.get_area());
  
  AVec2i client_area(
    (tarea.right - tarea.left) + (2 * spacing),
    (tarea.bottom - tarea.top) + (6 * spacing) + button_area.m_y);
  
  set_area_client(client_area);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Setup Close button
  int button_x = client_area.m_x - spacing - button_area.m_x;
  int button_y = client_area.m_y - spacing - button_area.m_y;

  m_ok_btn.set_region(button_x, button_y, button_area.m_x, button_area.m_y);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Setup Continue button
  button_x -= spacing + button_area.m_x;
  m_continue_btn.set_region(button_x, button_y, button_area.m_x, button_area.m_y);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Setup Recompile button
  button_x -= spacing + button_area.m_x;
  m_recompile_btn.set_region(button_x, button_y, button_area.m_x, button_area.m_y);
  }

//---------------------------------------------------------------------------------------
// Called when "Continue" button pressed
// Author(s):   Conan Reis
void SkErrorDialog::on_btn_continue()
  {
  close_default();
  SkConsole::ms_console_p->debug_continue();
  }

//---------------------------------------------------------------------------------------
// Called when "Recompile" button pressed
// Author(s):   Conan Reis
void SkErrorDialog::on_btn_recompile()
  {
  close_default();
  SkConsole::ms_console_p->compile_class_browser(true);
  }

//---------------------------------------------------------------------------------------
// Called when the window client area is to be drawn.
// Returns:    Boolean indicating whether the default Windows procedure with respect to
//             the given message should be called (true) or not (false)
// Modifiers:   virtual - Overridden from AgogGUI\AWindow.
// Author(s):   Conan Reis
bool SkErrorDialog::on_draw()
  {
  PAINTSTRUCT ps;
  HDC         hdc = ::BeginPaint(m_os_handle, &ps);
  AVec2i      area(get_area_client());
  int         spacing = get_spacing();
  RECT        tarea   = {spacing, spacing, area.m_x, area.m_y};

  ::DrawText(hdc, m_text.as_cstr(), m_text.get_length(), &tarea, DT_LEFT | DT_TOP | DT_NOPREFIX | DT_NOCLIP);

  ::EndPaint(m_os_handle, &ps);

  return true;
  }

//---------------------------------------------------------------------------------------
// Called whenever a key is pressed.
// Arg         key - key code corresponding to a physical key on the keyboard.
//             If Shift-'2' is pressed, AKey_shift is sent first and then AKey_2, not '@'.
//             Defines for codes are prefixed with "AKey_" and are in AgogIO/AKeyboard.hpp
//             AKey_0 thru AKey_9 are the same as ANSI '0' thru '9' (0x30 - 0x39)
//             AKey_A thru AKey_Z are the same as ANSI 'A' thru 'Z' (0x41 - 0x5A)
//             Special characters like AKey_control are also possible.
// Arg         repeated - true if this is a repeated send (holding down the key), false
//             if this is the first time key has been pressed.
// See:        on_character(), on_key_release()
// Notes:      AKeyboard::get_mod_keys() can be called to determine if any modifier keys
//             are in effect.
// Modifiers:   virtual - overridden from AWindow
// Author(s):   Conan Reis
bool SkErrorDialog::on_key_press(
  eAKey key,
  bool  repeated
  )
  {
  switch (key)
    {
    case AKey_escape:
    case AKey_return:
    case AKey_num_enter:
      if (AKeyboard::get_mod_keys() == AKeyMod_none)
        {
        close_default();
        return false;
        }
      break;
    }

  return SkConsole::ms_console_p->on_key_press(key, repeated);
  }


//=======================================================================================
// SkConsole Class Data Members
//=======================================================================================

SkConsole * SkConsole::ms_console_p = nullptr;


//=======================================================================================
// SkConsole Method Definitions
//=======================================================================================

//---------------------------------------------------------------------------------------

void SkConsole::initialize()
  {
  // Initialize static variables
  const_cast<AString&>(g_ini_workspace_file_def)   = "Scripts\\workspace.sk";  // Default workspace text file
  const_cast<AString&>(g_ini_compiled_file_def)    = "classes.sk-bin";
  const_cast<AString&>(g_ide_title)                = "SkookumIDE";
  const_cast<AString&>(g_ini_sound_open_def)       = "Media\\Skookum-Open.wav";
  const_cast<AString&>(g_ini_sound_error_def)      = "Media\\Skookum-Error.wav";
  const_cast<AString&>(g_ini_sound_close_def)      = "Media\\Skookum-Close.wav";
  const_cast<AString&>(g_ini_sound_breakpoint_def) = "Media\\Skookum-Breakpoint.wav";
  const_cast<AString&>(g_ini_scheme_def)           = "dark";

  const_cast<AString&>(g_overlay_text)       = "Object Hierarchy Overlays:";
  const_cast<AString&>(g_compiled_path_text) = "Compiled Binary Path:";
  const_cast<AString&>(g_eval_mind_text)     = "Start-up Master Mind Class:";

  SkEditBox::initialize();
  }

//---------------------------------------------------------------------------------------

void SkConsole::deinitialize()
  {
  SkEditBox::deinitialize();

  const_cast<AString&>(g_ini_workspace_file_def)   = AString::ms_empty;  // Default workspace text file
  const_cast<AString&>(g_ini_compiled_file_def)    = AString::ms_empty;
  const_cast<AString&>(g_ide_title)                = AString::ms_empty;
  const_cast<AString&>(g_ini_sound_open_def)       = AString::ms_empty;
  const_cast<AString&>(g_ini_sound_error_def)      = AString::ms_empty;
  const_cast<AString&>(g_ini_sound_close_def)      = AString::ms_empty;
  const_cast<AString&>(g_ini_sound_breakpoint_def) = AString::ms_empty;
  const_cast<AString&>(g_ini_scheme_def)           = AString::ms_empty;

  const_cast<AString&>(g_overlay_text)       = AString::ms_empty;
  const_cast<AString&>(g_compiled_path_text) = AString::ms_empty;
  const_cast<AString&>(g_eval_mind_text)     = AString::ms_empty;
  }

//---------------------------------------------------------------------------------------

bool SkConsole::is_initialized()
  {
  return !g_ini_workspace_file_def.is_empty();
  }

//---------------------------------------------------------------------------------------
// Constructor
// Returns:    itself
// Arg         init_type - start compilation using background phases or do all compilation
//             immediately before returning.  See SkCompiler::eInit
// Arg         close_action - action to take when request to close IDE console occurs
// Author(s):   Conan Reis
SkConsole::SkConsole(
  SkCompiler::eInit init_type,   // = SkCompiler::Init_phased
  eCloseAction      close_action // = CloseAction_hide
  ) :
  SkMainWindowBase(MainWindowBaseType_console),
  m_class_members_inited(init_class_members(this)),
  m_compiler_p(new SkCompiler(this)),
  m_ini_ide(m_compiler_p->get_ini_ide()),
  m_show_browser(false),
  m_play_sounds(false),
  m_pref_flags(Preference__default),
  m_dot_count(0u),
  m_browser_p(nullptr),
  m_goto_view_p(nullptr),
  m_browse(this, "Browser", *AFont::ms_default_p, ARegion(SkConsole_status_inset, SkConsole_status_inset)),
  //m_toggle_evaluate(this, "Evaluate Scripts", ACheckType_2_state, AFont::ms_default, ARegion(m_browse.get_right_rel_spaced(), SkConsole_status_inset)),
  m_split_text(this, ARegion(0, m_browse.get_bottom_rel() + SkConsole_status_inset)),
  m_log(&m_split_text, this),
  m_workspace(&m_split_text, this),
  m_status(this, "", AFont("Arial Narrow", 12.0f), 0, 0, Size_auto, false, false),
  m_print_func(this, &SkConsole::log_append),
  m_close_action(close_action),
  m_version_control(SkVersionControl_none),
  m_disassembly(get_ini_ide().get_value_bool_default(false, g_ini_key_disassembly_p, g_ini_section_view_settings_p)),
  m_expression_guide(get_ini_ide().get_value_bool_default(true, g_ini_key_expression_guide_p, g_ini_section_view_settings_p)),
  m_auto_parse(get_ini_ide().get_value_bool_default(true, g_ini_key_auto_parse_p, g_ini_section_view_settings_p)),
  m_auto_parse_sel(get_ini_ide().get_value_bool_default(true, g_ini_key_auto_parse_sel_p, g_ini_section_view_settings_p)),
  m_syntax_highlight(get_ini_ide().get_value_bool_default(true, g_ini_key_syntax_highlight_p, g_ini_section_view_settings_p)),
  m_current_line_highlight(get_ini_ide().get_value_bool_default(true, g_ini_key_current_line_highlight_p, g_ini_section_view_settings_p))
  {
  SK_ASSERTX(is_initialized(), "SkConsole must be initialized before use!");

  ADialogOS::set_common_parent(this);

  set_font(AFont("Arial Narrow", 10.0f));
  m_online_txt_width = m_font.get_avg_width() * 66;
  

  // Create the Skookum multi-phase compiler
  // Stored under SkCompiler::ms_compiler_p
  SkParser::initialize();

  #ifdef SKOOKUM_IDE_EMBEDDED
    enable_debugging_embedded();
  #endif

  load_settings();

  eSkLocale online_mode = get_ini_online_mode(); 

  // "Browser" button
  m_browse.set_on_pressed_func(new AMethod<SkConsole>(this, &SkConsole::toggle_browser));
  m_browse.show();

  // Evaluate Scripts toggle
  //m_toggle_evaluate.enable_toggle(SkCompiler::ms_compiler_p->m_evaluate_scripts_b);
  //m_toggle_evaluate.set_on_toggled_func(new AMethodArg<SkConsole, eAFlag>(this, &SkConsole::on_toggle_evaluate));
  //m_toggle_evaluate.show();


  // Init sound toggle
  //m_toggle_sound.enable_toggle(m_play_sounds);
  //m_toggle_sound.set_on_toggled_func(new AMethodArg<SkConsole, eAFlag>(this, &SkConsole::on_toggle_sound));
  //m_toggle_sound.show();

  //play_sound(Sound_open);


  // Log window setup
  m_log.set_border(Border_thin_sunken);
  m_log.enable_read_only();
  m_log.enable_culling();
  m_log.set_char_limit(128000);
  m_log.show();
  SkDebug::set_print_func(&m_print_func);
  SkDebug::register_print_with_agog();

  // Workspace window setup
  m_workspace.set_border(Border_sunken);
  m_workspace.enable_culling();
  m_workspace.set_char_limit(1048000);  // About 1MB
  m_workspace.show();

  // Text Splitter setup
  m_split_text.enable_auto_update(false);
  m_split_text.set_orientation(ASplitterOS::eOrient(m_ini_ide.get_value_int_default(
    ASplitterOS::Orient_vert_ab,
    g_ini_key_split_orient_p,
    g_ini_section_console_p)));
  m_split_text.set_ratio(m_ini_ide.get_value_default(
    AString::ctor_float(0.66f),
    g_ini_key_split_ratio_p,
    g_ini_section_console_p).as_float32());
  m_split_text.set_pane_a(&m_log);
  m_split_text.set_pane_b(&m_workspace);
  m_split_text.enable_auto_update();
  m_split_text.show();


  // Status bar setup
  m_status.set_border(Border_thin_sunken);
  m_status.enable_read_only();
  m_status.show();


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Setup main window
  enable_sizing();
  enable_title_bar();
  #ifndef SK_NO_RESOURCES
    set_icon(IDI_SKOOKUM);
  #else
    set_icon_file(make_qualified(AFile("Media\\SkookumScript.ico")).get_file_str().as_cstr());
  #endif
  set_title_buttons(TitleButton_min_max);
  update_title();
  enable_drag_drop();

  //  Setup the common menubar.
  setup_menubar();

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Bring window up using previous settings if available.
  set_region(ini_load_region(g_ini_section_console_p, ARegion(1, 1, 850, 1000), false, &m_ini_ide));

  // $Revisit - CReis Many launching apps seem to give an incorrect starting show state, so ignore it
  eAShowState show_state = ini_load_show_state(g_ini_section_console_p, AShowState_normal_focus, false, &m_ini_ide);
  // $HACK - CReis If init_type=Init_phased assume that it is stand-alone and needs the console displayed
  if (init_type == SkCompiler::Init_phased)
    {
    // Always show and activate - only allow normal or maximized show states
    show_state = (show_state == AShowState_maximized)
      ? AShowState_maximized
      : AShowState_normal_focus;
    }

  set_show_state(show_state);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Set online state
  if (!SkDebug::is_engine_present())
    {
    m_remote.set_mode(online_mode);
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Begin start-up process
  status_update();

  SkCompiler::ms_compiler_p->phases_init(init_type);
  }

//---------------------------------------------------------------------------------------
//  Destructor
// Author(s):    Conan Reis
SkConsole::~SkConsole()
  {
  SkDebug::set_print_func(nullptr);

  if (m_browser_p)
    {
    delete m_browser_p;
    m_browser_p = nullptr;
    }

  if (m_goto_view_p)
    {
    delete m_goto_view_p;
    m_goto_view_p = nullptr;
    }

  ms_console_p = nullptr;

  delete SkCompiler::ms_compiler_p;

  ADialogOS::set_common_parent(nullptr);

  m_member_images.destroy();
  }

//---------------------------------------------------------------------------------------
// Closes console & optionally shuts down
// Author(s):   Conan Reis
void SkConsole::close(
  bool shutdown_b // = false
  )
  {
  //play_sound(Sound_close);

  save_settings();

  SkDebug::breakpoint_remove_all();

  hide();

  if (shutdown_b)
    {
    AApplication::shut_down();
    }
  }

//---------------------------------------------------------------------------------------
// Get shared image list
// Author(s):   Conan Reis
const AImageListOS & SkConsole::get_member_images()
  {
  if (!m_member_images.is_initialized())
    {
    m_member_images.set_handle(
      #ifdef SK_NO_RESOURCES                                                                                                  // RGB(0, 255, 0)
        ::ImageList_LoadImage(nullptr, make_qualified(AFile("Media\\SkMemberIcons.bmp")).get_file_str().as_cstr(), SkConsole_image_width, 0, CLR_DEFAULT, IMAGE_BITMAP, LR_DEFAULTCOLOR | LR_LOADFROMFILE)
      #else
        ::ImageList_LoadImage(AApplication::ms_res_instance, MAKEINTRESOURCE(IDB_MEMBERS), SkConsole_image_width, 0, CLR_DEFAULT, IMAGE_BITMAP, LR_DEFAULTCOLOR)
      #endif
      );
    }

  return m_member_images;
  }

//---------------------------------------------------------------------------------------
// Brings up a dialog with the title "Browse for Folder" where the user can
//             select a folder/directory or cancel.  It is a "new style" type dialog that
//             can be resized, has drag-and-drop capability, has reordering, can delete,
//             has shortcut menus, and other features.
// Returns:    true if folder selected, false if not
// Arg         path_p - path of selected folder.  It is not changed if a folder is not
//             selected.  A ADirectory object can easily be constructed from this path
//             so that more can be done with it.
// Arg         message_p - message text to display between the title bar and the folder
//             tree.  If the text of one line is longer than the dialog is wide, it will
//             wrap to the next line.  Newlines (/n) will put a hard break in a line.
//             Only the first three lines of text are shown.
// Arg         path_start_p - This is an optional pre-selected starting folder.  If it
//             is empty ("") then the current directory will be used.  If it is nullptr
//             then no folder is preselected.  See ADirectory and AApplication for
//             handy starting directories.
// Modifiers:   virtual - Overridden from SkConsoleBase
// Author(s):   Conan Reis
bool SkConsole::browse_folder(
  AString *    path_p,
  const char * message_p,
  const char * path_start_p)
  {
  return ADialogOS::browse_folder(path_p, message_p, path_start_p);
  }

//---------------------------------------------------------------------------------------
// Appends string to log edit box - called by C++ debugger
// Arg         str - string to append to log edit box
// Author(s):   Conan Reis
void SkConsole::log_append(const SkPrintInfo & info)
  {
  ATextStyle style;

  style.m_effect_mask = AText__all;

  switch (info.m_type & SkDPrintType_non_flag_mask)
    {
    case SkDPrintType_standard:  // Skookum (white)
      style.m_font_color_p = &AColor::ms_white;
      break;

    case SkDPrintType_title:     // Title / link (yellow)
      style.m_font_color_p = &SkLog_colour_title;
      break;

    case SkDPrintType_note:      // Notable action (green)
      style.m_font_color_p = &SkLog_colour_note;
      break;

    case SkDPrintType_system:    // C++ (electric blue)
      style.m_font_color_p = &SkLog_colour_system;
      break;

    case SkDPrintType_error:     // (red)
      style.m_font_color_p = &SkLog_colour_error;
      display_ide();
      break;

    case SkDPrintType_warning:   // (orange)
      style.m_font_color_p = &SkLog_colour_warning;
      break;

    case SkDPrintType_result:    // (light yellow)
      style.m_font_color_p = &SkLog_colour_result;
      break;

    case SkDPrintType_trace:     // (lavender)
      style.m_font_color_p = &SkLog_colour_trace;
      break;
    }

  // Differentiate between remote/local prints
  bool remote_ide   = SkRemoteBase::ms_default_p->is_remote_ide();
  bool remote_print = (info.m_type & SkDPrintType_flag_remote) != 0u;

  if ((!remote_ide && remote_print) || (remote_ide && !remote_print))
    {
    style.m_effect_flags = AText_italics;
    }

  m_log.append_style(info.m_str, style);
  }

//---------------------------------------------------------------------------------------
// Plays the specified sound if sound is enabled
// Arg         sound - type of sound to play
// Author(s):   Conan Reis
void SkConsole::play_sound(eSound sound)
  {
  if (m_play_sounds)
    {
    // $Revisit - CReis The sound file should be checked to see if it exists
    ::PlaySound(
      m_sound_file_strs[sound],
      nullptr,
      SND_FILENAME | SND_NOWAIT | SND_ASYNC | SND_NODEFAULT);
    }
  }

//---------------------------------------------------------------------------------------
// Show/hide/toggle IDE
// Author(s):   Conan Reis
void SkConsole::display_ide(
  eAFlag show_flag,
  ASymbol focus_class_name,
  ASymbol focus_member_name,
  bool is_data_member,
  bool focus_member_class_scope
  )
  {
  if (this)
    {
    if (show_flag == AFlag__toggle)
      {
      show_flag = (is_minimized() || is_hidden()) ? AFlag_on : AFlag_off;
      }

    if (show_flag == AFlag_on)
      {
      // Optionally set focus on a given class or member
      SkClass * focus_class_p = nullptr;
      if (focus_class_name != ASymbol::ms_null)
        {
        focus_class_p = SkBrain::get_class(focus_class_name);
        if (focus_class_p)
          {
          display_browser();
          m_browser_p->show();

          bool focus_on_class_tree = true;

          if (focus_member_name != ASymbol::ms_null)
            {
            // Data member or method/coroutine?
            if (!is_data_member)
              {
              // Try to locate the method
              SkInvokableBase * invokable_p = focus_member_class_scope
                ? focus_class_p->find_class_method_inherited(focus_member_name)
                : focus_class_p->find_instance_method_inherited(focus_member_name);
              if (!invokable_p)
                {
                // No method found - see if it's a coroutine
                invokable_p = focus_class_p->find_coroutine_inherited(focus_member_name);
                }
              if (invokable_p)
                {
                SkContextInfo member(*invokable_p, eSkMember(invokable_p->get_invoke_type()), focus_member_class_scope);
                m_browser_p->set_class(invokable_p->get_scope());
                m_browser_p->focus_editor();
                m_browser_p->make_foreground();
                m_browser_p->set_member(member);
                focus_on_class_tree = false;
                }
              }
            }

          if (focus_on_class_tree)
            {
            m_browser_p->set_class(focus_class_p);
            m_browser_p->get_class_tree().set_focus();
            }
          }
        }

      // Browser
      if (m_show_browser || (m_browser_p && m_browser_p->is_minimized()))
        {
        display_browser();
        m_browser_p->show();
        }

      // Console
      if (!focus_class_p) // Bring up unless we focus on a class in the browser
        {
        show();
        make_foreground();
        }
      }
    else
      {
      minimize();

      m_show_browser = false;

      if (m_browser_p && !m_browser_p->is_hidden() && !m_browser_p->is_minimized())
        {
        m_browser_p->minimize();
        }
      }
    }
  }

//---------------------------------------------------------------------------------------
// Show/hide/toggle IDE
void SkConsole::display_ide(eAFlag show_flag /*= AFlag_on*/)
  {
  display_ide(show_flag, ASymbol::ms_null, ASymbol::ms_null, false, false);
  }

//---------------------------------------------------------------------------------------
// Toggles the displays of the SkookumIDE (console & browser/editor)
// Author(s):   Conan Reis
void SkConsole::toggle_ide()
  {
  display_ide(AFlag__toggle);
  }

//---------------------------------------------------------------------------------------
// Displays class settings dialog.
// Author(s):   Conan Reis
void SkConsole::display_about()
  {
  if (!m_about_dlg_p)
    {
    // Create on first showing
    m_about_dlg_p = new SkAbout;
    }

  m_about_dlg_p->set_pos_centered_display();
  m_about_dlg_p->show();
  }

//---------------------------------------------------------------------------------------
// Displays recompile error dialog.
// Author(s):   Conan Reis
void SkConsole::display_error(const AString & msg)
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Prep error string
  AString err_msg;

  err_msg.ensure_size_empty(msg.get_length() + 256u);
  err_msg.append("\nSkookum:  ", 11u);
  err_msg.append(msg);
  err_msg.append(
    "\n\nExamine errors described in log, fix them and either:\n"
    "  Recompile [Alt+F7] - recompile class and resume on success\n"
    "  Continue [F5] - abort recompile and resume runtime");

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Set up error dialog
  if (is_show_error_dialog())
    {
    if (!m_error_dlg_p)
      {
      // Create on first showing
      m_error_dlg_p = A_NEW(SkErrorDialog);
      }

    m_error_dlg_p->set_message(err_msg);
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Always print to the log
  err_msg.append("\n\n", 2u);

  SkDebug::print(err_msg, SkLocale_all, SkDPrintType_error);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Optionally display error dialog
  if (is_show_error_dialog())
    {
    m_error_dlg_p->set_pos_centered_display();
    m_error_dlg_p->show();
    m_error_dlg_p->make_foreground();
    m_error_dlg_p->flash_title();
    }
  }

//---------------------------------------------------------------------------------------
// Hides error dialog if it is up.
// Author(s):   Conan Reis
void SkConsole::hide_error()
  {
  if (m_error_dlg_p)
    {
    m_error_dlg_p->hide();
    }
  }

//---------------------------------------------------------------------------------------
// Displays class settings dialog.
// Author(s):   Conan Reis
void SkConsole::display_class_settings()
  {
  if (!m_classes_dlg_p)
    {
    // Create on first showing
    m_classes_dlg_p = new SkClassSettings(this);
    }

  m_classes_dlg_p->display();
  }

//---------------------------------------------------------------------------------------
// Displays the class browser
// Author(s):   Conan Reis
SkClassBrowser * SkConsole::display_browser(
  bool show_b // = true
  )
  {
  if (show_b)
    {
    if (SkCompiler::ms_compiler_p->get_phase() >= SkCompiler::Phase_bind_atomics)
      {
      m_show_browser = false;

      if (m_browser_p == nullptr)
        {
        m_browser_p = new SkClassBrowser(this);
        }
      else
        {
        if (m_browser_p->is_hidden())
          {
          m_browser_p->load_settings();
          }
        }

      m_browser_p->show();
      m_browser_p->make_foreground();
      }
    else
      {
      m_show_browser = true;
      }
    }
  else
    {
    if (m_browser_p)
      {
      m_browser_p->hide();
      }
    }

  save_settings();

  return m_browser_p;
  }

//---------------------------------------------------------------------------------------
// Toggles the class browser
// Author(s):   Conan Reis
void SkConsole::toggle_browser()
  {
  display_browser((m_browser_p == nullptr) || m_browser_p->is_hidden());
  }

//---------------------------------------------------------------------------------------
// Displays and focuses on the Browser and brings up the specified
//             member at the specified position.
// Arg         info - script/member to browse
// Arg         index_start - source file selection start
// Arg         index_end - source file selection end
// Author(s):   Conan Reis
void SkConsole::browse_member(
  const SkContextInfo & member_info,
  uint                   index_start, // = 0u
  uint                   index_end    // = ADef_uint32
  )
  {
  display_browser();
  m_browser_p->set_member(member_info, index_start, index_end);
  m_browser_p->focus_editor();
  }

//---------------------------------------------------------------------------------------
// Brings up the "Go To" dialog
// Author(s):   Conan Reis
void SkConsole::display_goto_dialog(
  eSkMatchKind            kind,        // = SkMatchKind_all
  const AString &         match_text,  // = AString::ms_empty
  const SkMatchCriteria * match_info_p // = nullptr
  )
  {
  if (!m_goto_view_p)
    {
    m_goto_view_p = new SkSearchDialog;
    }
  m_goto_view_p->display(kind, match_text, match_info_p);
  }

//---------------------------------------------------------------------------------------
// Brings up the "Go To" dialog and populates it with text based on the
//             current context.
// Author(s):   Conan Reis
void SkConsole::display_goto_context(const SkEditBox & editor)
  {
  AString      match_text;
  eSkMatchKind kind = SkMatchKind_all;

  SkMatchCriteria   match_info;
  SkMatchCriteria * match_info_p = nullptr;

  if (editor.is_selected())
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Use current selection if there is one
    editor.get_selection(&match_text);
    match_text.crop();

    if (match_text.is_empty())
      {
      kind = SkMatchKind__invalid;
      }
    }
  else
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Examine context around carat position
    kind = editor.caret_context(&match_info);

    if (kind != SkMatchKind__invalid)
      {
      match_info_p = &match_info;
      }

    match_text = match_info.as_string();
    }

  if (kind != SkMatchKind__invalid)
    {
    display_goto_dialog(kind, match_text, match_info_p);
    }
  else
    {
    display_goto_dialog(SkMatchKind_all);
    }
  }

//---------------------------------------------------------------------------------------
// Brings up the "Go To" dialog and populates it with text based on the
//             current member focus.
// Author(s):   Conan Reis
void SkConsole::display_goto_context_focus()
  {
  const SkContextInfo & info = SkClassBrowser::ms_browser_p->get_member_info();
  eSkMatchKind          kind = SkMatchKind_all;

  if (info.is_valid())
    {
    kind = (info.m_type == SkMember_data)
      ? SkMatchKind_data
      : ((info.m_type == SkMember_class_meta)
        ? SkMatchKind_classes
        : SkMatchKind_routines);
    }

  display_goto_dialog(kind);
  }

//---------------------------------------------------------------------------------------
// Brings up the "Go To" dialog and populates it with text based on the
//             current context in the class browser editor.
// Author(s):   Conan Reis
void SkConsole::display_goto_context_editor()
  {
  display_browser();
  display_goto_context(m_browser_p->get_edit_view().get_editor());
  }

//---------------------------------------------------------------------------------------
// Brings up the "Go To" dialog and populates it with text based on the
//             current context in the workspace window.
// Author(s):   Conan Reis
void SkConsole::display_goto_context_workspace()
  {
  display_goto_context(m_workspace);
  }

//---------------------------------------------------------------------------------------
// Automatically run most *obvious* action on supplied string - either:
//               1) Browse to class in class tree in SkookumIDE Browser if it is
//                  a class directory from a script overlay
//               2) Browse to member and open script file in SkookumIDE Browser if
//                  it is a member script file from a script overlay
//               3) execute file contents
// Returns:    true if string executed successfully and false if not
// Arg         simple_str - simple string to figure out what to do with
// See:        cmd_args_execute(), on_drag_drop()
// Author(s):   Conan Reis
bool SkConsole::cmd_args_execute_auto(const AString & simple_str)
  {
  // Determine if it is a file
  AString str(simple_str);

  // Remove any whitespace
  str.crop();

  SkContextInfo member;
  SkOverlay *   overlay_p = nullptr;
  bool          topmost   = false;
  
  if (cmd_arg_parse_ident(str, &member, &overlay_p, &topmost))
    {
    if (member.m_type == SkMember_class_meta)
      {
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Browse to class if in class hierarchy from a valid overlay
      SkClass * class_p = member.m_member_id.get_scope();

      // topmost is true if it was identifier based rather than file based
      if (topmost)
        {
        SkDebug::print(a_str_format("\nBrowsing to class '%s'...\n", class_p->get_name_cstr_dbg()), SkLocale_ide);
        }
      else
        {
        SkDebug::print(a_str_format("\nBrowsing to class '%s'\n  based on:  %s ...\n", class_p->get_name_cstr_dbg(), str.as_cstr()), SkLocale_ide);
        }

      display_browser();
      m_browser_p->set_class(member.m_member_id.get_scope());

      return true;
      }

    // Must be either a method, coroutine or data member
    AString ident(member.as_file_title());

    // Also ensures that the file is in an enabled overlay and it is the topmost overlay
    // override.
    if (topmost || (overlay_p == nullptr))
      {
      SkDebug::print(a_str_format("\nBrowsing to member:\n  %s ...\n", ident.as_cstr()), SkLocale_ide);
      }
    else
      {
      // Overridden member or member from unapplied overlay
      SkDebug::print(a_str_format("\nBrowsing *%s* member file:\n  %s ...\n", overlay_p->m_apply_b ? "overridden" : "unapplied overlay", ident.as_cstr()), SkLocale_ide, SkDPrintType_warning);
      }

    member.action_goto_browser();

    return true;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Not valid identifier or path overlay file

  // Determine if it is a valid file
  // Remove any quotes
  if (str.get_last() == '"')
    {
    str.remove_end(1u);
    }

  if (str.get_first() == '"')
    {
    str.remove_all(0u, 1u);
    }

  // Browse to class if in class hierarchy from a valid overlay
  eAPathType path_type = AFile::path_determine_type(str);

  if (path_type != APathType_file)
    {
    return false;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Compile and execute file
  AFile file(str);

  return SkCompiler::ms_compiler_p->execute_file(&file);
  }

//---------------------------------------------------------------------------------------
// Display command-line help
// 
// See:       cmd_arg*() methods
// Modifiers: static
// Author(s): Conan Reis
void SkConsole::cmd_args_help()
  {
  SkDebug::print(
    "\n----------------------------------\n"
    "SkookumScript IDE Command-line Arguments\n"
    "----------------------------------\n\n"
    "Arguments can be sent to the IDE on its start-up or after it is already running.\n"
    "Arguments can optionally be quoted \"arg\".\n\n"
    "The IDE can be called with a single identifier or file/path argument where:\n"
    "  Script overlay class name or directory\n"
    "    - browse to class in SkookumIDE Browser\n"
    "  Script overlay member name or file\n"
    "    - browse to member in SkookumIDE Browser\n"
    "  other file\n"
    "    - execute file contents as a script on the startup Mind object\n\n"
    "Alternatively the IDE can be called with the following switches:\n\n"
    "  -c[f]               Recompile all scripts.  -c Recompiles only if binaries are stale\n"
    "                      and -cf forces a recompile whether stale or not.\n"
    "                      [Ignored at start-up.]\n\n"
    "  -cc <Class>         Recompile specified class.\n"
    "      <class path>    [Ignored at start-up.  Send results to runtime if connected.]\n\n"
    "  -cs <Class>         Recompile specified class and any subclasses.\n"
    "      <class path>    [Ignored at start-up.  Send results to runtime if connected.]\n\n"
    "  -cm <Class@Member>  Recompile specified coroutine or method member.\n"
    "      <member path>   [Ignored at start-up.]\n\n"
    "  -b  <Class@Member>  Browse to specified member (coroutine or method) or class using its\n"
    "      <member path>   qualified name or file/path.\n"
    "      <Class>\n"
    "      <class path>\n\n"
    "  -x[i] <expressions> Execute specified expression(s) on the startup Mind object.\n"
    "                      -x runs script on the runtime and -xi runs the script locally\n"
    "                      on the IDE.\n\n"
    "  -xf[i] <file>       Execute specified file contents as a script on the startup Mind object.\n"
    "                      -xf runs script on the runtime and -xfi runs locally on the IDE.\n\n"
    "  -f                  Bring SkookumIDE to foreground.\n\n"
    "                      [Can be added prior to other switches.]\n\n"
    "  -? or -h            Displays this help blurb\n\n"
    "  [Switches may use forward slash (/) rather than dash (-) if desired.]\n\n"
    "See - http://skookumscript.com/docs/v3.0/ide/command-line/ \n\n",
    SkLocale_ide,
    SkDPrintType_warning);
  }

//---------------------------------------------------------------------------------------
// Removes double quotes (") from a fully quoted argument. If it starts with a quote and
// does not end with a quote then it assumes that it is a string literal or something
// similar and leaves the quotes.
// 
// Assumes argument is from `idx_begin` to end of `cmd_str`.
AString SkConsole::cmd_arg_parse_unquote(
  const AString & cmd_str,
  uint32_t        idx_begin
  )
  {
  bool         quoted = false;
  const char * cstr_p = cmd_str.as_cstr();

  // Find beginning
  if (cstr_p[idx_begin] == '"')
    {
    quoted = true;
    idx_begin++;

    if (!cmd_str.find(ACharMatch_not_white_space, 1u, &idx_begin, idx_begin))
      {
      return AString::ms_empty;
      }
    }

  // Find ending
  uint32_t idx_end;

  if (!cmd_str.find_reverse(ACharMatch_not_white_space, 1u, &idx_end, idx_begin))
    {
    return AString::ms_empty;
    }

  if (quoted)
    {
    if (cstr_p[idx_end] == '"')
      {
      idx_end--;

      if (idx_begin <= idx_end)
        {
        cmd_str.find_reverse(ACharMatch_not_white_space, 1u, &idx_end, idx_begin, idx_end);
        }
      }
    else
      {
      // Assume it was a string literal or something similar since it didn't have an ending quote.
      idx_begin--;
      }
    }

  idx_end++;

  if (idx_begin >= idx_end)
    {
    return AString::ms_empty;
    }

  return AString(cstr_p + idx_begin, idx_end - idx_begin);
  }

//---------------------------------------------------------------------------------------
// Parse supplied string at specified position for a file 
// 
// Returns: true if valid file string found (which may or may not exist) or false if not
// Params:
//   str: string to parse
//   idx_begin: character index position to start parse
//   idx_end_p: address to store character index position where parse ended
//   file_p: address to store file found
//   
// Modifiers: static
// Author(s): Conan Reis
bool SkConsole::cmd_arg_parse_file(
  const AString & str,
  eSkLocale exec_locale
  )
  {
  if (str.is_empty())
    {
    return false;
    }

  AFile file(str);

  return file.is_existing()
    && SkCompiler::ms_compiler_p->execute_file(&file, exec_locale);
  }

//---------------------------------------------------------------------------------------
// Parses member (non-inherited) or class identifier from qualified name or file/directory
// path.
// 
// Returns: true if arg parsed successfully and false if not
// 
// Params:
//   ident_str: class / member identifier or file
//   info_p:    member or class info
//   overlay_pp:
//     pointer to store address of overlay if a member (ignored if nullptr)
//   topmost_p: true if member is not overridden
//   
// Notes:
//   Some example identifiers:
//   ```
//   Master
//   D:\Agog\Projects\SkookumScript\_Scripts\Test\Object\Mind\Master
//   "D:\Agog\Projects\SkookumScript\_Scripts\Test\Object\Mind\Master"
//   
//   Master@_test
//   Master._test
//   D:\Agog\Projects\SkookumScript\_Scripts\Test\Object\Mind\Master\_test().sk
//   "D:\Agog\Projects\SkookumScript\_Scripts\Test\Object\Mind\Master\_test().sk"
//   ```
// See:       cmd_arg*() methods
// Modifiers: static
// Author(s): Conan Reis
bool SkConsole::cmd_arg_parse_ident(
  const AString & ident_str,
  SkMemberInfo *  info_p,
  SkOverlay **    overlay_pp, // = nullptr
  bool *          topmost_p   // = nullptr
  )
  {
  SkParser parser(ident_str);

  parser.identify_member_name(info_p, 0u);

  // $Revisit - CReis Would be nice to give a more detailed error

  bool try_path = true;

  if (info_p->m_type < SkMember__invalid)
    {
    try_path = false;

    if (topmost_p)
      {
      // Identifiers are always considered to be the topmost member
      *topmost_p = true;
      }
    }
  else
    {
    try_path = info_p->get_class() == nullptr;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Didn't determine member type via identifier names so try treating as a file
  if (try_path)
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Determine if directory representing a class in the hierarchy
    eAPathType path_type = AFile::path_determine_type(ident_str);

    if (path_type == APathType_directory)
      {
      ADirectory dir(ident_str);

      // Error unless parses as class
      info_p->m_type = SkMember__error;

      // Ensures that directories with any sort of extension are skipped
      if (!dir.is_extensioned())
        {
        ASymbol  class_name;
        uint32_t end_pos;
        SkParser dir_parser(dir.get_name());

        // Ensures that directories that do not have a valid class name are skipped
        if ((dir_parser.parse_name_class(0u, &end_pos, &class_name) == SkParser::Result_ok)
          && (end_pos == dir_parser.get_length()))
          {
          SkClass * class_p = SkBrain::get_class(class_name);

          if (class_p)
            {
            // It represents a class
            info_p->m_type = SkMember_class_meta;
            info_p->m_member_id.set_scope(class_p);
            }
          }
        }
      }


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Parse file
    if (path_type == APathType_file)
      {
      AFile file(ident_str);

      // Determine if it is a valid member
      SkOverlay * overlay_p = SkCompiler::ms_compiler_p->find_file_overlay(file, info_p, topmost_p);

      if (overlay_pp)
        {
        *overlay_pp = overlay_p;
        }
      }
    }

  return (info_p->m_type < SkMember__invalid);
  }

//---------------------------------------------------------------------------------------
// Parses single command-line argument
// 
// Returns: true if arg parsed successfully and false if not
// 
// Params:
//   cmd_str:   full command-line string
//   idx_begin: character index position to start parse
//   idx_end_p: address to store character index position where parse ended
//   flags:
//     parse state to keep track of earlier command line arguments within the same
//     command-line string so they can interact.  [A pointer to a 0 should be passed in
//     on the first call.]
//     
// See:       cmd_arg*() methods
// Modifiers: static
// Author(s): Conan Reis
bool SkConsole::cmd_arg_parse(
  const AString & cmd_str,
  uint32_t        idx_begin,
  uint32_t *      idx_end_p,
  uint32_t *      flags_p
  )
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  struct Nested
    {
    // Argument interaction flag update - brings log, class view, editor to foreground
    // depending on previous command-line arguments.
    static void update_flags(eSkCmdFlag new_flag, uint32_t * flags_p)
      {
      *flags_p |= new_flag;

      if (*flags_p & SkCmdFlag_foreground)
        {
        if ((*flags_p & SkCmdFlag_log) || ((*flags_p & SkCmdFlag__view_mask) == 0u))
          {
          ms_console_p->make_foreground();
          ms_console_p->get_log().set_focus();
          }

        if (*flags_p & SkCmdFlag_class)
          {
          SkClassTree & class_view = ms_console_p->display_browser()->get_class_tree();

          class_view.set_focus();
          }

        if (*flags_p & SkCmdFlag_member)
          {
          ms_console_p->display_browser()->focus_editor();
          }
        }
      }
    };

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
      // Skip rest of args
      idx_begin = length;
      Nested::update_flags(SkCmdFlag_log, flags_p);
      cmd_args_help();
      break;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case 'b':
      {
      // Browse
      idx_begin++;

      // Scan past any whitespace
      cmd_str.find(ACharMatch_not_white_space, 1u, &idx_begin, idx_begin);

      SkContextInfo member;
      SkOverlay *   overlay_p = nullptr;
      bool          topmost   = false;

      success = cmd_arg_parse_ident(
        cmd_arg_parse_unquote(cmd_str, idx_begin),
        &member,
        &overlay_p,
        &topmost);

      if (success)
        {
        idx_begin = length;

        if (member.m_type == SkMember_class_meta)
          {
          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          // Browse to class if in class hierarchy from a valid overlay
          Nested::update_flags(SkCmdFlag_class, flags_p);

          SkClass * class_p = member.m_member_id.get_scope();

          // topmost is true if it was identifier based rather than file based
          if (topmost)
            {
            SkDebug::print(a_str_format("\nBrowsing to class '%s'...\n", class_p->get_name_cstr_dbg()), SkLocale_ide);
            }
          else
            {
            SkDebug::print(a_str_format("\nBrowsing to class '%s'\n  based on:  %s ...\n", class_p->get_name_cstr_dbg(), cmd_str.as_cstr()), SkLocale_ide);
            }

          ms_console_p->display_browser();
          ms_console_p->m_browser_p->set_class(member.m_member_id.get_scope());
          }
        else
          {
          // Must be either a method, coroutine or data member
          Nested::update_flags(SkCmdFlag_member, flags_p);

          AString ident(member.as_file_title());

          // Also ensures that the file is in an enabled overlay and it is the topmost overlay
          // override.
          if (topmost || (overlay_p == nullptr))
            {
            SkDebug::print(a_str_format("\nBrowsing to member:\n  %s ...\n", ident.as_cstr()), SkLocale_ide);
            }
          else
            {
            // Overridden member or member from unapplied overlay
            SkDebug::print(a_str_format("\nBrowsing *%s* member file:\n  %s ...\n", overlay_p->m_apply_b ? "overridden" : "unapplied overlay", ident.as_cstr()), SkLocale_ide, SkDPrintType_warning);
            }

          member.action_goto_browser();
          }
        }
      break;
      }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case 'c':
      {
      // Compile
      bool force   = false;
      bool partial = false;  // Partial compile

      Nested::update_flags(SkCmdFlag_log, flags_p);
      idx_begin++;
      ch = cmd_cstr_p[idx_begin];

      switch (ch)
        {
        case 'c':  // Compile class
        case 's':  // Compile class & subclasses
        case 'm':  // Compile member
          idx_begin++;
          partial = true;
          break;

        case 'f':
          // Force compile all
          idx_begin++;
          ch = cmd_cstr_p[idx_begin];
          force = true;
          // Allow fall-through

        default:
          if ((ch == '\0') || !AString::ms_char_match_table[ACharMatch_identifier][uint8_t(ch)])
            {
            // Compile all
            if (force)
              {
              ms_console_p->compile_project();
              }
            else
              {
              ms_console_p->compile_project_stale();
              }
            }
          else
            {
            success = false;
            }
        }

      // Partial compile?
      if (partial)
        {
        // Scan past any whitespace
        cmd_str.find(ACharMatch_not_white_space, 1u, &idx_begin, idx_begin);

        // Determine class/member
        SkContextInfo member;
        SkOverlay *   overlay_p = nullptr;
        bool          topmost   = false;

        success = cmd_arg_parse_ident(
          cmd_arg_parse_unquote(cmd_str, idx_begin),
          &member,
          &overlay_p,
          &topmost);

        if (success)
          {
          idx_begin = length;

          switch (ch)
            {
            case 'c':
              // Compile class
              ms_console_p->compile_class(member.get_class(), false);
              break;

            case 's':
              // Compile class & subclasses
              ms_console_p->compile_class(member.get_class(), true);
              break;

            case 'm':
              // Compile member
              ms_console_p->compile_member(member);
              break;
            }
          }
        }

      break;
      }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case 'f':
      // Set focus/bring to foreground
      idx_begin++;
      Nested::update_flags(SkCmdFlag_foreground, flags_p);
      break;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case 'x':  // -x[f][i]
      {
      // Execute script
      idx_begin++;

      Nested::update_flags(SkCmdFlag_log, flags_p);

      // Determine if executing a file or an expression
      bool exec_file = false;

      if (cmd_cstr_p[idx_begin] == 'f')
        {
        idx_begin++;
        exec_file = true;
        }

      // Determine if executing on the runtime or the IDE
      eSkLocale exec_locale = SkLocale_runtime;

      if (cmd_cstr_p[idx_begin] == 'i')
        {
        idx_begin++;
        exec_locale = SkLocale_ide;
        }


      // $Revisit - CReis Add [c] modifier to store result in the clipboard so a command like:
      // `SkookumIDE -xic 5 * 7`
      // Would store 35 in the clipboard.


      // Ensure remainder of string is long enough to be interesting
      if ((length - idx_begin) <= 1)
        {
        success = false;
        break;
        }

      // Scan past any whitespace
      cmd_str.find(ACharMatch_not_white_space, 1u, &idx_begin, idx_begin);

      // Don't show help for a bad parse
      *flags_p |= SkCmdFlag_suppres_help;

      if (exec_file)
        {
        // Determine file to execute and invoke it
        success = cmd_arg_parse_file(cmd_arg_parse_unquote(cmd_str, idx_begin), exec_locale);
        idx_begin = length;
        }
      else
        {
        // Determine expression to execute
        AString code(cmd_arg_parse_unquote(cmd_str, idx_begin));

        idx_begin = length;

        // Invoke expression on runtime or IDE
        SkRemoteIDE * remote_p = ms_console_p->get_remote_ide();

        if ((exec_locale == SkLocale_runtime) && remote_p->is_remote_ide())
          {
          remote_p->cmd_invoke(code);

          // $Revisit - CReis Should get feedback from remote side that indicates whether it
          // executed successfully or not.
          success = true;
          }
        else
          {
          success = (SkParser::invoke_script(code) <= SkParser::Result_ok_deferred);
          }
        }

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
// Params:
//   cmd_str: full command-line string
//   
// See:       cmd_args_execute_auto(), on_drag_drop(), cmd_arg*() methods
// Modifiers: static
// Author(s): Conan Reis
void SkConsole::cmd_args_execute(const AString & cmd_str)
  {
  // Note that whitespace should already be cropped off.
  if (cmd_str.is_empty())
    {
    // If second instance of IDE called without arguments - ensure this instance is shown.
    ms_console_p->display_ide();
    return;
    }

  SkDebug::print(a_cstr_format("\nCommand-line arguments:\n  %s\n", cmd_str.as_cstr()));

  bool     arg_error   = false;
  char     ch          = cmd_str.get_first();
  uint32_t idx         = 0u;
  uint32_t parse_flags = 0u;

  if ((ch == '-') || (ch == '/'))
    {
    uint32_t cmd_length = cmd_str.get_length();

    // $Revisit - CReis Check all command-line arguments before any are executed?
    while ((idx < cmd_length) && cmd_arg_parse(cmd_str, idx, &idx, &parse_flags))
      {
      }

    arg_error = (idx != cmd_length);
    }
  else
    {
    arg_error = !ms_console_p->cmd_args_execute_auto(cmd_str);
    }

  if (arg_error && ((parse_flags & SkCmdFlag_suppres_help) == 0u))
    {
    SkDebug::print(
      a_cstr_format("\nError in SkookumIDE command-line arguments!\n  %s\n\n", cmd_str.as_cstr() + idx),
      SkLocale_ide,
      SkDPrintType_error);

    cmd_args_help();
    }
  }

//---------------------------------------------------------------------------------------
void SkConsole::update_title()
  {
  AString title(g_ide_title);

  title += ": Console (";
  title += SkCompiler::ms_compiler_p->get_ini_project_name();
  title += ")";
  set_title(title);

  // Update related window titles too
  if (m_browser_p)
    {
    m_browser_p->update_title();
    }

  SkClassSettings * project_view_p = m_classes_dlg_p;

  if (project_view_p)
    {
    project_view_p->update_title();
    }
  }

//---------------------------------------------------------------------------------------
// Update displayed connection status.
void SkConsole::refresh_status_remote()
  {
  m_remote_status.empty();
  m_remote_status.append(SkCompiler::ms_compiler_p->get_ini_project_name());
  m_remote_status.append(m_remote.is_authenticated()
    ? ": runtime connected"
    : ": disconnected");
  invalidate(true, true);
  }

//---------------------------------------------------------------------------------------
// Update the status bar
// Modifiers:   virtual - Overridden from SkConsoleBase
// Author(s):   Conan Reis
void SkConsole::status_update()
  {
  m_status.set_text(a_str_format(
    "Classes: %u  Methods: %u  Coroutines: %u  Data Members: %u    Errors: %u",
    SkCompiler::ms_compiler_p->m_classes,
    SkCompiler::ms_compiler_p->m_methods,
    SkCompiler::ms_compiler_p->m_coroutines,
    SkCompiler::ms_compiler_p->m_data_members,
    SkCompiler::ms_compiler_p->m_errors));
  m_status.invalidate(true, true);
  }

//---------------------------------------------------------------------------------------
// Add a 'progress' dot/period and occasionally yield to other windows apps.
// Arg         completed - if true any accumulated dots are printed and the dot count is
//             reset to 0.
// Modifiers:   virtual - Overridden from SkConsoleBase
// Author(s):   Conan Reis
void SkConsole::progress_dot(
  bool completed // = false
  )
  {
  m_dot_count++;

  // Only bother printing a whole line of dots
  if (completed || (m_dot_count >= SkConsole_dots_per_line_max))
    {
    char    buffer_p[100];
    AString str(buffer_p, 100u, 0u);

    str.append("\n  ", 3u);
    str.append('.', m_dot_count);
    log_append(str);

    m_dot_count = 0u;
    status_update();
    }
  }

//---------------------------------------------------------------------------------------
// Loads the console settings
// 
// See:         save_settings()
// Author(s):   Conan Reis
void SkConsole::load_settings()
  {
  // The view settings are initially loaded in the constructor

  AKeyboard::enable_locale_alt(m_ini_ide.get_value_bool_default(
    AKeyboard::is_locale_alt(),
    g_ini_key_locale_right_alt,
    g_ini_section_console_p));

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Version Control
  AString vcs_str = m_ini_ide.get_value_default(
    AString::ms_empty,
    g_ini_key_version_control_p,
    g_ini_section_sbrowser_p);

  if (vcs_str == "p4")
    {
    m_version_control = SkVersionControl_p4;
    ADialogOS::register_writable_p4_dialog();
    }
  else
    {
    m_version_control = SkVersionControl_none;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Script template settings
  m_template_dir = m_ini_ide.get_value_dir_default(
      ADirectory("Scripts\\_Templates\\"),
      "ScriptTemplateDir",
      g_ini_section_sbrowser_p);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Sound settings
  m_play_sounds = m_ini_ide.get_value_bool_default(
    SkConsole_play_sounds_def,
    g_ini_key_play_sounds_p,
    g_ini_section_console_p);

  // Load sound file strings
  m_sound_file_strs[Sound_open] = m_ini_ide.get_value_file_default(
    g_ini_sound_open_def, g_ini_key_sound_open_p, g_ini_section_console_p).get_file_str();
  m_sound_file_strs[Sound_close] = m_ini_ide.get_value_file_default(
    g_ini_sound_close_def, g_ini_key_sound_close_p, g_ini_section_console_p).get_file_str();
  m_sound_file_strs[Sound_error] = m_ini_ide.get_value_file_default(
    g_ini_sound_error_def, g_ini_key_sound_error_p, g_ini_section_console_p).get_file_str();
  m_sound_file_strs[Sound_breakpoint] = m_ini_ide.get_value_file_default(
    g_ini_sound_breakpoint_def, g_ini_key_sound_breakpoint_p, g_ini_section_console_p).get_file_str();


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Debug settings
  if (m_ini_ide.get_value_bool_default(
    true,
    g_ini_key_error_dialog_p,
    g_ini_section_console_p))
    {
    m_pref_flags |= Preference_error_dialog;
    }
  else
    {
    m_pref_flags &= ~Preference_error_dialog;
    }

  if (m_ini_ide.get_value_bool_default(
    true,
    g_ini_key_remote_update_p,
    g_ini_section_console_p))
    {
    m_pref_flags |= Preference_update_remote;
    }
  else
    {
    m_pref_flags &= ~Preference_update_remote;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Load workspace text from previous session if it exists
  m_workspace.load_settings();
  }

//---------------------------------------------------------------------------------------
// Saves the console and browser settings
// 
// See: load_settings()
void SkConsole::save_settings()
  {
  if (m_ini_ide.ensure_writable_query())
    {
    ini_save_view(g_ini_section_console_p, false, &m_ini_ide);

    m_ini_ide.set_value(
      AString::ctor_int(m_split_text.get_orientation()),
      g_ini_key_split_orient_p,
      g_ini_section_console_p);

    m_ini_ide.set_value(
      AString::ctor_float(m_split_text.get_ratio()),
      g_ini_key_split_ratio_p,
      g_ini_section_console_p);

    //  Save View settings
    m_ini_ide.set_value_bool( is_disassembly(),             g_ini_key_disassembly_p,            g_ini_section_view_settings_p);
    m_ini_ide.set_value_bool( is_expression_guide(),        g_ini_key_expression_guide_p,       g_ini_section_view_settings_p);
    m_ini_ide.set_value_bool( is_auto_parse(),              g_ini_key_auto_parse_p,             g_ini_section_view_settings_p);
    m_ini_ide.set_value_bool( is_auto_parse_sel(),          g_ini_key_auto_parse_sel_p,         g_ini_section_view_settings_p);
    m_ini_ide.set_value_bool( is_syntax_highlight(),        g_ini_key_syntax_highlight_p,       g_ini_section_view_settings_p);
    m_ini_ide.set_value_bool( is_current_line_highlight(),  g_ini_key_current_line_highlight_p, g_ini_section_view_settings_p);

    //  Save Auto-Complete settings
    m_ini_ide.set_value_bool(SkAutoComplete::is_active(), g_ini_auto_complete_active_p,       g_ini_section_auto_complete_p);

    // Save Browser Settings
    bool browser_shown = false;

    if (m_browser_p)
      {
      browser_shown = !m_browser_p->is_hidden();
      m_browser_p->save_settings();
      }

    m_ini_ide.set_value_bool(
      browser_shown,
      g_ini_key_show_browser_p,
      g_ini_section_sbrowser_p);
    }

  // Save out workspace text so that it can be reloaded the next time the console is run.
  m_workspace.save_settings(SkLog_silent);

  m_log.save_settings(SkLog_silent);
  }

//---------------------------------------------------------------------------------------
// Disables any existing breakpoints
// Author(s):   Conan Reis
void SkConsole::breakpoints_disable_all()
  {
  uint32_t bp_count = SkDebug::breakpoint_get_count();

  if (bp_count)
    {
    SkDebug::breakpoint_disable_all();

    if (m_browser_p)
      {
      m_browser_p->get_edit_view().refresh_annotations();
      }

    SkConsole::ms_console_p->get_remote_ide()->cmd_breakpoint_update(
      nullptr, SkBreakPoint::Update_disable);

    SkDebug::print(
      a_str_format("Disabled %u breakpoint%s.\n", bp_count, (bp_count > 1) ? "s" : ""),
      SkLocale_local);
    }
  else
    {
    SkDebug::print("No breakpoints to disable.\n", SkLocale_local);
    }
  }

//---------------------------------------------------------------------------------------
// Clears any existing breakpoints and optionally queries the user before
//             doing so.
// Arg         query - query user before removing
// Author(s):   Conan Reis
void SkConsole::breakpoints_remove_all(
  bool query // = true
  )
  {
  uint32_t bp_count = SkDebug::breakpoint_get_count();

  if (bp_count)
    {
    if (!query || ADialogOS::confirm(a_str_format("Remove %u breakpoint%s?", bp_count, (bp_count > 1) ? "s" : ""), "Confirm Clear Breakpoints", ADialogOS::Flag_disable_win))
      {
      SkDebug::breakpoint_remove_all();

      if (m_browser_p)
        {
        m_browser_p->get_edit_view().refresh_annotations();
        }

      SkConsole::ms_console_p->get_remote_ide()->cmd_breakpoint_update(nullptr, SkBreakPoint::Update_remove);

      SkDebug::print(
        a_str_format("Removed %u breakpoint%s.\n", bp_count, (bp_count > 1) ? "s" : ""),
        SkLocale_local);
      }
    }
  else
    {
    SkDebug::print("No breakpoints to remove.\n", SkLocale_local);
    }
  }

//---------------------------------------------------------------------------------------
// Prints current breakpoints to the output/log window
// Author(s):   Conan Reis
void SkConsole::breakpoints_list_all(
  bool focus_log // = true
  )
  {
  const APSortedLogicalFree<SkBreakPoint, SkMemberExpression> & breakpoints = SkDebug::breakpoints_get_all();

  AString  str;
  uint32_t bp_count = breakpoints.get_length();
  
  str.ensure_size_empty(512u);
  str.append_format("\n\nSkookum Breakpoint List - %u set\n", bp_count);
  str.append('-', 45u);
  str.append('\n');

  // $Revisit - CReis Sort breakpoints by class/member/index
  AString         member_str;
  SkBreakPoint *  bp_p;
  SkBreakPoint ** bp_pp     = breakpoints.get_array();
  SkBreakPoint ** bp_end_pp = bp_pp + bp_count;

  while (bp_pp < bp_end_pp)
    {
    bp_p = *bp_pp;

    member_str = bp_p->as_file_title();
    str.ensure_size(str.get_size() + member_str.get_length() + 32u);
    str.append_format("Member: %s[%u] - %s\n", member_str.as_cstr(), bp_p->get_source_idx(), bp_p->is_enabled() ? "enabled" : "disabled");

    bp_pp++;
    }

  if (bp_count)
    {
    str.append("\n[Double click a member to jump to the position in the Browser.]");
    }

  str.append("\n\n", 2u);

  if (focus_log)
    {
    show();
    m_log.make_foreground();
    m_log.set_focus();
    }
 
  SkDebug::print(str, SkLocale_all);
  }


#ifdef SKOOKUM_IDE_EMBEDDED

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkConsole::enable_debugging_embedded(
  bool enable_b // = true
  )
  {
  SkDebug::set_hook_expr(enable_b ? breakpoint_hit_embedded : nullptr);
  SkDebug::set_scripted_break(enable_b ? debug_scripted_break : nullptr);
  }

#endif  // SKOOKUM_IDE_EMBEDDED


//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkConsole::debug_expr(
  SkExpressionBase *    expr_p,
  const SkContextInfo & member_info
  )
  {
  #if (SKOOKUM & SK_DEBUG)
    // $Revisit - CReis Could optionally bring up prompt dialog.

    if (member_info.is_valid())
      {
      browse_member(member_info, expr_p ? expr_p->m_source_idx : 0u);
      }
    else
      {
      display_browser();
      }

    refresh_debug_ui();

    if (m_remote.is_embedded())
      {
      // Pause execution for embedded runtime
      while (SkDebug::get_next_expression().is_valid() && !AApplication::is_shutting_down())
        {
        AMessageTarget::process_messages(AAsyncFilter__no_idle);
        }
      }
  #endif
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkConsole::refresh_debug_ui()
  {
  // Update browser
  if (m_browser_p)
    {
    m_browser_p->get_edit_view().refresh_annotations();
    }
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkConsole::debug_reset()
  {
  // Alerts loop in debug_expr()
  SkDebug::invalidate_next_expression();
  hide_error();

  refresh_debug_ui();
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkConsole::debug_continue()
  {
  SkDebug::eState prev_state = SkDebug::get_execution_state();

  debug_reset();

  if ((prev_state != SkDebug::State_running) && m_remote.is_remote_ide())
    {
    // Let remote runtime know to continue
    m_remote.cmd_break_continue();
    }
  }

//---------------------------------------------------------------------------------------
// Run current statement and break at next statement
// Author(s):   Conan Reis
void SkConsole::debug_step(SkDebug::eStep step_type)
  {
  SkDebug::eState prev_state = SkDebug::get_execution_state();

  debug_reset();

  if ((prev_state != SkDebug::State_running) && m_remote.is_remote_ide())
    {
    // Let remote runtime know to continue
    m_remote.cmd_break_step(step_type);
    }
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
void SkConsole::show_debug_expr()
  {
  SkMemberExpression & expr_info = SkDebug::get_next_expression();

  if (!expr_info.is_valid())
    {
    return;
    }

  if (expr_info.is_origin_source())
    {
    SkExpressionBase * expr_p = expr_info.get_expr();

    browse_member(expr_info, expr_p ? expr_p->m_source_idx : 0u);
    }
  }

//---------------------------------------------------------------------------------------
// Called whenever there has been one or more parsing errors.
// Notes:      The compiler will only call this once even if there have been many parser
//             errors.
// Modifiers:   virtual - overridden from SkConsoleBase
// Author(s):   Conan Reis
void SkConsole::on_error(
  uint error_count // = 1u
  )
  {
  // The sound can take a bit to start so putting it first
  play_sound(Sound_error);
  display_ide();
  }

//---------------------------------------------------------------------------------------
// Called whenever the scripts are reparsed
// 
// Examples:  called by SkCompiler::reparse()
// Author(s): Conan Reis
void SkConsole::on_reparse(
  SkClass * class_p // = nullptr
  )
  {
  if (class_p == nullptr)
    {
    SkDebug::print("\n\n\nReparsing entire class hierarchy...\n\n");
    }

  status_update();

  SkDebug::breakpoint_release_all();
  hide_error();

  if (m_browser_p)
    {
    m_browser_p->save_settings();

    if (m_goto_view_p)
      {
      m_goto_view_p->empty();
      }

    m_browser_p->unhook();
    }
  }

//---------------------------------------------------------------------------------------
// Called when it is ready to load up a new project
void SkConsole::on_load_project_deferred()
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Save settings and deinitialize
  AIni & project_settings = SkCompiler::ms_compiler_p->get_ini_project();
  AIni & default_project_settings = SkCompiler::ms_compiler_p->get_ini_project_default();

  SkDebug::print(a_str_format("SkookumScript cleaning up previous project '%s'...\n", project_settings.as_cstr()), SkLocale_local, SkDPrintType_title);

  save_settings();

  breakpoints_disable_all();

  if (m_browser_p)
    {
    m_browser_p->unhook();
    }

  SkookumScript::deinitialize_gameplay();
  SkookumScript::deinitialize_sim();
  SkookumScript::deinitialize_program();
  SkookumScript::deinitialize();


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Load new project and reinitialize
  SkDebug::print(a_str_format("SkookumScript loading new project '%s'...\n", m_project_info.m_project_path.as_cstr()), SkLocale_local, SkDPrintType_title);

  AFile project_file(m_project_info.m_project_path);
  AFile default_project_file(m_project_info.m_default_project_path);

  project_file.qualify();
  default_project_file.qualify();
  set_last_project(project_file, default_project_file);
  project_settings.set_file(project_file);
  default_project_settings.set_file(default_project_file);

  // Now that we know the ini file exists, override project name from there
  m_project_info.m_project_name = SkCompiler::ms_compiler_p->get_ini_project_name();

  m_project_info.m_load_state = AProgess_processing;
  m_remote.set_flags(m_project_info.m_compiled_flags);

  update_title();
  refresh_status_remote();

  SkCompiler::ms_compiler_p->load_settings();

  AString msg;
  msg.append("SkookumScript Runtime Info:");
  msg.append("\n  Project name: ", 17u);
  msg.append(SkCompiler::ms_compiler_p->get_ini_project_name());
  msg.append("\n  Project path: ", 17u);
  msg.append(project_file.as_cstr());
  msg.append("\n  Default project path: ", 25u);
  msg.append(m_project_info.m_default_project_path);

  if (SkRemoteBase::ms_default_p->is_authenticated())
    {
    msg.append("\n  Engine: ", 11u);
    msg.append(m_project_info.m_engine_id);
    msg.append("\n  Platform: ", 13u);
    msg.append(m_project_info.m_platform_id);
    }

  msg.append("\n\n", 2u);
  SkDebug::print_agog(msg, SkLocale_all, SkDPrintType_system);

  SkCompiler::ms_compiler_p->phases_init(SkCompiler::Init_phased);
  }

//---------------------------------------------------------------------------------------
// Called when the compiler has completed parsing the scripts/code binary.
// Modifiers:   virtual - overridden from SkConsoleBase
// Author(s):   Conan Reis
void SkConsole::on_compile_complete()
  {
  //A_DPRINT(A_SOURCE_FUNC_STR "\n");

  SkDebug::breakpoint_acquire_all();

  status_update();
  if (SkConsole::is_syntax_highlight())
    {
    m_workspace.syntax_highlight(SkEditSyntax::Coverage_all);
    }
  else
    {
    m_workspace.syntax_highlight(SkEditSyntax::Coverage_none);
    }

  switch (m_project_info.m_load_state)
    {
    case AProgess_queued:
      // Request to load a new project was deferred. Load it now.
      on_load_project_deferred();
      return;

    case AProgess_processing:
      m_project_info.m_load_state = AProgess_processed;
    }

  if (m_browser_p)
    {
    // It must have been previously unhooked
    m_browser_p->rehook();
    m_browser_p->load_settings(LoadView_ignore);
    }
  else
    {
    if (m_show_browser
      || m_ini_ide.get_value_bool_default(SkConsole_show_browser_def, g_ini_key_show_browser_p, g_ini_section_sbrowser_p))
      {
      display_browser();
      }
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  
  // Execute any post-compilation IDE command-line arguments
  AString command_args(AApplication::ms_this_app_p->get_command_line_args());

  if (command_args.is_filled())
    {
    cmd_args_execute(command_args);

    // Clear out args
    AApplication::ms_this_app_p->set_command_line_args(AString::ms_empty);
    }
  }

//---------------------------------------------------------------------------------------
// Called when the compiler has completed parsing the scripts/code binary.
// Modifiers:   virtual - overridden from SkConsoleBase
// Author(s):   Conan Reis
void SkConsole::on_overlays_changed()
  {
  if (m_browser_p)
    {
    // It must have been previously unhooked
    m_browser_p->rehook();
    m_browser_p->load_settings();
    }
  }

//---------------------------------------------------------------------------------------
//  Called when the close button on the title bar is pressed.
// Returns:     true and the default closing behaviour is taken
// Author(s):    Conan Reis
bool SkConsole::on_close_attempt()
  {
  if (m_goto_view_p)
    {
    m_goto_view_p->empty();
    }

  switch (m_close_action)
    {
    case CloseAction_close:
      close(false);
      break;

    case CloseAction_hide:
      hide();
      display_browser(false);
      save_settings();
      break;

    default: // CloseAction_shutdown
      close(true);
    }

  return false;
  }

//---------------------------------------------------------------------------------------
// Called when the minimize or maximize/restore button on the title bar is
//             pressed (or any equivalent mechanism).
// Returns:    true if the request should be accepted and the default behaviour should be
//             taken, false if not accepted, or if custom behaviour is performed.
// Notes:      If the default behaviour is appropriate, but it would be handy to perform
//             some additional behaviour prior to it, just override this method, execute
//             the extra behaviour and return true.
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
bool SkConsole::on_show_zoom_attempt(eShowZoom type)
  {
  switch (type)
    {
    case ShowZoom_minimize:
      break;

    case ShowZoom_maximize:
      break;

    case ShowZoom_restore:
      break;
    }

  return true;
  }

//---------------------------------------------------------------------------------------
// Called at the beginning of a drag and drop sequence prior to the first
//             path is sent to on_drag_drop().
// See:        on_drag_drop(), on_drag_drop_end(), enable_drag_drop()
// Notes:      Override this virtual function for custom behaviour.
// Modifiers:   virtual
// Author(s):   Conan Reis
void SkConsole::on_drag_drop_begin()
  {
  //m_log.empty();
  }

//---------------------------------------------------------------------------------------
// Called whenever a file (or files) is dropped on to this window and drag and drop is
// enabled.
// 
// Arg         const AString & file_name - the fully qualified path name of the file /
//             folder / etc.  Note that directory paths do *not* end with a trailing
//             backslash '\'.
// Arg         pos - position where the drop occurs on the window
// Examples:   called by system
// See:        enable_drag_drop
// Notes:      Use ADirectory::is_existing_path() to determine if file_name is a file
//             or a directory.
//
//             If multiple files are dropped on the window simultaneously, this method
//             is called successively with one file name at a time - i.e. if 42 files
//             are dropped onto this window, this method will be called 42 times.
//
//             Directories are not recursed automatically.
//
//             Override this virtual function for custom behaviour.
// Modifiers:   virtual
// Author(s):   Conan Reis
void SkConsole::on_drag_drop(
  const AString & file_name,
  const AVec2i &  pos
  )
  {
  cmd_args_execute_auto(file_name);
  }

//---------------------------------------------------------------------------------------
// Called when input (keyboard) focus is gained.
// 
// Returns:  
//   In non-OS windows returning true indicates that changing the focus to this window is
//   allowed.  In OS windows (AEditOS, AListOS, ATreeOS, etc.) the return value is
//   ignored.
//   
// See:         on_focus_lost()
// Author(s):   Conan Reis
bool SkConsole::on_focus()
  {
//  A_DPRINT(A_SOURCE_FUNC_STR "\n");

  // If the console doesn't have a focus window, set the focus to the workspace.
  if (!SkMainWindowBase::get_focused_console())
    {
    m_workspace.set_focus();
    }

  return true;  // Allow focus
  }


//---------------------------------------------------------------------------------------
// Called when the menubar exits.
// 
// # Author(s): John Stenersen
void SkConsole::on_menu_modal(bool enter_b)
  {
//  A_DPRINT(A_SOURCE_FUNC_STR "\n");

  if (enter_b)
    {
    return;   //  Don't care about the entry.
    }

  // Return to the window that last had focus in the browser.
  if (!SkMainWindowBase::get_focused_console())
    {
    m_workspace.set_focus();
    }
  else
    {
    SkMainWindowBase::get_focused_console()->set_focus();
    }

  return;
  }


//---------------------------------------------------------------------------------------
// Called whenever a key is pressed.
// Arg         key - key code corresponding to a physical key on the keyboard.
//             If Shift-'2' is pressed, AKey_shift is sent first and then AKey_2, not '@'.
//             Defines for codes are prefixed with "AKey_" and are in AgogIO/AKeyboard.hpp
//             AKey_0 thru AKey_9 are the same as ANSI '0' thru '9' (0x30 - 0x39)
//             AKey_A thru AKey_Z are the same as ANSI 'A' thru 'Z' (0x41 - 0x5A)
//             Special characters like AKey_control are also possible.
// Arg         repeated - true if this is a repeated send (holding down the key), false
//             if this is the first time key has been pressed.
// See:        on_character(), on_key_release()
// Notes:      AKeyboard::get_mod_keys() can be called to determine if any modifier keys
//             are in effect.
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
bool SkConsole::on_key_press(
  eAKey key,
  bool  repeated
  )
  {
  //A_DPRINT(A_SOURCE_FUNC_STR "0x%p\n", this);

  eAKeyMod mod_keys = AKeyboard::get_mod_keys();

  // Ignore repeated keys
  if (!repeated)
    {
    switch (key)
      {
      case AKey_num_multiply:
        if (mod_keys == AKeyMod_alt)
          {
          show_debug_expr();
          return false;
          }
        break;

      case AKey_f4:
        if (mod_keys == AKeyMod_alt)  // Close SkookumIDE
          {
          on_close_attempt();
          return false;
          }
        break;

      case AKey_f5:  // Continue after debug break
        if (mod_keys == AKeyMod_none)
          {
          debug_continue();
          return false;
          }
        break;

      case AKey_f7:  // Compile
        switch (mod_keys)
          {
          case AKeyMod_none:
            compile_project_stale();
            break;

          case AKeyMod_alt:
            compile_class_browser(true);
            return false;

          case AKeyMod_ctrl:
            compile_member_browser();
            return false;

          case AKeyMod_alt_ctrl:
            compile_project();
            return false;
          }
        break;

      case AKey_f9:  // Toggle breakpoint
        switch (mod_keys)
          {
          case AKeyMod_alt:
            breakpoints_list_all();
            return false;

          case AKeyMod_alt_ctrl:
            breakpoints_disable_all();
            return false;

          case AKeyMod_ctrl_shift:
            breakpoints_remove_all();
            return false;
          }
        break;

    case AKey_f10:
      switch (mod_keys)
        {
        case AKeyMod_none:
          debug_step(SkDebug::Step_over);
          return false;

        case AKeyMod_shift:
          debug_step(SkDebug::Step_next);
          return false;
        }
      break;

    case AKey_f11:
      switch (mod_keys)
        {
        case AKeyMod_none:
          debug_step(SkDebug::Step_into);
          return false;

        case AKeyMod_shift:
          debug_step(SkDebug::Step_out);
          return false;

        case AKeyMod_ctrl:
          display_browser();
          toggle_disassembly();
          return false;
        }
      break;

      case AKey_tab:
        if (mod_keys == AKeyMod_ctrl_shift)
          {
          display_browser();
          return false;
          }
        break;

      case AKey_left:
        if (mod_keys == AKeyMod_alt)
          {
          display_browser();
          m_browser_p->get_edit_view().history_prev();
          return false;
          }
        break;

      case AKey_browser_back:
        display_browser();
        m_browser_p->get_edit_view().history_prev();
        return false;

      case AKey_right:
        if (mod_keys == AKeyMod_alt)
          {
          display_browser();
          m_browser_p->get_edit_view().history_next();
          return false;
          }
        break;

      case AKey_browser_forward:
        display_browser();
        m_browser_p->get_edit_view().history_next();
        return false;

      case AKey_tilde:
        if (mod_keys == AKeyMod_ctrl)
          {
          toggle_ide();
          return false;
          }
        break;

      case 'C':
        if (mod_keys == AKeyMod_alt)
          {
          display_goto_dialog(SkMatchKind_classes);
          return false;
          }
        break;

      case 'G':
        switch (mod_keys)
          {
          case AKeyMod_ctrl:
            display_goto_dialog(SkMatchKind_all);
            return false;

          case AKeyMod_alt:
            if (m_log.is_focused())
              {
              display_goto_context(m_log);
              }
            else
              {
              display_goto_context(m_workspace);
              }
            return false;

          case AKeyMod_alt_shift:
            display_goto_dialog(SkMatchKind_data);
            return false;

          case AKeyMod_alt_ctrl:
            display_goto_dialog(SkMatchKind_routines);
            return false;

          case AKeyMod_ctrl_shift:
            display_goto_dialog(SkMatchKind_all);
            return false;
          }
        break;
      }
    }

  return true;
  }


//---------------------------------------------------------------------------------------
//  Called when a submenu is about to become active/displayed.
//  Arg           Menu handle/ID
//  Modifiers:    virtual - override for custom behaviour
//  Author(s):    John Stenersen
bool SkConsole::on_submenu_init(HMENU submenu)
  {
  return refresh_menubar(submenu, get_focused_console(), get_focused_console_type());
  }


//---------------------------------------------------------------------------------------
// Called when the user makes a selection from associated menu bar sub-menu
//             or associated pop-up / context menu.
// Arg         item_id - id of item selected
// Modifiers:   virtual - overridden from AWindow
// Author(s):   Conan Reis
void SkConsole::on_menu_command(uint32_t item_id)
  {
  if (on_menubar(item_id))
    {
    return;
    }

  ADebug::print_format(A_SOURCE_STR " [Menu incomplete - item: %u]\n", item_id);
  }


//---------------------------------------------------------------------------------------
// Save online mode.  Only save manually for testing and never auto save.
// Author(s):   Conan Reis
void SkConsole::set_online_settings(eSkLocale locale)
  {
  if (!SkDebug::is_engine_present())
    {
    const char * locale_cstr_p = nullptr;

    switch (locale)
      {
      case SkLocale_embedded:
        locale_cstr_p = "solo";
        break;

      case SkLocale_runtime:
        locale_cstr_p = "runtime";
        break;

      case SkLocale_ide:
        locale_cstr_p = "ide";
        break;
      }

    m_ini_ide.set_value(
      locale_cstr_p, g_ini_key_online_mode_p, g_ini_section_console_p);
    }

  m_remote.set_mode(SkLocale_ide);
  }

//---------------------------------------------------------------------------------------
// Toggle remote runtime server that uses specific IP address to listen
// 
// Author(s):   Conan Reis
void SkConsole::toggle_remote_runtime()
  {
  bool remote_runtime = !m_remote.is_server_remote_enabled();

  // Update menu
  AMenuOS menu = AMenuOS::get_menu_bar(*this);
      
  menu.check_item(SkMenu_settings_remote_runtime, remote_runtime);

  if (remote_runtime)
    {
    SkDebug::print(
      "\nThe SkookumIDE is now listening for remote runtimes such as console and mobile apps\n"
      "with a specific IP address. Local desktop runtime connections are still accepted.\n\n"
      "The IP address selected by the SkookumIDE can be from any network adapter including\n"
      "virtual software adapters. If you want to have it use a specific IP address select\n"
      "'Settings'->'User Preferences...' and edit the [Remote Hosts] section to use the\n"
      "desired IP address. A dash '-' before an address indicates that a remote runtime\n"
      "connection is not currently being used.\n\n"
      "For additional information including how to specify the SkookumIDE address for the\n"
      "runtime, please see http://skookumscript.com/docs/v3.0/ide/ip-addresses/\n\n",
      SkLocale_ide,
      SkDPrintType_warning);
    }
  else
    {
    SkDebug::print(
      "\nThe SkookumIDE is no longer listening for remote runtimes such as console and mobile\n"
      "apps with a specific IP address. Local desktop runtime connections are still accepted.\n\n",
      SkLocale_ide,
      SkDPrintType_warning);
    }

  // Actually toggle server
  m_remote.server_remote_enable(remote_runtime);
  }

//---------------------------------------------------------------------------------------
// Sets version control system type.
// 
// Author(s):   Conan Reis
void SkConsole::set_version_control(eSkVersionControl system)
  {
  if (m_version_control != system)
    {
    const char * vcs_name_p = nullptr;

    switch (system)
      {
      case SkVersionControl_p4:
        ADialogOS::register_writable_p4_dialog();
        vcs_name_p = "Perforce";
        m_ini_ide.set_value(
          "p4",
          g_ini_key_version_control_p,
          g_ini_section_sbrowser_p);
        break;

      default: // SkVersionControl_none
        ADialogOS::register_writable_dialog();
        vcs_name_p = "no";
        m_ini_ide.set_value(
          AString::ms_empty,
          g_ini_key_version_control_p,
          g_ini_section_sbrowser_p);
      }

    m_version_control = system;

    // Update menu
    AMenuOS menu = AMenuOS::get_menu_bar(*this);
      
    menu.check_item(SkMenu_settings_perforce, m_version_control == SkVersionControl_p4);

    if (m_browser_p)
      {
      AMenuOS menu_browser = AMenuOS::get_menu_bar(*m_browser_p);
      
      menu_browser.check_item(SkMenu_settings_perforce, m_version_control == SkVersionControl_p4);
      }

    SkDebug::print(a_cstr_format("\nNow using %s version control system.\n", vcs_name_p));
    }
  }

//---------------------------------------------------------------------------------------
// Toggle version control system from none to Perforce or from Perforce to none
// 
// Author(s):   Conan Reis
void SkConsole::toggle_version_control()
  {
  switch (get_version_control_system())
    {
    case SkVersionControl_none:
      set_version_control(SkVersionControl_p4);
      break;

    default: // SkVersionControl_p4
      set_version_control(SkVersionControl_none);
    }
  }

//---------------------------------------------------------------------------------------
// Set whether dialog should be shown when there are compile errors or just
//             output errors to the log window.
// Arg         error_dialog - true if error dialog should be shown and false if not
// Author(s):   Conan Reis
void SkConsole::enable_error_dialog(
  bool error_dialog // = true
  )
  {
  if (is_show_error_dialog() != error_dialog)
    {
    if (error_dialog)
      {
      m_pref_flags |= Preference_error_dialog;
      }
    else
      {
      m_pref_flags &= ~Preference_error_dialog;
      }

    SkDebug::print(error_dialog ? "\nShow dialog on recompile errors: ON\n" : "\nShow dialog on recompile errors: OFF\n");

    // Put updated settings in .ini file
    m_ini_ide.set_value_bool(error_dialog, g_ini_key_error_dialog_p, g_ini_section_console_p);

    // Update menu
    AMenuOS menu = AMenuOS::get_menu_bar(*this);
      
    menu.check_item(SkMenu_compile_error_dialog, error_dialog);

    if (m_browser_p)
      {
      AMenuOS menu_browser = AMenuOS::get_menu_bar(*m_browser_p);
      
      menu_browser.check_item(SkMenu_compile_error_dialog, error_dialog);
      }
    }
  }

//---------------------------------------------------------------------------------------
// Set whether compiled changes should be sent to remote runtime (true) or
//             just compiled for correctness (false)
// Arg         update_remote - true if updates should be sent and false if not
// Author(s):   Conan Reis
void SkConsole::enable_remote_update(
  bool update_remote // = true
  )
  {
  if (is_remote_update_enabled() != update_remote)
    {
    if (update_remote)
      {
      m_pref_flags |= Preference_update_remote;
      }
    else
      {
      m_pref_flags &= ~Preference_update_remote;
      }

    SkDebug::print(update_remote ? "\nUpdate remote runtime: ON\n" : "\nUpdate remote runtime: OFF\n");

    // Put updated settings in .ini file
    m_ini_ide.set_value_bool(update_remote, g_ini_key_remote_update_p, g_ini_section_console_p);

    // Update menu
    AMenuOS menu = AMenuOS::get_menu_bar(*this);
      
    menu.check_item(SkMenu_compile_update_remote, update_remote);

    if (m_browser_p)
      {
      AMenuOS menu_browser = AMenuOS::get_menu_bar(*m_browser_p);
      
      menu_browser.check_item(SkMenu_compile_update_remote, update_remote);
      }
    }
  }

//---------------------------------------------------------------------------------------
// Update the online menu based on the locale state
// Author(s):   Conan Reis
void SkConsole::update_online_menu(eSkLocale locale)
  {
  //  $Revisit: JStenersn   This routine is ready to be removed...
  }


//---------------------------------------------------------------------------------------
// Enables/disables viewing disassembly version of script in editor
// Author(s):   Conan Reis
void SkConsole::enable_disassembly(
  bool show // = true
  )
  {
  if (m_disassembly != show)
    {
    m_disassembly = show;

    if (m_browser_p)
      {
      m_browser_p->get_edit_view().refresh_member();
      }
    }
  }

//---------------------------------------------------------------------------------------
// Enables/disables viewing expression span guide in editor
// Author(s):   Conan Reis
void SkConsole::enable_expression_guide(
  bool show // = true
  )
  {
  if (m_expression_guide != show)
    {
    m_expression_guide = show;

    if (m_browser_p)
      {
      m_browser_p->get_edit_view().get_editor().invalidate();
      }
    }
  }


//---------------------------------------------------------------------------------------
//  Enables/disables auto-parse underlining
//  Author(s):  John Stenersen
void SkConsole::enable_auto_parse(
  bool show // = true
  )
  {
  if (m_auto_parse != show)
    {
    m_auto_parse = show;

    if (m_browser_p)
      {
      m_browser_p->get_edit_view().get_editor().invalidate();
      }
    }
  }


//---------------------------------------------------------------------------------------
//  Enables/disables auto-parse underlining in a selection
//  Author(s):  John Stenersen
void SkConsole::enable_auto_parse_sel(
  bool show // = true
  )
  {
  if (m_auto_parse_sel != show)
    {
    m_auto_parse_sel = show;

    ms_console_p->get_workspace().invalidate();
    }
  }


//---------------------------------------------------------------------------------------
//  Enables/disables syntax highlighting
//  Author(s):  John Stenersen
void SkConsole::enable_syntax_highlight(
  bool show // = true
  )
  {
  if (m_syntax_highlight != show)
    {
    m_syntax_highlight = show;

    if (m_syntax_highlight)
      {
      m_workspace.syntax_highlight(SkEditSyntax::Coverage_all);
      if (m_browser_p)
        {
        m_browser_p->get_edit_view().get_editor().syntax_highlight(SkEditSyntax::Coverage_all);
        }
      }
      else
      {
      m_workspace.syntax_highlight(SkEditSyntax::Coverage_none);
      if (m_browser_p)
        {
        m_browser_p->get_edit_view().get_editor().syntax_highlight(SkEditSyntax::Coverage_none);
        }
      }
    }
  }


//---------------------------------------------------------------------------------------
//  Enables/disables current line highlighting.
//  Author(s):  John Stenersen
void SkConsole::enable_current_line_highlight(
  bool show // = true
  )
  {
  if (m_current_line_highlight != show)
    {
    m_current_line_highlight = show;

    m_log.invalidate();
    m_workspace.invalidate();
    if (m_browser_p)
      {
      m_browser_p->get_edit_view().get_editor().invalidate();
      }
    }
  }


//---------------------------------------------------------------------------------------
//  Called whenever "Compile Entire Project" is selected.
// Author(s):    Conan Reis
void SkConsole::compile_project()
  {
  if (SkDebug::is_engine_present())
    {
    if (!ADialogOS::confirm(
      "Are you sure you want to clear out all scripts while the engine is running?\n\n"
      "[This is almost guaranteed to crash the engine.  Try to recompile a single method or coroutine instead.]",
      "Skookum - Clear Scripts and Compile from Scratch",
      ADialogOS::Flag_disable_win,
      ADialogOS::Icon_warning))
      {
      return;
      }
    }

  // Always send a `Command_freshen_compiled_reply` after recompiling the entire project.
  // That allows the RT to reload if possible.
  m_remote.set_flags(SkRemoteIDE::CompiledFlag_notify | SkRemoteIDE::CompiledFlag_freshen);

  SkCompiler::ms_compiler_p->reparse();
  }

//---------------------------------------------------------------------------------------
//  Called whenever the "Reparse" button is pressed.
// Author(s):    Conan Reis
void SkConsole::compile_project_stale()
  {
  SkDebug::print("\nChecking SkookumScript " A_BITS_STR " code dependencies for compiled binary...\n");

  if (SkCompiler::ms_compiler_p->is_compiled_fresh())
    {
    SkDebug::print("  ...Skookum compiled binary is up-to-date.\n\n");
    return;
    }

  SkDebug::print("  Compiled binary is stale - script code is newer.\n  Switching to source file parsing...\n\n");

  compile_project();
  }

//---------------------------------------------------------------------------------------
// Called to recompile the specified class (including optional subclasses).
// Author(s):   Conan Reis
void SkConsole::compile_class(
  SkClass * class_p,
  bool      subclasses
  )
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Reparse class(es)
  uint32_t error_count = SkCompiler::ms_compiler_p->reparse_class(
    class_p,
    subclasses ? SkCompiler::Reparse__default : SkCompiler::Reparse_class_ctor);

  bool     update_runtime = false;
  uint32_t class_count    = subclasses ? class_p->get_class_recurse_count(false) : 1u;
  AString  subclass_str;

  if (class_count > 1u)
    {
    subclass_str.append(a_str_format(
      "and its %u subclass%s",
      class_count - 1u,
      (class_count != 2u) ? "es" : ""));
    }

  if (error_count)
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Errors
    AString err_msg(a_str_format(
      "%u compile error%s while attempting to update class '%s'%s%s!",
      error_count,
      (error_count == 1) ? "" : "s",
      class_p->get_name_cstr_dbg(),
      (class_count > 0) ? "\n" : "",
      subclass_str.as_cstr()));

    if (is_remote_update_enabled())
      {
      if (SkRemoteBase::ms_default_p->is_authenticated())
        {
        update_runtime = true;
        err_msg.append("\n\n[Suspending execution of runtime and no changes have been sent...]");
        }
      else
        {
        err_msg.append("\n\n[Cannot suspend execution - not connected to runtime.]");
        }
      }
    else
      {
      err_msg.append("\n\n[Live update of runtime is disabled.]");
      }

    err_msg.append(
      "\n\nNo individual scripts which correctly compiled previously will be replaced with\n"
      "scripts that did not compile - the earlier good ones are still in memory.\n");

    // Show error dialog
    display_error(err_msg);
    }
  else
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // No errors
    eSkDPrintType print_level = SkDPrintType_system;

    AString msg(a_str_format(
      "Updating class '%s'%s",
      class_p->get_name_cstr_dbg(),
      subclass_str.as_cstr()));

    if (is_remote_update_enabled())
      {
      if (SkRemoteBase::ms_default_p->is_authenticated())
        {
        update_runtime = true;
        msg.append("\n\n[Performing live update of runtime...]\n\n");
        }
      else
        {
        print_level = SkDPrintType_warning;
        msg.append(" on IDE\n\n[Cannot do live update - not connected to runtime.]\n\n");
        }
      }
    else
      {
      msg.append(" on IDE\n\n[Live update of runtime is disabled.]\n\n");
      }

    SkDebug::print(msg, SkLocale_all, print_level);
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Update remote runtime?
  if (!update_runtime)
    {
    return;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Notify runtime of any errors
  if (error_count)
    {
    // $Revisit - CReis Could not bother to notify runtime unless it is waiting
    m_remote.cmd_recompile_classes_reply(class_p, subclasses, error_count);

    return;
    }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Update remote runtime
  m_remote.cmd_class_update(class_p, subclasses);
  }

//---------------------------------------------------------------------------------------
// Called whenever the current class (optionally including its subclasses)
//             is reparsed.
// Author(s):   Conan Reis
void SkConsole::compile_class_browser(
  bool subclasses
  )
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure class is selected
  SkClass * class_p = nullptr;

  if (m_browser_p)
    {
    class_p = m_browser_p->get_class_tree().get_selected_class();
    }

  if (class_p == nullptr)
    {
    SkDebug::print(
      "\nThere is no class in the Browser to recompile!\n",
      SkLocale_all,
      SkDPrintType_warning);

    return;
    }

  compile_class(class_p, subclasses);
  }

//---------------------------------------------------------------------------------------
//  Reparses the specified member.
// Author(s):    Conan Reis
void SkConsole::compile_member(const SkMemberInfo & member)
  {
  SkDebug::breakpoint_release_all();

  if (m_browser_p)
    {
    // Ensure editor changes are saved
    m_browser_p->get_edit_view().save_changes();
    }

  AFile             member_file;
  SkParser::eResult result  = SkParser::Result_err_unimplemented;
  SkClass *         scope_p = member.m_member_id.get_scope();

  if (SkCompiler::ms_compiler_p->find_member_file(member, &member_file))
    {
    switch (member.m_type)
      {
      case SkMember_method:
      case SkMember_method_func:
      case SkMember_method_mthd:
        // $Revisit - CReis Should only bother to update member if reparse was successful
        result = SkCompiler::ms_compiler_p->reparse_file_method(
          const_cast<AFile *>(&member_file),
          member.m_member_id.get_name(),
          member.m_class_scope ? &scope_p->get_metaclass() : static_cast<SkClassUnaryBase *>(scope_p));
          member_file.close();
        break;
        
      case SkMember_coroutine:
      case SkMember_coroutine_func:
      case SkMember_coroutine_mthd:
        // $Revisit - CReis Should only bother to update member if reparse was successful
        result = SkCompiler::ms_compiler_p->reparse_file_coroutine(
          const_cast<AFile *>(&member_file),
          member.m_member_id.get_name(),
          scope_p);
          member_file.close();
        break;
      }
    }

  SkDebug::breakpoint_acquire_all();

  if (result == SkParser::Result_ok)
    {
    if (m_browser_p)
      {
      m_browser_p->get_member_view().get_code_list().update_visible();
      }
    }
  else
    {
    SkDebug::print(
      a_str_format(
        "\nCannot recompile:\n  %s\n  Only valid method & coroutine scripts can be recompiled.\n",
        member_file.as_cstr()),
      SkLocale_ide,
      SkDPrintType_error);
    }
  }

//---------------------------------------------------------------------------------------
//  Reparses the member currently selected in the class browser.
// Author(s):    Conan Reis
void SkConsole::compile_member_browser()
  {
  if (m_browser_p)
    {
    compile_member(m_browser_p->get_member_info());
    }
  else
    {
    SkDebug::print("\nThere is no member selected in the Browser to recompile!\n", SkLocale_ide, SkDPrintType_warning);
    }
  }

//---------------------------------------------------------------------------------------
// Set whether compiled changes should be sent to remote runtime (true) or just compiled
// for correctness (false)
// 
// Params:  
//   strict: true if updates should be sent and false if not
//   
// Author(s):   Conan Reis
void SkConsole::enable_strict_compile(
  bool strict // = true
  )
  {
  if (SkParser::is_strict() != strict)
    {
    SkParser::enable_strict(strict);

    SkDebug::print(strict ? "\nStrict compile: ON\n" : "\nStrict compile: OFF\n");

    // Put updated settings in .ini file
    SkCompiler::ms_compiler_p->set_ini_strict(strict);

    // Update menu
    AMenuOS menu = AMenuOS::get_menu_bar(*this);
      
    menu.check_item(SkMenu_compile_strict, strict);

    if (m_browser_p)
      {
      AMenuOS menu_browser = AMenuOS::get_menu_bar(*m_browser_p);
      
      menu_browser.check_item(SkMenu_compile_strict, strict);
      }
    }
  }

//---------------------------------------------------------------------------------------
// Sets whether `AKeyboard::get_mod_keys()` polls just left Alt key or both left and
// right Alt keys.
// 
// Many international keyboards use right Alt+key to get characters that are not standard
// for their locale - including many ASCII characters. So enabling this allows these
// international keyboards to better interoperate with keyboard shortcuts.
// 
// Params:  
//   uses_alt_chars: If `false` `AKeyboard::get_mod_keys()` will poll the key state of the
//   right Alt key for use with keyboard shortcuts. If `true` `AKeyboard::get_mod_keys()`
//   will ignore the state of the right Alt key.
// 
// Notes:  
//   `AKeyboard::is_pressed()` can always be called to get the separate states for left
//   and right Alt keys if needed.
// 
// See:         AKeyboard::get_mod_keys(), AKeyboard::is_locale_alt()
// Author(s):   Conan Reis
void SkConsole::enable_locale_alt(
  bool uses_alt_chars // = true
  )
  {
  if (AKeyboard::is_locale_alt() != uses_alt_chars)
    {
    AKeyboard::enable_locale_alt(uses_alt_chars);

    SkDebug::print(
      uses_alt_chars
        ? "\nKeyboard locale uses right Alt key for characters and shortcuts ignore right Alt key\n"
        : "\nKeyboard locale does not use right Alt key for characters and shortcuts use right Alt key\n",
      SkLocale_local);

    // Put updated settings in .ini file
    m_ini_ide.set_value_bool(
      uses_alt_chars,
      g_ini_key_locale_right_alt,
      g_ini_section_console_p);

    // Update menu
    AMenuOS menu = AMenuOS::get_menu_bar(*this);
      
    menu.check_item(SkMenu_settings_right_alt, uses_alt_chars);

    if (m_browser_p)
      {
      AMenuOS menu_browser = AMenuOS::get_menu_bar(*m_browser_p);
      
      menu_browser.check_item(SkMenu_settings_right_alt, uses_alt_chars);
      }
    }
  }

//---------------------------------------------------------------------------------------
void SkConsole::set_last_project(const AFile & project_file, const AFile & default_project_file)
  {
  AIni & ini_user = SkCompiler::ms_compiler_p->get_ini_ide();

  if (ini_user.ensure_writable_query())
    {
    ini_user.set_value_file_rel(
      project_file, g_ini_key_last_project_p, g_ini_section_project_p);
    ini_user.set_value_file_rel(
      default_project_file, g_ini_key_last_default_project_p, g_ini_section_project_p);
    }
  }

//---------------------------------------------------------------------------------------
bool SkConsole::connect_new_runtime(const SkProjectInfo & project_info)
  {
  return load_project(project_info);
  }

//---------------------------------------------------------------------------------------
bool SkConsole::load_project(const SkProjectInfo & project_info)
  {
  // Remember settings 
  m_project_info.set(project_info);

  // Make sure important paths are not empty
  if (m_project_info.m_default_project_path.is_empty())
    {
    m_project_info.m_default_project_path = SkCompiler::ms_compiler_p->get_ini_file_proj_default();
    }
  if (m_project_info.m_project_path.is_empty())
    {
    m_project_info.m_project_path = m_project_info.m_default_project_path;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Determine if project different than current
  if (m_project_info.m_project_path == SkCompiler::ms_compiler_p->get_ini_project().get_file()
   && m_project_info.m_default_project_path == SkCompiler::ms_compiler_p->get_ini_project_default().get_file())
    {
    // No, bail
    update_title();
    refresh_status_remote();
    m_project_info.m_load_state = AProgess_processed;
    return false;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Load new project
  m_project_info.m_load_state = AProgess_queued;

  if (!SkCompiler::ms_compiler_p->is_idle())
    {
    return false;
    }

  on_load_project_deferred();

  return true;
  }

//---------------------------------------------------------------------------------------
bool SkConsole::load_project_query()
  {
  if (m_remote.is_connected() && !ADialogOS::confirm(
    "Are you sure you want change all scripts while the runtime is connected?\n"
    "The scripts will be out of sync with the runtime!\n\n"
    "[Though you could change back before running any scripts.]",
    "SkookumScript - Load project while connected to runtime?",
    ADialogOS::Flag_disable_win,
    ADialogOS::Icon_warning))
    {
    return false;
    }

  AFile proj_file;

  SkDebug::print("\nNote: SkookumScript project files are currently simple .ini configuration files custom info.\n", SkLocale_ide, SkDPrintType_title);

  if (!ADialogOS::open_file(
    &proj_file,
    "Select project .ini file to load (usually located inside the project 'Scripts' folder)",
    g_project_ext_filter_p))
    {
    return false;
    }

  AIni proj_ini(proj_file);

  if (!proj_ini.is_value(g_ini_key_startup_mind_p, g_ini_section_project_p))
    {
    SkDebug::print("\nError: Does not seem to be a properly formatted SkookumScript project file!\nIgnored.\n", SkLocale_ide, SkDPrintType_error);
    return false;
    }

  SkProjectInfo project_info;
  project_info.m_project_path = proj_file;
  project_info.m_default_project_path = SkCompiler::ms_compiler_p->get_ini_file_proj_default();

  return load_project(project_info);
  }

//---------------------------------------------------------------------------------------
bool SkConsole::load_project_default_query()
  {
  if (m_remote.is_connected() && !ADialogOS::confirm(
    "Are you sure you want change all scripts while the runtime is connected?\n"
    "The scripts will be out of sync with the runtime!\n\n"
    "[Though you could change back before running any scripts.]",
    "SkookumScript - Load project while connected to runtime?",
    ADialogOS::Flag_disable_win,
    ADialogOS::Icon_warning))
    {
    return false;
    }

  return load_project(SkProjectInfo());
  }

//---------------------------------------------------------------------------------------
// Determine if editing this project is permitted
// Will make project editable under the hood if user desires
bool SkConsole::verify_project_editable()
  {
  if (!SkCompiler::ms_compiler_p->get_ini_project_editable())
    {
    if (SkCompiler::ms_compiler_p->get_ini_project_can_make_editable())
      {
      if (m_remote.is_authenticated())
        {
        if (ADialogOS::info_abort(
          "You are about to add SkookumScript code to this project for the first time - exciting! "
          "For this to work, SkookumScript needs to store its script files and compiled binaries inside your project folder.",
          "Allow SkookumScript to store code in your game project folder?",
          ADialogOS::Flag_disable_win,
          ADialogOS::Icon_warning))
          {
          // Send off command to runtime
          m_remote.cmd_make_editable();
          }
        }
      else
        {
        ADialogOS::info(
          "In order to add code to a project for the very first time, the IDE needs to be connected to the game engine. "
          "Please start up the engine, and when connected to the IDE, try again. Sorry for the inconvenience!",
          "Need to be connected to the game engine!",
          ADialogOS::Flag_none,
          ADialogOS::Icon_warning);
        }
      }
    else
      {
      ADialogOS::info(
        "This project only contains scripts from the Core & Engine overlays which are not editable.\n"
        "Add your own project overlays to create custom scripts.",
        "Project not editable!",
        ADialogOS::Flag_none,
        ADialogOS::Icon_warning);
      }

    return false;
    }

  return true;
  }

//---------------------------------------------------------------------------------------
// Called whenever the toggle state of the 'Evaluate Scripts' checkbox is changed.
// Author(s):   Conan Reis
void SkConsole::on_toggle_evaluate(eAFlag new_state)
  {
  SkCompiler::ms_compiler_p->enable_evaluation(new_state == AFlag_on);
  }

//---------------------------------------------------------------------------------------
// Called whenever the toggle state of the sound checkbox is changed.
// Author(s):   Conan Reis
void SkConsole::on_toggle_sound(eAFlag new_state)
  {
  m_play_sounds = (new_state == AFlag_on);

  SkDebug::print(m_play_sounds
    ? "\nSounds: ON\n"
    : "\nSounds: OFF\n");

  // Put updated sound settings in .ini file
  m_ini_ide.set_value_bool(m_play_sounds, g_ini_key_play_sounds_p, g_ini_section_console_p);
  }

//---------------------------------------------------------------------------------------
//  Called whenever a window's client size is changing.  Usually this is
//              associated with a user dragging a window's sizing border.
// Examples:    called by the system
// See:         on_size(), on_sized()
// Notes:       This applies to the window's client area size changing and not
//              necessarily the outer edge of the window.
// Author(s):    Conan Reis
void SkConsole::on_sizing()
  {
  int    button_height = m_browse.get_height();
  int    status_height = m_status.get_font().get_height() + SkConsole_status_offset + SkConsole_status_inset2;
  AVec2i carea(get_area_client());

  m_split_text.set_area(carea.m_x, carea.m_y - button_height - status_height - SkConsole_status_inset2);

  m_status.set_region(
    SkConsole_status_inset,
    carea.m_y - status_height + SkConsole_status_inset,
    carea.m_x - SkConsole_status_inset2,
    status_height - SkConsole_status_inset2);

  refresh();
  }

//---------------------------------------------------------------------------------------
// Called when the window client area is to be drawn.
// Returns:    Boolean indicating whether the default Windows procedure with respect to
//             the given message should be called (true) or not (false)
// Modifiers:   virtual - Overridden from AgogGUI\AWindow.
// Author(s):   Conan Reis
bool SkConsole::on_draw()
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Online/Remote Status
  uint online_length = 0u;
  bool authenticated = m_remote.is_authenticated();

  const char * online_cstr_p = nullptr;

  COLORREF color_text    = 0u;
  HBRUSH   color_bkgr    = nullptr;
  bool     delete_bkgr_b = false;

  switch (m_remote.get_mode())
    {
    case SkLocale_embedded:
      online_cstr_p = "Stand-alone / off-line";
      online_length = 22;
      color_text    = ::GetSysColor(COLOR_INFOTEXT);
      color_bkgr    = ::GetSysColorBrush(COLOR_INFOBK);
      break;

    case SkLocale_runtime:
      color_text = ::GetSysColor(COLOR_INFOTEXT);
      color_bkgr = ::GetSysColorBrush(COLOR_INFOBK);

      if (!authenticated)
        {
        online_cstr_p = "SkookumScript: connecting to IDE...";
        online_length = 35;
        }
      else
        {
        online_cstr_p = "SkookumScript: IDE connected";
        online_length = 28;
        }
      break;

    case SkLocale_ide:
      if (!authenticated)
        {
        online_cstr_p = "SkookumScript: Listening for Runtime...";
        online_length = 39;
        color_text    = ::GetSysColor(COLOR_INFOTEXT);
        color_bkgr    = ::GetSysColorBrush(COLOR_INFOBK);
        }
      else
        {
        online_cstr_p = m_remote_status.as_cstr();
        online_length = m_remote_status.get_length();
        color_text    = AColor::ms_white;
        color_bkgr    = ::CreateSolidBrush(AColor::ms_forest_green);
        delete_bkgr_b = true;
        }
      break;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  AVec2i carea(get_area_client());
  RECT   online_rect =
    {
    carea.m_x - SkConsole_status_inset - m_online_txt_width,
    m_browse.get_x_rel(),
    carea.m_x - SkConsole_status_inset,
    m_browse.get_bottom_rel()
    };

  int text_x = online_rect.left + SkConsole_status_inset2;


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  PAINTSTRUCT ps;
  HDC         hdc = ::BeginPaint(m_os_handle, &ps);

  ::SelectObject(hdc, ((ATrueTypeFont *)m_font.m_sys_font_p)->m_font_handle_p);
  ::SetTextColor(hdc, color_text);
  ::SetBkMode(hdc, TRANSPARENT);  // TRANSPARENT OPAQUE
  ::SelectObject(hdc, color_bkgr);
  ::PatBlt(hdc, online_rect.left, online_rect.top, online_rect.right - online_rect.left, online_rect.bottom - online_rect.top, PATCOPY);
  ::DrawEdge(hdc, &online_rect, EDGE_ETCHED, BF_RECT);
  ::ExtTextOut(hdc, text_x, online_rect.top + SkConsole_status_inset, 0u, nullptr, online_cstr_p, online_length, nullptr);

  if (delete_bkgr_b)
    {
    ::DeleteObject(color_bkgr);
    }

  ::EndPaint(m_os_handle, &ps);

  return true;
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Settings Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//---------------------------------------------------------------------------------------
AFile SkConsole::get_ini_file_proj_startup()
  {
  bool load_last = false;

  AIni & ini_user = SkCompiler::ms_compiler_p->get_ini_ide();
  
  if (ini_user.ensure_writable_query())
    {
    load_last = ini_user.get_value_bool_default(
      true, g_ini_key_load_last_project_p, g_ini_section_project_p);
    }

  if (!load_last)
    {
    // If 'load last project' not specified then just use default/specified startup project.
    return SkCompiler::ms_compiler_p->get_ini_file_proj_default();
    }

  AFile last_proj_file(ini_user.get_value_file(g_ini_key_last_project_p, g_ini_section_project_p));
  AFile last_default_proj_file(ini_user.get_value_file(g_ini_key_last_default_project_p, g_ini_section_project_p));

  if (!last_proj_file.is_titled() || !last_proj_file.is_existing() 
   || !last_default_proj_file.is_titled() || !last_default_proj_file.is_existing())
    {
    // If 'last project' invalid or not existing then just use default/specified startup project.
    last_proj_file = SkCompiler::ms_compiler_p->get_ini_file_proj_default();
    set_last_project(last_proj_file, last_proj_file);
    }

  return last_proj_file;
  }

//---------------------------------------------------------------------------------------
// Gets the file to save and load the SkookumScript compiled binary from the
//             ini file.
// Returns:    File name to use for the compiled binary
// Arg         loading - true if loading the file, false if saving the file
// Modifiers:   virtual - Overridden from SkConsoleBase
// Author(s):   Conan Reis
AFile SkConsole::get_ini_compiled_file_query(
  bool loading_b // = true
  )
  {
  bool  save_to_ini = false;
  AFile file        = SkCompiler::ms_compiler_p->get_ini_compiled_file();

  if (!file.is_named())
    {
    file.set_file_str(g_ini_compiled_file_def);
    save_to_ini = true;
    }

  if (save_to_ini || !file.is_titled() || (loading_b && !file.is_existing()))
    {
    save_to_ini = loading_b
      ? ADialogOS::open_file(&file, "Select compiled runtime structures binary to load [Cancel parses code instead]", g_compiled_ext_filter_p)
      : ADialogOS::save_file(&file, "Save compiled runtime structures binary file as", g_compiled_ext_filter_p, false);

    if (!save_to_ini)
      {
      return AFile();
      }
    }

  if (save_to_ini)
    {
    SkCompiler::ms_compiler_p->set_ini_compiled_file(file);
    }

  return file;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
AFont SkConsole::get_ini_font()
  {
  return AFont(
    m_ini_ide.get_value_default(
      g_ini_edit_font_def_p,
      g_ini_key_edit_font_p,
      g_ini_section_console_p),
    f32(m_ini_ide.get_value_int_default(
      g_ini_edit_font_size_def,
      g_ini_key_edit_font_size_p,
      g_ini_section_console_p)));
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
AFont SkConsole::get_ini_font_code_narrow()
  {
  return AFont(
    m_ini_ide.get_value_default(
      g_ini_code_list_font_def_p,
      g_ini_key_code_list_font_p,
      g_ini_section_console_p),
    f32(m_ini_ide.get_value_int_default(
      g_ini_code_list_font_size_def,
      g_ini_key_code_list_size_p,
      g_ini_section_console_p)));
  }

//---------------------------------------------------------------------------------------
// Get initial online mode from ini file
// See:        
// Author(s):   Conan Reis
eSkLocale SkConsole::get_ini_online_mode()
  {
  bool online_menu = m_ini_ide.get_value_bool_default(
    false, g_ini_key_online_menu_p, g_ini_section_console_p);

  if (online_menu)
    {
    m_pref_flags |= Preference_online_menu;

    AString style_str = m_ini_ide.get_value_default(
      SkDebug::is_engine_present() ? "runtime" : "ide", g_ini_key_online_mode_p, g_ini_section_console_p);

    if (style_str.get_length() >= 2u)
      {
      style_str.lowercase();

      switch (style_str(0u))
        {
        case 'i':  // assume "ide"
          return SkLocale_ide;

        case 'r':  // assume "runtime"
           return SkLocale_runtime;  
        }
      }

    // assume "solo"
    return SkLocale_embedded;
    }

  m_pref_flags &= ~Preference_online_menu;

  // Mode saved in ini is ignored if the menu is not shown
  return SkDebug::is_engine_present() ? SkLocale_runtime : SkLocale_ide;
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Internal Class Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifdef SKOOKUM_IDE_EMBEDDED

//---------------------------------------------------------------------------------------
// Expression breakpoint test debug function
// Arg         expr_p - expression about to be invoked.
// Arg         scope_p - scope for data/method/etc. look-ups.  It should always be an
//             object derived from SkInvokedContextBase.
// Arg         caller_p - object that called/invoked this expression and that may await
//             a result.  If it is nullptr, then there is no object that needs to be
//             returned to and notified when this invocation is complete.
// See:        SkDebug::set_hook_expr(), SkDebug::breakpoint_get_by_expr()
// Author(s):   Conan Reis
void SkConsole::breakpoint_hit_embedded(
  SkBreakPoint *  bp_p,
  SkObjectBase *  scope_p,
  SkInvokedBase * caller_p
  )
  {
  ADebug::print(SkDebug::context_string("\nHit Skookum script breakpoint!\n", scope_p, caller_p));

  SkInvokedContextBase * context_p   = scope_p->get_scope_context();
  SkInvokableBase *      member_p    = context_p->get_invokable();
  SkInstance *           top_scope_p = context_p->get_topmost_scope();
  SkContextInfo          member_info(*member_p, member_p->get_member_type(), top_scope_p->is_metaclass());
      
  ms_console_p->debug_expr(expr_p, member_info);
  }

#endif  // SKOOKUM_IDE_EMBEDDED


//---------------------------------------------------------------------------------------
// Scripted break debug function
// Arg         caller_p - object that called/invoked this expression and that may await
// Arg         scope_p - scope for data/method/etc. look-ups.
// See:        SkDebug::set_scripted_break()
// Author(s):   Conan Reis
void SkConsole::debug_scripted_break(
  const AString &   message,
  SkInvokedMethod * scope_p
  )
  {
  AString context_str(SkDebug::get_context_string(message, scope_p, nullptr, SkInvokeInfo_skip_this | SkInvokeInfo__callstack_def));

  ADebug::print_format("\nSkookum scripted break: %s\n", context_str.as_cstr());

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Bring up IDE
  SkInvokedBase *        caller_p    = scope_p->get_caller();
  SkInvokedContextBase * context_p   = caller_p->get_scope_context();
  SkInvokableBase *      member_p    = context_p->get_invokable();
  SkInstance *           top_scope_p = context_p->get_topmost_scope();
  SkContextInfo          member_info(*member_p, member_p->get_member_type(), top_scope_p->is_metaclass());
  SkDebugInfo            debug_info  = scope_p->get_debug_info();

  // $Revisit - CReis If it the current context did not come from a standard source file then
  // the callstack could be searched to see if any of them comes from standard source files.
  if (debug_info.is_origin_source())
    {
    ms_console_p->browse_member(member_info, (debug_info.m_source_idx != ADef_uint16) ? debug_info.m_source_idx : 0u);
    }

  ADialogOS::info(context_str, "Skookum Scripted Break", ADialogOS::Flag_none, ADialogOS::Icon_warning);
  ADebug::print("Done break\n");
  }


//=======================================================================================
// SkIncrementalSearchEditBox Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Constructor
// Author(s):   John Stenersen
SkIncrementalSearchEditBox::SkIncrementalSearchEditBox(
  SkEditBox     * parent_p,
  eParentContext  parent_context,
  int             width
  ) :
  AWindow(parent_p->get_os_handle(), WS_EX_TOOLWINDOW, 0),
  m_parent_p(parent_p),
  m_parent_context(parent_context),
  m_width(SkConsole::ms_console_p->get_ini_ide().get_value_int_default(300, g_ini_search_key_width_p, g_ini_section_search_p) ),
  m_search_key(this, "", SkConsole::ms_console_p->get_ini_font_code_narrow(), SkEditBox::RichFlag_show_selection_always | SkEditBox::RichFlag_single_line),
  m_case_sensitive_tgl(this, "Aa", ACheckType_2_state, SkConsole::ms_console_p->get_ini_font_code_narrow()),
  m_fuzzy_tgl(this, "Fz", ACheckType_2_state, SkConsole::ms_console_p->get_ini_font_code_narrow())
  {
  //  Setup and parent the incremental search editbox window.
  hide();
  set_border(Border_raised);

  //  Calculate the initial size
  int spacing = SkIncrementalSearchEditBox_spacing;
  m_height = get_font().get_height() + spacing * 2 + 1;   //  +1 because the shading makes the illusion the search key editbox is not centered.
  set_area(m_width, m_height);

  // Setup device context (DC) drawing properties - info is retained since it has its own private DC.
  HDC hdc = ::GetDC(m_os_handle);
  ::SelectObject(hdc, ((ATrueTypeFont *)m_font.m_sys_font_p)->m_font_handle_p);
  ::SetBkColor(hdc, ::GetSysColor(COLOR_3DFACE));
  ::SetBkMode(hdc, OPAQUE);  // TRANSPARENT OPAQUE
  ::ReleaseDC(m_os_handle, hdc);

  //  Setup and parent the search key editbox to the incremental search window.
  m_search_key.show();
  m_search_key.set_parent(this);
  m_search_key.set_border(Border_sunken);
  m_search_key.set_on_modified_func(new AMethod<SkIncrementalSearchEditBox>(this, & SkIncrementalSearchEditBox::on_search_key_modified));

  //  Setup the case sensitive checkbox.
  m_case_sensitive_tgl.set_on_toggled_func( new AMethodArg<SkIncrementalSearchEditBox, eAFlag>(this, &SkIncrementalSearchEditBox::on_toggle_case_sensitive));
  m_case_sensitive_tgl.enable_subclass_messages();
  m_case_sensitive_tgl.show();

  if (SkConsole::ms_console_p->get_ini_ide().get_value_bool_default(false, g_ini_search_key_case_sensitive_p, g_ini_section_search_p))
    {
    m_case_sensitive_tgl.set_toggle_state(AFlag_on);
    }
  else
    {
    m_case_sensitive_tgl.set_toggle_state(AFlag_off);
    }

  //  Setup the fuzzy search checkbox.
  m_fuzzy_tgl.set_on_toggled_func( new AMethodArg<SkIncrementalSearchEditBox, eAFlag>(this, &SkIncrementalSearchEditBox::on_toggle_fuzzy));
  m_fuzzy_tgl.enable_subclass_messages();
  m_fuzzy_tgl.show();

  if (SkConsole::ms_console_p->get_ini_ide().get_value_bool_default(false, g_ini_search_key_fuzzy_p, g_ini_section_search_p))
    {
    m_fuzzy_tgl.set_toggle_state(AFlag_on);
    }
  else
    {
    m_fuzzy_tgl.set_toggle_state(AFlag_off);
    }
  } //  SkIncrementalSearchEditBox::SkIncrementalSearchEditBox()


//---------------------------------------------------------------------------------------
//  Shows (displays) the Incremental Search editbox and sets the input focus to the search key editbox.
// 
//  #Author(s) John Stenersen
void SkIncrementalSearchEditBox::show()
  {
  //A_DPRINT(A_SOURCE_FUNC_STR "\n");

  AWindow::show();
  m_search_key.set_focus();

  } //  SkIncrementalSearchEditBox::show()


//---------------------------------------------------------------------------------------
//  Called when the input focus is attained by the incremental search editbox.
// 
//  #Author(s) John Stenersen
bool SkIncrementalSearchEditBox::on_focus()
  {
//  A_DPRINT(A_SOURCE_FUNC_STR "parent_context = %d\n", m_parent_context);

  switch (m_parent_context)
    {
    case ParentContext_other:
      A_DPRINT(A_SOURCE_FUNC_STR "Unexpected parent_context = %d\n", m_parent_context);
      break;

    //  Console
    case ParentContext_log:
      SkMainWindowBase::on_focus(&this->m_search_key, eFocusType(FocusType_isearch | FocusType_log));
      break;

    case ParentContext_workspace:
      SkMainWindowBase::on_focus(&this->m_search_key, eFocusType(FocusType_isearch | FocusType_workspace));
      break;

    //  Browser
    case ParentContext_editor:
      SkMainWindowBase::on_focus(&this->m_search_key, eFocusType(FocusType_isearch | FocusType_editor));
      break;

    case ParentContext_create_new:
      SkMainWindowBase::on_focus(&this->m_search_key, eFocusType(FocusType_isearch | FocusType_editsyntax));
      break;

    case ParentContext_search_dialog:
      SkMainWindowBase::on_focus(&this->m_search_key, eFocusType(FocusType_isearch | FocusType_editsyntax));
      break;

    case ParentContext_class_settings:
      SkMainWindowBase::on_focus(&this->m_search_key, eFocusType(FocusType_isearch | FocusType_editsyntax));
      break;

    default:
      A_DPRINT(A_SOURCE_FUNC_STR "Unknown parent_context = %d\n", m_parent_context);
    }

  return true;
  } //  SkIncrementalSearchEditBox::on_focus()


//---------------------------------------------------------------------------------------
//  Hides the Incremental Search editbox and returns input focus to the parent editbox.
// 
//  #Author(s) John Stenersen
void SkIncrementalSearchEditBox::hide()
  {
  // A_DPRINT(A_SOURCE_FUNC_STR "\n");

  if (!is_hidden())
    {
    AWindow::hide();
    m_parent_p->set_focus();
    }
  } //  SkIncrementalSearchEditbox::hide()


//---------------------------------------------------------------------------------------
// Reposition to top right of parent window
void SkIncrementalSearchEditBox::reposition()
  {
  set_position(
    m_parent_p->xy_client2screen(AVec2i(m_parent_p->get_width_client() - m_width, 0)));
  }

//---------------------------------------------------------------------------------------
//  Updates the Incremental Search editbox's size and location on a sizing event.
// 
//  #Author(s) John Stenersen
void SkIncrementalSearchEditBox::on_sizing()
  {
  if (is_hidden())
    {
    //A_DPRINT(A_SOURCE_FUNC_STR " - 0x%p hidden\n", this);
    return;
    }

  //A_DPRINT(A_SOURCE_FUNC_STR " - 0x%p\n", this);

  //  Calculate the size of the editbox.
  int spacing = SkIncrementalSearchEditBox_spacing;

  //  Determine the size and location of the case sensitive and fuzzy checkboxes.
  m_case_sensitive_tgl.resize();
  m_fuzzy_tgl.resize();
  m_case_sensitive_tgl.set_position(m_width - m_case_sensitive_tgl.get_width() - m_fuzzy_tgl.get_width() - spacing * 3 + 1, spacing / 2 + 1 );
  m_fuzzy_tgl.set_position(m_width - m_fuzzy_tgl.get_width() - spacing * 2 + 1, spacing / 2 + 1 );

  //  Determine and set the search key editbox's size and location.
  m_search_key.set_area(m_width - spacing * 4 - m_case_sensitive_tgl.get_width() - m_fuzzy_tgl.get_width(), get_font().get_height());
  m_search_key.set_position(spacing / 2, spacing / 2);

  } //  SkIncrementalSearchEditBox::on_sizing

//---------------------------------------------------------------------------------------
//  Called whenever a key is pressed in the parent editbox or the child's (search key) editbox.
//  The functioning of the incremental search is based on VS2013's Find (not it's Incremental Search).
//
//  Ctrl+I | Ctrl+F   Initiates showing of the incremental search editbox and searches forward for first match.
//                    While the incremental search editbox is showing, subsequent uses will search to the next match.
//
//  Ctrl+Shift+I |    Initiates showing of the search key editbox and searches backwards for first match.
//  Ctrl+Shift+F      While the incremental search editbox is showing, subsequent uses will search to the previous match.
//
//  Return | NumEnter While the incremental search editbox is shown and has input focus, this searches
//                    forward to next match.
// 
//  Shift+Return |    While the incremental search editbox is shown and has input focus, this searches
//  Shift+NumEnter    backward to previous match.
//
//  F3                Searches forward using last search key whether the incremental search editbox is shown or not and
//                    whether any matches are highlighted or not.
// 
//  Shift+F3          Searches backwards using last search key whether the incremental search editbox is shown or not and
//                    whether any matches are highlighted or not.
// 
//  Esc               If the incremental search editbox is showing, this hides the editbox leaving any matched
//                    text highlighted. If the editbox is hidden and matched text is highlighted, this unhighlights
//                    any matches.
//
//  #Notes
//    Call AKeyboard::get_mod_keys() to determine if any modifier keys are in effect.
//   
//  #Author(s)  John Stenersen
bool SkIncrementalSearchEditBox::on_key_press_bidirectional(
  // key code corresponding to a physical key on the keyboard.
  eAKey key,
  // true if this is a repeated send (holding down the key), false if this is the first
  // time key has been pressed.
  bool      repeated,
  eAKeyMod  mod_keys,
  bool      from_parent  //  = false
  )
  {
  //A_DPRINT(A_SOURCE_FUNC_STR " from " + AString((from_parent ? "parent" : "child")) + "\n");

  //  $Revisit - JStenersen If no text is select, take the current token at the caret location.
  //  Get the current selection.
  uint32_t  select_start;
  uint32_t  select_end;
  m_parent_p->get_selection(&select_start, &select_end);

  switch (key)
    {
    case 'I':
    case 'F':           //  Note: If key binding changes, be sure to change call to this routine in on_menu_common_goto().
      //  Enter incremental search mode.
      if (mod_keys == AKeyMod_ctrl)
        {
        if (is_hidden())
          {
          m_initial_start = select_start;
          m_initial_end   = select_end;
          m_accepted      = false;

          if (select_start != select_end)
            {
            m_search_key.set_text(m_parent_p->get_text(select_start, select_end));
            m_search_key.select_all();
            }
          else
            {
            m_search_key.set_text("");
            m_search_key.select_end();
            }

          //  Show the incremental search key editbox.
          m_parent_p->freeze();
          matches_highlight();
          on_sizing();
          show();
          m_parent_p->unfreeze();
          m_parent_p->ensure_visible_caret();
          }
        else
          {
          //  The search key editbox is already shown, so search for the next match.
          m_search_key.set_focus();

          //  If there is a selection, use that as the search key otherwise use the last search key.
          if ((select_start != select_end) &&
             (m_parent_p->get_text(select_start, select_end).compare(m_search_key_last, AStrCase_ignore )))
            {
            m_search_key.set_text(m_parent_p->get_text(select_start, select_end));
            m_search_key.select_end();
            search_forward(select_start);
            }
          else
            {
            search_forward(select_start + 1);
            }
          }

        return false;
        }

      if (mod_keys == AKeyMod_ctrl_shift)
        {
        if (!repeated)
          {
          //  Reverse search...
          search_reverse(0, max(select_start, select_end - 1));
          return false;
          }
        }

      break;

    case AKey_escape :
      //  Exit incremental search mode in steps.
      if (mod_keys == AKeyMod_none)
        {
        if (!repeated)
          {
          if (!is_hidden())
            {
            //  Exiting the first step - hiding the incremental search editbox (leave matches highlighted).
            m_parent_p->ensure_visible_caret();
            hide();
            return false;
            }
          
          if (m_match_count > 0)
            {
            //  Exiting second step - unhighlight everything.
            m_parent_p->deselect();
            matches_unhighlight();
            return false;
            }
          }
        }
      break;

    case AKey_return:
    case AKey_num_enter:
      //  Search forward while the incremental search editbox is shown.
      if (mod_keys == AKeyMod_none)
        {
        if (!repeated)
          {
          if (!is_hidden() && (select_start != select_end))
            {
            //  Forward search...
            m_accepted = true;
            search_forward(select_start + 1);
            return false;
            }
          }
        }

      if (!from_parent && (mod_keys == AKeyMod_shift))
        {
        if (!repeated)
          {
          //  Reverse search...
          m_accepted = true;
          search_reverse(0, max(select_start, select_end - 1));
          return false;
          }
        }
      break;

    case 'V' :
      if (mod_keys == AKeyMod_ctrl)
        {
        if (!is_hidden() && !from_parent)
          {
          m_search_key.clipboard_paste_plain();
          return false;
          }
        }
      break;

    case AKey_delete:     //  Cut the current line if nothing selected.
      if (mod_keys == AKeyMod_shift)
        {
        if (!is_hidden() && !from_parent && !m_search_key.is_selected())
          {
          m_search_key.clipboard_copy_plain_sel_or_row();
          m_search_key.remove_row(m_search_key.get_row_caret(), true);
          return false;
          }
        }
      break;

    case 'X':     //  Cut the current line if nothing selected.
      if (mod_keys == AKeyMod_ctrl)
        {
        if (!is_hidden() && !from_parent && !m_search_key.is_selected())
          {
          m_search_key.clipboard_copy_plain_sel_or_row();
          m_search_key.remove_row(m_search_key.get_row_caret(), true);
          return false;
          }
        }
      break;

    case 'Z':     //  Redo
      if (mod_keys == AKeyMod_ctrl_shift)
        {
        if (!is_hidden() && !from_parent)
          {
          m_search_key.redo();
          return false;
          }
        }
      break;

    case AKey_f3 :                   //  Note: If key binding changes, be sure to change call to this routine in on_menu_common_goto().
      //  Search whether the incremental search editbox is shown or not using the last search key.
      if (mod_keys == AKeyMod_none)
        {
        if (!repeated)
          {
          //  Forward search...
          search_forward(select_start + 1);
          return false;
          }
        }

      if (mod_keys == AKeyMod_shift)
        {
        if (!repeated)
          {
          //  Reverse search...
          search_reverse(0, max(select_start, select_end - 1));
          return false;
          }
        }
      break;
    }

  //  This is a little different because keypressed are coming from both the parent editbox (with editable text) and the search key editbox.
  if (from_parent)
    {
    return true;  //  Allow the parent to process a keypress unused by incremental search that it sent.
    }

  if (AKeyboard::is_edit_key(key, mod_keys))
    {
    return true;  //  Don't allow parent to process a keypress that will edit the text if coming from the search key editbox.

    }

  return m_parent_p->on_key_press(key, repeated);  //  Allow keypress processing by parent sent by the search key editbox e.g. an Alt+F4 key.
  } //  SkIncrementalSearchEditBox::on_key_press_bidirectional()


//---------------------------------------------------------------------------------------
//  Clear any highlighting related to the incremental search.
//
//  # Author(s) John Stenersen
void SkIncrementalSearchEditBox::matches_unhighlight()
  {
  m_parent_p->enable_on_modified(false);
  m_parent_p->freeze();
  m_parent_p->set_text_style(0, ALength_remainder, ATextStyle(AColor::ms_void, AColor::ms_default));
  m_match_count = 0;
  m_parent_p->unfreeze();
  m_parent_p->enable_on_modified(true);
  }


//---------------------------------------------------------------------------------------
//  Searches the entire parent editbox for the search key and highlights them.
//
//  #Author(s)  John Stenersen
void SkIncrementalSearchEditBox::matches_highlight()
  {
  m_parent_p->freeze();
  matches_unhighlight();

  eAStrCase case_sensitive = (m_case_sensitive_tgl.get_toggle_state() == AFlag_off) ? AStrCase_ignore : AStrCase_sensitive;

  AStringBM search_key(m_search_key.get_text(), case_sensitive);
  AString   edit_text   = m_parent_p->get_text();
  //A_DPRINT(A_SOURCE_FUNC_STR " \"" + search_key + "\"\n");

  if (search_key.get_length() <= 0)
    {
    set_color_background(g_color_bg);             // blue web bg AColor(0.11f, 0.13f, 0.19f)
    invalidate(true, true);
    m_parent_p->unfreeze();
    return;
    }

  //  Highlight all the matches.
  uint32_t find_start = 0;
  uint32_t find_end   = 0;
  m_parent_p->enable_on_modified(false);
  if (m_fuzzy_tgl.get_toggle_state())
    {
    while ((find_start < edit_text.get_length()) && edit_text.find_fuzzy(search_key, 1, & find_start, &find_end, find_start, ALength_remainder, case_sensitive))
      {
      set_text_style_fuzzy(find_start, find_end);
      m_match_count ++;
      find_start ++;
      }
    }
  else
    {
    while ((find_start < edit_text.get_length()) && edit_text.find(search_key, 1, & find_start, find_start, ALength_remainder, case_sensitive))
      {
      m_parent_p->set_text_style(find_start, find_start + search_key.get_length(), ATextStyle(AColor::ms_void, AColor(0.0, 0.0, 0.7)));
      m_match_count ++;
      find_start ++;
      }
    }
  m_parent_p->enable_on_modified(true);

  //  Preserve the last search key used.
  m_search_key_last = m_search_key.get_text();

  if ((m_match_count <= 0 ) && (search_key.get_length() > 0))
    {
    set_color_background(SkLog_colour_warning);   //  (1.0f,  0.61f, 0.2f,  1.0f)
    m_case_sensitive_tgl.set_color_background(SkLog_colour_warning);
    m_fuzzy_tgl.set_color_background(SkLog_colour_warning);
    invalidate(true, true);
    }
  else
    {
    set_color_background(g_color_bg);             // blue web bg AColor(0.11f, 0.13f, 0.19f)
    m_case_sensitive_tgl.set_color_background(g_color_bg);
    m_fuzzy_tgl.set_color_background(g_color_bg);
    invalidate(true, true);
    }

  m_parent_p->unfreeze();
  } //  SkIncrementalSearchEditbox::matches_highlight()


//---------------------------------------------------------------------------------------
//  Called to update all the highlighted matching text if the search key or file/parent editbox are changed.
//
//  #Author(s)  John Stenersen
void SkIncrementalSearchEditBox::on_search_key_modified()
  {
  //A_DPRINT(A_SOURCE_FUNC_STR "\n");

  search_forward(m_initial_start);
  m_parent_p->ensure_visible_caret();
  } //  SkIncrementalSearchEditBox::on_search_key_modified()


//---------------------------------------------------------------------------------------
//  Called to update all the highlighted matching if the parent editbox has been changed.
//  If there are no current matches highlighted, then everything remains unhighlighted.
//
//  #Author(s)  John Stenersen
void SkIncrementalSearchEditBox::on_parent_modified()
  {
  //A_DPRINT(A_SOURCE_FUNC_STR "\n");

  if (m_match_count <= 0)
    {
    return;
    }

    matches_highlight();
  } //  SkIncrementalSearchEditBox::on_parent_modified()


//---------------------------------------------------------------------------------------
//  Searches forward for the seach key in the range specified. If not found and the wrap_flag
//  is set, the search continues thru the remainder of the editbox text and then wraps to
//  the top of the editbox.
//
//  The found search key is selected otherwise any selection is cleared.
//
//  Returns:    "true" if the search key was found in the parent editbox range.
//              "false" if not found or if either search key or editbox are empty.
//  #Author(s)  John Stenersen
bool SkIncrementalSearchEditBox::search_forward(
  uint32_t  search_start, //  = 0
  uint32_t  search_end,   //  = ALength_remainder
  bool      wrap_flag     //  = true
  )
  {
  eAStrCase case_sensitive = m_case_sensitive_tgl.get_toggle_state() == AFlag_off ? AStrCase_ignore : AStrCase_sensitive;
  AStringBM search_key(m_search_key.get_text(), case_sensitive);
  AString   edit_text   = m_parent_p->get_text();

  m_parent_p->freeze();
  matches_highlight();

  //  Check to see if caret (search_start) is already at the end of the editbox text.
  if ((search_start >= edit_text.get_length()) && wrap_flag)
    {
    search_start = 0;
    }

  //  If either the search key or the editbox are empty, then no search will succeed.
  if ((search_key.get_length() <= 0) || (edit_text.get_length() <= 0))
    {
    m_parent_p->select( search_start, search_start );   //  Important to use select() and not deselect() so caret is at search_start
    m_parent_p->unfreeze();
    m_parent_p->ensure_visible_caret();
    return false;
    }

  //  Search for and select the next match within the range.
  uint32_t find_start = 0;
  uint32_t find_end   = 0;

  if (m_fuzzy_tgl.get_toggle_state())
    {
    if (edit_text.find_fuzzy(search_key, 1, & find_start, & find_end, search_start, search_end, case_sensitive))
      {
      m_parent_p->select(find_start, find_end);
      set_text_style_fuzzy(find_start, find_end);
      m_parent_p->unfreeze();
      m_parent_p->ensure_visible_caret();
      return true;
      }
    }
  else
    {
    if (edit_text.find(search_key, 1, & find_start, search_start, search_end, case_sensitive))
      {
      m_parent_p->select(find_start, find_start + search_key.get_length());
      m_parent_p->unfreeze();
      m_parent_p->ensure_visible_caret();
      return true;
      }
    }

  if (!wrap_flag)
    {
    m_parent_p->select( search_start, search_start );   //  Important to use select() and not deselect() so caret is at search_start
    m_parent_p->unfreeze();
    m_parent_p->ensure_visible_caret();
    return false;
    }

  //  Search from the remainder of the file/editbox.
  if (m_fuzzy_tgl.get_toggle_state())
    {
    if ((search_end < ALength_remainder) && (edit_text.find_fuzzy(search_key, 1, & find_start, & find_end, search_end, ALength_remainder, case_sensitive)))
      {
      m_parent_p->select(find_start, find_end);
      set_text_style_fuzzy(find_start, find_end);
      m_parent_p->unfreeze();
      m_parent_p->ensure_visible_caret();
      return true;
      }
    }
  else
    {
    if ((search_end < ALength_remainder) && (edit_text.find(search_key, 1, & find_start, search_end, ALength_remainder, case_sensitive)))
      {
      m_parent_p->select(find_start, find_start + search_key.get_length());
      m_parent_p->unfreeze();
      m_parent_p->ensure_visible_caret();
      return true;
      }
    }

  //  Search from the top file/editbox.
  if (m_fuzzy_tgl.get_toggle_state())
    {
    if ((search_start > 0) && (edit_text.find_fuzzy(search_key, 1, & find_start, & find_end, 0, ALength_remainder, case_sensitive)))
      {
      m_parent_p->select(find_start, find_end );
      set_text_style_fuzzy(find_start, find_end);
      m_parent_p->unfreeze();
      m_parent_p->ensure_visible_caret();
      return true;
      }
    }
  else
    {
    if ((search_start > 0) && (edit_text.find(search_key, 1, & find_start, 0, ALength_remainder, case_sensitive)))
      {
      m_parent_p->select(find_start, find_start + search_key.get_length());
      m_parent_p->unfreeze();
      m_parent_p->ensure_visible_caret();
      return true;
      }
    }

  m_parent_p->select( search_start, search_start );   //  Important to use select() and not deselect() so caret is at search_start
  m_parent_p->unfreeze();
  m_parent_p->ensure_visible_caret();
  return false;
  } //  SkIncrementalSearchEditBox::search_forward()


//---------------------------------------------------------------------------------------
//  Searches backwards for the seach key in the range specified. If not found and the wrap_flag
//  is set, the search continues thru the beginning of the editbox text and then wraps to
//  the bottom of the editbox.
//
//  The found search key is selected otherwise any selection is cleared.
//
//  Returns:    "true" if the search key was found in the parent editbox range.
//              "false" if not found or if either search key or editbox are empty.
//  #Author(s)  John Stenersen
bool SkIncrementalSearchEditBox::search_reverse(
  uint32_t  search_start, //  = 0
  uint32_t  search_end,   //  = ALength_remainder
  bool      wrap_flag     //  = true
  )
  {
  eAStrCase case_sensitive = m_case_sensitive_tgl.get_toggle_state() == AFlag_off ? AStrCase_ignore : AStrCase_sensitive;
  AString   search_key(m_search_key.get_text(), case_sensitive);
  AString   edit_text   = m_parent_p->get_text();

  m_parent_p->freeze();
  matches_highlight();

  if ((search_key.get_length() <= 0) || (edit_text.get_length() <= 0))
    {
    m_parent_p->select( search_start, search_start );   //  Important to use select() and not deselect() so caret is at search_start
    m_parent_p->unfreeze();
    m_parent_p->ensure_visible_caret();
    return false;
    }

  //  Search for and select the previous match within the range.
  uint32_t find_start = search_end;   //  Yes, no intuitive.
  uint32_t find_end   = search_end;
  if (m_fuzzy_tgl.get_toggle_state())
    {
    if (edit_text.find_fuzzy_reverse(search_key, 1, & find_start, & find_end, search_start, search_end, case_sensitive))
      {
      m_parent_p->select(find_start, find_end);
      set_text_style_fuzzy(find_start, find_end);
      m_parent_p->unfreeze();
      m_parent_p->ensure_visible_caret();
      return true;
      }
    }
  else
    {
    if (edit_text.find_reverse(search_key, 1, & find_start, search_start, search_end, case_sensitive))
      {
      m_parent_p->select(find_start, find_start + search_key.get_length());
      m_parent_p->unfreeze();
      m_parent_p->ensure_visible_caret();
      return true;
      }
    }

  if (!wrap_flag)
    {
    m_parent_p->select( search_start, search_start );   //  Important to use select() and not deselect() so caret is at search_start
    m_parent_p->unfreeze();
    m_parent_p->ensure_visible_caret();
    return false;
    }

  //  Search to the beginning of the file/editbox.
  if (m_fuzzy_tgl.get_toggle_state())
    {
    if ((search_end < ALength_remainder) && edit_text.find_fuzzy_reverse(search_key, 1, & find_start, & find_end, 0, search_start, case_sensitive))
      {
      m_parent_p->select(find_start, find_end);
      set_text_style_fuzzy(find_start, find_end);
      m_parent_p->unfreeze();
      m_parent_p->ensure_visible_caret();
      return true;
      }
    }
  else
    {
    if ((search_end < ALength_remainder) && (edit_text.find_reverse(search_key, 1, & find_start, 0, search_start, case_sensitive)))
      {
      m_parent_p->select(find_start, find_start + search_key.get_length());
      m_parent_p->unfreeze();
      m_parent_p->ensure_visible_caret();
      return true;
      }
    }

  //  Search from the bottom of the file/editbox.
  if (m_fuzzy_tgl.get_toggle_state())
    {
    if (edit_text.find_fuzzy_reverse(search_key, 1, & find_start, & find_end, search_start, ALength_remainder, case_sensitive))
      {
      m_parent_p->select(find_start, find_end);
      set_text_style_fuzzy(find_start, find_end);
      m_parent_p->unfreeze();
      m_parent_p->ensure_visible_caret();
      return true;
      }
    }
  else
    {
    if (edit_text.find_reverse(search_key, 1, & find_start, search_start, ALength_remainder, case_sensitive))
      {
      m_parent_p->select(find_start, find_start + search_key.get_length());
      m_parent_p->unfreeze();
      m_parent_p->ensure_visible_caret();
      return true;
      }
    }

  m_parent_p->select( search_start, search_start );   //  Important to use select() and not deselect() so caret is at search_start
  m_parent_p->unfreeze();
  m_parent_p->ensure_visible_caret();
  return false;
  } //  SkIncrementalSearchEditBox::search_reverse()


//---------------------------------------------------------------------------------------
//  The case sensitive checkbox has changed state.
//
//  #Author(s)  John Stenersen
void SkIncrementalSearchEditBox::on_toggle_case_sensitive(eAFlag toggle_state)
  {
  //A_DPRINT(A_SOURCE_FUNC_STR "\n");

  on_search_key_modified();
  m_search_key.set_focus();
  SkConsole::ms_console_p->get_ini_ide().set_value(m_case_sensitive_tgl.get_toggle_state() == AFlag_off ? "false" : "true", g_ini_search_key_case_sensitive_p, g_ini_section_search_p);
  } //  SkIncrementalSearchEditBox::on_toggle_case_sensitive()


//---------------------------------------------------------------------------------------
//  The fuzzy search checkbox has changed state.
//
//  #Author(s)  John Stenersen
void SkIncrementalSearchEditBox::on_toggle_fuzzy(eAFlag toggle_state)
  {
  //A_DPRINT(A_SOURCE_FUNC_STR "\n");

  on_search_key_modified();
  m_search_key.set_focus();
  SkConsole::ms_console_p->get_ini_ide().set_value(m_fuzzy_tgl.get_toggle_state() == AFlag_off ? "false" : "true", g_ini_search_key_fuzzy_p, g_ini_section_search_p);
  } //  SkIncrementalSearchEditBox::on_toggle_fuzzy()


//---------------------------------------------------------------------------------------
//  Highlights the fuzzy search key letters.
//
//  #Author(s)  John Stenersen
void SkIncrementalSearchEditBox::set_text_style_fuzzy(uint32_t fuzzy_start, uint32_t fuzzy_end)
  {
  eAStrCase case_sensitive = (m_case_sensitive_tgl.get_toggle_state() == AFlag_off) ? AStrCase_ignore : AStrCase_sensitive;
  AString   search_key(m_search_key.get_text(), case_sensitive);
  AString   edit_text( m_parent_p->get_text() );

  m_parent_p->enable_on_modified(false);
  m_parent_p->set_text_style(fuzzy_start, fuzzy_end, ATextStyle(AColor::ms_void, AColor(0.0, 0.0, 0.7)));

  uint32_t key_index = 0;
  for (uint32_t i = fuzzy_start; i <= fuzzy_end; i++ )
    {
    if (m_case_sensitive_tgl.get_toggle_state())
      {
      if (edit_text[ i ] == search_key[ key_index ])
        {
        m_parent_p->set_text_style(i, i + 1, ATextStyle(AColor::ms_void, AColor(0.3, 0.0, 0.7)));
        key_index++;
        }
      }
    else
      {
      if (!AString::compare_insensitive(edit_text[ i ], search_key[ key_index ]))
        {
        m_parent_p->set_text_style(i, i + 1, ATextStyle(AColor::ms_void, AColor(0.3, 0.0, 0.7)));
        key_index++;
        }
      }
    }

  m_parent_p->enable_on_modified(true);
  } //  SkIncrementalSearchEditBox::set_text_style_fuzzy()
