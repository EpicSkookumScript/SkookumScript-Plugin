// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumIDE Application
//
// SkookumScript IDE Symbol Search Dialog
//=======================================================================================


#ifndef __SKSEARCHDIALOG_HPP
#define __SKSEARCHDIALOG_HPP


//=======================================================================================
// Includes
//=======================================================================================

#include <SkookumIDE/SkConsole.hpp>
#include <AgogCore/AList.hpp>
#include <AgogCore/AObjReusePool.hpp>
#include <AgogIO/ATimer.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

// Pre-declarations
class SkSearchDialog;


//---------------------------------------------------------------------------------------
struct SkMatchReference : public SkContextInfo, public AListNode<SkMatchReference>
  {
  // Methods

    SkMatchReference()                                            {}

    SkMatchReference & operator=(const SkMatchReference & ref)    { SkContextInfo::operator=(ref); return *this; }
    void init(const SkContextInfo & member_info)                  { SkContextInfo::operator=(member_info); }

  // Class Methods

    static void                              pool_delete(SkMatchReference * info_p);
    static AObjReusePool<SkMatchReference> & get_pool();
    static SkMatchReference *                pool_new(const SkContextInfo & member_info);
    SkMatchReference **                      get_pool_unused_next() { return (SkMatchReference **)m_member_id.get_pool_unused_next(); } // Area in this class where to store the pointer to the next unused object when not in use

  };  // SkMatchReference


typedef AColumnOS<SkMatchReference>::SubItemText tSkMatchText;

//---------------------------------------------------------------------------------------
class SkMatchList : public AListOS<SkMatchReference>
  {
  friend class SkSearchDialog;

  public:

  // Nested Data-structures

    enum eColumn
      {
      Column_name,      // member name
      Column_instance,  // class instance or class scope (i/c)
      Column_scope,     // class scope belonged to (Scope)
      Column_info       // parameters / type
      };

  // Methods

    SkMatchList(SkSearchDialog * search_dlg_p);
    ~SkMatchList();

    void append_member(const SkContextInfo & info);

  protected:

  // Internal Methods

    void append_invokables(SkInvokableBase ** members_pp, uint length, eSkInvokable custom_type, bool show_custom, bool show_atomic, bool class_scope);

  // Event Methods

    virtual bool on_mouse_press(eAMouse button, eAMouse buttons, const AVec2i & client_pos, bool double_click);
    virtual bool on_context_menu(const AVec2i & screen_pos);
    virtual bool on_key_press(eAKey key, bool repeated);
    virtual void on_subitem_activated(uint row, uint rank);
    virtual void on_item_focused(SkMatchReference * item_p, uint row);
    virtual void on_item_removed_row(uint row, uintptr_t item_info);

    virtual LRESULT on_custom_draw(NMLVCUSTOMDRAW * info_p);
    void            on_text_name(tSkMatchText * info_p);
    void            on_text_scope(tSkMatchText * info_p);
    void            on_text_info(tSkMatchText * info_p);
    eAEquate        on_compare_instance(const SkMatchReference & lhs, const SkMatchReference & rhs);
    eAEquate        on_compare_name(const SkMatchReference & lhs, const SkMatchReference & rhs);
    eAEquate        on_compare_scope(const SkMatchReference & lhs, const SkMatchReference & rhs);

  // Internal Class Methods

    static eSkMember unify_type(eSkMember type);

  // Data Members

    SkSearchDialog * m_search_dlg_p;

    AColumnOS<SkMatchReference> * m_col_name_p;
    AColumnOS<SkMatchReference> * m_col_instance_p;
    AColumnOS<SkMatchReference> * m_col_scope_p;

  };  // SkMatchList


//---------------------------------------------------------------------------------------
struct SkMatchCriteria
  {
  // Nested Structures

    // Expected number of matches based on supplied info.
    enum eExpected
      {
      Expected_none,
      Expected_one,
      Expected_some,
      Expected_all
      };


  // Public Data Members

    eAStrMatch m_class_match_type;
    AString    m_class_match_str;
    eAStrMatch m_member_match_type;
    AString    m_member_match_str;
    // $Revisit - CReis Eventually rewrite these to use AStringBM

  // Common Methods

    SkMatchCriteria() : m_class_match_type(AStrMatch_exact), m_member_match_type(AStrMatch_exact) {}

    SkMatchCriteria & operator=(const SkMatchCriteria & criteria);

    AString as_string() const;

    eExpected get_class_expected() const;
    eExpected get_member_expected() const;
    eExpected get_match_expected() const;
    bool      is_class_match(const SkClass & cls) const      { return cls.get_name_str().is_imatch(m_class_match_str, m_class_match_type); }
    bool      is_member_match(const ANamed & member) const   { return member.get_name_str().is_imatch(m_member_match_str, m_member_match_type); }

  };


//---------------------------------------------------------------------------------------
class SkSearchDialog : public AWindow
  {
  friend class SkMatchList;

  public:
  // Common Methods

    static void initialize();
    static void deinitialize();
    static bool is_initialized();

    SkSearchDialog(eSkMatchKind kind = SkMatchKind_all);
    ~SkSearchDialog();

    eSkMatchKind get_kind() const                                   { return m_kind; }

    void empty();
    void load_settings(eSkMatchKind kind);
    void display(eSkMatchKind kind = SkMatchKind_all, const AString & match_text = AString::ms_empty, const SkMatchCriteria * match_info_p = nullptr);
    void find_matches(const SkMatchCriteria * match_info_p = nullptr);
    void find_matches_countdown();
    bool parse_match_text(const AString & match_text, SkMatchCriteria * criteria_p);

  // Event Methods

    // UI Custom Events

      void on_ok();
      void on_idle();
      void on_search_text_modified();
      void on_toggle_filter(eAFlag new_state);

    // Overridden Events

      virtual bool on_key_press(eAKey key, bool repeated);
      virtual bool on_draw();
      virtual bool on_focus();
      virtual void on_sizing();

  protected:

    void               set_title_kind(bool searching = false);
    void               find_data_matches(const tSkTypedNameArray * data_table_p, SkClass * class_p, SkMatchCriteria::eExpected expected_members, SkMatchCriteria & criteria, SkContextInfo & info);
    eAStrMatch         parse_match_part(const AString & match_str, uint32_t start_idx, uint32_t * end_idx_p, AString * match_part_p);
    void               pool_delete_info(SkMatchReference * match_p)  { m_matches.remove(match_p); SkMatchReference::pool_delete(match_p); }
    SkMatchReference * pool_new_info(const SkContextInfo & member_info);

  // Data Members

    eSkMatchKind            m_kind;
    AList<SkMatchReference> m_matches;

    SkMatchList m_match_list;

    ATimer   m_timer;
    uint32_t m_countdown_start;

    // Filters

      AMethodArg<SkSearchDialog, eAFlag> m_on_toggle_method;

      // Class Filters
      ARegion     m_rgn_class;
      ACheckBoxOS m_toggle_classes;
      ACheckBoxOS m_toggle_superclasses;
      ACheckBoxOS m_toggle_subclasses;

      // Routine Filters
      ARegion     m_rgn_routine;
      ACheckBoxOS m_toggle_coroutines;
      ACheckBoxOS m_toggle_methods_instance;
      ACheckBoxOS m_toggle_methods_class;
      ACheckBoxOS m_toggle_script;
      ACheckBoxOS m_toggle_cpp;

      // Data Filters
      ARegion     m_rgn_data;
      ACheckBoxOS m_toggle_data_instance;
      ACheckBoxOS m_toggle_data_class;

    SkEditSyntax m_search_text;
    int          m_search_text_y;

    AButtonOS   m_ok_btn;
    AButtonOS   m_cancel_btn;

  };  // SkSearchDialog


//=======================================================================================
// Inline Methods
//=======================================================================================

//=======================================================================================
// SkMatchReference Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Determines expected number of class matches.
// Returns:    see SkMatchCriteria::eExpected
// Author(s):   Conan Reis
inline SkMatchCriteria::eExpected SkMatchCriteria::get_class_expected() const
  {
  bool exact_match = (m_class_match_type == AStrMatch_exact);

  return m_class_match_str.is_filled()
    ? (exact_match ? Expected_one : Expected_some)
    : (exact_match ? Expected_none : Expected_all);
  }

//---------------------------------------------------------------------------------------
// Determines expected number of member matches per class.
// Returns:    see SkMatchCriteria::eExpected
// Author(s):   Conan Reis
inline SkMatchCriteria::eExpected SkMatchCriteria::get_member_expected() const
  {
  bool exact_match = (m_member_match_type == AStrMatch_exact);

  return m_member_match_str.is_filled()
    ? (exact_match ? Expected_one : Expected_some)
    : (exact_match ? Expected_none : Expected_all);
  }

//---------------------------------------------------------------------------------------
// Determines expected number matches.
// Returns:    see SkMatchCriteria::eExpected
// Author(s):   Conan Reis
inline SkMatchCriteria::eExpected SkMatchCriteria::get_match_expected() const
  {
  eExpected class_expected  = get_class_expected();
  eExpected member_expected = get_member_expected();

  if (class_expected == member_expected)
    {
    return member_expected;
    }

  if ((class_expected == Expected_none) || (member_expected == Expected_none))
    {
    return Expected_none;
    }

  return Expected_some;
  }


//=======================================================================================
// SkMatchReference Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
//  Frees up a object and puts it into the dynamic pool ready for its next
//              use.  This should be used instead of 'delete' because it  prevents
//              unnecessary deallocations by saving previously allocated objects.
// See:         pool_new()
// Notes:       To 'allocate' an object use 'pool_new()' rather than 'new'.
// Modifiers:    static
// Author(s):    Conan Reis
inline void SkMatchReference::pool_delete(SkMatchReference * info_p)
  {
  get_pool().recycle(info_p);
  }


//=======================================================================================
// SkMatchList Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
inline eSkMember SkMatchList::unify_type(eSkMember type)
  {
  switch (type)
    {
    case SkMember_method_func:
    case SkMember_method_mthd:   return SkMember_method;
    case SkMember_coroutine_func:
    case SkMember_coroutine_mthd: return SkMember_coroutine;
    default:                     return type;
    }
  }


#endif  // __SKSEARCHDIALOG_HPP

