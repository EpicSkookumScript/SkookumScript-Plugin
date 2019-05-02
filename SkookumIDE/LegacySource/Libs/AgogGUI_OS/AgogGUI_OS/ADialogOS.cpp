// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// Common dialogs definition module
//=======================================================================================


//=======================================================================================
// Includes
//=======================================================================================

#define _WIN32_DCOM  // Needed for CoInitializeEx()

#include <AgogGUI_OS\ADialogOS.hpp>
#include <AgogGUI_OS\AgogGUI_OS.hpp>
#include <AgogCore\AFunctionArg.hpp>
#include <AgogIO\AFile.hpp>
#include <AgogGUI\AWindow.hpp>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>  // SHBrowseForFolder() & related info
#include <shellapi.h>  // Uses ShellExecute()


//=======================================================================================
// Local Functions
//=======================================================================================

namespace
  {
  HWND g_common_parent_handle = nullptr;

  //---------------------------------------------------------------------------------------
  // Callback for ADialogOS::browse_folder()
  // Modifiers:   static
  // Author(s):   Conan Reis
  int CALLBACK browse_callback(HWND win_handle, UINT msg, LPARAM l_param, LPARAM user_data) 
    {
    switch(msg) 
      {
      case BFFM_INITIALIZED:
        {
        char   path_a[MAX_PATH];
        char * path_p = reinterpret_cast<char *>(user_data);

        if (path_p[0] == '\0')
          {
          if (::GetCurrentDirectory(MAX_PATH, path_a))
            {
            path_p = path_a;
            }
          else
            {
            path_p = nullptr;
            }
          }
        
        if (path_p)
          {
          // w_param is TRUE since a path is passed.  It would be FALSE a pidl were passed.
          ::SendMessage(win_handle, BFFM_SETSELECTION, TRUE, LPARAM(path_p));
          }
        }
        break;

      //case BFFM_SELCHANGED: 
      //  // Set the status window to the currently selected path.
      //  if (::SHGetPathFromIDList(LPITEMIDLIST(l_param), path_a))
      //    {
      //    ::SendMessage(win_handle, BFFM_SETSTATUSTEXT, 0, LPARAM(path_a));
      //    }
      //  break;
      }
    return 0;
    }


  } // End unnamed namespace


//=======================================================================================
// ADialogOS Functions
//=======================================================================================

namespace ADialogOS
  {

  //---------------------------------------------------------------------------------------
  // Modifiers:   static
  // Author(s):   Conan Reis
  uint flags_to_type(uint flags)
    {
    uint type = 0u;

    //MB_APPLMODAL = 0  - The user must respond to the message box before continuing work in the window identified by the hWnd parameter. However, the user can move to the windows of other threads and work in those windows.  Depending on the hierarchy of windows in the application, the user may be able to move to other windows within the thread. All child windows of the parent of the message box are automatically disabled, but popup windows are not.  MB_APPLMODAL is the default if neither MB_SYSTEMMODAL nor MB_TASKMODAL is specified.
    //MB_SYSTEMMODAL    - Same as MB_APPLMODAL except that the message box has the WS_EX_TOPMOST style. Use system-modal message boxes to notify the user of serious, potentially damaging errors that require immediate attention (for example, running out of memory). This flag has no effect on the user's ability to interact with windows other than those associated with hWnd.
    //MB_TASKMODAL      - Same as MB_APPLMODAL except that all the top-level windows belonging to the current thread are disabled if the hWnd parameter is nullptr. Use this flag when the calling application or library does not have a window handle available but still needs to prevent input to other windows in the calling thread without suspending other threads.

    //MB_SETFOREGROUND  - The message box becomes the foreground window. Internally, the system calls the SetForegroundWindow function for the message box.
    //MB_TOPMOST        - The message box is created with the WS_EX_TOPMOST window style.

    if (flags & (Flag_disable_wins | Flag_disable_win))
      {
      type |= MB_TASKMODAL;
      }

    if (flags & Flag_topmost)
      {
      type |= MB_TOPMOST;
      }

    return type;
    }

  //---------------------------------------------------------------------------------------
  // Modifiers:   static
  // Author(s):   Conan Reis
  uint icon_to_type(ADialogOS::eIcon icon)
    {
    switch (icon)
      {
      case ADialogOS::Icon_info:
        // An icon consisting of a lowercase letter i in a circle appears in the message box.
        return MB_ICONINFORMATION;

      case ADialogOS::Icon_query:
        // A question-mark icon appears in the message box.
        return MB_ICONQUESTION;

      case ADialogOS::Icon_warning:
        // An exclamation-point icon appears in the message box.
        return MB_ICONWARNING;

      case ADialogOS::Icon_error:
        // A stop-sign icon appears in the message box.
        return MB_ICONERROR;
      }

    return 0u;
    }

  //---------------------------------------------------------------------------------------
  // Modifiers:   static
  // Author(s):   Conan Reis
  inline HWND use_common_parent(uint flags)
    {
    return (flags & ADialogOS::Flag_disable_win)
      ? g_common_parent_handle
      : nullptr;
    }

  //---------------------------------------------------------------------------------------
  // Sets the common parent window shared by the common dialogs.  A parent
  //             window is not necessary, but dialogs will center on their parent window if
  //             one is specified.  This function is used to reduce the number of arguments
  //             that are passed to the dialog functions.
  // Arg         AWindow * win_p - common parent window or nullptr
  // Modifiers:   static
  // Author(s):   Conan Reis
  void set_common_parent(AWindow * win_p)
    {
    g_common_parent_handle = win_p ? win_p->get_os_handle() : nullptr;
    }

  //---------------------------------------------------------------------------------------
  // Sets the common parent window shared by the common dialogs.  A parent
  //             window is not necessary, but dialogs will center on their parent window if
  //             one is specified.  This function is used to reduce the number of arguments
  //             that are passed to the dialog functions.
  // Arg         AWindow * win_p - common parent window or nullptr
  // Modifiers:   static
  // Author(s):   Conan Reis
  void set_common_parent_handle(HWND parent_handle)
    {
    g_common_parent_handle = parent_handle;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Info & Confirmation Dialogs
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  //---------------------------------------------------------------------------------------
  // Creates an information dialog with an 'OK' button.
  // The dialog is modal - i.e. does not return until user makes a choice.
  // 
  // Params:
  //   message_p: Info to the user explaining what has/will occur.
  //   title_p:   Title string for the dialog.
  //   flags:     see ADialogOS::eFlag
  //   icon:      see ADialogOS::eIcon
  //   
  // Author(s): Conan Reis
  void info(
    const char * message_p,
    const char * title_p, // = "Info"
    uint         flags,   // = Flag_disable_wins
    eIcon        icon     // = Icon_info
    )
    {
    ::MessageBox(
      use_common_parent(flags),
      message_p,
      title_p,
      MB_OK | flags_to_type(flags) | icon_to_type(icon));
    }

  //---------------------------------------------------------------------------------------
  // Creates an information dialog with 'OK' and 'Cancel' buttons.  The dialog
  //             is modal - i.e. does not return until user makes a choice.
  // Returns:    true if 'OK' pressed, false if canceled.
  // Arg         message_p - Info to the user explaining what has/will occur.
  // Arg         title_p - Title string for the dialog.
  // Arg         flags - see ADialogOS::eFlag
  // Arg         icon - see ADialogOS::eIcon
  // Author(s):   Conan Reis
  bool info_abort(
    const char * message_p,
    const char * title_p, // = "Info"
    uint          flags,   // = Flag_disable_wins
    eIcon        icon     // = Icon_info
    )
    {
    return (::MessageBox(
      use_common_parent(flags),
      message_p,
      title_p,
      MB_OKCANCEL | flags_to_type(flags) | icon_to_type(icon)) == IDOK);
    }

  //---------------------------------------------------------------------------------------
  // Creates a confirmation dialog with 'Yes' and 'No' buttons.
  // The dialog is modal - i.e. does not return until user makes a choice.
  // 
  // Returns: `true` if 'OK' pressed, `false` if 'No' pressed.
  // Params:
  //   message_p: Info to the user explaining what has/will occur.
  //   title_p:   Title string for the dialog.
  //   flags:     see ADialogOS::eFlag
  //   icon:      see ADialogOS::eIcon
  //   
  // Author(s): Conan Reis
  bool confirm(
    const char * message_p,
    const char * title_p, // = "Confirm?"
    uint         flags,   // = Flag_disable_wins
    eIcon        icon     // = Icon_query
    )
    {
    return (::MessageBox(
      use_common_parent(flags),
      message_p,
      title_p,
      MB_YESNO | flags_to_type(flags) | icon_to_type(icon)) == IDYES);
    }

  //---------------------------------------------------------------------------------------
  // Creates a confirmation dialog with 'Yes', 'No', and 'Cancel' buttons.  The
  //             dialog is modal - i.e. does not return until user makes a choice.
  // Returns:    AConfirm_abort, AConfirm_no, or AConfirm_yes.
  // Arg         message_p - Info to the user explaining what has/will occur.
  // Arg         title_p - Title string for the dialog.
  // Arg         flags - see ADialogOS::eFlag
  // Arg         icon - see ADialogOS::eIcon
  // Author(s):   Conan Reis
  eAConfirm confirm_or_cancel(
    const char * message_p,
    const char * title_p, // = "Confirm?"
    uint         flags,   // = Flag_disable_wins
    eIcon        icon     // = Icon_query
    )
    {
    int result = ::MessageBox(
      use_common_parent(flags),
      message_p,
      title_p,
      MB_YESNOCANCEL | flags_to_type(flags) | icon_to_type(icon));

    return (result == IDYES)
      ? AConfirm_yes
      : ((result == IDNO)
        ? AConfirm_no
        : AConfirm_abort);
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // File & Folder Dialogs
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  //---------------------------------------------------------------------------------------
  // Brings up a dialog with the title "Browse for Folder" where the user can
  //             select a folder/directory or cancel.  It is a "new style" type dialog that
  //             can be resized, has drag-and-drop capability, has reordering, can delete,
  //             has shortcut menus, and other features.
  // Returns:    true if folder selected, false if not
  // Arg         path_p - path of selected folder.  It is not changed if a folder is not
  //             selected.  A ADirectory object can easily be constructed from this path
  //             so that more can be done with it.
  // Arg         message_p - message text to display between the title bar and the folder
  //             tree.  If the text of one line is longer than the dialog is wide, it will
  //             wrap to the next line.  Newlines (/n) will put a hard break in a line.
  //             Only the first three lines of text are shown.
  // Arg         path_start_p - This is an optional pre-selected starting folder.  If it
  //             is empty ("") then the current directory will be used.  If it is nullptr
  //             then no folder is preselected.  See ADirectory and AApplication for
  //             handy starting directories.
  // Arg         allow_new_folders - if true a "Make New Folder" button is added to the
  //             dialog, if false it is not.
  // Author(s):   Conan Reis
  bool browse_folder(
    AString *    path_p,
    const char * message_p,        // = "Please select a folder."
    const char * path_start_p,     // = ""
    bool         allow_new_folders // = true
    )
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Initialize Dialog

    AgogGUI_OS::initialize_com();

    BROWSEINFO info;

    info.hwndOwner = g_common_parent_handle;
    info.lpszTitle = message_p;  // Message displayed above the tree view control in the dialog box (not the title).
    info.pidlRoot  = nullptr;       // Location of the root folder from which to start browsing.  nullptr for desktop.  ::SHILCreateFromPath() can convert a path to an ITEMIDLIST but it is only Win2000+.  SHGetDesktopFolder() and ParseDisplayName() must be used for WinMe-.

    // Dialog Callback
    if (path_start_p)
      {
      info.lpfn   = browse_callback;
      info.lParam = LPARAM(path_start_p);
      }
    else
      {
      info.lpfn   = nullptr;
      info.lParam = 0;
      }

    // Output Values
    info.pszDisplayName = nullptr;  // Buffer to receive the display name ("Temp" rather than "C:\Temp" etc.) of the folder selected.  Can be nullptr
    info.iImage         = 0;     // Variable to receive the image associated with the selected folder. The image is specified as an index to the system image list.

    // Flags specifying the options for the dialog box.

    // BIF_NEWDIALOGSTYLE     - Use the new user interface. Setting this flag provides the user with a larger dialog box that can be resized.  OleInitialize or CoInitialize must be called before calling SHBrowseForFolder.
    // BIF_RETURNONLYFSDIRS   - If the user selects folders that are not part of the file system (like "My Computer"), the OK button is grayed.
    // BIF_BROWSEINCLUDEFILES - The browse dialog box will display files as well as folders.
    // BIF_NONEWFOLDERBUTTON  - Do not include the New Folder button in the browse dialog box.
    // BIF_EDITBOX            - Include an edit control in the browse dialog box that allows the user to type the name of an item
    // BIF_VALIDATE           - If the user types an invalid name into the edit box, the browse dialog box will call the application's BrowseCallbackProc with the BFFM_VALIDATEFAILED message. This flag is ignored if BIF_EDITBOX is not specified.
    // BIF_UAHINT             - When combined with BIF_NEWDIALOGSTYLE, adds a usage hint to the dialog box in place of the edit box. BIF_EDITBOX overrides this flag.
    // BIF_SHAREABLE          - The browse dialog box can display shareable resources on remote systems. It is intended for applications that want to expose remote shares on a local system. The BIF_NEWDIALOGSTYLE flag must also be set.
    // BIF_RETURNFSANCESTORS  - If the user selects an ancestor of the root folder that is not part of the file system, the OK button is grayed.

    info.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | (allow_new_folders ? 0 : BIF_NONEWFOLDERBUTTON);


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Show Dialog
    LPITEMIDLIST item_id_list_p = ::SHBrowseForFolder(&info);

    if (!item_id_list_p)
      {
      return false;
      }


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Get Dialog Info

    bool success = false;
    char path_a[MAX_PATH];

    // $Revisit - CReis Windows 2000 and earlier may need shortcuts, etc. to be converted to their proper paths.
    if (::SHGetPathFromIDList(item_id_list_p, path_a))
      {
      if (path_p)
        {
        path_p->set_cstr(path_a, ALength_calculate, false);
        }
      success = true;
      }

    // Free Memory
    ::CoTaskMemFree(item_id_list_p);

    return success;
    }

  //---------------------------------------------------------------------------------------
  // Brings up the standard file open dialog.
  // 
  // Returns: `true` if file selected to open, `false` if not
  // 
  // Params:
  //   file_p:
  //     [in/out] In: optional suggested name of file to open and/or starting directory
  //     path.  Out: Selected file to open if one is selected. If no file is selected, it
  //     is unchanged.
  //     
  //   title_p:
  //     Title for dialog
  //     
  //   ext_filter_p:
  //     file extension filters string or nullptr for no file extension filtering.  It has
  //     a strange string format:
  //
  //       filters  := '"' {filter} '"'
  //       filter   := description null patterns null
  //       patterns := pattern {';' pattern}
  //       pattern  := < Any combination of valid file name characters not including a space and the * wildcard >
  //
  //       examples:
  //         "All Files (*.*)\0*.*\0"
  //         "C++ Module Files (cpp c)\0*.cpp;*.c\0C++ Header Files (hpp h inl)\0*.hpp;*.h;*.inl\0"
  //
  //   file_must_exist:
  //     if true when a file is selected it ensures that it exists and if it does not then
  //     it notifies the user and returns to the dialog.  If false then no extra checking
  //     is done and a non-existent file may be returned.
  //     
  // See:       AFile, ADirectory
  // Modifiers: static
  // Author(s): Conan Reis
  bool open_file(
    AFile *      file_p,
    const char * title_p,        // = "Open File"
    const char * ext_filter_p,   // = ext_filter_all_p
    bool         file_must_exist // = true
    )
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Initialize Dialog

    OPENFILENAME info;

    ::ZeroMemory(&info, sizeof(OPENFILENAME));
    info.lStructSize = sizeof(OPENFILENAME);
    info.hwndOwner   = g_common_parent_handle;
    info.lpstrTitle  = title_p;
    info.lpstrFilter = ext_filter_p;
    //info.lpstrDefExt  // Default extension


    // Starting path & file name - also receives selected path & file name
    char    file_str_a[MAX_PATH];
    char    init_path_a[MAX_PATH];
    AString file_str(file_str_a, MAX_PATH, 0u);

    if (file_p && file_p->is_file_str())
      {
      if (file_p->is_titled())
        {
        // Specify suggested file and optional starting path
        file_str.append(file_p->get_file_str());
        }
      else
        {
        // Only starting path is specified
        AString init_path(init_path_a, MAX_PATH, 0u);

        init_path.append(file_p->get_path());
        info.lpstrInitialDir = init_path_a;
        }
      }

    info.lpstrFile = file_str_a;
    info.nMaxFile  = MAX_PATH;

    // Interesting Flag Values

      // OFN_ENABLESIZING       - Enables the Explorer-style dialog box to be resized using either the mouse or the keyboard.
      // OFN_SHOWHELP           - Causes the dialog box to display the Help button.
      // OFN_NONETWORKBUTTON    - Hides and disables the Network button.
      // OFN_NOVALIDATE         - Specifies that the common dialog boxes allow invalid characters in the returned file name. 
      // OFN_NOCHANGEDIR        - Restores the current directory to its original value if the user changed the directory while searching for files.  Windows NT 4.0/2000/XP: This flag is ineffective for GetOpenFileName.
      // OFN_NODEREFERENCELINKS - Directs the dialog box to return the path and file name of the selected shortcut (.LNK) file. If this value is not specified, the dialog box returns the path and file name of the file referenced by the shortcut.
      // OFN_DONTADDTORECENT    - [Windows 2000/XP] Prevents the system from adding a link to the selected file in the file system directory that contains the user's most recently used documents
      // OFN_FORCESHOWHIDDEN    - [Windows 2000/XP] Forces the showing of system and hidden files, thus overriding the user setting to show or not show hidden files. However, a file that is marked both system and hidden is not shown.

      // OFN_HIDEREADONLY       - Hides the Read Only check box.
      // OFN_READONLY           - Causes the Read Only check box to be selected initially when the dialog box is created. This flag indicates the state of the Read Only check box when the dialog box is closed.
      // OFN_PATHMUSTEXIST      - Specifies that the user can type only valid paths and file names.
      // OFN_FILEMUSTEXIST      - Specifies that the user can type only names of existing files in the File Name entry field. If this flag is specified and the user enters an invalid name, the dialog box procedure displays a warning in a message box. If this flag is specified, the OFN_PATHMUSTEXIST flag is also used.
      // OFN_SHAREAWARE         - Specifies that if a call to the OpenFile function fails because of a network sharing violation, the error is ignored and the dialog box returns the selected file name
      // OFN_ALLOWMULTISELECT   - Specifies that the File Name list box allows multiple selections

    info.Flags = OFN_ENABLESIZING | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;

    if (file_must_exist)
      {
      info.Flags |= OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
      }


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Display Dialog

    if (!::GetOpenFileName(&info) || (info.lpstrFile[0] == '\0'))
      {
      return false;
      }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Get Dialog Results

    if (file_p)
      {
      file_str.set_length();

      // Ensure there is a extension dot
      if (info.nFileExtension == 0)
        {
        file_str.append('.');
        info.nFileExtension = WORD(file_str.get_length());
        }

      file_str.rebuffer();
      file_p->set_file_str(file_str, info.nFileOffset, info.nFileExtension - 1u);
      }

    return true;
    }

  //---------------------------------------------------------------------------------------
  // Brings up the standard file save as dialog.
  // Returns:    true if file selected to save, false if not
  // Arg         file_p - [in/out] In: optional suggested name of file to save and/or
  //             starting directory path.  Out: Selected file to save if one is selected.
  //             If no file is selected, it is unchanged.
  // Arg         title_p - Title for dialog
  // Arg         ext_filter_p - file extension filters string or nullptr for no file extension
  //             filtering.  It has a strange string format:
  //
  //               filters  := '"' {filter} '"'
  //               filter   := description null patterns null
  //               patterns := pattern {';' pattern}
  //               pattern  := < Any combination of valid file name characters not including a space and the * wildcard >
  //
  //               examples:
  //                 "All Files (*.*)\0*.*\0"
  //                 "C++ Module Files (cpp c)\0*.cpp;*.c\0C++ Header Files (hpp h inl)\0*.hpp;*.h;*.inl\0"
  //
  // Arg         overwrite_prompt - if true when a file is selected it ensures that it
  //             exists and if it does not then it notifies the user and returns to the
  //             dialog.  If false then no extra checking is done and a non-existent file
  //             may be returned.
  // See:        AFile, ADirectory
  // Modifiers:   static
  // Author(s):   Conan Reis
  bool save_file(
    AFile *      file_p,
    const char * title_p,         // = "Save File As"
    const char * ext_filter_p,    // = ext_filter_all_p
    bool         overwrite_prompt // = true
    )
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Initialize Dialog

    OPENFILENAME info;

    ::ZeroMemory(&info, sizeof(OPENFILENAME));
    info.lStructSize = sizeof(OPENFILENAME);
    info.hwndOwner   = g_common_parent_handle;
    info.lpstrTitle  = title_p;
    info.lpstrFilter = ext_filter_p;
    //info.lpstrDefExt  // Default extension


    // Starting path & file name - also receives selected path & file name
    char    file_str_a[MAX_PATH];
    char    init_path_a[MAX_PATH];
    AString file_str(file_str_a, MAX_PATH, 0u);

    if (file_p && file_p->is_file_str())
      {
      if (file_p->is_titled())
        {
        // Specify suggested file and optional starting path
        file_str.append(file_p->get_file_str());
        }
      else
        {
        // Only starting path is specified
        AString init_path(init_path_a, MAX_PATH, 0u);

        init_path.append(file_p->get_path());
        info.lpstrInitialDir = init_path_a;
        }
      }

    info.lpstrFile = file_str_a;
    info.nMaxFile  = MAX_PATH;

    // Interesting Flag Values

      // OFN_ENABLESIZING       - Enables the Explorer-style dialog box to be resized using either the mouse or the keyboard.
      // OFN_SHOWHELP           - Causes the dialog box to display the Help button.
      // OFN_NONETWORKBUTTON    - Hides and disables the Network button.
      // OFN_NOVALIDATE         - Specifies that the common dialog boxes allow invalid characters in the returned file name. 
      // OFN_NOCHANGEDIR        - Restores the current directory to its original value if the user changed the directory while searching for files.  Windows NT 4.0/2000/XP: This flag is ineffective for GetOpenFileName.
      // OFN_NODEREFERENCELINKS - Directs the dialog box to return the path and file name of the selected shortcut (.LNK) file. If this value is not specified, the dialog box returns the path and file name of the file referenced by the shortcut.
      // OFN_DONTADDTORECENT    - [Windows 2000/XP] Prevents the system from adding a link to the selected file in the file system directory that contains the user's most recently used documents
      // OFN_FORCESHOWHIDDEN    - [Windows 2000/XP] Forces the showing of system and hidden files, thus overriding the user setting to show or not show hidden files. However, a file that is marked both system and hidden is not shown.

      // OFN_PATHMUSTEXIST      - Specifies that the user can type only valid paths and file names.
      // OFN_FILEMUSTEXIST      - Specifies that the user can type only names of existing files in the File Name entry field. If this flag is specified and the user enters an invalid name, the dialog box procedure displays a warning in a message box. If this flag is specified, the OFN_PATHMUSTEXIST flag is also used.

      // OFN_CREATEPROMPT       - If the user specifies a file that does not exist, this flag causes the dialog box to prompt the user for permission to create
      // OFN_OVERWRITEPROMPT    - Causes the Save As dialog box to generate a message box if the selected file already exists.
      // OFN_NOTESTFILECREATE   - Specifies that the file is not created before the dialog box is closed. This flag should be specified if the application saves the file on a create-nonmodify network share. When an application specifies this flag, the library does not check for write protection, a full disk, an open drive door, or network protection. Applications using this flag must perform file operations carefully, because a file cannot be reopened once it is closed

    info.Flags = OFN_ENABLESIZING | OFN_NOCHANGEDIR;
    if (overwrite_prompt)
      {
      info.Flags |= OFN_OVERWRITEPROMPT;
      }


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Display Dialog

    if (!::GetSaveFileName(&info) || (info.lpstrFile[0] == '\0'))
      {
      return false;
      }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Get Dialog Results

    if (file_p)
      {
      file_str.set_length();

      // Ensure there is a extension dot
      if (info.nFileExtension == 0)
        {
        file_str.append('.');
        info.nFileExtension = WORD(file_str.get_length());
        }

      file_str.rebuffer();
      file_p->set_file_str(file_str, info.nFileOffset, info.nFileExtension - 1u);
      }

    return true;
    }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // File Change Writeable Attribute Dialog
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  //---------------------------------------------------------------------------------------
  // Brings up a modal dialog box to query the user for appropriate action if
  //             seems that the file is not writable.
  // Arg         file_p - file that needs to be writable
  // Examples:   // This should be located in Agog::entry_point().
  //             AFile::set_writable_query(new AFunctionArg<AFile *>(ADialogOS::ensure_writable_dialog));
  // See:        AFile::ensure_writable_query(), AFile::set_writable_query()
  // Author(s):   Conan Reis
  void ensure_writable_dialog(AFile * file_p)
    {
    eAFileAttr attr        = file_p->get_attributes();
    bool       query       = true;
    char *     attr_cstr_p = nullptr;

    switch (attr & AFileAttr_h_r)
      {
      case AFileAttr_none:
        query = false;
        break;

      case AFileAttr_read_only:
        attr_cstr_p = "  read only";
        break;

      case AFileAttr_hidden:
        attr_cstr_p = "  hidden";
        break;

      case AFileAttr_h_r:
        attr_cstr_p = "  read only\n  hidden";
        break;
      }

    if (query)
      {
      AString query_str(nullptr, 1024u, 0u);

      // $Revisit - CReis Should not ask to clear attributes if on non-writable media such as a CD/DVD
      query_str.format(
        "Write access is needed for the file:\n  %s\n\n"
        "It currently has the following attributes set:\n%s\n\n",
        file_p->get_file_str().as_cstr(),
        attr_cstr_p);

      query_str.append(
        "Attempt to clear necessary file attributes?\n\n"
        "Select 'Yes' to have the program attempt to clear these file attributes and then reattempt write access.\n\n"
        "Select 'No' to reattempt file write access.  If there are any steps that you would like to perform yourself "
        "to enable write access (such as checking out the file with Version Control or changing settings in File "
        "Explorer), they should be performed now and *then* press the 'No' button to reattempt access.\n\n"
        "[If write access cannot be obtained for the file, the write operation will be skipped.]");

      if (confirm(
        query_str,
        "Access Rights Needed For File",
        Flag_disable_wins,
        Icon_warning))
        {
        file_p->clear_attributes(AFileAttr_h_r);
        }
      }
    else
      {
      info(
        a_cstr_format("Access is needed for the file:\n  %s\n\nIf there are any steps that are required to enable access, they should be performed now and *then* press the 'OK' button to attempt access.", file_p->get_file_str().as_cstr()),
        "Access Rights Needed For File",
        Flag_disable_wins,
        Icon_warning);
      }
    }

  //---------------------------------------------------------------------------------------
  // Brings up a modal dialog box to query the user for appropriate action if
  //             seems that the file is not writable - and offers to check it out from
  //             Perforce version control.
  // Arg         file_p - file that needs to be writable
  // Examples:   // This should be located in Agog::entry_point().
  //             AFile::set_writable_query(new AFunctionArg<AFile *>(ADialogOS::ensure_writable_dialog));
  // See:        AFile::ensure_writable_query(), AFile::set_writable_query()
  // Author(s):   Conan Reis
  void ensure_writable_p4_dialog(AFile * file_p)
    {
    eAFileAttr attr        = file_p->get_attributes();
    bool       query       = true;
    char *     attr_cstr_p = nullptr;

    switch (attr & AFileAttr_h_r)
      {
      case AFileAttr_none:
        query = false;
        break;

      case AFileAttr_read_only:
        attr_cstr_p = "  read only";
        break;

      case AFileAttr_hidden:
        attr_cstr_p = "  hidden";
        break;

      case AFileAttr_h_r:
        attr_cstr_p = "  read only\n  hidden";
        break;
      }

    if (query)
      {
      AString query_str(nullptr, 1024u, 0u);

      // $Revisit - CReis Should not ask to clear attributes if on non-writable media such as a CD/DVD
      query_str.format(
        "Write access is needed for the file:\n  %s\n\n"
        "It currently has the following attributes set:\n%s\n\n",
        file_p->get_file_str().as_cstr(),
        attr_cstr_p);

      query_str.append(
        "Attempt to check it out from Perforce version control?\n\n"
        "Select 'Yes' to check it out with Perforce & reattempt write access.\n\n"
        "Select 'No' to make it writable and then reattempt write access.\n\n"
        "Select 'Cancel' to reattempt file write access.  If there are any steps that you would like "
        "to perform yourself to enable write access (such as changing settings in File Explorer), "
        "they should be performed now and *then* press the 'Abort' button to reattempt access.\n\n"
        "[If write access cannot be obtained for the file, the write operation will be skipped.]");

      switch (confirm_or_cancel(query_str, "Check-out or Change Access Rights For File", Flag_disable_wins, Icon_warning))
        {
        case AConfirm_yes:
          file_p->p4_checkout(true);
          break;

        case AConfirm_no:
          file_p->clear_attributes(AFileAttr_h_r);
          break;
        }
      }
    else
      {
      info(
        a_cstr_format(
          "Access is needed for the file:\n  %s\n\nIf there are any steps that are required "
          "to enable access, they should be performed now and *then* press the 'OK' button "
          "to attempt access.",
          file_p->get_file_str().as_cstr()),
        "Access Rights Needed For File",
        Flag_disable_wins,
        Icon_warning);
      }
    }

  //---------------------------------------------------------------------------------------
  // Registers ensure_writable_dialog() method with AFile
  // Author(s):   Conan Reis
  void register_writable_dialog()
    {
    AFile::set_writable_query(new AFunctionArg<AFile *>(ADialogOS::ensure_writable_dialog));
    }

  //---------------------------------------------------------------------------------------
  // Registers ensure_writable_dialog() method with AFile
  // Author(s):   Conan Reis
  void register_writable_p4_dialog()
    {
    AFile::set_writable_query(new AFunctionArg<AFile *>(ADialogOS::ensure_writable_p4_dialog));
    }


  }  // End ADialogOS namespace

