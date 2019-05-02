// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// AMenuOS class declaration header
//
// ##### Function descriptions located at implementations rather than declarations. #####
//=======================================================================================


#ifndef __AMENUOS_HPP
#define __AMENUOS_HPP


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogIO\AgogIO.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

// Pre-declarations
struct AVec2i;
class  AWindow;
class  APopMenuOS;
PREDECLARE_HANDLE(HMENU);


//---------------------------------------------------------------------------------------
// Notes      A AMenuOS is a built-in top-level menu bar control from the operating system.
// Subclasses APopMenuOS
// UsesLibs   User32.lib
// Author(s)  Conan Reis
class AMenuOS
  {
  public:
    
  // Nested Structures
  
    enum eBreak
      {
      Break_none,
      Break_column,
      Break_colum_bar
      };
      

  // Common Methods

    AMenuOS(bool modeless_b = true);
    AMenuOS(HMENU menu_handle, bool owned_b = true);
    AMenuOS(const AMenuOS & menu);
    ~AMenuOS();
    // Future:
      // Menu resource constructor
    
  // Methods

    void  append_column_break(eBreak break_type = Break_colum_bar);
    void  append_item(const char * item_cstr_p, uint32_t id, bool enable = true, bool checked = false, bool default_on_abort = false);
    void  append_separator();
    void  append_submenu(APopMenuOS * submenu_p, const char * submenu_cstr_p, bool enable = true, bool checked = false);
    void  remove_item(uint32_t id);
    void  enable_item(uint32_t id, bool enable = true);
    void  enable_modeless(bool modeless_b = true);
    HMENU get_handle() const              { return m_menu_handle; }
    uint32_t  get_length() const;
    bool  is_item_enabled(uint32_t id) const;
    bool  is_modeless() const;
    bool  is_valid() const                { return (m_menu_handle != nullptr); }
    void  set_item_bitmap(uint id, HBITMAP bitmap_handle);
    void  set_item_text(uint item_id, AString text);
    void  set_owned(bool owned_b = true)  { m_owned_b = owned_b; }

    // Check Mark Methods

      void check_item(uint32_t id, bool checked = true);
      void check_item_toggle(uint32_t id);
      bool is_item_checked(uint32_t id) const;

    // Future:
      //void insert_submenu(uint32_t id, const AMenuOS & submenu, const AString & submenu_str, bool enable = true);
      //void set_item(-);
      //void remove_item(uint32_t id);
      // Load menu from resource?

    // Window Methods
    // - defined here rather than in AWindow since AMenuOS knows of AWindow and not the
    //   other way around.

      void           set_menu_bar(AWindow * win_p);
      static AMenuOS get_menu_bar(const AWindow & win);
      static void    redraw_menu_bar(const AWindow & win);

  protected:
  // Data Members

    HMENU m_menu_handle;
    bool  m_owned_b;    // If true owned and eventually destroyed by another menu or window
  
    // Indicate if next append related call will occur on an adjacent column
    eBreak m_break_next;

  };  // AMenuOS


//=======================================================================================
// Inline Functions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Constructor
// Returns:    itself
// Author(s):   Conan Reis
inline AMenuOS::AMenuOS(
  HMENU menu_handle,
  bool  owned_b // = true
  ) :
  m_menu_handle(menu_handle),
  m_owned_b(owned_b),
  m_break_next(Break_none)
  {
  }

//---------------------------------------------------------------------------------------
// Copy constructor - makes a reference to the same menu
// Returns:    itself
// Arg         menu - menu to reference (not actually copy)
// Author(s):   Conan Reis
inline AMenuOS::AMenuOS(const AMenuOS & menu) :
  m_menu_handle(menu.m_menu_handle),
  m_owned_b(true),
  m_break_next(Break_none)
  {
  }


#endif  // __AMENUOS_HPP


