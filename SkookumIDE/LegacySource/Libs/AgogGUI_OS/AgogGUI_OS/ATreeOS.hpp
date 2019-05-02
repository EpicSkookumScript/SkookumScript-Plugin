// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// ATreeOS class declaration header
//
// ##### Function descriptions located at implementations rather than declarations. #####
//=======================================================================================


#ifndef __ATREEOS_HPP
#define __ATREEOS_HPP


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogGUI\AWindow.hpp>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>


//=======================================================================================
// Global Structures
//=======================================================================================

class ATreeItemOS;  // Forward Declaration

//---------------------------------------------------------------------------------------
// Notes      A ATreeOS is a built-in tree view control from the operating system.
// UsesLibs   AgogCore\AgogCore.lib, AgogIO\AgogIO.lib, ComCtl32.lib
// Inlibs     AgogGUI_OS\AgogGUI_OS.lib
// Author(s)  Conan Reis
class ATreeOS : public AWindow
  {
  public:
  // Common Methods

    static void initialize();
    static void deinitialize();

    ATreeOS(AWindow * parent_p, const AFont & font = *AFont::ms_default_p, const ARegion & region = AWindow::ms_region_def);
    ~ATreeOS();
    
  // Methods
    
    uint           get_length() const;
    uint           get_length_visible() const;
    ATreeItemOS * get_root() const;
    ATreeItemOS * get_selected() const;
    ATreeItemOS * get_first_visible() const;
    ATreeItemOS * get_last_visible() const;
    ATreeItemOS * get_by_pos(int x, int y) const;
    ATreeItemOS * get_by_pos(const AVec2i & client_pos) const     { return get_by_pos(client_pos.m_x, client_pos.m_y); }
    ATreeItemOS * get_named(const AString & item_name, uint instance = 1u) const;
    ATreeItemOS * get_next(const ATreeItemOS & item) const;

    void append(ATreeItemOS * item_p, const ATreeItemOS * parent_p = nullptr, bool sort_item = false);
    void remove(const ATreeItemOS & item);
    void remove_all();
    void select(const ATreeItemOS & item);
    void ensure_visible(const ATreeItemOS & item);
    bool expand(const ATreeItemOS & item, bool expand = true);
    bool toggle_expand(const ATreeItemOS & item);
    void expand_all();

    //ATreeItemOS * get_parent(const ATreeItemOS & item) const;
    //ATreeItemOS * get_next_visible(const ATreeItemOS & item) const;
    //ATreeItemOS * get_previous_visible(const ATreeItemOS & item) const;
    //ATreeItemOS * get_child(const ATreeItemOS & item) const;
    //ATreeItemOS * get_next_sibling(const ATreeItemOS & item) const;
    //ATreeItemOS * get_previous_sibling(const ATreeItemOS & item) const;

    //AImageListOS & get_image_list(void) const;
    //void          append(const AImageListOS & image_list); 
    // ? void sort_children();
    // get/set child item indent
    // get/set tooltips
    // get/set color - background, text, insert mark, line color
    // get/set item height
    // get/set scroll time
    // get/set checked state


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

  protected:

  // Event Methods

    virtual LRESULT on_custom_draw(NMTVCUSTOMDRAW * info_p);
    virtual bool    on_selecting(ATreeItemOS * item_p, ATreeItemOS * old_item_p);
    virtual void    on_selected(ATreeItemOS * item_p, ATreeItemOS * old_item_p);
    virtual void    on_removed(ATreeItemOS * item_p);
    //virtual bool on_toggle_expanding(ATreeItemOS * item_p, bool expanding);
    //virtual void on_toggle_expanded(ATreeItemOS * item_p, bool expanding);
    //virtual void on_edit(ATreeItemOS * item_p);
    //virtual void on_edited(ATreeItemOS * item_p);
    //virtual void on_drag(ATreeItemOS * item_p);
    //virtual void on_drag_rbutton(ATreeItemOS * item_p);

    // Inherited events from AWindow

    virtual bool on_control_event(NMHDR * info_p, LRESULT * result_p);
    //virtual bool on_focus();
    //virtual bool on_focus_lost();

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

  // Internal Methods

    ATreeItemOS * handle2item(HTREEITEM handle) const;
    HTREEITEM     get_next_handle(HTREEITEM handle) const;

  // Data Members

    bool m_free_items;  // Delete items when removed

  // Class Data

    static AMessageTargetClass * ms_default_class_p;

  };  // ATreeOS


//---------------------------------------------------------------------------------------
// Notes      A ATreeItemOS is an entry in a ATreeOS - derive from it if more information
//            needs to be stored/associated with it.
// See Also   ATreeInfoOS
// Author(s)  Conan Reis
class ATreeItemOS
  {
  public:

    friend class ATreeOS;

  // Methods

    ATreeItemOS(); 
    ATreeItemOS(const AString & title, bool bolded = false, bool self_destruct = true);
    virtual ~ATreeItemOS();

    // Accessors

    const AString & get_title() const { return m_title; }
    ATreeOS *       get_tree() const  { return m_tree_p; }

    bool is_self_destruct() const                        { return m_self_destruct; }
    void enable_self_destruct(bool self_destruct = true) { m_self_destruct = self_destruct; }
    void attempt_self_destruct();


    // In-ATreeOS Related Methods

    ATreeItemOS * get_next() const   { m_tree_p->get_next(*this); }

    void remove()                    { m_tree_p->remove(*this); }
    void select()                    { m_tree_p->select(*this); }
    void ensure_visible()            { m_tree_p->ensure_visible(*this); }
    bool expand(bool expand = true)  { m_tree_p->expand(*this, expand); }
    bool toggle_expand()             { m_tree_p->toggle_expand(*this); }

    //bool is_appended() const;
    //bool is_parent() const;
    //bool is_visible() const;
    //get_parent
    //get_previous
    //get_next_visible
    //get_previous_visible
    //get_next_sibling
    //get_previous_sibling
    //get_first_child

  // Data Members

    AString   m_title;
    ATreeOS * m_tree_p;
    HTREEITEM m_handle;
    bool      m_self_destruct;
    bool      m_bolded;

  };  // ATreeItemOS


//---------------------------------------------------------------------------------------
// Notes      A ATreeInfoOS is an entry in a ATreeOS with custom info stored in it.
// See Also   ATreeItemOS
// Author(s)  Conan Reis
template<class _InfoType>
class ATreeInfoOS
  : public ATreeItemOS
  {
  public:

    ATreeInfoOS() {}
    ATreeInfoOS(const _InfoType & info, const AString & title, bool bolded = false, bool self_destruct = true)
      : ATreeItemOS(title, bolded, self_destruct), m_info(info) {}

    virtual ~ATreeInfoOS() {}

  // Public Data Members

    _InfoType m_info;

  };  // ATreeInfoOS<>



//=======================================================================================
// ATreeOS Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
inline uint ATreeOS::get_length() const
  {
  return TreeView_GetCount(m_os_handle);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
inline uint ATreeOS::get_length_visible() const
  {
  return TreeView_GetVisibleCount(m_os_handle);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
inline ATreeItemOS * ATreeOS::get_root() const
  {
  return handle2item(TreeView_GetRoot(m_os_handle));
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
inline ATreeItemOS * ATreeOS::get_selected() const
  {
  return handle2item(TreeView_GetSelection(m_os_handle));
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
inline ATreeItemOS * ATreeOS::get_first_visible() const
  {
  return handle2item(TreeView_GetFirstVisible(m_os_handle));
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
inline ATreeItemOS * ATreeOS::get_last_visible() const
  {
  return handle2item(TreeView_GetLastVisible(m_os_handle));
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
inline void ATreeOS::remove_all()
  {
  TreeView_DeleteAllItems(m_os_handle);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
inline void ATreeOS::select(const ATreeItemOS & item)
  {
  TreeView_SelectItem(m_os_handle, item.m_handle);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
inline void ATreeOS::ensure_visible(const ATreeItemOS & item)
  {
  TreeView_EnsureVisible(m_os_handle, item.m_handle);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
inline bool ATreeOS::expand(
  const ATreeItemOS & item,
  bool                expand // = true
  )
  {
  return (TreeView_Expand(m_os_handle, item.m_handle, expand ? TVE_EXPAND : TVE_COLLAPSE) == TRUE);
  }

//---------------------------------------------------------------------------------------
// Author(s):   Conan Reis
inline bool ATreeOS::toggle_expand(const ATreeItemOS & item)
  {
  return (TreeView_Expand(m_os_handle, item.m_handle, TVE_TOGGLE) == TRUE);
  }


//=======================================================================================
// ATreeItemOS Inline Methods
//=======================================================================================

//---------------------------------------------------------------------------------------
// Deletes this item if it is set to self destruct; otherwise it disconnects
//             from a ATreeOS if it is associated with one.
// Author(s):   Conan Reis
inline void ATreeItemOS::attempt_self_destruct()
  {
  if (m_self_destruct)
    {
    delete this;
    }
  else
    {
    m_tree_p = nullptr;
    m_handle = 0;
    }
  }


#endif  // __ATREEOS_HPP


