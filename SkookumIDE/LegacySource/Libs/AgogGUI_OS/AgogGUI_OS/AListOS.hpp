// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// AListOS<> class declaration header (also AColumnOS<> and AListIdxOS<>)
//
// ##### Function descriptions located at implementations rather than declarations. #####
//=======================================================================================


#ifndef __ALISTOS_HPP
#define __ALISTOS_HPP


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
#include <commctrl.h>


//=======================================================================================
// Global Structures
//=======================================================================================

// AListOS Enumerated Constants
enum
  {
  AListOS_invalid_index         = -1,
  AListOS_image_default         = 0,
  AListOS_column_width_autosize = LVSCW_AUTOSIZE,  // Sizes column to largest subitem - which may be a sliver if there are no subitems
  AListOS_column_width_title    = LVSCW_AUTOSIZE_USEHEADER,  // Sizes column to largest subitem as above, but ensures that it is no smaller than the column title text and makes the last column take up any remaining space
  AListOS_column_width_padding  = 12, // Extra pixels to add to column with above and beyond title
  };

enum eAColumnAlign
  {
  AColumnAlign_left     = AHorizAlign_left,
  AColumnAlign_right    = AHorizAlign_right,
  AColumnAlign_centered = AHorizAlign_centered
  };

enum eAListImage
  {
  AListImage_icon       = LVSIL_NORMAL,
  AListImage_small_icon = LVSIL_SMALL,
  AListImage_state      = LVSIL_STATE
  };

enum eAListHover
  {
  AListHover_off,
  AListHover_track_auto_select,  // Track mouse over subitems and auto select if mouse rests over subitem
  AListHover_track_activate2,    // Track mouse over subitems and auto activate if a selected subitem is clicked a second time
  AListHover_track_activate1,    // Track mouse over subitems and auto activate if a subitem is clicked
  };


//---------------------------------------------------------------------------------------
// Notes      A AListIdxOS is an index based built-in list view control from the operating
//            system.  AListOS<> (defined lower in this file) is probably more convenient /
//            intuitive to use than this class.
// Subclasses AListOS<>
// See Also   
// UsesLibs   AgogCore\AgogCore.lib, AgogIO\AgogIO.lib, ComCtl32.lib
// Inlibs     AgogGUI_OS\AgogGUI_OS.lib
// Examples:      
// Author(s)  Conan Reis
class AListIdxOS : public AWindow
  {
  public:
  // Common Methods

    static void initialize();
    static void deinitialize();

    AListIdxOS(AWindow * parent_p, const AFont & font = *AFont::ms_default_p, const ARegion & region = ARegion(0, 0, ADef_int, ADef_int));
    virtual ~AListIdxOS();
    
  // Methods
    
    void enable_gridlines(bool draw_grid = true);
    void enable_user_label_edit(bool allow_edit = true);
    void enable_header_swapping(bool allow_drag = true);
    void enable_remove_events(bool call_on_remove = true)  { m_remove_events = call_on_remove; }
    void enable_multi_select(bool multi_select = true);
    //void enable_column_headers(bool show_headers = true);

    void set_hover(eAListHover mode);
    void label_user_edit(uint row);
    void label_user_edit_cancel();


  // Row (Item Index) Based Methods

    uintptr_t row2info(uint row) const;
    uint       info2row(uintptr_t info) const;
    uint       pos2row(const AVec2i & client_pos, uint * rank_p = nullptr) const;


  // Visibility Methods

    uint  get_length_visible() const;
    uint  get_first_visible_row() const;
    uint  get_last_visible_row() const;
    void ensure_visible_row(uint row);
    void update_row(uint row);
    void update_rows(uint start_row, uint end_row);
    void update_visible();


  // Focused and Selected Methods

    void ensure_size(uint item_count);
    uint  get_focus_row() const;
    uint  get_length() const;
    bool is_focused_row(uint row) const;
    void focus_row(uint row, bool focus = true);

    uint  get_selected_count() const;
    uint  get_selected_rows(uint rows_p[]) const;
    bool is_selected_row(uint row) const;
    void select_row(uint row, bool select = true);


  // Item Addition and Removal

    uint  append(uintptr_t item_info, uint icon_image = AListOS_image_default, uint state_image = AListOS_image_default);
    void remove_row(uint row);
    void remove_all();


  // Image Methods

    void         enable_image_sharing(bool share_images = true);
    void         set_image_list(const AImageListOS & images, eAListImage type = AListImage_small_icon);
    AImageListOS get_image_list(eAListImage type = AListImage_small_icon) const;

    void set_image_index(uint image_idx, uint row, uint rank = 0u, eAListImage type = AListImage_icon);
    uint  get_image_index(uint row, uint rank = 0u, eAListImage type = AListImage_icon) const;

    // get/set background image


  // Rank (Column Index) Methods

    uint  column_append(const AString & title, int width = AListOS_column_width_title, eAColumnAlign align = AColumnAlign_left);
    void column_remove_rank(uint rank);
    void column_set_title_rank(uint rank, const AString & title);

    int  column_get_width_rank(uint rank) const;
    void column_set_width_rank(uint rank, int width = AListOS_column_width_title);

    eAColumnAlign column_get_align_rank(uint rank) const;
    void          column_set_align_rank(uint rank, eAColumnAlign align);

    //uint  column_get_order_index(uint rank) const;  // ListView_GetColumn
    //void column_get_order_indexes(uint * ranks_a) const;  // ListView_GetColumnOrderArray
    //void column_set_order_indexes(uint * ranks_a) const;  // ListView_SetColumnOrderArray
    //uint  column_get_selected() const;  // ListView_GetSelectedColumn


  // Possible Future Methods:

    // uint     get_hot_row() const;
    // void    set_hot_row(uint row);
    // HCURSOR get_hot_cursor() const;
    // void    set_hot_cursor(HCURSOR cursor);

    // Support for view modes other than 'report' - i.e icon, small icon, and list views
    // Item check box commands
    // get/set color - background, text, text bg
    // Get Item position, bounding area, and spacing
    // Get list origin, scroll list
    // get/set tooltips
    // header? commands


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
    //?AString        get_title() const;
    //?bool          is_input_enabled() const;
    //?bool          is_sizable() const;
    //?void          set_cursor(HCURSOR cursor_handle);
    //?void          set_icon(HICON icon_handle);

  // Class Methods

  protected:

  // Event Methods

    virtual LRESULT on_custom_draw(NMLVCUSTOMDRAW * info_p);
    virtual bool    on_label_user_edit(uint row, uintptr_t item_info);
    virtual bool    on_label_user_edited(uint row, uintptr_t item_info, AString * label_p);
    virtual void    on_item_focused_row(uint row, uintptr_t item_info);
    virtual void    on_item_selected_row(uint row, uintptr_t item_info, bool focused);
    virtual void    on_item_removed_row(uint row, uintptr_t item_info);
    virtual void    on_column_clicked_rank(uint rank);
    virtual void    on_subitem_get_text(uint row, uint rank, AString * subitem_str_p, bool * save_text_p);
    virtual void    on_subitem_hovered(uint row, uint rank);
    virtual void    on_subitem_clicked(uint row, uint rank, eAMouse button, bool double_click);
    virtual void    on_subitem_activated(uint row, uint rank);

    //virtual bool on_edit(...);
    //virtual void on_edited(...);
    //virtual void on_drag(..., eAMouse button);
    //virtual void on_scroll();
    //virtual void on_scrolled();


    // Inherited events from AWindow

    //virtual bool on_focus();
    //virtual bool on_focus_lost();

      // These should probably not be overridden

      virtual bool on_control_event(NMHDR * info_p, LRESULT * result_p);


  // Internal Methods

    void parse_attempt_click(NMITEMACTIVATE * info_p, eAMouse button, bool double_click);
    void parse_subitem_get_text(NMLVDISPINFO * info_p);
    void parse_item_changed(NMLISTVIEW * info_p);

  // Data Members

    bool m_remove_events;

  // Class Data

    static AMessageTargetClass * ms_default_class_p;

  };  // AListIdxOS


template<class _ItemType> class AColumnOS;  // Forward Declaration

//---------------------------------------------------------------------------------------
// Notes      A AListOS is a built-in list view control from the operating system.
//
//            Any modifications to this template should be compile-tested by adding an
//            explicit instantiation declaration such as:
//              template class AListOS<AString>;
//
// Subclasses 
// See Also   AColumnOS<>, AListIdxOS
// UsesLibs   AgogCore\AgogCore.lib, AgogIO\AgogIO.lib, ComCtl32.lib
// Inlibs     AgogGUI_OS\AgogGUI_OS.lib
// Examples:      
// Author(s)  Conan Reis
template<class _ItemType>
class AListOS : public AListIdxOS
  {
  public:
    // Unhide Inherited Methods

      // Methods in this class with the same name as methods in AListIdxOS are 'hidden'
      // (even if they do not have the same interface), this makes them visible again.
      // Ensure that any new methods added to this class that also have the same name
      // as methods in AListIdxOS are included in this list to preserve the 'is-type-of'
      // relationship.  These using directives must precede the new methods in this class.
      using AListIdxOS::remove_all;


  // Common Methods

    AListOS(AWindow * parent_p, const AFont & font = *AFont::ms_default_p, const ARegion & region = AWindow::ms_region_def);
    virtual ~AListOS();
    
  // Accessor & Style Methods    

    _ItemType * row2item(uint row) const;
    uint         item2row(const _ItemType & item) const;
    _ItemType * pos2item(const AVec2i & client_pos, uint * row_p = nullptr, uint * rank_p = nullptr) const;
    void        get_items(APArray<_ItemType> * items_p) const;

    // Inherited from AListIdxOS

    //uint get_length() const;
    //void ensure_size(uint item_count);
    //void set_hover(eAListHover mode);
    //void enable_gridlines(bool draw_grid = true);
    //void enable_header_swapping(bool allow_drag = true);

    //uintptr_t row2info(uint row) const;
    //uint       info2row(uintptr_t info) const;
    //uint       pos2row(const AVec2i & client_pos, uint * rank_p = nullptr) const;

  // Visibility Methods

    _ItemType * get_first_visible() const;
    _ItemType * get_last_visible() const;
    void        ensure_visible(const _ItemType & item);
    void        update(const _ItemType & item);

    // Inherited from AListIdxOS

    //uint  get_length_visible() const;
    //uint  get_first_visible_row() const;
    //uint  get_last_visible_row() const;
    //void ensure_visible_row(uint row);
    //void update_row(uint row);
    //void update_rows(uint start_row, uint end_row);

  // Focused and Selected Methods
    
    _ItemType * get_focus() const;
    bool        is_focused(const _ItemType & item) const;
    void        focus(const _ItemType & item, bool focus_item = true);

    bool        is_selected(const _ItemType & item) const;
    uint         get_selected_all(APArray<_ItemType> * items_p) const;
    _ItemType * get_selected_first() const;
    void        remove_all(const APArrayBase<_ItemType> & items);
    void        select(const _ItemType & item, bool select_item = true);

    // Inherited from AListIdxOS

    //uint  get_focus_row() const;
    //bool is_focused_row(uint row) const;
    //void focus_row(uint row, bool focus = true);

    //uint  get_selected_count() const;
    //bool is_selected_row(uint row) const;
    //void select_row(uint row, bool select = true);

 // Item Addition and Removal

    uint  append(const _ItemType & item, uint icon_image = AListOS_image_default, uint state_image = AListOS_image_default);
    void append_all(const APArrayBase<_ItemType> & items);
    void remove(const _ItemType & item);

    // Inherited from AListIdxOS

    //void remove_row(uint row);
    //void remove_all();

    
  // Image Methods

    void set_image(uint image_idx, const _ItemType & item, uint rank = 0u, eAListImage type = AListImage_icon);
    uint  get_image(const _ItemType & item, uint rank = 0u, eAListImage type = AListImage_icon) const;

    // Inherited from AListIdxOS

    //void         enable_image_sharing(bool share_images = true);
    //void         set_image_list(const AImageListOS & images, eAListImage type = AListImage_small_icon);
    //AImageListOS get_image_list(eAListImage type = AListImage_small_icon) const;
    //void         set_image_index(uint image_idx, uint row, uint rank = 0u, eAListImage type = AListImage_icon);
    //uint          get_image_index(uint row, uint rank = 0u, eAListImage type = AListImage_icon) const;


  // Column Methods

    uint column_append(AColumnOS<_ItemType> * column_p);

    // The methods in AColumnOS should generally be preferred over these methods

      void                   column_remove(AColumnOS<_ItemType> * column_p);
      AColumnOS<_ItemType> * rank2column(uint rank) const;

      void column_remove_rank(uint rank);
      void column_set_title_rank(uint rank, const AString & title);

      void columns_set_width(int width = AListOS_column_width_title);

      // Inherited from AListIdxOS

      //int  column_get_width_rank(uint rank) const;
      //void column_set_width_rank(uint rank, int width = AListOS_column_width_title);

      //eAColumnAlign column_get_align_rank(uint rank) const;
      //void          column_set_align_rank(uint rank, eAColumnAlign align);


  // Sorting Methods

    void sort(ACompareBase<_ItemType> * compare_p, bool ascending = true);


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
    //?AString       get_title() const;
    //?bool          is_input_enabled() const;
    //?bool          is_sizable() const;
    //?void          set_cursor(HCURSOR cursor_handle);
    //?void          set_icon(HICON icon_handle);

  // Class Methods

  protected:

  // Nested Structures

    struct SortInfo
      {
      ACompareBase<_ItemType> * m_compare_p;
      bool                      m_ascending;
      };

  // Event Methods

    virtual void on_item_focused(_ItemType * item_p, uint row);
    virtual void on_item_selected(_ItemType * item_p, uint row, bool selected);

    // Inherited Events from AListIdxOS

    //virtual void on_subitem_hovered(uint row, uint rank);
    //virtual void on_subitem_clicked(uint row, uint rank, eAMouse button, bool double_click);
    //virtual void on_subitem_activated(uint row, uint rank);

      // These should probably not be overridden

      virtual void on_item_focused_row(uint row, uintptr_t item_info);
      virtual void on_item_selected_row(uint row, uintptr_t item_info, bool focused);
      virtual void on_item_removed_row(uint row, uintptr_t item_info);
      virtual void on_column_clicked_rank(uint rank);
      virtual void on_subitem_get_text(uint row, uint rank, AString * subitem_str_p, bool * save_text_p);
      //virtual bool on_control_event(NMHDR * info_p, LRESULT * result_p);


    // Inherited Events from AWindow

    //virtual bool on_focus();
    //virtual bool on_focus_lost();


  // Internal Class Methods

    static int CALLBACK compare_items(LPARAM item_info_rhs, LPARAM item_info_lhs, LPARAM compare_info);

  // Data Members

    APArray< AColumnOS<_ItemType> > m_columns;

  };  // AListOS


//---------------------------------------------------------------------------------------
// Notes      A AColumnOS is a column in a AListOS - derive from it if more information
//            needs to be stored/associated with it.
//
//            Any modifications to this template should be compile-tested by adding an
//            explicit instantiation declaration such as:
//              template class AColumnOS<AString>;
//
// See Also   ListInfoOS
// Author(s)  Conan Reis
template<class _ItemType>
class AColumnOS
  {
  public:

    friend class AListOS<_ItemType>;

  // Nested Structures

    struct SubItemText
      {
      uint         m_row;
      _ItemType * m_item_p;
      AColumnOS * m_column_p;
      AString *   m_subitem_str_p;
      bool        m_save_text;
      };

    typedef AFunctionArgBase<typename AColumnOS::SubItemText *> tSubItemCall;
    typedef ACompareBase<_ItemType>                             tSortCall;

  // Public Data Members

    // This is can be toggled/incremented with each on_clicked() - switch between
    // different sort modes by using m_sort_info % sort_states - but also ensure that
    // enable_sort_toggle(false) is called so that the result is not toggled again.
    uint m_sort_info;

    // This is toggled with each call to on_clicked() and is passed to each call to the
    // list's sort when this column's sort is called if "sort toggling" is enabled
    bool m_sort_ascending;

  // Methods

    AColumnOS(const AString & title, tSubItemCall * subitem_text_p = nullptr, tSortCall * sort_p = nullptr, bool sort_toggle = true, int width = AListOS_column_width_title, eAColumnAlign align = AColumnAlign_left, bool self_destruct = true);
    virtual ~AColumnOS();

    AListOS<_ItemType> * get_list() const;

    uint get_rank() const;

    void set_width(int width = AListOS_column_width_title);
    int  get_width() const;

    void            set_title(const AString & title);
    const AString & get_title() const;

    eAColumnAlign get_alignmemt() const;
    void          set_alignment(eAColumnAlign align);

    void remove();

    void sort();
    void enable_sort_toggle(bool toggle = true)  { m_sort_toggle = toggle };

    bool is_self_destruct() const;
    void enable_self_destruct(bool self_destruct = true);
    void attempt_self_destruct();

  protected:

  // Event Methods

      virtual void on_clicked();
      virtual void on_subitem_get_text(uint row, AString * subitem_str_p, bool * save_text_p);

  // Data Members

    AString              m_title;
    AListOS<_ItemType> * m_list_p;
    uint                  m_rank;
    int                  m_init_width;
    eAColumnAlign        m_init_align;
    bool                 m_self_destruct;

    tSubItemCall * m_subitem_text_p;

    tSortCall * m_sort_p;
    bool        m_sort_toggle;

  };  // AColumnOS


//=======================================================================================
// AListOS Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
inline void AListIdxOS::ensure_visible_row(uint row)
  {
  ListView_EnsureVisible(m_os_handle, row, FALSE);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
inline void AListIdxOS::update_row(uint row)
  {
  ListView_Update(m_os_handle, row);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
inline void AListIdxOS::update_rows(uint start_row, uint end_row)
  {
  // $Revisit - CReis UpdateWindow() ?
  ListView_RedrawItems(m_os_handle, start_row, end_row);
  }

//---------------------------------------------------------------------------------------
// Updates visible rows
// Author(s):   Conan Reis
inline void AListIdxOS::update_visible()
  {
  uint rows_visible = get_length_visible();

  if (rows_visible)
    {
    uint row_first = get_first_visible_row();

    update_rows(row_first, row_first + rows_visible);
    }
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
inline uint AListIdxOS::get_first_visible_row() const
  {
  return ListView_GetTopIndex(m_os_handle);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
inline uint AListIdxOS::get_length_visible() const
  {
  return ListView_GetCountPerPage(m_os_handle);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
inline uint AListIdxOS::get_last_visible_row() const
  {
  return get_first_visible_row() + get_length_visible();
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
inline uint AListIdxOS::get_length() const
  {
  return ListView_GetItemCount(m_os_handle);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
inline uint AListIdxOS::get_selected_count() const
  {
  return ListView_GetSelectedCount(m_os_handle);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
inline uint AListIdxOS::get_focus_row() const
  {
  return ListView_GetNextItem(m_os_handle, -1, LVNI_FOCUSED);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
inline void AListIdxOS::ensure_size(uint item_count)
  {
  ListView_SetItemCount(m_os_handle, item_count);
  }

//---------------------------------------------------------------------------------------
// Notes:      Successive calls to focus_row() on the same row will not call on_item_focused()
// Author(s):   Conan Reis
inline void AListIdxOS::focus_row(
  uint row,
  bool focus // = true
  )
  {
  ListView_SetItemState(m_os_handle, row, LVIS_FOCUSED, (focus ? LVIS_FOCUSED : 0));
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
inline void AListIdxOS::select_row(
  uint row,
  bool select // = true
  )
  {
  ListView_SetItemState(m_os_handle, row, LVIS_SELECTED, (select ? LVIS_SELECTED : 0));
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
inline bool AListIdxOS::is_selected_row(uint row) const
  {
  return (ListView_GetItemState(m_os_handle, row, LVIS_SELECTED) == LVIS_SELECTED);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
inline bool AListIdxOS::is_focused_row(uint row) const
  {
  return (ListView_GetItemState(m_os_handle, row, LVIS_FOCUSED) == LVIS_FOCUSED);
  }

//---------------------------------------------------------------------------------------
// Notes:      Can call on_item_focused() if row was the current focus
// Author(s):   Conan Reis
inline void AListIdxOS::remove_row(uint row)
  {
  ListView_DeleteItem(m_os_handle, row);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
inline void AListIdxOS::remove_all()
  {
  ListView_DeleteAllItems(m_os_handle);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
inline void AListIdxOS::enable_image_sharing(
  bool share_images // = true
  )
  {
  if (share_images)
    {
    append_style(LVS_SHAREIMAGELISTS);
    }
  else
    {
    remove_style(LVS_SHAREIMAGELISTS);
    }
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
inline void AListIdxOS::column_remove_rank(uint rank)
  {
  // $Revisit - CReis Deleting a column decrements the rank of all the columns that follow the
  // deleted column.
  ListView_DeleteColumn(m_os_handle, rank);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
inline int AListIdxOS::column_get_width_rank(uint rank) const
  {
  return ListView_GetColumnWidth(m_os_handle, rank);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
inline void AListIdxOS::column_set_width_rank(
  uint rank,
  int width // = AListOS_column_width_title
  )
  {
  ListView_SetColumnWidth(m_os_handle, rank, width);
  }


//=======================================================================================
// AListOS<> Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Constructor
// Arg         region - position and area of window.  (Default Region(0, 0, 320, 200))
// Author(s):   Conan Reis
template<class _ItemType>
inline AListOS<_ItemType>::AListOS(
  AWindow *       parent_p,
  const AFont &   font,   // = AFont::ms_default
  const ARegion & region  // = AWindow::ms_region_def
  ) :
  AListIdxOS(parent_p, font, region)
  {
  }

//---------------------------------------------------------------------------------------
// Destructor
// Author(s):   Conan Reis
template<class _ItemType>
AListOS<_ItemType>::~AListOS()
  {
  // Some windows/controls need to call destroy() in their own destructor
  // rather than letting the AMessageTarget destructor call it since destroy()
  // will end up sending windows messages and the windows/controls need to have
  // their virtual table still intact.
  destroy();

  // Remove all columns
  m_columns.apply_method(&AColumnOS<_ItemType>::attempt_self_destruct);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
inline _ItemType * AListOS<_ItemType>::row2item(uint row) const
  {
  return reinterpret_cast<_ItemType *>(row2info(row));
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
inline uint AListOS<_ItemType>::item2row(const _ItemType & item) const
  {
  return info2row(uintptr_t(&item));
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
_ItemType * AListOS<_ItemType>::pos2item(
  const AVec2i & client_pos,
  uint *           row_p, // = nullptr
  uint *           rank_p // = nullptr
  ) const
  {
  uint row = pos2row(client_pos, rank_p);

  if (row_p)
    {
    *row_p = row;
    }

  return (row != UINT32_MAX) ? row2item(row) : nullptr;
  }

//---------------------------------------------------------------------------------------
// Appends the currently selected items to the supplied array.
// Returns:    The number of selected items/rows
// Arg         items_p- array to append to.
// See:        get_selected_count()
// Author(s):   Conan Reis
template<class _ItemType>
inline uint AListOS<_ItemType>::get_selected_all(APArray<_ItemType> * items_p) const
  {
  uint selected_count = 0u;
  int next_item      = -1;

  do
    {
    next_item = ListView_GetNextItem(m_os_handle, next_item, LVNI_SELECTED);

    if (next_item != -1)
      {
      items_p->append(*row2item(next_item));
      selected_count++;
      }
    }
  while (next_item != -1);

  return selected_count;
  }

//---------------------------------------------------------------------------------------
// Appends the currently selected items to the supplied array.
// Returns:    The number of selected items/rows
// Arg         items_p- array to append to.
// See:        get_selected_count()
// Author(s):   Conan Reis
template<class _ItemType>
inline _ItemType * AListOS<_ItemType>::get_selected_first() const
  {
  int row = ListView_GetNextItem(m_os_handle, -1, LVNI_SELECTED);

  if (row != -1)
    {
    return row2item(row);
    }

  return nullptr;
  }

//---------------------------------------------------------------------------------------
// Appends all the list items to the supplied array
// Arg         items_p - array to append to (it may already have some items)
// Author(s):   Conan Reis
template<class _ItemType>
inline void AListOS<_ItemType>::get_items(APArray<_ItemType> * items_p) const
  {
  uint length = get_length();

  items_p->ensure_size(items_p->get_length() + length);

  LVITEM item_info;
  uint    row = 0u;

  item_info.mask     = LVIF_PARAM;
  item_info.iSubItem = 0;

  while (row < length)
    {
    item_info.iItem = row;
    ListView_GetItem(m_os_handle, &item_info);
    items_p->append(*reinterpret_cast<_ItemType *>(item_info.lParam));
    row++;
    }
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
inline _ItemType * AListOS<_ItemType>::get_first_visible() const
  {
  return row2item(get_first_visible_row());
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
inline _ItemType * AListOS<_ItemType>::get_last_visible() const
  {
  return row2item(get_last_visible_row());
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
inline void AListOS<_ItemType>::ensure_visible(const _ItemType & item)
  {
  ensure_visible_row(item2row(item));
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
inline void AListOS<_ItemType>::update(const _ItemType & item)
  {
  update_row(item2row(item));
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
inline _ItemType * AListOS<_ItemType>::get_focus() const
  {
  return row2item(get_focus_row());
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
inline bool AListOS<_ItemType>::is_focused(const _ItemType & item) const
  {
  return is_focused_row(item2row(item));
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
inline void AListOS<_ItemType>::focus(
  const _ItemType & item,
  bool              focus_item // = true
  )
  {
  focus_row(item2row(item), focus_item);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
inline bool AListOS<_ItemType>::is_selected(const _ItemType & item) const
  {
  return is_selected_row(item2row(item));
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
inline void AListOS<_ItemType>::select(
  const _ItemType & item,
  bool              select_item // = true
  )
  {
  select_row(item2row(item), select_item);
  }

//---------------------------------------------------------------------------------------
// Returns:    Row index of appended info.
// Author(s):   Conan Reis
template<class _ItemType>
inline uint AListOS<_ItemType>::append(
  const _ItemType & item,
  uint               icon_image, // = AListOS_image_default
  uint               state_image // = AListOS_image_default
  )
  {
  return AListIdxOS::append(uintptr_t(&item), icon_image, state_image);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
inline void AListOS<_ItemType>::append_all(const APArrayBase<_ItemType> & items)
  {
  _ItemType ** items_pp     = items.get_array();
  _ItemType ** items_end_pp = items_pp + items.get_length();

  for (; items_pp < items_end_pp; items_pp++)
    {
    AListIdxOS::append(uintptr_t(*items_pp));
    }
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
inline void AListOS<_ItemType>::remove(const _ItemType & item)
  {
  remove_row(item2row(item));
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
inline void AListOS<_ItemType>::remove_all(const APArrayBase<_ItemType> & items)
  {
  _ItemType ** items_pp     = items.get_array();
  _ItemType ** items_end_pp = items_pp + items.get_length();

  for (; items_pp < items_end_pp; items_pp++)
    {
    remove_row(item2row(**items_pp));
    }
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
inline void AListOS<_ItemType>::set_image(
  uint               image_idx,
  const _ItemType & item,
  uint               rank, // = 0u
  eAListImage       type  // = AListImage_icon
  )
  {
  set_image_index(image_idx, item2row(item), rank, type);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
inline uint AListOS<_ItemType>::get_image(
  const _ItemType & item,
  uint               rank, // = 0u
  eAListImage       type  // = AListImage_icon
  ) const
  {
  return get_image_index(item2row(item), rank, type);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
uint AListOS<_ItemType>::column_append(AColumnOS<_ItemType> * column_p)
  {
  column_p->m_rank = AListIdxOS::column_append(
    column_p->m_title,
    column_p->m_init_width,
    column_p->m_init_align);
  column_p->m_list_p = this;
  m_columns.append(*column_p);

  return column_p->m_rank;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
void AListOS<_ItemType>::column_remove(AColumnOS<_ItemType> * column_p)
  {
  uint remove_rank = column_p->m_rank;

  m_columns.remove(column_p->m_rank);

  column_p->attempt_self_destruct();

  // Decrement rank indexes of any columns that follow.
  uint length = m_columns.get_length();

  if (length > remove_rank)
    {
    AColumnOS<_ItemType> ** cols_pp     = m_columns.get_array() + remove_rank;
    AColumnOS<_ItemType> ** cols_end_pp = cols_pp + (length - remove_rank);

    for (; cols_pp < cols_end_pp; cols_pp++)
      {
      (*cols_pp)->m_rank--;
      }
    }
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
inline AColumnOS<_ItemType> * AListOS<_ItemType>::rank2column(uint rank) const
  {
  return m_columns.get_at(rank);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
inline void AListOS<_ItemType>::column_remove_rank(uint rank)
  {
  column_remove(rank2column(rank));
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
inline void AListOS<_ItemType>::column_set_title_rank(uint rank, const AString & title)
  {
  rank2column(rank)->set_title(title);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
inline void AListOS<_ItemType>::columns_set_width(
  int width // = AListOS_column_width_title
  )
  {
  uint rank   = 0u;
  uint length = m_columns.get_length();

  for (; rank < length; rank++)
    {
    column_set_width_rank(rank, width);
    }
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
inline void AListOS<_ItemType>::sort(
  ACompareBase<_ItemType> * compare_p,
  bool                      ascending // = true
  )
  {
  SortInfo info = {compare_p, ascending};

  ListView_SortItems(m_os_handle, compare_items, LPARAM(&info));
  }

//---------------------------------------------------------------------------------------
// See:        focus_row(), remove_row()
// Author(s):   Conan Reis
template<class _ItemType>
void AListOS<_ItemType>::on_item_focused(_ItemType * item_p, uint row)
  {
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
void AListOS<_ItemType>::on_item_selected(_ItemType * item_p, uint row, bool selected)
  {
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
void AListOS<_ItemType>::on_item_focused_row(uint row, uintptr_t item_info)
  {
  on_item_focused((_ItemType *)item_info, row);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
void AListOS<_ItemType>::on_item_selected_row(uint row, uintptr_t item_info, bool selected)
  {
  on_item_selected((_ItemType *)item_info, row, selected);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
void AListOS<_ItemType>::on_item_removed_row(uint row, uintptr_t item_info)
  {
  delete (_ItemType *)item_info;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
void AListOS<_ItemType>::on_column_clicked_rank(uint rank)
  {
  rank2column(rank)->on_clicked();
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
void AListOS<_ItemType>::on_subitem_get_text(uint row, uint rank, AString * subitem_str_p, bool * save_text_p)
  {
  rank2column(rank)->on_subitem_get_text(row, subitem_str_p, save_text_p);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
int CALLBACK AListOS<_ItemType>::compare_items(
  LPARAM item_info_rhs,
  LPARAM item_info_lhs,
  LPARAM compare_info
  )
  {
  SortInfo * info_p = reinterpret_cast<SortInfo *>(compare_info);
  int        result = int(info_p->m_compare_p->compare(
    *reinterpret_cast<_ItemType *>(item_info_rhs),
    *reinterpret_cast<_ItemType *>(item_info_lhs)));

  return info_p->m_ascending ? result : -result;
  }


//=======================================================================================
// AColumnOS<> Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
AColumnOS<_ItemType>::AColumnOS(
  const AString &                   title,
  AFunctionArgBase<SubItemText *> * subitem_text_p, // = nullptr
  ACompareBase<_ItemType> *         sort_p,         // = nullptr
  bool                              sort_toggle,    // = true
  int                               width,          // = AListOS_column_width_title
  eAColumnAlign                     align,          // = AColumnAlign_left
  bool                              self_destruct   // = true
  ) :
  m_title(title),
  m_list_p(nullptr),
  m_rank(uint(AListOS_invalid_index)),
  m_init_width(width),
  m_init_align(align),
  m_self_destruct(self_destruct),
  m_subitem_text_p(subitem_text_p),
  m_sort_p(sort_p),
  m_sort_toggle(sort_toggle),
  m_sort_ascending(true),
  m_sort_info(0u)
  {
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
inline AColumnOS<_ItemType>::~AColumnOS()
  {
  // $Revisit - CReis A better method might be to use attempt_self_destruct() instead of
  // deleting these callbacks.
  delete m_subitem_text_p;
  delete m_sort_p;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
inline AListOS<_ItemType> * AColumnOS<_ItemType>::get_list() const
  {
  return m_list_p;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
inline uint AColumnOS<_ItemType>::get_rank() const
  {
  return m_rank;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
inline void AColumnOS<_ItemType>::set_width(
  int width // = AListOS_column_width_title
  )
  {
  if (m_list_p)
    {
    m_list_p->column_set_width_rank(m_rank, width);
    }
  else
    {
    m_init_width = width;
    }
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
inline int AColumnOS<_ItemType>::get_width() const
  {
  return (m_list_p)
    ? m_list_p->column_get_width_rank(m_rank)
    : m_init_width;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
inline void AColumnOS<_ItemType>::set_title(const AString & title)
  {
  m_title = title;

  if (m_list_p)
    {
    m_list_p->AListIdxOS::column_set_title_rank(m_rank, title);
    }
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
inline const AString & AColumnOS<_ItemType>::get_title() const
  {
  return m_title;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
inline eAColumnAlign AColumnOS<_ItemType>::get_alignmemt() const
  {
  return (m_list_p)
    ? m_list_p->column_get_align_rank(m_rank)
    : m_init_align;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
inline void AColumnOS<_ItemType>::set_alignment(eAColumnAlign align)
  {
  if (m_list_p)
    {
    m_list_p->column_set_align_rank(m_rank, align);
    }
  else
    {
    m_init_align = align;
    }
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
inline void AColumnOS<_ItemType>::remove()
  {
  A_ASSERT(m_list_p, "The column must be associated/appended to a AListOS to use this method.", AErrId_invalid_request, ADebug);
  m_list_p->column_remove(this);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
inline void AColumnOS<_ItemType>::sort()
  {
  if (m_sort_p)
    {
    A_ASSERT(m_list_p, "The column must be associated/appended to a AListOS to use this method.", AErrId_invalid_request, ADebug);
    m_list_p->sort(m_sort_p, m_sort_toggle ? m_sort_ascending : true);
    }
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
inline bool AColumnOS<_ItemType>::is_self_destruct() const
  {
  return m_self_destruct;
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
inline void AColumnOS<_ItemType>::enable_self_destruct(
  bool self_destruct // = true
  )
  {
  m_self_destruct = self_destruct;
  }

//---------------------------------------------------------------------------------------
// Deletes this item if it is set to self destruct; otherwise it disconnects
//             from a AListOS if it is associated with one.
// Author(s):   Conan Reis
template<class _ItemType>
void AColumnOS<_ItemType>::attempt_self_destruct()
  {
  if (m_self_destruct)
    {
    delete this;
    }
  else
    {
    m_list_p = nullptr;
    m_rank   = uint(AListOS_invalid_index);
    }
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
void AColumnOS<_ItemType>::on_clicked()
  {
  m_sort_ascending = !m_sort_ascending;
  m_sort_info++;
  sort();
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
template<class _ItemType>
void AColumnOS<_ItemType>::on_subitem_get_text(uint row, AString * subitem_str_p, bool * save_text_p)
  {
  if (m_subitem_text_p)
    {
    SubItemText text_info = {row, m_list_p->row2item(row), this, subitem_str_p, *save_text_p};

    m_subitem_text_p->invoke(&text_info);

    *save_text_p = text_info.m_save_text;
    }
  //else
  //  {
  //  subitem_str_p->format("Row: %i(0x%p),  Col: %i", row, m_list_p->row2info(row), m_rank);
  //  }
  }


#endif  // __ALISTOS_HPP


