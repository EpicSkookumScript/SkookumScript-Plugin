// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumIDE Application
//
// SkookumScript IDE: Navigation / Go to View
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumIDE/SkNavigationView.hpp>
#include <stdio.h>
#include <AgogCore/ACompareMethod.hpp>
#include <AgogCore/AMethod.hpp>
#include <AgogIO/AIni.hpp>
#include <AgogGUI/ATrueTypeFont.hpp>
#include <AgogGUI_OS/ADialogOS.hpp>
#include <SkookumScript/SkBrain.hpp>
#include <SkookumScript/SkExpressionBase.hpp>
#include <SkookumScript/SkNone.hpp>
#include <SkookumScript/SkSymbol.hpp>
#include <SkookumIDE/SkClassBrowser.hpp>
#include <SkookumIDE/SkookumIDE_Res.h>
#include <objidl.h>

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
    // $Revisit - CReis Many of these initial values should be provided by application defined functions.

    SkCTInfo_population_init   = 256,  // Initial simultaneous tree info structures in class tree
    SkCTInfo_population_expand = 64    // Member info structure grow amount - used in class tree
    };

  const char * g_ini_section_tooltip_p              = "ToolTip";                  //  Note: Same as in SkConsole.cpp  $Revisit: JStenersen - INI settings should be in a global INI namespace) since the .ini file is a global thingy.
  const char * g_ini_tooltip_enable_on_startup_p    = "ToolTipEnableOnStartup";   //  Note: Same as in SkConsole.cpp  $Revisit: JStenersen - INI settings should be in a global INI namespace) since the .ini file is a global thingy.
  const char * g_ini_tooltip_enable_create_popup_p  = "ToolTipEnableCreatePopup";

  static const AColor g_colour_okay(        0.7f,  1.0f,  0.25f);  // Light Green
  static const AColor g_colour_okay_dark(   0.52f, 0.75f, 0.15f);  // Med Green
  static const AColor g_colour_unbound(     1.0f,  0.65f, 0.0f);   // Light orange
  static const AColor g_colour_unbound_dark(0.8f,  0.52f, 0.0f);   // Med orange
  static const AColor g_color_error(        0.86f, 0.08f, 0.24f);  // Crimson
  static const AColor g_color_bg(           0.18f, 0.18f, 0.22f);  // Pro
  static const AColor g_color_text_bg(      0.15f, 0.15f, 0.19f);  // Pro Edit partial
  static const AColor g_color_text_edit_bg( 0.13f, 0.13f, 0.16f);  // Pro Edit
  static const AColor g_color_bg_select(    0.39f, 0.43f, 0.52f);  // Selected item background
  static const AColor g_color_bg_focus(     0.36f, 0.52f, 0.77f);  // Focused item background

  // *IDE* config (.ini) file
  const char * g_ini_section_browser_p            = "Script Browser";
  const char * g_ini_key_overlay_working_p        = "WorkingOverlay";

} // End unnamed namespace


//=======================================================================================
// SkClassTree Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Constructor
// Author(s):   Conan Reis
SkClassTree::SkClassTree(
  AWindow *        parent_p,
  SkClassBrowser * browser_p
  ) :
  ATreeOS(parent_p),
  m_show_meta_override(AFlag__toggle),
  m_browser_p(browser_p)
  {
  TreeView_SetBkColor(m_os_handle, g_color_text_bg);
  set_border(Border_line);
  build_tree();
  }

//---------------------------------------------------------------------------------------
// Constructor
// Author(s):   Conan Reis
SkClassTree::~SkClassTree()
  {
  // Some windows/controls need to call destroy() in their own destructor
  // rather than letting the AMessageTarget destructor call it since destroy()
  // will end up sending windows messages and the windows/controls need to have
  // their virtual table still intact.
  destroy();
  }

//---------------------------------------------------------------------------------------
// Gets the currently selected class (or nullptr if none selected)
// Author(s):   Conan Reis
SkClass * SkClassTree::get_selected_class() const
  {
  SkCTInfo * selected_p = static_cast<SkCTInfo *>(get_selected());

  return selected_p ? selected_p->m_class_p : nullptr;
  }

//---------------------------------------------------------------------------------------
// Adds all classes to the class tree
// Author(s):   Conan Reis
void SkClassTree::build_tree()
  {
  m_items.ensure_size_empty(SkBrain::get_classes().get_length());
  remove_all();

  append_classes(*SkBrain::ms_object_class_p);
  }

//---------------------------------------------------------------------------------------
// Changes the currently focused class to the one indicated
// Author(s):   Conan Reis
void SkClassTree::set_class(
  SkClass * class_p,
  eAFlag    show_meta // = AFlag__toggle
  )
  {
  if (class_p)
    {
    SkCTInfo * select_class_p = m_items.get(class_p->get_name());

    if (select_class_p)
      {
      if (get_selected_class() != class_p)
        {
        m_show_meta_override = show_meta;
        select(*select_class_p);
        }

      ensure_visible(*select_class_p);
      }
    }
  else
    {
    remove_all();
    }
  }

//---------------------------------------------------------------------------------------
// Ensures that the specified class is visible
// Author(s):   Conan Reis
void SkClassTree::ensure_visible_class(SkClass * class_p)
  {
  SkCTInfo * veiw_class_p = m_items.get(class_p->get_name());

  if (veiw_class_p)
    {
    ensure_visible(*veiw_class_p);
    }
  }

//---------------------------------------------------------------------------------------
// Called at various stages of draw cycle - allowing user to alter or replace
//             the default render mechanism.
// Modifiers:   virtual - override for custom behaviour
// Author(s):   Conan Reis
LRESULT SkClassTree::on_custom_draw(NMTVCUSTOMDRAW * info_p)
  {
  switch (info_p->nmcd.dwDrawStage)
    {
    case CDDS_PREPAINT:
      // Ask for subitem notifications
      return CDRF_NOTIFYSUBITEMDRAW;

    case CDDS_ITEMPREPAINT:
    case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
      {
      // item_handle = info_p->nmcd.dwItemSpec;
      // item_lparam = info_p->nmcd.lItemlParam;
      SkCTInfo * item_p  = reinterpret_cast<SkCTInfo *>(info_p->nmcd.lItemlParam);
      SkClass *  class_p = item_p->m_class_p;

      if (class_p->is_demand_loaded())
        {
        SkClass * root_p     = class_p->get_demand_loaded_root();
        bool      root_class = class_p == root_p;

        info_p->clrText = class_p->is_loaded()
          ? (root_p->is_unload_deferred()
            ? (root_class ? AColor::ms_yellow : AColor::ms_tan)
            : (root_class ? g_colour_okay : g_colour_okay_dark))
          : (root_class ? g_colour_unbound : g_colour_unbound_dark);
        }
      else
        {
        info_p->clrText = AColor::ms_white;
        }

      if (info_p->nmcd.uItemState & CDIS_SELECTED)
        {
        info_p->clrTextBk = g_color_bg_select;
        }
      if (info_p->nmcd.uItemState & CDIS_FOCUS)
        {
        info_p->clrTextBk = g_color_bg_focus;
        }

      return CDRF_NEWFONT | CDRF_SKIPPOSTPAINT;
      }
    }

  return CDRF_DODEFAULT;
  }

//---------------------------------------------------------------------------------------
// Called whenever a key is pressed.
// 
// #Notes
//   Call AKeyboard::get_mod_keys() to determine if any modifier keys are in effect.
//   
// #Modifiers virtual - Override for custom behaviour.
// #See Also  on_character(), on_key_release()
// #Author(s) Conan Reis
bool SkClassTree::on_key_press(
  // key code corresponding to a physical key on the keyboard.
  // If Shift-'2' is pressed, AKey_shift is sent first and then AKey_2, not '@'.  Defines
  // for codes are prefixed with "AKey_" and are in AgogIO/AKeyboard.hpp
  // AKey_0 to AKey_9 are the same as ANSI '0' to '9' (0x30 - 0x39)
  // AKey_A to AKey_Z are the same as ANSI 'A' to 'Z' (0x41 - 0x5A)
  // Special characters like AKey_control are also possible.
  eAKey key,
  // true if this is a repeated send (holding down the key), false if this is the first
  // time key has been pressed.
  bool repeated
  )
  {
  //A_DPRINT(A_SOURCE_FUNC_STR "0x%p\n", this);

  switch (key)
    {
    case 'G':
      if (AKeyboard::get_mod_keys() == AKeyMod_ctrl)
        {
        // Ignore repeated keys
        if (!repeated)
          {
          SkConsole::ms_console_p->display_goto_dialog(SkMatchKind_classes);
          }

        return false;
        }
    }

  return static_cast<AWindow *>(m_parent_p)->on_key_press(key, repeated);
  }


//---------------------------------------------------------------------------------------
// Called when input (keyboard) focus is attained.
// # See:      on_focus()
// # Modifiers: virtual
// # Author(s): John Stenersen
bool SkClassTree::on_focus()
  {
//  A_DPRINT(A_SOURCE_FUNC_STR "\n");

  SkClassBrowser::ms_browser_p->set_focus_splitter(reinterpret_cast<ASplitterOS *>(this->get_parent()->get_parent()->get_parent()));
  SkMainWindowBase::on_focus(reinterpret_cast<SkEditSyntax *>(this), SkMainWindowBase::FocusType_class_tree);

  return true;
  }


//---------------------------------------------------------------------------------------
// Signals that the selection has changed from one item to another.
// Arg         item_p - item newly selected
// Arg         old_item_p - item previously selected
// Modifiers:   virtual
// Author(s):   Conan Reis
void SkClassTree::on_selected(ATreeItemOS * item_p, ATreeItemOS * old_item_p)
  {
  SkClass * class_p = static_cast<SkCTInfo *>(item_p)->m_class_p;

  //A_DPRINT(A_SOURCE_STR " SkClassTree::on_selected(%s)%s\n", class_p->get_name_cstr(),
  //  (AKeyboard::get_mod_keys() == AKeyMod_ctrl) ? " + Ctrl" : "");
    
  bool show_meta = (m_show_meta_override == AFlag__toggle)
    ? (AKeyboard::get_mod_keys() != AKeyMod_ctrl)
    : (m_show_meta_override == AFlag_on);

  m_show_meta_override = AFlag__toggle;

  m_browser_p->on_class_selected(class_p, show_meta);
  }

//---------------------------------------------------------------------------------------
// Called when an item is removed
// Arg         item_p - removed item
// Modifiers:   virtual
// Author(s):   Conan Reis
void SkClassTree::on_removed(ATreeItemOS * item_p)
  {
  m_items.remove(*static_cast<SkCTInfo *>(item_p));
  pool_delete(static_cast<SkCTInfo *>(item_p));
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
bool SkClassTree::on_mouse_press(
  eAMouse        button,
  eAMouse        buttons,
  const AVec2i & client_pos,
  bool           double_click
  )
  {
  //A_DPRINT(A_SOURCE_FUNC_STR "0x%p\n", this);

  bool mod_ctrl = AKeyboard::get_mod_keys() == AKeyMod_ctrl;

  // If item clicked on without Ctrl and is already selected but not displaying
  // !Class.sk-meta then show it.
  if (!mod_ctrl)
    {
    SkClass * old_class_p = get_selected_class();

    if (old_class_p)
      {
      SkCTInfo * info_p = static_cast<SkCTInfo *>(get_by_pos(client_pos));
      SkClass *  new_class_p = info_p ? info_p->m_class_p : nullptr;

      if (new_class_p == old_class_p)
        {
        m_browser_p->on_class_selected(old_class_p);
        }
      }
    }

  if (buttons == AMouse_right)
    {
    if (mod_ctrl)
      {
      on_context_menu(xy_client2screen(client_pos));
      }

    // Allow context menu event to get through
    // $Revisit - CReis this prevents a nice custom draw highlight of the item - it would be nice to keep it.
    return false;
    }

  return true;
  }

//---------------------------------------------------------------------------------------
// Called whenever a mouse button is released in the client region of this
//             window (or anywhere if the mouse is 'captured').
// Arg         button - the button that was just released - see eAMouse
// Arg         buttons - the press state of all the mouse buttons - see eAMouse
// Arg         client_pos - client position of mouse cursor
// See:        on_mouse_press(), on_mouse_moving(), on_mouse_spinning(),
//             enable_mouse_capture(), AMouse
// Notes:      AKeyboard::get_mod_keys() can be called to determine if any modifier keys
//             are in effect.
// Modifiers:   virtual - Override for custom behaviour.
// Author(s):   Conan Reis
void SkClassTree::on_mouse_release(
  eAMouse        button,
  eAMouse        buttons,
  const AVec2i & client_pos
  )
  {
  //A_DPRINT(A_SOURCE_FUNC_STR "0x%p\n", this);

  m_browser_p->on_mouse_release(button, buttons, client_pos);
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
bool SkClassTree::on_context_menu(const AVec2i & screen_pos)
  {
  bool          call_parent = false;
  ATreeItemOS * item_p      = get_by_pos(xy_screen2client(screen_pos));

  if (item_p)
    {
    SkClass * class_p = static_cast<SkCTInfo *>(item_p)->m_class_p;

    set_class(class_p);

    enum eClassPop
      {
      //SkBrowseMenu_edit_copy,
      //SkBrowseMenu_edit_copy_path,

      ClassPop_recompile,
      ClassPop_recompile_recurse,

      ClassPop_obj_id_validate,
      ClassPop_obj_id_validate_recurse,

      ClassPop_rename,
      ClassPop_remove,
      ClassPop_new,

      ClassPop_goto_class,

      // Memory sub-menu
        ClassPop_demand_load,
        ClassPop_toggle_load_lock,
        ClassPop_toggle_load,
        ClassPop_demand_load_all,

        ClassPop_memory_usage,
        ClassPop_memory_usage_recursive,
        ClassPop_memory_usage_recursive_demand,

      ClassPop_count_expr,
      ClassPop_count_expr_recurse,

      //SkBrowseMenu_file_open_associated,
      //SkBrowseMenu_file_open_explorer,

      ClassPop_arrange_panes
      };


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Build the pop-up
    APopMenuOS   pop_menu;
    const char * class_name_p   = class_p->get_name_cstr();
    uint32_t     subclass_count = class_p->get_subclasses().get_length();

    pop_menu.append_item(a_cstr_format("Recompile '%s'", class_name_p),                ClassPop_recompile);
    if (subclass_count)
      {
      pop_menu.append_item(a_cstr_format("Recompile '%s' + subclasses", class_name_p), ClassPop_recompile_recurse);
      }

    pop_menu.append_item(a_cstr_format("Validate '%s' Object IDs", class_name_p),                ClassPop_obj_id_validate);
    if (subclass_count)
      {
      pop_menu.append_item(a_cstr_format("Validate '%s' + subclasses Object Ids", class_name_p), ClassPop_obj_id_validate_recurse);
      }

    //--------------------
    pop_menu.append_separator();
    pop_menu.append_item("&New subclass / member...\t[Ctrl+N]",       ClassPop_new);
    //pop_menu.append_item(a_cstr_format("Rename '%s'...", class_name_p), ClassPop_rename, false);
    //pop_menu.append_item(a_cstr_format("Remove '%s'...", class_name_p), ClassPop_remove, false);
    // $Revisit - CReis Move / copy class to current overlay - prompt if overlay is different

    //--------------------
    pop_menu.append_separator();
    pop_menu.append_item("Goto Class/Context...\t[Alt+C or Ctrl+G]", ClassPop_goto_class);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    APopMenuOS subpop_memory;

      pop_menu.append_separator();
      pop_menu.append_submenu(&subpop_memory, a_cstr_format("Memory Usage for '%s'", class_name_p));

      bool      demand_loaded = class_p->is_demand_loaded();
      SkClass * load_root_p   = nullptr;

      subpop_memory.append_item("Demand load/unload this group",     ClassPop_demand_load, false, demand_loaded);

      if (demand_loaded)
        {
        load_root_p = class_p->get_demand_loaded_root();

        bool load_locked = load_root_p->is_load_locked();

        subpop_memory.append_item(a_cstr_format("Lock '%s' group in memory once loaded", load_root_p->get_name_cstr()), ClassPop_toggle_load_lock, true, load_locked);
        subpop_memory.append_item(
          a_cstr_format(class_p->is_loaded() ? "Unload '%s' class group" : "Load '%s' class group", load_root_p->get_name_cstr()),
          ClassPop_toggle_load, !class_p->is_loaded() || !load_locked);
        }

      subpop_memory.append_item("Load *all* demand loaded classes",  ClassPop_demand_load_all);

      //--------------------
      subpop_memory.append_separator();
      subpop_memory.append_item("Memory Usage for class",                        ClassPop_memory_usage);
      subpop_memory.append_item("Memory for class + subclasses",                 ClassPop_memory_usage_recursive);
      subpop_memory.append_item("Memory for class + subclasses + demand loaded", ClassPop_memory_usage_recursive_demand);


      pop_menu.append_item(a_cstr_format("'%s' expression #", class_name_p),              ClassPop_count_expr);

    if (subclass_count)
      {
      pop_menu.append_item(a_cstr_format("'%s' + subclass expression #", class_name_p), ClassPop_count_expr_recurse);
      }

    //--------------------
    pop_menu.append_separator();
    pop_menu.append_item("Arrange panes...",             ClassPop_arrange_panes);


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Show the pop-up
    uint32_t pop_id   = 0u;
    bool selected = pop_menu.show(screen_pos, *this, &pop_id);

    if (selected)
      {
      switch (eClassPop(pop_id))
        {
        case ClassPop_recompile:
          SkConsole::ms_console_p->compile_class_browser(false);
          break;

        case ClassPop_recompile_recurse:
          SkConsole::ms_console_p->compile_class_browser(true);
          break;

        case ClassPop_obj_id_validate:
          SkCompiler::ms_compiler_p->load_and_validate_object_ids(class_p, AHierarchy_current);
          break;

        case ClassPop_obj_id_validate_recurse:
          SkCompiler::ms_compiler_p->load_and_validate_object_ids(class_p);
          break;

        case ClassPop_new:
          m_browser_p->get_navigation_view().show_create_popup();
          break;

        case ClassPop_goto_class:
          SkConsole::ms_console_p->display_goto_dialog(SkMatchKind_classes);
          break;

        case ClassPop_count_expr:
          SkDebug::print(a_str_format("\nCounting number of expressions used by '%s' ...\n", class_p->get_name_cstr_dbg()), SkLocale_local);
          SkDebug::print(a_str_format("  %u expressions used.\n\n", class_p->count_expressions_recurse(AHierarchy_current)), SkLocale_local);
          break;

        case ClassPop_count_expr_recurse:
          SkDebug::print(a_str_format("\nCounting number of expressions used by '%s' and its subclasses...\n", class_p->get_name_cstr_dbg()), SkLocale_local);
          SkDebug::print(a_str_format("  %u expressions used.\n\n", class_p->count_expressions_recurse(AHierarchy__all)), SkLocale_local);
          break;

        case ClassPop_memory_usage:
          SkDebug::print_memory_code(SkCodeSerialize_static_demand, class_p, AHierarchy_current);
          break;

        case ClassPop_memory_usage_recursive:
          SkDebug::print_memory_code(SkCodeSerialize_static, class_p);
          break;

        case ClassPop_memory_usage_recursive_demand:
          SkDebug::print_memory_code(SkCodeSerialize_static_demand, class_p);
          break;

        case ClassPop_demand_load_all:
          ADebug::print("\nLoading all demand load class groups...\n");
          SkRuntimeBase::ms_singleton_p->load_compiled_class_group_all();
          ADebug::print("  ...done!\n\n");

          // Update Browser
          invalidate();
          m_browser_p->get_member_view().refresh_members();
          break;

        case ClassPop_toggle_load_lock:
          load_root_p->lock_load(!load_root_p->is_load_locked());
          break;

        case ClassPop_toggle_load:
          if (class_p->is_loaded())
            {
            ADebug::print_format("\nUnloading '%s' demand load class group...\n", load_root_p->get_name_cstr());
            if (load_root_p->demand_unload())
              {
              ADebug::print("  ...done\n");
              }
            else
              {
              ADebug::print("  ...deferred!  It will unload when all its instances are destroyed.\n");
              }

            // Update Browser
            invalidate();
            m_browser_p->get_member_view().refresh_members();
            }
          else
            {
            ADebug::print_format("\nLoading '%s' demand load class group...\n", load_root_p->get_name_cstr());
            SkRuntimeBase::ms_singleton_p->load_compiled_class_group(load_root_p);
            ADebug::print("  ...done\n");

            // Update Browser
            invalidate();
            m_browser_p->get_member_view().refresh_members();
            }
          break;

        case ClassPop_arrange_panes:
          call_parent = true;
          break;
        }
      }
    }

  // Call parent's on_context_menu()?
  return call_parent;
  }

//---------------------------------------------------------------------------------------
// Adds a class and all its subclasses to the class tree
// Author(s):   Conan Reis
void SkClassTree::append_class(
  const SkClass & new_class,
  bool            select_it // = true
  )
  {
  SkCTInfo * item_p        = pool_new(new_class);
  SkCTInfo * parent_item_p = m_items.get(new_class.get_superclass()->get_name());

  m_items.append(*item_p);
  append(item_p, parent_item_p, true);

  if (select_it)
    {
    select(*item_p);
    ensure_visible(*item_p);
    }
  }

//---------------------------------------------------------------------------------------
// Adds a class and all its subclasses to the class tree
// Author(s):   Conan Reis
void SkClassTree::append_classes(
  const SkClass & new_class,
  SkCTInfo *      parent_p // = nullptr
  )
  {
  SkCTInfo * item_p = pool_new(new_class);

  m_items.append(*item_p);
  append(item_p, parent_p, true);

  const tSkClasses & subclasses     = new_class.get_subclasses();
  SkClass **         classes_pp     = subclasses.get_array();
  SkClass **         classes_end_pp = classes_pp + subclasses.get_length();

  for (; classes_pp < classes_end_pp; classes_pp++)
    {
    append_classes(**classes_pp, item_p);
    }
  }

//---------------------------------------------------------------------------------------
// Author(s):    Conan Reis
SkCTInfo * SkClassTree::pool_new(const SkClass & ssclass)
  {
  SkCTInfo * info_p = get_pool().allocate();

  info_p->m_class_p = const_cast<SkClass *>(&ssclass);
  info_p->m_title = ssclass.get_name_str();
  info_p->m_bolded = 
       ssclass.is_class(*SkBrain::ms_mind_class_p) 
    || (SkBrain::ms_actor_class_p 
      && (ssclass.is_class(*SkBrain::ms_actor_class_p) 
        || (&ssclass != SkBrain::ms_object_class_p && SkBrain::ms_actor_class_p->is_class(ssclass))));

  return info_p;
  }

//---------------------------------------------------------------------------------------
// Author(s):    Conan Reis
void SkClassTree::pool_delete(SkCTInfo * info_p)
  {
  get_pool().recycle(info_p);
  }

//---------------------------------------------------------------------------------------
// Returns dynamic reference pool. Pool created first call and reused on successive calls.
// 
// #Notes
//   Uses Scott Meyers' tip "Make sure that objects are initialized before they're used"
//   from "Effective C++" [Item 47 in 1st & 2nd Editions and Item 4 in 3rd Edition]
//   This is instead of using a non-local static object for a singleton.
//   
// #Modifiers  static
// #Author(s)  Conan Reis
AObjReusePool<SkCTInfo> & SkClassTree::get_pool()
  {
  static AObjReusePool<SkCTInfo> s_pool(SkCTInfo_population_init, SkCTInfo_population_expand);

  return s_pool;
  }


//=======================================================================================
// SkOverlayPicker Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Arg         region - if height is not supplied, it will be auto determined
// Author(s):   Conan Reis
SkOverlayPicker::SkOverlayPicker(
  AWindow * parent_p
  ) :
  AComboBoxOS(parent_p, AComboOSFlags__static_no_sort, parent_p->get_font().as_variant(parent_p->get_font().get_point_size() - 1.5f)),
  m_overlay_p(nullptr)
  {
  //set_color_background(g_color_bg);
  }

//---------------------------------------------------------------------------------------
// Add script overlays to this picker
// Author(s):   Conan Reis
void SkOverlayPicker::populate()
  {
  // Clear any existing overlays
  remove_all();

  // Get previously saved overlay
  const APArrayFree<SkOverlay> & overlays = SkCompiler::ms_compiler_p->get_overlays();

  // Get previous working overlay
  AString working_name = SkConsole::ms_console_p->get_ini_ide().get_value(
    g_ini_key_overlay_working_p,
    g_ini_section_browser_p);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Copy current overlays
  uint         length         = overlays.get_length();
  SkOverlay ** overlay_pp     = overlays.get_array();
  SkOverlay ** overlay_end_pp = overlay_pp + length;
  SkOverlay *  overlay_p      = nullptr;
  SkOverlay *  top_overlay_p  = nullptr;

  for (; overlay_pp < overlay_end_pp; overlay_pp++)
    {
    overlay_p = *overlay_pp;

    if (overlay_p->m_apply_b && overlay_p->m_editable_b)
      {
      append(overlay_p->m_name.as_cstr(), *overlay_p);

      top_overlay_p = overlay_p;

      if (working_name == overlay_p->m_name)
        {
        m_overlay_p = overlay_p;
        }
      }
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // if overlay not found set to last enabled/applied one
  if (!m_overlay_p)
    {
    m_overlay_p = top_overlay_p;

    // If top overlay exists then set it to working overlay
    if (top_overlay_p)
      {
      SkConsole::ms_console_p->get_ini_ide().set_value(
        top_overlay_p->m_name,
        g_ini_key_overlay_working_p,
        g_ini_section_browser_p);
      }
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Set to working overlay
  if (m_overlay_p)
    {
    select(*m_overlay_p);
    }
  }

//---------------------------------------------------------------------------------------
// Called whenever a new selection is accepted.
// Notes:      Call row2info() or get_selected_info() to get the the row's item info.
// Modifiers:   virtual - override for custom behaviour
// Author(s):   Conan Reis
void SkOverlayPicker::on_selected(uint row, uint row_prev)
  {
  //A_DPRINT(A_SOURCE_FUNC_STR "New: %u(%u)  Prev: %u(%u)\n", row, row2info(row), row_prev, row2info(row_prev));
  SkOverlay * overlay_p     = row2obj<SkOverlay>(row);
  SkOverlay * old_overlay_p = m_overlay_p;

  if (overlay_p != old_overlay_p)
    {
    m_overlay_p = overlay_p;

    // Save working overlay to settings file.
    SkConsole::ms_console_p->get_ini_ide().set_value(
      overlay_p->m_name,
      g_ini_key_overlay_working_p,
      g_ini_section_browser_p);

    static_cast<SkCreatePopup *>(m_parent_p)->update();
    }
  }


//=======================================================================================
// SkCreatePopup Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Constructor
// Author(s):   Conan Reis
SkCreatePopup::SkCreatePopup(
  AWindow * parent_p,
  SkClassBrowser * browser_p
  ) :
  AWindow(AWindow::ms_region_def, parent_p),
  m_browser_p(browser_p),
  m_create_type(SkMember__invalid),
  m_source_type(SourceType_template),
  m_source_overlay_p(nullptr),
  m_source_routine_p(nullptr),
  m_btn_close(this, "x", SkConsole::ms_console_p->get_ini_font_code_narrow()),
  m_line2_y(0),
  m_display_class_b(true),
  m_parent_class_p(nullptr),
  m_parent_class_name("???."),
  m_create_text(SkEditSyntax::Type_single_line, this, SkIncrementalSearchEditBox::ParentContext_create_new, "", SkConsole::ms_console_p->get_ini_font_code_narrow()),
  m_tooltip(&m_create_text,
    "As you type in the Class, method, coroutine or data member name the IDE determines what you want to create.\n"
    "Classes start with an uppercase letter, methods start with lowercase, coroutines start with _lower and data members start with @lower.\n"
    "When it is green it is ready to create by pressing 'Enter'.\n"
    "<A>See Website Docs</A> or Right-Click for more options.\n"
    "<A>Disable</A> this ToolTip, Ctrl+T to re-enable.",
    SkConsole::ms_console_p->get_ini_ide().get_value_bool_default(false, g_ini_tooltip_enable_on_startup_p, g_ini_section_tooltip_p) ||
      SkConsole::ms_console_p->get_ini_ide().get_value_bool_default(true, g_ini_tooltip_enable_create_popup_p, g_ini_section_tooltip_p),
    4000  //  delay_reshow milliseconds -- The m_create_text editbox is so small there needs to be a delay so there's time to click the links before the tooltip fades away.
    ),
  m_btn_instance_class(this, "[i]|c", SkConsole::ms_console_p->get_ini_font_code_narrow()),
//  m_btn_ic_tip(&m_btn_instance_class,         $Revisit: JStenersen wasn't working, commented out so as not imply is working.
//    "[i]nstance or [c]lass member toggle."),
  m_scope(SkScope_instance),
  m_overlays(this),
  m_class_width(0),
  m_overlay_x(0)
  {
  hide();
  set_border(Border_raised);

  m_tooltip.set_header("New Class/Member", IDI_SKOOKUM);
  m_tooltip.set_link("http://www.skookumscript.com/docs/");
  m_tooltip.set_link_funct(7, AToolTipOS::on_link_disable);

  AVec2i parent_carea(parent_p->get_area_client());

  int spacing      = get_spacing();
  int spacing_half = spacing / 2;
  int row_height   = m_overlays.get_height();

  m_btn_close.set_on_pressed_func(new AMethod<SkCreatePopup>(this, &SkCreatePopup::toggle_display));
  m_btn_close.resize();
  m_btn_close.set_position(parent_carea.m_x - m_btn_close.get_width(), 0);
  m_btn_close.enable_subclass_messages();
  //m_btn_close.set_color_background(AColor::ms_void);  // pro (0.18f, 0.18f, 0.2f) blue web bg AColor(0.11f, 0.13f, 0.19f)
  m_btn_close.show();

  // Height can be calculated after close button positioned.
  calc_height();

  // Setup device context (DC) drawing properties - info is retained since it has its own
  // private DC.
  HDC hdc = ::GetDC(m_os_handle);

  ::SelectObject(hdc, ((ATrueTypeFont *)m_font.m_sys_font_p)->m_font_handle_p);
  ::SetBkColor(hdc, ::GetSysColor(COLOR_3DFACE));
  ::SetBkMode(hdc, TRANSPARENT);  // TRANSPARENT OPAQUE
  ::ReleaseDC(m_os_handle, hdc);
  set_color_background(g_color_bg);  // blue web bg AColor(0.11f, 0.13f, 0.19f)

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Line

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Member Line

  // Class Text
  m_class_width = AFont::ms_header2_p->get_width(m_parent_class_name);

  m_create_text.set_on_modified_func(new AMethod<SkCreatePopup>(this, &SkCreatePopup::on_create_text_modified));
  m_create_text.set_border(Border_sunken);
  m_create_text.set_color_background(g_color_text_edit_bg);
  m_create_text.set_text_background(g_color_text_edit_bg);
  m_create_text.syntax_highlight();
  m_create_text.set_region(
    spacing + m_class_width + spacing_half,
    m_line2_y,
    parent_carea.m_x - (spacing + m_class_width + spacing_half + spacing),
    m_create_text.get_height_line_auto());
  m_create_text.show();

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Overlay Line
  int y = m_line2_y + row_height;

  m_btn_instance_class.set_on_pressed_func(new AMethod<SkCreatePopup>(this, &SkCreatePopup::toggle_scope));
  m_btn_instance_class.set_position(spacing, y);
  m_btn_instance_class.resize();
  m_btn_instance_class.set_height(row_height);
  m_btn_instance_class.enable_input(false);
  m_btn_instance_class.show();

  m_overlay_x = m_btn_instance_class.get_right_rel_spaced() + m_font.get_width("Overlay:") + spacing_half;
  m_overlays.set_position(m_overlay_x, y);
  m_overlays.show();
  }


//---------------------------------------------------------------------------------------
//  Destructor
//  Author(s):    John Stenersen
SkCreatePopup::~SkCreatePopup()
  {
  SkConsole::ms_console_p->get_ini_ide().set_value_bool(m_tooltip.is_enabled(), g_ini_tooltip_enable_create_popup_p, g_ini_section_tooltip_p);
  }


//---------------------------------------------------------------------------------------
// Gets ready for use to create something
// 
// #Author(s) Conan Reis
void SkCreatePopup::display()
  {
  if (is_hidden())
    {
    populate();
    update();
    show();
    }

  m_create_text.set_focus();
  m_create_text.select_all();
  }

//---------------------------------------------------------------------------------------
// Gets ready for use to create something
// 
// #Author(s) Conan Reis
void SkCreatePopup::populate()
  {
  m_overlays.populate();
  }

//---------------------------------------------------------------------------------------
// Determines location for line2 underneath description used to describe creation action
// 
// #Notes
//   m_description should be set and m_btn_close should already be appropriately placed
//   since its position is used in the calculation.
//   
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // true if y position changed, false if not
  bool
SkCreatePopup::calc_height()
  {
  if (m_browser_p->is_minimized())
    {
    return false;
    }

  int spacing = get_spacing();
  int spacing_half = spacing / 2;

  // Determine height of description which may span multiple lines
  RECT tarea = {spacing, spacing_half, m_btn_close.get_x_rel() - spacing, 0};
  HDC  hdc   = ::GetDC(m_os_handle);

  ::DrawText(hdc, m_description.as_cstr(), m_description.get_length(), &tarea, DT_CALCRECT | DT_LEFT | DT_TOP | DT_NOPREFIX | DT_NOCLIP | DT_WORDBREAK);
  ::ReleaseDC(m_os_handle, hdc);
    
  int line2_y = tarea.bottom + spacing;

  if (m_line2_y == line2_y)
    {
    // No change
    return false;
    }

  // height changed
  m_line2_y = line2_y;

  // Resize to new height
  int row_height = m_overlays.get_height();
  int height     = m_line2_y + row_height + spacing_half + row_height + spacing;

  static_cast<SkNavigationView *>(m_parent_p)->resize_as_needed(!is_hidden(), height);

  return true;
  }

//---------------------------------------------------------------------------------------
// Sets class to append to
// 
// #Author(s) Conan Reis
void SkCreatePopup::set_class(SkClass * current_class_p)
  {
  if (current_class_p != m_parent_class_p)
    {
    // Class Changed
    m_parent_class_p = current_class_p;

    if (m_parent_class_p)
      {
      m_parent_class_name = current_class_p->get_name_str();
      m_parent_class_name.append('.');
      }
    else
      {
      m_parent_class_name.set_cstr("???.", 3u);
      }

    m_class_width = AFont::ms_header2_p->get_width(m_parent_class_name);

    AVec2i carea(get_area_client());

    int spacing       = get_spacing();
    int spacing_half  = spacing / 2;
    int create_text_x = spacing + (m_display_class_b ? (m_class_width + spacing_half) : 0);
    
    m_create_text.set_region(
      create_text_x,
      m_line2_y,
      carea.m_x - (create_text_x + spacing),
      m_create_text.get_height());
    }
  }

//---------------------------------------------------------------------------------------
// Specify whether instance or class scope is used.
// 
// #Author(s) Conan Reis
void SkCreatePopup::set_scope(eSkScope scope)
  {
  if (scope != m_scope)
    {
    m_scope = scope;
    m_btn_instance_class.set_text((scope == SkScope_instance) ? "[i]|c" : "i|[c]");
    update();
    }
  }

//---------------------------------------------------------------------------------------
// Toggle between instance and class scope
// 
// #Author(s) Conan Reis
void SkCreatePopup::toggle_scope()
  {
  set_scope((m_scope == SkScope_instance) ? SkScope_class : SkScope_instance);
  }

//---------------------------------------------------------------------------------------
// Toggle display of this create pop-up
// 
// #Author(s) Conan Reis
void SkCreatePopup::toggle_display()
  {
  static_cast<SkNavigationView *>(m_parent_p)->toggle_create_popup();
  }

//---------------------------------------------------------------------------------------
// Refreshes / redraws pop-up
// 
// #Author(s) Conan Reis
void SkCreatePopup::update()
  {
  SkClass * class_p = static_cast<SkNavigationView *>(m_parent_p)->get_class_tree().get_selected_class();
  eSkScope  create_scope = m_scope;
  bool      instance_class_toggle_b = false;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Examine create text
  SkParser create_str(m_create_text.get_text());

  m_create_type = SkMember__invalid;
  m_source_type = SourceType_template;
  m_source_overlay_p = nullptr;
  m_source_routine_p = nullptr;
  m_display_class_b = true;

  create_str.crop();

  uint32_t create_length = create_str.get_length();

  if (create_length)
    {
    const char * create_cstr_a = create_str.as_cstr();
    char         ch = create_cstr_a[0];
    uint32_t     idx = 0u;

    m_description.ensure_size_empty(128u);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Look for optional class scope/superclass to use instead of class from class tree
    AString owner_class_str;
    bool    class_specified_b = false;
    bool    scope_op_b = false;

    SkParser::eIdentify class_kind = SkParser::Identify_lexical_error;

    if (AString::ms_is_uppercase[uint8_t(ch)])
      {
      SkClass * class_scope_p = nullptr;

      class_kind = create_str.identify_class(0, &idx, &m_create_name, &class_scope_p);

      ch = create_cstr_a[idx];

      switch (ch)
        {
        case '@':
          // Scope resolution operator for everything but data members
          scope_op_b = true;
          // Intentional fall through to '.' case

        case '.':
          // Member access operator

          // New class scope specified
          class_specified_b = true;
          idx++;
          ch = create_cstr_a[idx];
          class_p = class_scope_p;
          owner_class_str = m_create_name;
          // Class used for scope - so not using for creation name
          class_kind = SkParser::Identify_lexical_error;
          break;
        }
      }

    if (class_specified_b)
      {
      m_display_class_b = false;
      }
    else
      {
      owner_class_str = class_p ? class_p->get_name_str() : "???";
      ch = create_cstr_a[0];
      }

    // Ensure existing class scope is set
    if (class_p == nullptr)
      {
      m_create_type = SkMember__error;
      m_description.append("Owner / superclass '");
      m_description.append(owner_class_str);
      m_description.append("' does not exist! Either a typo or it needs to be created first.");
      }
    else
      {
      // Disallow classes to be derived from "Class" and "None"
      if ((class_p == SkBrain::ms_class_class_p)
        || (class_p == SkNone::get_class()))
        {
        m_create_type = SkMember__error;
        m_description.append("The classes 'Class' and 'None' may not have subclasses!");
        }
      }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Look for member identifier
    if (m_create_type != SkMember__error)
      {
      uint32_t name_start = idx;

      SkParser::eResult result = SkParser::Result_ok;

      m_create_type = SkMember__error;

      switch (ch)
        {
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        case '_':
          // Assume it is a coroutine
          m_description.append("Coroutine ");

          result = create_str.parse_name_coroutine(name_start, &idx);

          if (result == SkParser::Result_ok)
            {
            m_create_type = SkMember_coroutine;
            create_str.get(&m_create_name, name_start, idx - name_start);
            m_description.append(owner_class_str);
            m_description.append('.');
            m_description.append(m_create_name);
            m_description.append("()");
            }
          break;

        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        case '@':
          {
          // Assume it is a data member
          if (scope_op_b)
            {
            m_description.append("Use . after scope for data member");
            break;
            }

          bool class_member_b = false;

          result = create_str.parse_name_data_member(name_start, &idx, nullptr, nullptr, &class_member_b);

          if (result == SkParser::Result_ok)
            {
            m_create_type = SkMember_data;
            create_scope = class_member_b ? SkScope_class : SkScope_instance;
            create_str.get(&m_create_name, name_start, idx - name_start);
            m_description.append(class_member_b ? "Class data member " : "Instance data member ");
            m_description.append(owner_class_str);
            m_description.append('.');
            m_description.append(m_create_name);
            }
          else
            {
            m_description.append("Data member ");
            }
          break;
          }

        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        case '!':
          // Assume it is a constructor or destructor
          instance_class_toggle_b = true;
          m_description.append((create_scope == SkScope_instance)
            ? "Instance "
            : "Class ");

          switch (create_cstr_a[idx + 1u])
            {
            case '!':
              m_description.append("destructor ");
              break;

            case '\0':
              m_description.append("default constructor ");
              break;

            default:
              m_description.append("named constructor ");
              break;
            }

          result = create_str.parse_name_method(name_start, &idx);

          if (result == SkParser::Result_ok)
            {
            m_create_type = SkMember_method;
            create_str.get(&m_create_name, name_start, idx - name_start);
            m_description.append(owner_class_str);
            m_description.append('.');
            m_description.append(m_create_name);
            m_description.append("()");
            }
          break;
      
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        case '\0':
          if (class_specified_b)
            {
            m_create_type = SkMember__invalid;
            m_description.append("Now using '");
            m_description.append(owner_class_str);
            m_description.append("' as superclass / owner");
            }
          break;

        default:
          if (AString::ms_is_lowercase[uint8_t(ch)])
            {
            //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            // Assume it is a method
            instance_class_toggle_b = true;
            m_description.append((create_scope == SkScope_instance)
              ? "Instance method "
              : "Class method ");

            result = create_str.parse_name_method(name_start, &idx);

            if (result == SkParser::Result_ok)
              {
              m_create_type = SkMember_method;
              create_str.get(&m_create_name, name_start, idx - name_start);
              m_description.append(owner_class_str);
              m_description.append('.');
              m_description.append(m_create_name);
              m_description.append("()");

              // $Revisit - CReis else determine if it is an operator
              }
            }
          else if (AString::ms_is_uppercase[uint8_t(ch)])
            {
            //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            // Could be class (if new) or conversion method (if existing)

            // Determine if class name already parsed (when looking for optional scope)
            if (class_kind == SkParser::Identify_lexical_error)
              {
              class_kind = create_str.identify_class(idx, &idx, &m_create_name);
              }

            if (class_kind == SkParser::Identify_class)
              {
              // Existing Class - assume it is a conversion method
              m_create_type = SkMember_method;
              m_description.append("Conversion method ");
              m_description.append(owner_class_str);
              m_description.append('.');
              m_description.append(m_create_name);
              m_description.append("()");
              }
            else
              {
              // Non-existing class - assume new class to be created
              m_create_type = SkMember_class_meta;
              m_description.append("Class ");
              m_description.append(m_create_name);
              m_description.append(" as subclass of ");
              m_description.append(owner_class_str);
              }
            }
          else
            {
            // $Revisit - CReis else determine if it is an operator symbol
            m_create_type = SkMember__invalid;
            }
        } // switch identifier

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Tack on any error info
      if (result != SkParser::Result_ok)
        {
        m_description.append("\nError: ");
        m_description.append(SkParser::get_result_string(result));
        }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Tack on common hierarchy info
      SkInvokableBase * override_routine_p = nullptr;
      SkInvokableBase * overridden_routine_p = nullptr;
      SkClass *         superclass_p = class_p->get_superclass();
      ASymbol           create_id = ASymbol::create_existing(m_create_name);
      bool              member_dupe_b = false; // If dupe determine if replace on different overlay or already exists on same overlay

      switch (m_create_type)
        {
        case SkMember_method:
          if (!create_id.is_null())
            {
            // Tack on operator name if it is an operator
            ASymbol op_sym = SkParser::method_to_operator(create_id);

            if (!op_sym.is_null())
              {
              m_description.append(" [", 2u);
              m_description.append(op_sym.as_string());
              m_description.append("] ", 2u);
              }

            if (superclass_p)
              {
              // Determine if overriding from superclass
              override_routine_p = (create_scope == SkScope_instance)
                ? superclass_p->find_instance_method_inherited(create_id)
                : superclass_p->find_class_method_inherited(create_id);
              }

            // Determine if it will be overridden by any subclasses
            overridden_routine_p = (create_scope == SkScope_instance)
              ? class_p->find_instance_method_overridden(create_id)
              : class_p->find_class_method_overridden(create_id);

            // Determine if member already exists (in creation overlay or a different overlay)
            member_dupe_b = (create_scope == SkScope_instance)
              ? class_p->is_instance_method_valid(create_id)
              : class_p->is_class_method_valid(create_id);

            // $Revisit - CReis Should look at both class and instance methods for dupes
            // regardless of its create scope
            }
          break;

        case SkMember_coroutine:
          if (!create_id.is_null())
            {
            if (superclass_p)
              {
              // Determine if overriding from superclass
              override_routine_p = superclass_p->find_coroutine_inherited(create_id);
              }

            // Determine if it will be overridden by any subclasses
            overridden_routine_p = class_p->find_coroutine_overridden(create_id);

            // Determine if member already exists (in creation overlay or a different overlay)
            member_dupe_b = class_p->is_coroutine_valid(create_id);
            }
          break;

        case SkMember_data:
          // Determine if data member is a dupe
          if (!create_id.is_null())
            {
            SkClass * dupe_class_p = (create_scope == SkScope_instance)
              ? class_p->find_instance_data_scope(create_id)
              : class_p->find_class_data_scope(create_id);

            // $Revisit - CReis Should look at both class and instance data for dupes
            // regardless of its create scope
             
            if (dupe_class_p)
              {
              // Duplicate data member was found
              m_create_type = SkMember__error;
              m_description.append("\nDuplicate - already present in ");

              if (dupe_class_p == class_p)
                {
                m_description.append("same class '");
                }
              else
                {
                if (dupe_class_p->is_superclass(*class_p))
                  {
                  m_description.append("superclass '");
                  }
                else
                  {
                  m_description.append("subclass(es) including '");
                  }
                }

              m_description.append(dupe_class_p->get_name_str());
              m_description.append("'!", 2u);
              }
            }

          if (m_create_type < SkMember__invalid)
            {
            set_scope(create_scope);
            }
          break;
        }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Overriding superclass member?

      // $Note - CReis For both overriding and being overridden:
      // Should probably use same interface or file for routine being created.
      // Could also make hot-link to bring up search box with superclass, subclass and
      // peer class methods with the same name so that one could be selected to copy.
      
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Overridden by subclass member?
      if (overridden_routine_p)
        {
        // Note any superclass routine will be preferred over any tracked subclass routine
        m_source_type = SourceType_subclass;
        m_source_routine_p = overridden_routine_p;
        m_description.append(" - overridden by subclass(es) including ");
        m_description.append(m_source_routine_p->get_scope()->get_name_str_dbg());
        }

      // Overriding superclass member?
      if (override_routine_p)
        {
        // Note this prefers superclass routine over any tracked subclass routine
        m_source_type = SourceType_superclass;
        m_source_routine_p = override_routine_p;
        m_description.append(" - overrides superclass ");
        m_description.append(override_routine_p->as_string_name());
        }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Member already exists?
      if (member_dupe_b)
        {
        if (m_create_type < SkMember__invalid)
          {
          SkMemberInfo member(SkQualifier(create_id, class_p), m_create_type, create_scope == SkScope_class);
          SkOverlay *  overlay_existing_p = SkCompiler::ms_compiler_p->find_member_file(member);

          m_description.append(" - member already exists in ");

          const char * overlay_seq_cstr_p    = nullptr;
          const char * overlay_action_cstr_p = nullptr;
          SkOverlay *  overlay_desired_p     = m_overlays.m_overlay_p;

          m_source_routine_p = nullptr;

          if (overlay_desired_p == overlay_existing_p)
            {
            overlay_seq_cstr_p    = "same '";
            overlay_action_cstr_p = "' overlay!";
            }
          else
            {
            m_source_type = SourceType_overlay;

            if (overlay_existing_p && (overlay_existing_p->m_sequence < overlay_desired_p->m_sequence))
              {
              m_source_overlay_p    = overlay_existing_p;
              overlay_seq_cstr_p    = "lower / earlier sequence '";
              overlay_action_cstr_p = "' overlay so this new member will be used instead!";
              }
            else
              {
              m_source_overlay_p    = overlay_existing_p;
              overlay_seq_cstr_p    = "higher / later sequence '";
              overlay_action_cstr_p = "' overlay so it will be used instead of this new member!";
              }
            }

          m_description.append(overlay_seq_cstr_p);
          m_description.append(overlay_existing_p->m_name);
          m_description.append(overlay_action_cstr_p);
          }
        else
          {
          m_description.append(" - member already exists!");
          }
        }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Consider any unused / trailing characters an error
      if ((m_create_type <= SkMember__invalid) && (idx < create_length))
        {
        m_create_type = SkMember__error;
        m_description.append(" [Error unknown trailing: \"");
        m_description.append(create_cstr_a + idx, create_length - idx);
        m_description.append("\"]");
        }
      }
    }
  else
    {
    m_description =
      "New member - 'Enter' to create:\n"
      "[Class.] NewClass, _coroutine, method[?], ![constructor], !!(destructor), ConvertClass, @instance_data[?], @@class_data[?]";
    }

  m_btn_instance_class.enable_input(instance_class_toggle_b);

  set_class(class_p);

  calc_height();
  refresh();
  }

//---------------------------------------------------------------------------------------
// Hides the window and deactivates it.
// 
// #Examples
//   window.hide()
// 
// #See Also
//   get_show_state(), is_maximized(), is_minimized(), maximize(), minimize(), restore(),
//   set_show_state(), show(), is_hidden()
//   
// #Author(s) Conan Reis
void SkCreatePopup::hide()
  {
  m_overlays.show_dowpdown(false);
  AWindow::hide();
  }

//---------------------------------------------------------------------------------------
// Called whenever the create text edit box is modified
// 
// Author(s):   Conan Reis
void SkCreatePopup::action_create()
  {
  switch (m_create_type)
    {
    case SkMember_method:
    case SkMember_coroutine:
      action_create_routine();
      break;

    case SkMember_data:
      action_create_data();
      break;

    case SkMember_class_meta:
      action_create_class();
      break;

    default:
      // Create info is invalid - do nothing other than give message.
      // Note that create pop-up is kept up.
      eSkDPrintType print_type = (m_create_type < SkMember__invalid)
        ? SkDPrintType_note
        : ((m_create_type == SkMember__invalid)
          ? SkDPrintType_warning
          : SkDPrintType_error);

      AString log_str;

      log_str.ensure_size_empty(m_description.get_length() + 4u);
      log_str.append("\n\n", 2u);
      log_str.append(m_description);
      log_str.append("\n\n", 2u);
      SkDebug::print_ide(log_str, SkLocale_ide, print_type);
      return;
    }

  // Ensure editor changes are saved
  m_browser_p->get_edit_view().save_changes();

  // Hide create pop-up
  toggle_display();
  }

//---------------------------------------------------------------------------------------
// Called called when creating a routine - method or coroutine.
// 
// #Author(s) Conan Reis
void SkCreatePopup::action_create_routine()
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Determine file that needs to be created / modified
  SkMemberInfo member(
    SkQualifier(ASymbol::create(m_create_name), m_parent_class_p),
    m_create_type,
    m_scope == SkScope_class);
  AString member_path(m_overlays.m_overlay_p->get_path_member(member));
  AFile   member_file(member_path);
  bool    file_exists_b = member_file.is_existing();

  AString       log_str;
  eSkDPrintType print_type = SkDPrintType_note;

  log_str.append("\n\n", 2u);

  if (file_exists_b)
    {
    print_type = SkDPrintType_title;
    log_str.append("Going to existing routine...\n  ");
    }
  else
    {
    log_str.append("Creating new routine...\n  ");
    }

  log_str.append(m_description);
  log_str.append("\n    File: ");
  log_str.append(member_path);
  log_str.append('\n');
  SkDebug::print_ide(log_str, SkLocale_ide, print_type);



  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // If already exists go to member and done
  if (file_exists_b)
    {
    m_browser_p->set_member(member);
    m_browser_p->focus_editor();

    return;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Display script command equivalent
  // $Revisit - Incomplete: create script command to create member


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Determine source to copy
  AFile source_file;

  // $Revisit - CReis Source could also be selected member [from copy or move, etc.]

  log_str.empty();

  switch (m_source_type)
    {
    case SourceType_overlay:
      {
      // - earlier overlay - to replace (full copy)
      // - later overlay - to be replaced by (full copy)
      source_file = m_source_overlay_p->get_path_member(member);

      log_str.append("  Copying existing routine from overlay '");
      log_str.append(m_source_overlay_p->m_name.as_cstr());
      log_str.append("':\n    File: ");
      break;
      }

    case SourceType_subclass:
    case SourceType_superclass:
      {
      // - superclass version being overridden (copy to params + no body or [Super@routine(...)])
      // - subclass version that will override (copy 1st found to params + no body)
      SkMemberInfo source_member(member);
      SkClass *    source_class_p = m_source_routine_p->get_scope();

      source_member.set_class(source_class_p);
      SkCompiler::ms_compiler_p->find_member_file(source_member, &source_file);

      log_str.append("  Copying existing routine from ");
      log_str.append((m_source_type == SourceType_superclass) ? "superclass '" : "subclass '");
      log_str.append(source_class_p->get_name_cstr_dbg());
      log_str.append("':\n    File: ");
      break;
      }

    //case SourceType_template:
    default:
      // - use default routine template (full copy + () [] or () Boolean [false])
      source_file.set_path_and_name(
        SkConsole::ms_console_p->get_script_template_dir(),
        (m_create_type == SkMember_coroutine)
          ? "_coroutine().sk"
          : "method().sk");
      log_str.append("  Copying from template:\n    ");
      break;
    }

  log_str.append(source_file.as_cstr());
  log_str.append('\n');
  SkDebug::print_ide(log_str, SkLocale_ide, print_type);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure source file exists
  if (!source_file.is_existing())
    {
    log_str.empty();
    log_str.append("  Error: Could not find source file to copy from!\n    File: ");
    log_str.append(source_file.as_cstr());
    log_str.append("\n\n");
    SkDebug::print_ide(log_str, SkLocale_ide, SkDPrintType_error);

    return;
    }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // $Revisit - Consider move / copy !Class.sk-meta if first time a member added to this
  // class in this overlay?


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Create / copy new routine script file
  SkParser script(source_file.map_string());

  switch (m_source_type)
    {
    case SourceType_overlay:
      // - earlier overlay - to replace (full copy)
      // - later overlay - to be replaced by (full copy)
      // Full copy so no work needed.
      break;

    case SourceType_subclass:
    case SourceType_superclass:
      {
      // - superclass version being overridden (copy to params + no body or [Super@routine(...)])
      // - subclass version that will override (copy 1st found to params + no body)
      SkParser::Args args(0u, SkParser::ArgFlag__default_no_struct);

      // Include initial comments and whitespace + parameters
      if (m_create_type == SkMember_method)
        {
        script.preparse_method_source(member.m_member_id, member.get_class(), args);
        }
      else
        {
        script.preparse_coroutine_source(member.m_member_id, member.get_class(), args);
        }


      // If found end of parameters then truncate to that point
      if (args.is_ok())
        {
        script.set_length(args.m_end_pos);
        script.append(
          "\r\n"
          "\r\n"
          "  // Replace with custom code.\r\n");

        if (m_source_type == SourceType_superclass)
          {
          script.append("  // May want to call superclass version of this routine.\r\n");
          }

        script.append("\r\n");
        }

      break;
      }

    //case SourceType_template:
    default:
      {
      // - use default routine template (full copy + () [] or () Boolean [false])
      AString result_class_str;

      if (m_create_name.get_last() == '?')
        {
        // Predicate method
        result_class_str = "Boolean";
        }
      else
        {
        if (AString::ms_is_uppercase[uint8_t(m_create_name.get_first())])
          {
          // Conversion method
          result_class_str = m_create_name;
          }
        }

      // $Revisit - CReis Using DOS line endings for now - should be more sophisticated
      script.append("\r\n() ");
      script.append(result_class_str);
      script.append(
          "\r\n"
          "\r\n"
          "  // Replace with custom code body or leave blank for C++ routine.\r\n"
          "\r\n");
      break;
      }
    }

  member_file.write_text(script);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Update class hierarchy by parsing etc.
  SkParser::Args args;

  if (m_create_type == SkMember_method)
    {
    script.preparse_method_source(member.m_member_id, member.get_class_scope(), args);
    }
  else
    {
    script.preparse_coroutine_source(member.m_member_id, member.get_class_scope(), args);
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Update editor and focus on routine
  SkCodeList & routine_list = m_browser_p->get_routine_list();

  routine_list.append_member(member);
  routine_list.sort();
  routine_list.columns_set_width();
  m_browser_p->set_member(member);
  m_browser_p->focus_editor();


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Add to version control as needed
  log_str.empty();

  if (SkConsole::ms_console_p->get_version_control_system() == SkVersionControl_p4)
    {
    log_str.append("  Adding routine script file to version control...\n");
    SkDebug::print_ide(log_str, SkLocale_ide, print_type);
    member_file.p4_add();
    }
  else
    {
    log_str.append("  You may need to add the routine script file to version control.\n");
    SkDebug::print_ide(log_str, SkLocale_ide, SkDPrintType_warning);
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Notify about still needing compile and remote update
  log_str.empty();
  log_str.append(
    "  The new routine will need to be completed and compiled before it can be used.\n"
    "  Likewise the compiled binary has not been updated yet.\n");

  if (SkConsole::ms_console_p->is_remote_update_enabled())
    {
    log_str.append("  The remote runtime will be updated when the ");
    log_str.append(member.get_class()->get_name_str_dbg());
    log_str.append(" class is recompiled.");
    }

  log_str.append("\n\n", 2u);
  SkDebug::print_ide(log_str, SkLocale_ide, SkDPrintType_warning);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // $Revisit - Incomplete: Register with undo / redo system
  }

//---------------------------------------------------------------------------------------
// Called called when creating a data member.
// 
// #Author(s) Conan Reis
void SkCreatePopup::action_create_data()
  {
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Determine !Data file that needs to be created / modified
  SkMemberInfo  member(
    SkQualifier(ASymbol::create(m_create_name), m_parent_class_p),
    m_create_type,
    m_scope == SkScope_class);
  AString member_path(m_overlays.m_overlay_p->get_path_member(member));
  AFile   member_file(member_path);

  bool member_exists_b = (member.m_class_scope
    ? m_parent_class_p->find_instance_data_scope(member.m_member_id)
    : m_parent_class_p->find_class_data_scope(member.m_member_id)) != nullptr;

  AString       log_str;
  eSkDPrintType print_type = SkDPrintType_note;

  log_str.append("\n\n", 2u);

  if (member_exists_b)
    {
    // Data Member already exists
    print_type = SkDPrintType_title;

    log_str.append("Going to existing data member...\n  ");
    }
  else
    {
    log_str.append("Adding new data member...\n  ");
    }

  log_str.append(m_description);
  log_str.append("\n    File: ");
  log_str.append(member_path);
  log_str.append('\n');
  SkDebug::print_ide(log_str, SkLocale_ide, print_type);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // If already exists go to member and done
  if (member_exists_b)
    {
    m_browser_p->set_member(member);
    m_browser_p->focus_editor();

    return;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Display script command equivalent
  // $Revisit - Incomplete: create script command to create member


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Load existing !Data file if present
  AString script;
  bool    file_exists_b = member_file.is_existing();

  if (file_exists_b)
    {
    script = member_file.map_string();
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Append data member
 script.ensure_size_extra(2u + 24u + 2u + m_create_name.get_length());

  if (file_exists_b)
    {
    member_file.close();
    script.append("\r\n", 2u);
    }

  // Note comment index position
  uint       comment_idx = script.get_length() + 3u;
  const uint comment_hint_length = 18u;  // Length of "[Class] !data-name"
   
  script.append("// [Class] !data-name\r\n!", 24u);
  script.append(m_create_name);
  script.append("\r\n", 2u);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Make writable / check out from version control as needed
  if (file_exists_b && !member_file.is_read_only())
    {
    log_str.empty();

    if (SkConsole::ms_console_p->get_version_control_system() == SkVersionControl_p4)
      {
      log_str.append("  Checking out data member script file from version control...");
      SkDebug::print_ide(log_str, SkLocale_ide, print_type);

      if (!member_file.p4_checkout(true))
        {
        log_str.empty();
        log_str.append(" timed out!\nRetry adding data member.\n\n");
        SkDebug::print_ide(log_str, SkLocale_ide, SkDPrintType_warning);

        return;
        }
      }
    else
      {
      log_str.append("  Making data member script file writable...");
      SkDebug::print_ide(log_str, SkLocale_ide, print_type);
      member_file.enable_read_only(false);
      }

    log_str.empty();
    log_str.append(" done\n");
    SkDebug::print_ide(log_str, SkLocale_ide, print_type);
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Create / append to script file
  member_file.write_text(script);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Update class and add data member
  SkClass * initial_class_p = (m_create_name.get_last() == '?')
    ? SkBrain::ms_boolean_class_p
    : SkBrain::ms_object_class_p;

  member.get_class_scope()->append_data_member(member.m_member_id, initial_class_p);

  // $Vital - CReis Iterate through all known instances and add data member to help keep
  // stability for existing objects.


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Update editor and focus on data member
  SkDataList & data_list = m_browser_p->get_data_list();

  data_list.set_class(m_parent_class_p);
  data_list.columns_set_width();

  m_browser_p->set_member(
    member,
    comment_idx,
    comment_idx + comment_hint_length);
  m_browser_p->focus_editor();


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Add to version control as needed
  if (!file_exists_b)
    {
    if (SkConsole::ms_console_p->get_version_control_system() == SkVersionControl_p4)
      {
      log_str.empty();
      log_str.append("  Adding data member script file to version control...\n");
      SkDebug::print_ide(log_str, SkLocale_ide, print_type);
      member_file.p4_add();
      }
    else
      {
      log_str.append("  You may need to add the data member script file to version control.\n");
      SkDebug::print_ide(log_str, SkLocale_ide, SkDPrintType_warning);
      }
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Notify about still needing compile and remote update
  log_str.empty();
  log_str.append(
    "  The new data member will need to be completed - specifying a class type as needed\n"
    "  Type !@data - and compiled before it can be used.\n"
    "  Likewise the compiled binary has not been updated yet.\n");

  if (SkConsole::ms_console_p->is_remote_update_enabled())
    {
    log_str.append("  The remote runtime will be updated when the ");
    log_str.append(member.get_class()->get_name_str_dbg());
    log_str.append(" class is recompiled.");
    }

  log_str.append("\n\n", 2u);
  SkDebug::print_ide(log_str, SkLocale_ide, SkDPrintType_warning);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // $Revisit - Incomplete: Register with undo / redo system
  }

//---------------------------------------------------------------------------------------
// Called called when creating a class.
// 
// #Author(s) Conan Reis
void SkCreatePopup::action_create_class()
  {
  ASymbol   class_name(ASymbol::create(m_create_name));
  SkClass * class_p = SkBrain::get_class(class_name);

  AString       log_str;
  eSkDPrintType print_type = SkDPrintType_note;

  log_str.append("\n\n", 2u);

  if (class_p)
    {
    print_type = SkDPrintType_title;
    log_str.append("Going to existing class...\n  ");
    }
  else
    {
    log_str.append("Creating new class...\n  ");
    }

  log_str.append(m_description);
  log_str.append('\n');
  SkDebug::print_ide(log_str, SkLocale_ide, print_type);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // If already exists go to it and done
  if (class_p)
    {
    m_browser_p->set_class(class_p);

    return;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Display script command equivalent
  // $Revisit - Incomplete: create script command to create class
  
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Add class to hierarchy
  class_p = SkBrain::create_class(class_name, m_parent_class_p);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Set class meta file template to copy
  AFile class_meta_file(
    m_overlays.m_overlay_p->get_path_class(*class_p),
    "!Class.sk-meta");
  AFile source_file(
    SkConsole::ms_console_p->get_script_template_dir(),
    "!Class.sk-meta");

  log_str.empty();
  log_str.append(
    "  Creating class meta info file:\n"
    "    File: ");
  log_str.append(class_meta_file.as_string());
  log_str.append(
    "\n"
    "  Copying class meta info file from template:\n"
    "    File: ");
  log_str.append(source_file.as_string());
  log_str.append('\n');
  SkDebug::print_ide(log_str, SkLocale_ide, SkDPrintType_note);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure source file exists
  if (!source_file.is_existing())
    {
    log_str.empty();
    log_str.append("  Error: Could not find template file to copy from!\n    File: ");
    log_str.append(source_file.as_cstr());
    log_str.append("\n\n");
    SkDebug::print_ide(log_str, SkLocale_ide, SkDPrintType_error);

    return;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Create new class meta file
  source_file.copy(class_meta_file);
  class_meta_file.enable_read_only(false);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Update editor and focus on class meta file in editor
  m_browser_p->get_class_tree().append_class(*class_p);
  m_browser_p->on_class_selected(class_p);
  m_browser_p->focus_editor();


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Add to version control as needed
  log_str.empty();

  if (SkConsole::ms_console_p->get_version_control_system() == SkVersionControl_p4)
    {
    log_str.append("  Adding class meta file to version control...\n");
    SkDebug::print_ide(log_str, SkLocale_ide, SkDPrintType_note);
    class_meta_file.p4_add();
    }
  else
    {
    log_str.append("  You may need to add the class meta file to version control.\n");
    SkDebug::print_ide(log_str, SkLocale_ide, SkDPrintType_warning);
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Notify about still needing compile and remote update
  log_str.empty();
  log_str.append(
    "  The new class will need members (methods, coroutines, data, etc.) and need to be\n"
    "  compiled before it can be used.\n"
    "  Likewise the compiled binary has not been updated yet.\n");

  if (SkConsole::ms_console_p->is_remote_update_enabled())
    {
    //log_str.append("  The remote runtime will be updated when the ");
    //log_str.append(class_p->get_name_str_dbg());
    //log_str.append(" class is recompiled.");
     
    // $Vital - CReis New classes need to be sent/updated to the remote runtime!
    log_str.empty();
    log_str.append(
      "  New classes are not *currently* sent/updated to the remote runtime!\n"
      "  The runtime must be restarted to use the new class.\n");
    }

  SkDebug::print_ide(log_str, SkLocale_ide, SkDPrintType_warning);

  log_str.empty();
  log_str.append("\n\n", 2u);
  SkDebug::print_ide(log_str, SkLocale_ide);


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // $Revisit - Incomplete: Register with undo / redo system
  }

//---------------------------------------------------------------------------------------
// Called whenever the create text edit box is modified
// 
// #Author(s) Conan Reis
void SkCreatePopup::on_create_text_modified()
  {
  m_create_text.syntax_highlight(SkEditSyntax::Coverage_visible);
  update();
  }

//---------------------------------------------------------------------------------------
//  Called whenever a window's client size is changing.  Usually this is
//              associated with a user dragging a window's sizing border.
// Examples:    called by the system
// See:         on_size(), on_sized()
// Notes:       This applies to the window's client area size changing and not
//              necessarily the outer edge of the window.
// Author(s):    Conan Reis
void SkCreatePopup::on_sizing()
  {
  if (m_class_width)
    {
    AVec2i carea(get_area_client());
    int    spacing = get_spacing();
    int    spacing_half = spacing / 2;
    int    row_height = m_overlays.get_height();

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Line

    m_btn_close.set_position(carea.m_x - m_btn_close.get_width(), 0);

    if (calc_height())
      {
      // Height was changed so on_sizing() was called again.
      return;
      }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Member Line
    int y             = m_line2_y;
    int create_text_x = spacing + (m_display_class_b ? (m_class_width + spacing_half) : 0);
    
    m_create_text.set_region(
      create_text_x,
      m_line2_y,
      carea.m_x - (create_text_x + spacing),
      m_create_text.get_height());

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Overlay Line
    y += row_height;

    m_btn_instance_class.set_position(spacing, y);

    m_overlays.set_region(
      m_overlay_x,
      y,
      carea.m_x - m_overlay_x - spacing,
      row_height);
    }
  }

//---------------------------------------------------------------------------------------
// Called when the window client area is to be drawn.
// 
// #Modifiers   virtual - Overridden from AgogGUI\AWindow.
// #Author(s)   Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Boolean indicating whether the default Windows procedure with respect to the given
  // message should be called (true) or not (false)
  bool
SkCreatePopup::on_draw()
  {
  PAINTSTRUCT ps;

  HDC hdc          = ::BeginPaint(m_os_handle, &ps);
  int spacing      = get_spacing();
  int spacing_half = spacing / 2;
  int row_height   = m_overlays.get_height();

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Line
  RECT tarea = {spacing, spacing_half, m_btn_close.get_x_rel() - spacing, m_line2_y};

  switch (m_create_type)
    {
    case SkMember__invalid:
      if (m_create_text.is_empty())
        {
        // No create text entered yet - display instructions
        ::SetTextColor(hdc, AColor(1.0f, 0.87f, 0.0f));
        }
      else
        {
        // Text in transitory invalid state
        ::SetTextColor(hdc, AColor(1.0f,  1.0f, 0.5f));
        }
      break;

    case SkMember__error:
      ::SetTextColor(hdc, AColor(1.0f, 0.33f, 0.5f));
      break;

    default:
      // Good to go
      ::SetTextColor(hdc, AColor(0.77f, 1.0f, 0.78f));
      break;
    }
  ::DrawText(hdc, m_description.as_cstr(), m_description.get_length(), &tarea, DT_LEFT | DT_TOP | DT_NOPREFIX | DT_NOCLIP | DT_WORDBREAK);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Overlay Line
  ::SetTextColor(hdc, AColor::ms_white);
  ::ExtTextOut(
    hdc,
    m_btn_instance_class.get_right_rel_spaced(),
    m_line2_y + row_height + ((row_height - m_font.get_height()) / 2),
    0u, nullptr, "Overlay:", 8u, nullptr);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Member Line
  ::SetTextColor(hdc, AColor::ms_white);

  // Set header2 font
  ::SelectObject(hdc, ((ATrueTypeFont *)AFont::ms_header2_p->m_sys_font_p)->m_font_handle_p);

  int offset_y = (m_create_text.get_height() - AFont::ms_header2_p->get_height()) / 2;
  ::ExtTextOut(
    hdc, spacing, m_line2_y + offset_y, 0u, nullptr, m_parent_class_name, m_parent_class_name.get_length(), nullptr);

  // Put default font back
  ::SelectObject(hdc, ((ATrueTypeFont *)m_font.m_sys_font_p)->m_font_handle_p);

  ::EndPaint(m_os_handle, &ps);

  return true;
  }

//---------------------------------------------------------------------------------------
// Called whenever a key is pressed.
// 
// #Notes
//   Call AKeyboard::get_mod_keys() to determine if any modifier keys are in effect.
//   
// #Modifiers virtual - Override for custom behaviour.
// #See Also  on_character(), on_key_release()
// #Author(s) Conan Reis
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // true if default behaviour should be called or false if should consider key handled. 
  bool
SkCreatePopup::on_key_press(
  // key code corresponding to a physical key on the keyboard.
  // If Shift-'2' is pressed, AKey_shift is sent first and then AKey_2, not '@'.  Defines
  // for codes are prefixed with "AKey_" and are in AgogIO/AKeyboard.hpp
  // AKey_0 to AKey_9 are the same as ANSI '0' to '9' (0x30 - 0x39)
  // AKey_A to AKey_Z are the same as ANSI 'A' to 'Z' (0x41 - 0x5A)
  // Special characters like AKey_control are also possible.
  eAKey key,
  // true if this is a repeated send (holding down the key), false if this is the first
  // time key has been pressed.
  bool repeated
  )
  {
  //A_DPRINT(A_SOURCE_FUNC_STR "0x%p\n", this);

  eAKeyMod mod_keys = AKeyboard::get_mod_keys();

  switch (key)
    {
    case 'N':
      if (mod_keys == AKeyMod_ctrl)
        {
        if (!repeated)
          {
          toggle_display();
          }

        return false;
        }
      break;

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

    case AKey_return:
    case AKey_num_enter:
      if (mod_keys == AKeyMod_none)
        {
        if (!repeated)
          {
          action_create();
          }

        return false;
        }
      break;
    }

  return static_cast<SkNavigationView *>(m_parent_p)->on_key_press(key, repeated);
  } //  SkCreatePopup::on_key_press


//=======================================================================================
// SkNavigationView Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Constructor
// Author(s):   Conan Reis
SkNavigationView::SkNavigationView(
  AWindow *        parent_p,
  SkClassBrowser * browser_p
  ) :
  AWindow(AWindow::ms_region_def, parent_p),
  m_browser_p(browser_p),
  m_navigation_tabs(this, ARectEdge_left, *AFont::ms_header2_p),
  m_create_btn(&m_navigation_tabs, "+", SkConsole::ms_console_p->get_ini_font_code_narrow()),
  m_tree(&m_navigation_tabs, browser_p),
  m_create_popup(this, browser_p)
  {
  //set_color_background(g_color_bg);

  // ClassTree setup
  m_tree.show();

  // Navigation Tabs setup
  m_navigation_tabs.enable_control_event();
  //m_navigation_tabs.set_color_background(g_color_bg);
  m_navigation_tabs.append_win("Classes", &m_tree);
  m_navigation_tabs.show();

  m_create_btn.set_on_pressed_func(new AMethod<SkNavigationView>(this, &SkNavigationView::toggle_create_popup));
  m_create_btn.resize();
  AVec2i create_btn_area = m_create_btn.get_area();
  m_create_btn.set_position(
    (m_tree.get_x_rel() - create_btn_area.m_x) / 2, get_height_client() - create_btn_area.m_y);
  m_create_btn.show();
  }

//---------------------------------------------------------------------------------------
// Called after significant recompile.
// 
// #Author(s) Conan Reis
void SkNavigationView::rehook()
  {
  m_tree.build_tree();

  if (is_create_popup())
    {
    m_create_popup.populate();
    }
  }

//---------------------------------------------------------------------------------------
// Resize tabs and new pop-up as needed
// 
// #Author(s) Conan Reis
void SkNavigationView::resize_as_needed(
  bool show_create_popup,
  int create_popup_height // = ADef_int
  )
  {
  AVec2i carea(get_area_client());
  AVec2i old_tab_area(m_navigation_tabs.get_area());
  AVec2i new_tab_area(carea);

  if (show_create_popup || (create_popup_height != ADef_int))
    {
    create_popup_height = (create_popup_height == ADef_int) ? m_create_popup.get_height() : create_popup_height;
    ARegion create_rgn(0, carea.m_y - create_popup_height, carea.m_x, create_popup_height);

    m_create_popup.set_region(create_rgn);

    if (show_create_popup)
      {
      new_tab_area.m_y = carea.m_y - create_popup_height;
      }
    }

  if (new_tab_area != old_tab_area)
    {
    m_navigation_tabs.set_area(new_tab_area);
    }
  }

//---------------------------------------------------------------------------------------
// Toggles show/hide of "new" pop-up
// 
// #Author(s) Conan Reis
void SkNavigationView::show_create_popup()
  {
  if (m_create_popup.is_hidden())
    {
    if (!SkConsole::ms_console_p->verify_project_editable())
      {
      return;
      }

    // Make sure that we have editable overlays before we proceed
    if (!SkCompiler::ms_compiler_p->have_any_editable_overlays())
      {
      ADialogOS::info(
        "Cannot create any members since there is no editable overlay available in this project. All overlays are marked as non-editable.",
        "Can't create members - no editable overlay",
        ADialogOS::Flag_none,
        ADialogOS::Icon_warning);
      return;
      }

    m_create_btn.hide();
    resize_as_needed(true);
    }

  m_create_popup.display();
  }

//---------------------------------------------------------------------------------------
// Toggles show/hide of "new" pop-up
// 
// #Author(s) Conan Reis
void SkNavigationView::toggle_create_popup()
  {
  if (is_create_popup())
    {
    // Hide create pop-up and resize
    m_create_popup.hide();
    resize_as_needed(false);
    m_create_btn.show();

    return;
    }

  // Show create pop-up and resize
  show_create_popup();
  }

//---------------------------------------------------------------------------------------
//  Called whenever a window's client size is changing.  Usually this is
//              associated with a user dragging a window's sizing border.
// Examples:    called by the system
// See:         on_size(), on_sized()
// Notes:       This applies to the window's client area size changing and not
//              necessarily the outer edge of the window.
// Author(s):    Conan Reis
void SkNavigationView::on_sizing()
  {
  resize_as_needed(is_create_popup());

  AVec2i create_btn_area = m_create_btn.get_area();

  m_create_btn.set_position(
    (m_tree.get_x_rel() - create_btn_area.m_x) / 2, get_height_client() - create_btn_area.m_y);

  //refresh();
  }
