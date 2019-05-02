// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// APopMenuOS class declaration header
//
// ##### Function descriptions located at implementations rather than declarations. #####
//=======================================================================================


#ifndef __APOPMENUOS_HPP
#define __APOPMENUOS_HPP


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogGUI_OS\AMenuOS.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

const uint32_t APopMenuOS_no_id = UINT32_MAX;

//---------------------------------------------------------------------------------------
// Notes      A APopMenuOS is a built-in pop-up menu control from the operating system.
//            [Currently all pop menus are modal and suspend other message targets (like
//            windows and other controls) including idle processing - it does this by
//            creating its own message loop.]
// UsesLibs   User32.lib
// Author(s)  Conan Reis
class APopMenuOS : public AMenuOS
  {
  friend class AMenuOS;

  public:

  // Common Methods

    APopMenuOS(bool modeless_b = false, bool auto_abort_b = false);
    APopMenuOS(HMENU menu_handle, bool owned_b = true);
    APopMenuOS(const APopMenuOS & pop_menu);
    
  // Methods

    uint32_t get_default_item() const;
    uint32_t get_item_id(uint32_t item_index) const;
    bool show(const AVec2i & screen_pos, HWND owner_handle, uint32_t * item_id_p = nullptr) const;
	  bool show(const AVec2i & screen_pos, const AWindow & owner, uint32_t * item_id_p = nullptr) const;

    // Inherited from AMenuOS
      //void  append_column_break(bool draw_line_b = false);
      //void  append_submenu(APopMenuOS * submenu_p, const char * submenu_cstr_p, bool enable = true);
      //void  enable_item(uint32_t id, bool enable = true);
      //void  enable_modeless(bool modeless_b = true);
      //HMENU get_handle() const;
      //uint32_t  get_length() const;
      //bool  is_item_enabled(uint32_t id) const;
      //bool  is_modeless() const;
      //bool  is_valid() const;
      //void  set_item_bitmap(uint id, HBITMAP bitmap_handle);
      //void  set_owned(bool owned_b = true);

    // Future:
      //void insert_item(-);
      //void set_radio_item(uint32_t active_id, uint32_t first_id, uint32_t last_id);

    // Window Methods
    // - defined here rather than in AWindow since AMenuOS knows of AWindow and not the
    //   other way around.

      static APopMenuOS get_win_menu(const AWindow & win);

      // Inherited from AMenuOS
        //void           set_menu_bar(AWindow * win_p);
        //static AMenuOS get_menu_bar(const AWindow & win);
        //static void    redraw_menu_bar(const AWindow & win);

  protected:
  // Data Members

  };  // APopMenuOS


//=======================================================================================
// Inline Functions
//=======================================================================================

//---------------------------------------------------------------------------------------
// Constructor
// Returns:    itself
// Author(s):   Conan Reis
inline APopMenuOS::APopMenuOS(
  HMENU menu_handle,
  bool  owned_b // = true
  ) :
  AMenuOS(menu_handle, owned_b)
  {
  }

//---------------------------------------------------------------------------------------
// Copy constructor - makes a reference to the same menu
// Returns:    itself
// Arg         pop_menu - pop menu to reference (not actually copy)
// Author(s):   Conan Reis
inline APopMenuOS::APopMenuOS(const APopMenuOS & pop_menu) :
  AMenuOS(pop_menu)
  {
  }


#endif  // __APOPMENUOS_HPP


