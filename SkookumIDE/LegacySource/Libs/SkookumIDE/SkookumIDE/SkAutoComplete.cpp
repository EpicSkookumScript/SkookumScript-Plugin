// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumIDE Application
//
// SkookumScript IDE Auto-Complete Class and supporting utilities.
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/ASymbolTable.hpp>
#include <SkookumIDE/SkAutoComplete.hpp>
#ifdef A_INL_IN_CPP
  #include <SkookumIDE/SkAutoComplete.inl>
#endif
#include <SkookumIDE/SkConsole.hpp>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <CommCtrl.h>


//=======================================================================================
// SkAutoComplete Class Static Members
//=======================================================================================

bool      SkAutoComplete::ms_active          = true;
uint32_t  SkAutoComplete::ms_key_length_min  = 1;


namespace
  {
  static const AColor g_color_text_bg(      0.15f, 0.15f, 0.19f);  // Pro Edit partial
  }


//=======================================================================================
// SkAutoComplete Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
//  Constructor - Creates the auto-complete listbox, initializes it for subclass message
//  processing, hidden to start with.
// 
//  Author(s):  John Stenersen
SkAutoComplete::SkAutoComplete(
  AWindow * parent_p
  ) :
  AWindow(parent_p, ms_default_class_p)
  {
//  A_DPRINT(A_SOURCE_STR "\n");

  m_os_handle = ::CreateWindowEx(
    WS_EX_CLIENTEDGE,
    WC_LISTBOX,                 // Extended Styles
    m_class_p->get_name(),      // Must use the name of the class for predefined classes
    WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_AUTOVSCROLL | LBS_NOTIFY,
    30, 30, 300, 200,           //  Resized later
    parent_p->get_os_handle(),  //  Parent window is either an SkWorkspace or SkEditor
    nullptr,                    //  Menu id - This object handles its own messages so an id is not necessary
    AApplication::ms_instance,
    nullptr);                   // l_param argument to the WM_CREATE message

  A_VERIFY_OS(m_os_handle != nullptr, "SkAutoComplete()", SkAutoComplete);

  common_setup();
  enable_subclass_messages();

  set_border(Border_line);
  set_font(parent_p->get_font());
  hide();
  cancel();
  } //  SkAutoComplete::SkAutoComplete()


//---------------------------------------------------------------------------------------
//  Destructor -  Cleans up any remaining choices and symbols.
//  
//  Author(s):  John Stenersen
SkAutoComplete::~SkAutoComplete()
  {
  // Some windows/controls need to call destroy() in their own destructor
  // rather than letting the AMessageTarget destructor call it since destroy()
  // will end up sending windows messages and the windows/controls need to have
  // their virtual table still intact.
  m_choices.free_all();
  m_symbols.free_all();
  destroy();
  }


//---------------------------------------------------------------------------------------
//  Calls the symbol less-than operator.
// 
//  Author(s):  John Stenersen
bool SkAutoCompleteSymbol::operator<(const SkAutoCompleteSymbol & symbol_rhs) const
  {
  return this->compare_sub(symbol_rhs) == AEquate_less;
  }


//---------------------------------------------------------------------------------------
//  Calls the symbol equals operator.
// 
//  Author(s):  John Stenersen
bool SkAutoCompleteSymbol::operator==(const SkAutoCompleteSymbol & symbol_rhs) const
  {
  return this->compare_sub(symbol_rhs) == AEquate_equal;
  }


//---------------------------------------------------------------------------------------
//  Determines whether the choice is "less than" the specified right-hand-side choice.
//  
//  The choice's weight has priority over the key and display. Thus, elements are ordered
//  by weight (importance/value), by key (what the user types) and finally by the display (what's
//  displayed in the listbox).
// 
//  Author(s):  John Stenersen
bool SkAutoCompleteChoice::operator<(const SkAutoCompleteChoice & choice_rhs) const
  {
  //  When looking for duplicates, the weight is not considered, only the key and display text.
  if (m_parent_p && m_parent_p->get_consider_weight())
    {
    if (this->m_weight < choice_rhs.m_weight)
      {
      return true;
      }
    if (this->m_weight > choice_rhs.m_weight)
      {
      return false;
      }
    }

  if (this->m_key < choice_rhs.m_key)
    {
    return true;
    }
  if ((this->m_key == choice_rhs.m_key) && (this->m_display < choice_rhs.m_display))
    {
    return true;
    }

  return false;
  }

    
//---------------------------------------------------------------------------------------
//  For a choice to be "equal" to another choice, the key and display text must be the
//  same - the weight is not a determinant.
// 
//  Author(s):  John Stenersen
bool SkAutoCompleteChoice::operator==(const SkAutoCompleteChoice & choice_rhs) const
  {
  return (this->m_key == choice_rhs.m_key) && (this->m_display == choice_rhs.m_display);
  }


//---------------------------------------------------------------------------------------
//  Create and insert a new choice into the auto-complete choice array and listbox. The ordering
//  is first by weight, then by key and finally by the displayed text.
//  
//  Arg:  key     - the string the user types
//  Arg:  display - the text displayed in the listbox
//  Arg:  replace - once accepted by the user, this string replaces the key string the user has typed
//  Arg:  weight  - a higher puts the choice higher up in the listbox
// 
//  Author(s):  John Stenersen
void SkAutoComplete::insert(const AString key, AString display, AString replace, float weight)
  {
  if (display.is_empty())
    {
    display = key;
    }
  if (replace.is_empty())
    {
    replace = key;
    }

  uint32_t find_pos;
  if (key.find(m_key, 1, &find_pos, 0, ALength_remainder, AStrCase_ignore) && (find_pos == 0))
    {
    SkAutoCompleteChoice *  choice = new SkAutoCompleteChoice;
    choice->m_key       = key;
    choice->m_display   = display;
    choice->m_replace   = replace;
    choice->m_weight    = weight;
    choice->m_parent_p  = this;

    //  Check to see if there's a duplicate already in the array. If so, remove it so the most
    //  recent choice insertion is made current - "last wins."
    uint32_t choice_find_pos;
    m_consider_weight_b = false;
    if (m_choices.find(*choice, 0, &choice_find_pos))
      {
      SendMessage(m_os_handle, LB_DELETESTRING, choice_find_pos, 0);
      m_choice_count--;
      m_choices.free(choice_find_pos);
      }

    uint32_t insert_pos;
    m_consider_weight_b = true;
    if (m_choices.append_absent(*choice, &insert_pos))
      {
      SendMessage(m_os_handle, LB_INSERTSTRING, insert_pos, (LPARAM)display.as_cstr());
      m_choice_count++;
      m_choice_length_max = max(m_choice_length_max, display.get_length());
      m_choice_width_max  = max(m_choice_width_max, (uint32_t)m_font.get_width(display));
      }
    }
  }


//---------------------------------------------------------------------------------------
//  This cancels the auto-complete feature and hides the listbox.
// 
//  Author(s):  John Stenersen
void SkAutoComplete::cancel()
  {
//  A_DPRINT(A_SOURCE_FUNC_STR "\n");

  m_key_start = ALength_remainder;
  m_key_end   = ALength_remainder;
  m_key       = "";

  if (!is_hidden())
    {
    hide();
    }
 }


//---------------------------------------------------------------------------------------
//  This routine is called when the user "accepts" the auto-complete choice (e.g. by pressing "enter").
//  The key, as typed by the user, is replaced by the display text and the listbox is then canceled.
// 
//  Author(s):  John Stenersen
void SkAutoComplete::accept()
  {
  //  Replace the string upto the end of the current identifier with the choice.
  SkEditSyntax *  editbox_p = static_cast<SkEditSyntax *>(m_parent_p);
  editbox_p->select(m_key_start, m_key_end);
  editbox_p->replace_selection(m_choices.get_at(m_choice_selection)->m_replace, true);

  //  TBD:  For snippet choices, perform a smarter replace where indentation is taken into account.

  cancel();
  }


//---------------------------------------------------------------------------------------
//  This routine is called as characters are typed or deleted. This listbox size has a
//  minimum dimension and a maximum number of choices that can be displayed before a
//  scrollbar is automatically applied to the listbox.
// 
//  Author(s):  John Stenersen
void SkAutoComplete::resize()
  {
  int font_height   = m_font.get_height();
  int adjust_width  = m_font.get_avg_width() + 4;

  if (m_choice_count > m_choice_display_max)
    {
    adjust_width += m_font.get_avg_width() * 2;   //  aka scrollbar width
    }

  set_area(m_choice_width_max + adjust_width, min(m_choice_count, m_choice_display_max) * font_height + 6);
  }


//---------------------------------------------------------------------------------------
//  This routine generates an alphabetically sorted version of the main symbol table.
// 
//  Returns:    Number of alphabetically sorted symbols.
//  
//  Author(s):  John Stenersen
void SkAutoComplete::symbols_populate()
  {
  //  Free any old symbols.
  m_symbols.free_all();

  if (ASymbolTable::ms_main_p && (ASymbolTable::ms_main_p->get_length() > 0))
    {
    SkAutoCompleteSymbol  * symbol_new;
    ASymbolRef            * symbol_ref;
    uint32_t table_length = ASymbolTable::ms_main_p->get_length();

    //  Pre-allocate the Auto-Complete alphabetically sorted symbol table.
    m_symbols.set_size(table_length);
    for (uint32_t i = 0; i < table_length; i++)
      {
      symbol_ref = ASymbolTable::ms_main_p->get_symbol_by_index(i);
      symbol_new = new SkAutoCompleteSymbol(AString(symbol_ref->m_str_ref_p));
      m_symbols.append_absent(*symbol_new);
      }
    }
  }


//---------------------------------------------------------------------------------------
//  This routine is called to populate the listbox (and choice array) with applicable auto-complete
//  choices. This is the "smarts" of the auto-complete feature.
//  
//  Auto-complete choices are added to the choice array and listbox from various sources/filters.
//  Weights are applied so the choices/suggestions are presented in a meaningful or best-selection order.
//  The best-selection order is rather subjective - the user having the option to choose a simply
//  alphabetically sorted list.
//
//  Returns:    Number of choices in the listbox.
//  
//  Author(s):  John Stenersen
uint32_t SkAutoComplete::populate()
  {
  //  Create the Auto-Complete alphabetically sorted symbol table.
  //  TBD: Only do this after a compile (aka each time the main symbol table changes -- or as each symbol is added to the main table.
  symbols_populate();

  //  Clear out the previous choices.
  SendMessage(m_os_handle, LB_RESETCONTENT, 0, 0);
  m_choice_count      = 0;
  m_choice_length_max = 4;
  m_choice_width_max  = m_font.get_avg_width() * m_choice_length_max;
  m_choices.free_all();

  //  TBD:  Consider when leading is character is capitalized, weight other capitalized symbols heavier.
  //  TBD:  If the character before the key is a dot '.' seek out the class and weigh its methods and data members higher.
  //  TBD:  Within a method or coroutine, apply a heavier weight for parameters.
  //  TBD:  Weigh temporary variables defined previously in the source higher than members from outer scope(s).
  //  TBD:  If at the beginning of an expression, weigh keywords and constructs higher.

  //  Search the alphabetically sorted symbol table for partial key matches.
  //  TBD:  Use a binary search to find the first partial match, then scan forward and backward for additional matches.
  uint32_t table_length = m_symbols.get_length();
  for (uint32_t i = 0; i < table_length; i++)
    {
    SkAutoCompleteSymbol * symbol = m_symbols.get_at(i);
    if (symbol->icompare_sub(m_key) == AEquate_equal)
      {
      insert(*symbol, *symbol, *symbol, 1.0);
      }
    }

  //  TBD: Insert the list of keywords

//  insert("one");
//  insert("two");
//  insert("three");
//  insert("four");
//  insert("forty");
//  insert("fort");
//  insert("from");
//  insert("front");
//  insert("free");
//  insert("_coroutine");
//  insert("_berserk");
//  insert("_berzerk");
//  insert("_berzerk()");
//  insert("_test");
//  insert("_test(4)");
  
  //  TBD: Insert applicable snippets.
  insert("do", "do[...]", "do\n  [\n    \n  ]\n", 0.5);
  insert("do_reverse", "do_reverse[...]", "do_reverse\n  [\n    \n  ]\n", 1.5);

  insert("if", "if", "if", 0.5);

  //  If the newly populated listbox has a string the same as the last selection, set that as the selection.
  if (m_choice_count > 0)
    {
    int selection = (int)SendMessage(m_os_handle, LB_SELECTSTRING, (WPARAM)-1, (LPARAM)m_choice_last.as_cstr());
    if (selection == LB_ERR)
      {
      m_choice_selection = 0;
      }
    else
      {
      m_choice_selection = selection;
      }

    SendMessage(m_os_handle, LB_SETCURSEL, m_choice_selection, 0);
    m_choice_last = m_choices.get_at(m_choice_selection)->m_display;
    }

  return m_choice_count;
  }


//---------------------------------------------------------------------------------------
//  This routine is called when printable characters (of identifiers) are typed in the parent
//  window (workspace or editor pane). As each character is added, this routine searches back
//  for the beginning of an identifier to generate the key. The listbox is positioned and then
//  populated with the auto-complete choices.
// 
//  Returns:    true  - if key press is not consumed and should be further processed (eventually passed to default proc).
//              false - if key press is consumed and shouldn't be further processed.
//              
//  Author(s):  John Stenersen
bool SkAutoComplete::on_parent_character(char ch, bool repeated, eAKeyMod mod_keys)
  {
  //A_DPRINT(A_SOURCE_FUNC_STR "\n" );

   if (!ms_active)
    {
    return true;    //  Auto-complete is off, the character is not consumed.
    }

  //  Make sure the new character is valid.
  if (AString::ms_char_match_table[ACharMatch_not_identifier][uint8_t(ch)] && ch != '@') // Allow '@' as well for data members
    {
    cancel();
    return true;    //  The ch was not an identifier, hence not consumed.
    }

  int             font_height     = m_font.get_height();
  SkEditSyntax *  editbox_p       = static_cast<SkEditSyntax *>(m_parent_p);
  AString         editbox_string  = editbox_p->get_text(0, editbox_p->get_caret_index());

  //  Remove any selection, this needs to be done before searching for the beginning of the identifier.
  if (editbox_p->is_selected())
    {
    editbox_p->remove_selection(true);
    }

  //  Search for the beginning of the identifier key and append the new character.
  m_key_start = 0;
  if (editbox_string.find_reverse(ACharMatch_not_identifier, 1, &m_key_start))
    {
    if (int32_t(m_key_start) >= 0 && editbox_string[m_key_start] == '@') --m_key_start; // Allow leading '@' as well for instance data members
    if (int32_t(m_key_start) >= 0 && editbox_string[m_key_start] == '@') --m_key_start; // Allow leading '@@' as well for class data members
    m_key_start++;
    }
  m_key_end = editbox_p->get_caret_index() + 1;
  m_key = editbox_p->get_text(m_key_start, m_key_end - 1);
  m_key.append(ch);

  //  If the key is not long enough, cancel the auto-complete listbox.
  if (m_key.get_length() < ms_key_length_min)
    {
    cancel();
    return true;    //  The character is not "consumed".
    }

  //  Set the location of the listbox.
  int start_x;
  int start_y;
  editbox_p->get_index_position(m_key_start, &start_x, &start_y);
  set_position(start_x - 3, start_y + font_height);

  //  Populate the listbox, resize it and then display it.
  if (populate() <= 0)
    {
    cancel();
    }
  else
    {
    resize();
    if (is_hidden())
      {
      show();
      }
    }

  return true;    //  The key triggered showing the listbox but doesn't "consume" the key.
  }


//---------------------------------------------------------------------------------------
//  Intercept key presses from the parent window (workspace or editor pane) whenever the
//  auto-complete listbox is displayed. The user can scroll up and down the listbox choices,
//  cancel the auto-complete by several methods and accept a choice.
//
//  Returns:    true  - if key press is not consumed and should be further processed (eventually passed to default proc).
//              false - if key press is consumed and shouldn't be further processed.
//  Author(s):  John Stenersen
bool SkAutoComplete::on_parent_key_press(eAKey key, bool repeated, eAKeyMod mod_keys)
  {
//  A_DPRINT(A_SOURCE_FUNC_STR "\n" );

  //  Ignore any modified key presses.
  if (mod_keys != AKeyMod_none)
    {
    return true;    //  Key not used so pass to default processing.
    }

  switch (key)
    {
    case AKey_left :
    case AKey_right :
      if (!is_hidden())
        {
        cancel();
        }
      break;

    case AKey_escape :
      if (!is_hidden())
        {
        cancel();
        return false;   //  The key was "consumed."
        }
      break;

    case AKey_return :
      if (!is_hidden())
        {
        //  Accept the auto-complete choice and replace text in the parent window.
        accept();
        return false;   //  The key was "consumed."
        }
      break;

    case AKey_backspace :
      if (!is_hidden())
        {
        if (m_key.get_length() > 1)
          {
          m_key_end--;
          m_key.crop(0, m_key.get_length() - 1);

          if (m_key.get_length() >= ms_key_length_min)
            {
            populate();
            resize();
            }
          else
            {
            cancel();
            }
          }
        else
          {
          cancel();
          }
        }
      break;

    case AKey_down :
      if (!is_hidden())
        {
        uint32_t selection = (uint32_t)SendMessage(m_os_handle, LB_GETCURSEL, 0, 0);
        m_choice_selection = min(selection + 1, m_choice_count - 1);
        SendMessage(m_os_handle, LB_SETCURSEL, m_choice_selection, 0);
        m_choice_last = m_choices.get_at(m_choice_selection)->m_display;

        return false;   //  The key was "consumed."
        }
      break;

    case AKey_up :
      if (!is_hidden())
        {
        uint32_t selection = (uint32_t)SendMessage(m_os_handle, LB_GETCURSEL, 0, 0);
        if (selection > 0)
          {
          m_choice_selection = max(selection - 1, 0);
          SendMessage(m_os_handle, LB_SETCURSEL, m_choice_selection, 0);
          m_choice_last = m_choices.get_at(m_choice_selection)->m_display;
          }

        return false;   //  The key was "consumed."
        }
      break;

     }

  return true;    //  Key was not "consumed" so pass it back to the caller.
  }


//---------------------------------------------------------------------------------------
//  If the caret is moved in the parent window, then cancel the auto-complete listbox.
//  
//  Author(s):  John Stenersen
void SkAutoComplete::on_parent_mouse_release(eAMouse button, eAMouse buttons, const AVec2i & client_pos)
  {
//  A_DPRINT(A_SOURCE_FUNC_STR "\n");

  if (!is_hidden())
    {
    if (m_key_end != reinterpret_cast<SkEditSyntax *>(m_parent_p)->get_caret_index())
      {
      cancel();
      }
    }
  }


//---------------------------------------------------------------------------------------
//  This routine handles the listbox selection and selection change caused by the mouse.
//  
//  Note: Keyboard selection and selection changes do not generate the LBN_ notification messages
//        because they are handled explicitly by on_parent_key_press() *and* use the LB_ to set
//        selections explicitly (thus Windows doesn't generate LBN_ notifications).
//        
//  Author(s):  John Stenersen
bool SkAutoComplete::on_control_event_standard(uint32_t code)
  {
//  A_DPRINT(A_SOURCE_FUNC_STR "code = %ld\n", code);

  switch (code)
  {
    case LBN_SELCHANGE:
      m_choice_selection  = (uint32_t)SendMessage(m_os_handle, LB_GETCURSEL, 0, 0);
      m_choice_last       = m_choices.get_at(m_choice_selection)->m_display;
      ::SetFocus(get_os_handle_parent());
      return true;

    case LBN_DBLCLK:
      accept();
      return true;
  }

  return false;     //  Not handled, do normal default processing.
  }
