// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// Agog Labs C++ library.
//
// AToolTipOS class declaration header
//=======================================================================================


#ifndef __ATOOLTIPOS_HPP
#define __ATOOLTIPOS_HPP


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogGUI\AWindow.hpp>
#include <AgogGUI_OS\ARichEditOS.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
// A AToolTipOS is a built-in button control from the operating system.
class AToolTipOS : public AWindow
  {
  public:

  // Nested Structures

    // Example use: `set_header("header", AToolTipOS::Icon_warning)`
    // These must match TTI_NONE ... TTI_ERROR_LARGE
    enum eIcon
      {
      Icon_none          = 0,
      Icon_info          = 1,
      Icon_warning       = 2,
      Icon_error         = 3,
      Icon_info_large    = 4,
      Icon_warning_large = 5,
      Icon_error_large   = 6,
      Icon__last
      };

    static const int  ms_link_func_max  = 10;         //  Maximum number of lines in a tooltip that have associated link function.
    static const int  ms_link_y_offset  = 32;         //  Pixel vertical offset of first line of tooltip text.

//    typedef void (WINAPI * AToolTipOSLinkFunc)(void);


  // Methods

    static void initialize();
    static void deinitialize();

    AToolTipOS(
      AWindow * parent_p,
      const AString & text,
      bool enabled = true,
      int delay_reshow = -1,                        //  TTDT_RESHOW - millisecond delay before showing next tooltip i.e. delays fading.
      const AFont & font = *AFont::ms_narrow_p,
      ARichEditOS * parent_rich_edit_p = nullptr    //  When parented by a rich editbox, can process the Ctrl+T to re-enable the tooltip. $Revisit: JStenersen
      );
    ~AToolTipOS();
    
    void enable_activate(bool activate = true);
    bool is_enabled() { return m_enabled; }
    void pop();
    void set_header(const AString & header, LONG_PTR icon_resource_id = Icon_none);
    void set_link(const AString & link_url);
    void set_text(const AString & text);
    void set_width_word_break(int width);
    void show_at_mouse();

    // Interesting messages to hook up:
    //   TTM_NEWTOOLRECT
    //   TTM_SETDELAYTIME
    //   TTM_SETMARGIN

    // Note that many AWindow methods may not work properly with AToolTipOS

  // Class Methods

    static AToolTipOS * create_attached(AWindow * parent_p, const AString & text, const AFont & font);
    static void on_link_local_message_example(AToolTipOS * tooltip_p);
    static void on_link_remote_message_example(AToolTipOS * tooltip_p);
    static void on_link_url(AToolTipOS * tooltip_p);
    static void on_link_disable(AToolTipOS * tooltip_p);

    int   set_link_funct(uint link_func_index, void (*link_func)(AToolTipOS * tooltip_p));
    void  set_parent_rich_edit(ARichEditOS * parent_p) {}

  protected:
  // Event Methods

    virtual void on_tooltip_link() override;
    virtual bool on_control_event(NMHDR * info_p, LRESULT * result_p) override;
//    void set_enable(bool enable = true) { m_enabled = enable; }

    //virtual bool on_control_event_standard(uint32_t code);

  // Internal Data Members

    // If non-empty will open url/file in associated application

    bool                m_enabled;
    int                 m_delay_reshow;         //  Fade delay of the tooltip in milliseconds, See TTDT_RESHOW.
    AString             m_link_url; 
    ARichEditOS       * m_parent_rich_edit_p;
    int                 m_link_func_count = 0;
    void                (* m_link_func[ms_link_func_max])(AToolTipOS * tooltip_p);

  // Class Data

    static AMessageTargetClass * ms_default_class_p;

  };  // AToolTipOS


//=======================================================================================
// Inline Functions
//=======================================================================================


#endif  // __ATOOLTIPOS_HPP


