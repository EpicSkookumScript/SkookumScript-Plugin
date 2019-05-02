                                           
// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumIDE Application
//
// SkookumScript IDE Auto-Complete Class and supporting utilities.
//=======================================================================================

#ifndef __SKAUTOCOMPLETE_HPP
#define __SKAUTOCOMPLETE_HPP


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogGUI/AWindow.hpp>
#include <SkookumScript/SkDebug.hpp>

class SkAutoComplete;


//---------------------------------------------------------------------------------------
class SkAutoCompleteSymbol : public AString
  {
  public:
    A_NEW_OPERATORS(SkAutoCompleteSymbol)

    SkAutoCompleteSymbol(const AString & str);

    bool  operator<(const SkAutoCompleteSymbol  & symbol_rhs) const;
    bool  operator==(const SkAutoCompleteSymbol & symbol_rhs) const;
  };


//---------------------------------------------------------------------------------------
class SkAutoCompleteChoice
  {
  public:
    SkAutoCompleteChoice::~SkAutoCompleteChoice() { m_parent_p = nullptr; }

    A_NEW_OPERATORS(SkAutoCompleteChoice)

    bool  operator<(const SkAutoCompleteChoice  & choice_rhs) const;
    bool  operator==(const SkAutoCompleteChoice & choice_rhs) const;

    AString           m_key;
    AString           m_display;
    AString           m_replace;
    float             m_weight;
    SkAutoComplete  * m_parent_p;
  };


//---------------------------------------------------------------------------------------
class SkAutoComplete : public AWindow
  {
  friend class SkAutoCompleteChoice;
  friend class SkWorkspace;
  friend class SkEditor;
  friend class SkConsole;
  friend class SkMainWindowBase;

  protected:

    SkAutoComplete(AWindow * parent_p);
    SkAutoComplete::~SkAutoComplete();

    static bool is_active()     { return ms_active;       }
    static void toggle_active() { ms_active = !ms_active; }

  //  Accessors

    bool      get_consider_weight() { return m_consider_weight_b; }

  //  Methods

    void      insert(const AString key, AString display = AString::ms_empty, AString replace = AString::ms_empty, float weight = 1);
    void      cancel();
    void      accept();
    void      resize();
    uint32_t  populate();

    void      symbols_populate();

  //  Events

    bool on_parent_character(char ch,   bool repeated, eAKeyMod mod_keys);
    bool on_parent_key_press(eAKey key, bool repeated, eAKeyMod mod_keys);
    void on_parent_mouse_release(eAMouse button, eAMouse buttons, const AVec2i & client_pos);
    bool on_control_event_standard(uint32_t code) override;

  //  Members
    bool      m_consider_weight_b   = false;                  //  Set true when ordering the choice array employing weights, false when checking for duplicate entries.
    uint32_t  m_choice_count        = 0;                      //  Number of choices in list box.
    uint32_t  m_choice_length_max   = 0;                      //  Maximum length of chars of all choices in the list box.
    uint32_t  m_choice_width_max    = 0;                      //  Maximum width in pixels of all choices.
    uint32_t  m_choice_display_max  = 5;                      //  Number of choices displayed in the list box before using a scrollbar.
    AString   m_choice_last;                                  //  The last choice made, used for setting listbox selection when shown next time.
    uint32_t  m_choice_selection    = 0;                      //  Index of the current selection.
    uint32_t  m_key_start           = ALength_remainder;      //  Start of the auto-complete key string in the source rich editbox.
    uint32_t  m_key_end             = ALength_remainder;      //  End of the auto-complete key string in the source rich editbox.
    AString   m_key;                                          //  Current auto-complete key string.

    APSortedLogical<SkAutoCompleteChoice> m_choices;          //  The choices (weights and replacement text) in the auto-complete listbox.
    APSortedLogical<SkAutoCompleteSymbol> m_symbols;          //  Alphabetically sorted copy of the main symbol table.

  //  Static Members
    static  bool      ms_active;                              //  True when auto-complete feature is on.
    static  uint32_t  ms_key_length_min;                      //  The minimum number of characters in the key before the listbox is generated/displayed.
  };


//=======================================================================================
// Inline Functions
//=======================================================================================

#ifndef A_INL_IN_CPP
  #include <SkookumIDE/SkAutoComplete.inl>
#endif


#endif  //  _SK_AUTOCOMPLETE_HPP
