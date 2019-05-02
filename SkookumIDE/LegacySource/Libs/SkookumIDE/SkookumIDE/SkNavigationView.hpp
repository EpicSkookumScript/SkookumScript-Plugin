// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumIDE Application
//
// SkookumScript IDE: Navigation / Go to View
//=======================================================================================


#ifndef __SKNAVIGATIONVIEW_HPP
#define __SKNAVIGATIONVIEW_HPP


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogIO/AKeyboard.hpp>
#include <AgogCore/AMethodArg.hpp>
#include <AgogCore/AObjReusePool.hpp>
#include <AgogGUI_OS/AComboBoxOS.hpp>
#include <AgogGUI_OS/AListOS.hpp>
#include <AgogGUI_OS/ATabViewOS.hpp>
#include <AgogGUI_OS/ATreeOS.hpp>
#include <SkookumScript/SkClass.hpp>
#include <SkookumIDE/SkConsole.hpp>
#include <SkookumIDE/SkSearchDialog.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

// Pre-declaration
class SkConsole;
class SkMemberView;
class SkEditView;
class SkClassBrowser;
class SkCreatePopup;


//---------------------------------------------------------------------------------------
// Notes      A ATreeInfoOS is an entry in a ATreeOS with custom info stored in it.
// See Also   ATreeItemOS
// Author(s)  Conan Reis
class SkCTInfo
  : public ATreeItemOS
  {
  public:

    SkCTInfo() : m_class_p(nullptr) {}
    virtual ~SkCTInfo() {}

    operator const ASymbol & () const  { return m_class_p->get_name(); }

  // Public Data Members

    SkClass * m_class_p;

  protected:

    friend class AObjReusePool<SkCTInfo>;

    SkCTInfo ** get_pool_unused_next() { return (SkCTInfo **)&m_class_p; } // Area in this class where to store the pointer to the next unused object when not in use

  };  // SkCTInfo


//---------------------------------------------------------------------------------------
class SkClassTree : public ATreeOS
  {
  friend class SkCreatePopup;

  public:
  // Methods

    SkClassTree(AWindow * parent_p, SkClassBrowser * browser_p);
    virtual ~SkClassTree();

    SkClass * get_selected_class() const;
    void      build_tree();
    void      set_class(SkClass * class_p, eAFlag show_meta = AFlag__toggle);
    void      ensure_visible_class(SkClass * class_p);

  protected:

  // Event Methods

    virtual LRESULT on_custom_draw(NMTVCUSTOMDRAW * info_p);
    virtual bool    on_key_press(eAKey key, bool repeated);
    virtual bool    on_focus();
    virtual void    on_selected(ATreeItemOS * item_p, ATreeItemOS * old_item_p);
    virtual void    on_removed(ATreeItemOS * item_p);

    virtual bool    on_mouse_press(eAMouse button, eAMouse buttons, const AVec2i & client_pos, bool double_click);
    virtual void    on_mouse_release(eAMouse button, eAMouse buttons, const AVec2i & client_pos);
    virtual bool    on_context_menu(const AVec2i & screen_pos);

  // Internal Methods

    void append_class(const SkClass & new_class, bool select_it = true);
    void append_classes(const SkClass & new_class, SkCTInfo * parent_p = nullptr);

  // Internal Class Methods

    static void                      pool_delete(SkCTInfo * info_p);
    static AObjReusePool<SkCTInfo> & get_pool();
    static SkCTInfo *                pool_new(const SkClass & esclass);

  // Data Members

    // If true - shows class meta info file when class selected ignoring other inputs.
    eAFlag m_show_meta_override;

    SkClassBrowser * m_browser_p;

    APSortedLogical<SkCTInfo, ASymbol> m_items;

  };  // SkClassTree


//---------------------------------------------------------------------------------------
class SkOverlayPicker : public AComboBoxOS
  {
  public:

  // Methods

    SkOverlayPicker(AWindow * parent_p);

    void populate();
    void set_overlay(SkOverlay * overlay_p)  { m_overlay_p = overlay_p; }

  // Events

    virtual void on_selected(uint row, uint row_prev);

  // Data Members

    SkOverlay * m_overlay_p;

  };  // SkOverlayPicker


//---------------------------------------------------------------------------------------
class SkCreatePopup : public AWindow
  {
  public:

  // Nested Types

    // Source to copy for new routine - see m_source_type
    enum eSourceType
      {
      SourceType_overlay,
      SourceType_superclass,
      SourceType_subclass,
      SourceType_template
      };

  // Methods

    SkCreatePopup(AWindow * parent_p, SkClassBrowser * browser_p);
    virtual ~SkCreatePopup();

    void display();
    void populate();
    void set_class(SkClass * current_class_p);
    void set_overlay(SkOverlay * overlay_p)  { m_overlays.set_overlay(overlay_p); }
    void set_scope(eSkScope scope);
    void toggle_scope();
    void toggle_display();
    void update();

    void action_create();
    void action_create_class();
    void action_create_data();
    void action_create_routine();


    // Inherited from AWindow

      virtual void hide();

  // Event Methods

    void         on_create_text_modified();

    virtual void on_sizing();
    virtual bool on_draw();
    virtual bool on_key_press(eAKey key, bool repeated);

  protected:

  // Internal Methods

    bool calc_height();

  // Data Members

    AString m_create_name;
    eSkMember m_create_type;

    // Create from source routine

      // - [selected copy member]
      // - earlier overlay: >overlay & !routine
      // - later overlay:   <overlay & !routine
      // - superclass:      !overlay & routine with superclass
      // - subclass:        !overlay & routine with subclass
      // - template:        !overlay & !routine
      eSourceType m_source_type;
      SkOverlay * m_source_overlay_p;
      SkInvokableBase * m_source_routine_p;


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Line
    AString m_description;
    AButtonOS m_btn_close;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Member Line
    int m_line2_y;
    bool m_display_class_b;
    SkClass * m_parent_class_p;
    AString m_parent_class_name;
    int m_class_width;

    SkEditSyntax m_create_text;
    AToolTipOS   m_tooltip;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Overlay Line
    AButtonOS m_btn_instance_class;
    eSkScope m_scope;
//    AToolTipOS m_btn_ic_tip;    //  Not yet implemented

    SkOverlayPicker m_overlays;
    int m_overlay_x;

    SkClassBrowser * m_browser_p;

  };  // SkCreatePopup


//---------------------------------------------------------------------------------------
class SkNavigationView : public AWindow
  {
  public:
  // Methods

    SkNavigationView(AWindow * parent_p, SkClassBrowser * browser_p);
    virtual ~SkNavigationView()                 { }

    SkClassTree & get_class_tree()              { return m_tree; }
    void          set_class(SkClass * class_p, eAFlag show_meta = AFlag__toggle)  { m_tree.set_class(class_p, show_meta); m_create_popup.update(); }
    void          rehook();

    bool          is_create_popup()             { return !m_create_popup.is_hidden(); }
    void          show_create_popup();
    void          toggle_create_popup();
    void          resize_as_needed(bool show_create_popup, int create_popup_height = ADef_int);

  // Event Methods

    //virtual bool on_draw();
    virtual void on_sizing();

  protected:

  // Data Members

    SkClassBrowser * m_browser_p;

    ATabViewOS     m_navigation_tabs;
    SkClassTree    m_tree;
    AButtonOS      m_create_btn;
    SkCreatePopup  m_create_popup;

  };  // SkNavigationView


#endif  // __SKNAVIGATIONVIEW_HPP

