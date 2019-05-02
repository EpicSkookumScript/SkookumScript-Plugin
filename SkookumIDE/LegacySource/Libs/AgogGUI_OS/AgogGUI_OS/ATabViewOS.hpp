// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// ATabViewOS class declaration header
//
// ##### Function descriptions located at implementations rather than declarations. #####
//=======================================================================================


#ifndef __ATABVIEWOS_HPP
#define __ATABVIEWOS_HPP


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogGUI\AWindow.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

// ATabViewOS Enumerated Constants
enum
  {
  ATabViewOS_error                 = -1,
  ATabViewOS_invalid_index         = ATabViewOS_error
  };

enum eARectEdge
  {
  ARectEdge_top,
  ARectEdge_left,
  ARectEdge_bottom,
  ARectEdge_right
  };

//---------------------------------------------------------------------------------------
// Notes      A ATabGroupOS is one or more graphical labeled tabs with one of them 
//            selected at any one time.
// Author(s)  Conan Reis
class ATabGroupOS : public AWindow
  {
  public:
  // Common Methods

    static void initialize();
    static void deinitialize();

    ATabGroupOS(AWindow * parent_p, eARectEdge placement = ARectEdge_top, const AFont & font = *AFont::ms_default_p, const ARegion & region = ARegion(0, 0, ADef_int, ADef_int));
    virtual ~ATabGroupOS();
    
    uint get_count() const;

  // Visual Methods
  
    ARegion get_tab_region() const;
    ARegion get_tab_window_region(const ARegion & tab_region) const;
    void    set_tab_region(const ARegion & tab_region);
  
  // Index Conversion Methods

    uintptr_t idx2info(uint idx) const;
    uint       info2idx(uintptr_t info) const;

    template<class _ItemClass> _ItemClass * idx2obj(uint idx) const             { return reinterpret_cast<_ItemClass *>(idx2info(idx)); }
    template<class _ItemClass> uint          obj2idx(_ItemClass & obj) const   { return info2idx(reinterpret_cast<uintptr_t>(&obj)); }

  // Selection Methods

    uint       get_selected_idx() const;
    uintptr_t get_selected_info() const;
    bool      is_selected_idx(uint idx) const                                   { return get_selected_idx() == idx; }
    void      select_clear()                                                  { select_idx(uint(ATabViewOS_invalid_index)); }
    uint       select_info(uintptr_t info);

    virtual void select_idx(uint idx);

    template<class _ItemClass> _ItemClass * get_selected() const              { uintptr_t info = get_selected_info(); return (info != ATabViewOS_error) ? reinterpret_cast<_ItemClass *>(info) : nullptr; }
    template<class _ItemClass> uint          select(_ItemClass & obj)          { return select_info(reinterpret_cast<uintptr_t>(&obj)); }

  // Focus Methods

    uint  get_focus_idx() const;
    void focus_idx(uint idx);

  // Item Addition and Removal

    uint  append_tab(const char * tab_cstr_p, uintptr_t info = 0u);
    uint  insert_tab(const char * tab_cstr_p, uintptr_t info = 0u, uint idx = 0u);
    void remove_idx(uint idx);
    void remove_info(uintptr_t info)                                          { remove_idx(info2idx(info)); }
    void remove_all();

    template<class _ItemClass> uint  append(const char * tab_cstr_p, _ItemClass & obj)                 { return append_tab(tab_cstr_p, reinterpret_cast<uintptr_t>(&obj)); }
    template<class _ItemClass> void insert(const char * tab_cstr_p, _ItemClass & obj, uint idx = 0u)  { insert_tab(tab_cstr_p, reinterpret_cast<uintptr_t>(&obj), idx); }
    template<class _ItemClass> void remove(_ItemClass & obj)                                          { remove_info(reinterpret_cast<uintptr_t>(&obj)); }

  // Useful Methods inherited from AWindow - methods with ? might not be useful

    // Position / Size Methods - see AWindow
    // Display State Methods - see AWindow
    // const AFont & get_font() const;
    // void          set_font(const AFont & font_p);
    // void          set_focus();
    // void          set_border(eBorder border = Border_raised);
    //?void          enable_input(bool input_accepted = true);
    //?void          enable_sizing(bool user_sizable = true);
    //?HCURSOR       get_cursor() const;
    //?HICON         get_icon() const;
    //?bool          is_input_enabled() const;
    //?bool          is_sizable() const;
    //?void          set_cursor(HCURSOR cursor_handle);
    //?void          set_icon(HICON icon_handle);

  // Class Methods

  protected:

  // Event Methods

    virtual void on_selected_idx(uint new_idx, uint old_idx);
    virtual void on_focused_idx(uint idx);

    // Inherited events from AWindow

      //virtual bool on_focus();
      //virtual bool on_focus_lost();


      // These should probably not be overridden

      virtual bool on_control_event(NMHDR * info_p, LRESULT * result_p);

  // Data Members

    uint m_prev_idx;

  // Class Data

    static AMessageTargetClass * ms_default_class_p;

  };  // ATabGroupOS


//---------------------------------------------------------------------------------------
// Notes      A ATabViewOS is one or more graphical labeled tabs with each tab associated
//            with one child window - and one of them selected at any one time.
// Author(s)  Conan Reis
class ATabViewOS : public ATabGroupOS
  {
  public:
  // Common Methods

    ATabViewOS(AWindow * parent_p, eARectEdge placement = ARectEdge_top, const AFont & font = *AFont::ms_default_p, const ARegion & region = ARegion(0, 0, ADef_int, ADef_int));
    virtual ~ATabViewOS()                         {}

    uint  append_win(const char * tab_cstr_p, AWindow * win_p);
    void insert_win(const char * tab_cstr_p, AWindow * win_p, uint idx = 0u);
    void remove_win(AWindow * win_p);

    AWindow * get_selected_win() const            { uintptr_t info = get_selected_info(); return (info != ATabViewOS_error) ? reinterpret_cast<AWindow *>(info) : nullptr; }
    uint       select_win(AWindow * win_p)         { return select_info(reinterpret_cast<uintptr_t>(win_p)); }
    AWindow * idx2win(uint idx) const               { return reinterpret_cast<AWindow *>(idx2info(idx)); }
    uint       win2idx(AWindow * win_p) const      { return info2idx(reinterpret_cast<uintptr_t>(win_p)); }

    virtual void select_idx(uint idx);

    // See ATabGroupOS for many inherited methods.

  // Class Methods

  protected:

  // Event Methods

    virtual bool on_control_event(NMHDR * info_p, LRESULT * result_p);
    virtual void on_sizing();
    virtual void on_sized();
    
    virtual void on_selected_win(AWindow * new_win_p, uint new_idx, AWindow * old_win_p, uint old_idx);

      //virtual void on_selected_idx(uint new_idx, uint old_idx);
      //virtual void on_focused_idx(uint idx);


  // Data Members

  };  // ATabGroupOS


#endif  // __ATABVIEWOS_HPP


