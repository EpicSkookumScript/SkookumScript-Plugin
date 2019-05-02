// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// AComboBoxOS class declaration header
//
// ##### Function descriptions located at implementations rather than declarations. #####
//=======================================================================================


#ifndef __AComboBoxOS_HPP
#define __AComboBoxOS_HPP


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogGUI_OS\AImageListOS.hpp>
#include <AgogCore\AFunctionArgBase.hpp>
#include <AgogCore\APArrayBase.hpp>
#include <AgogGUI\AWindow.hpp>
#include <AgogGUI\AMouse.hpp>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <CommCtrl.h>


//=======================================================================================
// Global Structures
//=======================================================================================

// AComboBoxOS Enumerated Constants
enum
  {
  AComboBoxOS_error                 = -1,
  AComboBoxOS_invalid_index         = AComboBoxOS_error
  };

enum eAComboOSFlag
  {
  AComboOSFlag_edit   = 1 << 0,  // If edit is not set then edit box is static
  AComboOSFlag_sort   = 1 << 1,  // Auto sort strings placed in combo list

  AComboOSFlag_no_vscroll   = 1 << 2,  // disable vscroll

  AComboOSFlags__static_no_sort  = 0x0,
  AComboOSFlags__static_sort     = AComboOSFlag_sort,
  AComboOSFlags__edit_no_sort    = AComboOSFlag_edit,
  AComboOSFlags__edit_sort       = AComboOSFlag_edit | AComboOSFlag_sort,
  };

//---------------------------------------------------------------------------------------
// Notes      A AComboBoxOS is an index based built-in combination edit/static text box
//            with a drop-down list box.
//            system.  AComboBoxOS<> (defined lower in this file) is may be more 
//            convenient / intuitive to use than this class.
// Subclasses AComboBoxOS<>
// Author(s)  Conan Reis
class AComboBoxOS : public AWindow
  {
  public:
  // Common Methods

    static void initialize();
    static void deinitialize();

    AComboBoxOS(AWindow * parent_p, uint32_t flags = AComboOSFlags__static_no_sort, const AFont & font = *AFont::ms_default_p, const ARegion & region = ARegion(0, 0, ADef_int, ADef_int));
    virtual ~AComboBoxOS();
    
    uint get_length() const;
    void show_dowpdown(bool show_b = true);

    // ComboBox_Enable
    // ComboBox_GetDroppedState
    // ComboBox_GetItemHeight
    // ComboBox_SetItemHeight
    // ComboBox_GetMinVisible
    // ComboBox_SetMinVisible

    // Edit Methods

      void set_text(const char * cstr_p)                                      { set_title(cstr_p); }

      // ComboBox_LimitText
      // ComboBox_GetCueBannerText
      // ComboBox_SetCueBannerText

  // Row (Item Index) Based Methods

    uintptr_t row2info(uint row) const;
    uint      info2row(uintptr_t info) const;

    template<class _ItemClass> _ItemClass * row2obj(uint row) const           { return reinterpret_cast<_ItemClass *>(row2info(row)); }
    template<class _ItemClass> uint         obj2row(_ItemClass & item) const  { return info2row(reinterpret_cast<uintptr_t>(&item)); }

    // ComboBox_FindString
    // ComboBox_FindStringExact
    // ComboBox_GetLBText
    // ComboBox_GetLBTextLen
    // ComboBox_SetItemData

  // Visibility Methods

    // ComboBox_GetMinVisible
    // ComboBox_SetMinVisible


  // Selection Methods

    uint      get_selected_row() const;
    uintptr_t get_selected_info() const;
    bool      is_selected_row(uint row) const                                 { return get_selected_row() == row; }
    void      select_clear()                                                  { select_row(uint(AComboBoxOS_invalid_index)); }
    void      select_row(uint row);
    uint      select_info(uintptr_t info);

    template<class _ItemClass> _ItemClass * get_selected() const              { return reinterpret_cast<_ItemClass *>(get_selected_info()); }
    template<class _ItemClass> uint          select(_ItemClass & item)        { return select_info(reinterpret_cast<uintptr_t>(&item)); }

    // ComboBox_SelectItemData
    // ComboBox_SelectString

  // Item Addition and Removal

    uint append_text(const char * item_cstr_p);
    uint append_info(const char * item_cstr_p, uintptr_t info);
    uint insert_text(const char * item_cstr_p, uint32_t row = 0u);
    uint insert_info(const char * item_cstr_p, uintptr_t info, uint32_t row = 0u);
    void remove_row(uint row);
    void remove_info(uintptr_t info)                                          { remove_row(info2row(info)); }
    void remove_all();

    template<class _ItemClass> void append(const char * item_cstr_p, _ItemClass & item)                  { append_info(item_cstr_p, reinterpret_cast<uintptr_t>(&item)); }
    template<class _ItemClass> void insert(const char * item_cstr_p, _ItemClass & item, uint32_t row = 0u)   { insert_info(item_cstr_p, reinterpret_cast<uintptr_t>(&item), row); }
    template<class _ItemClass> void remove(_ItemClass & item)                                            { remove_info(reinterpret_cast<uintptr_t>(&item)); }

  // Useful Methods inherited from AWindow - methods with ? might not be useful

    // ComboBox_GetText
    // ComboBox_GetTextLength
    //?AString get_title() const;
    // ComboBox_SetText
    //?void    set_title(const char * cstr_p);

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

    virtual void on_selected(uint row, uint row_prev);

    // Inherited events from AWindow

      //virtual bool on_focus();
      //virtual bool on_focus_lost();


      // These should probably not be overridden

      virtual bool on_control_event_standard(uint32_t code);


      // Inherited, but not available due to being an OS control

      //#virtual bool on_close_attempt();
      //#virtual bool on_context_menu(int x, int y);
      //#virtual void on_drag_drop(const AString & file_name, int x, int y);
      //#virtual bool on_hide(bool state_changed);
      //#virtual void on_move();
      //#virtual void on_moving();
      //#virtual void on_moved();
      //#virtual bool on_show(bool state_changed);
      //#virtual void on_size();
      //#virtual void on_sizing();
      //#virtual void on_sized();

  // Data Members

    uint  m_selected_prev;
    bool m_select_okay;

  // Class Data

    static AMessageTargetClass * ms_default_class_p;

  };  // AComboBoxOS


//=======================================================================================
// AComboBoxOS Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
inline uint AComboBoxOS::get_length() const
  {
  return ComboBox_GetCount(m_os_handle);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
inline void AComboBoxOS::show_dowpdown(
  bool show_b // = true
  )
  {
  ComboBox_ShowDropdown(m_os_handle, show_b);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
inline uint AComboBoxOS::get_selected_row() const
  {
  return ComboBox_GetCurSel(m_os_handle);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
inline uintptr_t AComboBoxOS::get_selected_info() const
  {
  uint row = get_selected_row();

  return (row != AComboBoxOS_error)
    ? row2info(row)
    : AComboBoxOS_error;
  }

//---------------------------------------------------------------------------------------
// Notes:      Successive calls to select_row() on the same row will not call on_item_selected()
// Author(s):   Conan Reis
inline void AComboBoxOS::select_row(uint row)
  {
  ComboBox_SetCurSel(m_os_handle, row);
  }

//---------------------------------------------------------------------------------------
// Notes:      Can call on_item_selected() if row was the current selection
// Author(s):   Conan Reis
inline void AComboBoxOS::remove_row(uint row)
  {
  ComboBox_DeleteString(m_os_handle, row);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
inline void AComboBoxOS::remove_all()
  {
  ComboBox_ResetContent(m_os_handle);
  }


#endif  // __AComboBoxOS_HPP


