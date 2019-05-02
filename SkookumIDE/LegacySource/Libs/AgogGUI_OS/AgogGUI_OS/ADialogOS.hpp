// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// Common dialogs declaration header
//
// ##### Function descriptions located at implementations rather than declarations. #####
//=======================================================================================


#ifndef __ADIALOGOS_HPP
#define __ADIALOGOS_HPP


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogIO\AWinTypes.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

// Pre-declarations:
class AFile;
class AWindow;

namespace ADialogOS
  {
  // Nested Structures

    enum eIcon
      {
      Icon_none,
      Icon_info,
      Icon_query,
      Icon_warning,
      Icon_error
      };

    enum eFlag
      {
      Flag_none         = 0,

      Flag_disable_win  = 1 << 0,  // Prevents focus change - Disables common parent window but allows interaction with this dialog and other windows
      Flag_disable_wins = 1 << 1,  // Prevents focus change - Disables all other windows in same thread but this dialog
      Flag_topmost      = 1 << 2   // Make dialog stay on top of all windows until choice is made.  [Should only be used for critical events]
      };

  // Nested Constants

    static const char * ext_filter_all_p = "All Files (*.*)\0*.*\0";


  // Functions

    void set_common_parent(AWindow * win_p);
    void set_common_parent_handle(HWND parent_handle);

    // Message box with 'OK' [abort version also has 'Cancel']

      void info(const char * message_p, const char * title_p = "Info", uint flags = Flag_disable_wins, eIcon icon = Icon_info);
      bool info_abort(const char * message_p, const char * title_p = "Info", uint flags = Flag_disable_wins, eIcon icon = Icon_info);

    // Message box with 'Yes' and 'No' [abort version also has 'Cancel']

      bool      confirm(const char * message_p, const char * title_p = "Confirm?", uint flags = Flag_disable_wins, eIcon icon = Icon_query);
      eAConfirm confirm_or_cancel(const char * message_p, const char * title_p = "Confirm?", uint flags = Flag_disable_wins, eIcon icon = Icon_query);

    // Folder/Directory Browse Dialog

      bool browse_folder(AString * path_p, const char * message_p = "Please select a folder.", const char * path_start_p = "", bool allow_new_folders = true);

    // File Dialogs

      bool open_file(AFile * file_p, const char * title_p = "Open File", const char * ext_filter_p = ext_filter_all_p, bool file_must_exist = true);
      bool save_file(AFile * file_p, const char * title_p = "Save File As", const char * ext_filter_p = ext_filter_all_p, bool overwrite_prompt = true);

    // Can be used when trying to write to files that may or may not be writable

      void ensure_writable_dialog(AFile * file_p);
      void ensure_writable_p4_dialog(AFile * file_p);
	  void register_writable_dialog();
	  void register_writable_p4_dialog();

  }  // End ADialogOS namespace

#endif  // __ADIALOGOS_HPP


