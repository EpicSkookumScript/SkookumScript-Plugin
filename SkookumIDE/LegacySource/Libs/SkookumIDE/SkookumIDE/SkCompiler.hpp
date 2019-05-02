// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumIDE Application
//
// SkookumScript Compiler / Evaluator & supporting classes
//=======================================================================================


#ifndef __SKCOMPILER_HPP
#define __SKCOMPILER_HPP


//=======================================================================================
// Includes
//=======================================================================================

#include <AgogCore/AMethodArg.hpp>
#include <AgogIO/AMessageTarget.hpp>
#include <AgogIO/ATimer.hpp>
#include <AgogIO/AIni.hpp>
#include <SkookumScript/SkRuntimeBase.hpp>
#include <SkookumScript/SkRemoteBase.hpp>
#include <SkookumScript/SkBrain.hpp>
#include <SkookumScript/SkClass.hpp>
#include <SkookumScript/SkParser.hpp>


//=======================================================================================
// Global Structures
//=======================================================================================

// Pre-declarations
class  ASymbol;
struct ADirEntry;
struct SkMemberInfo;
struct SkPrintInfo;
struct SkScriptEntry;  // Declared in the cpp


//---------------------------------------------------------------------------------------
enum eSkLoad
  {
  SkLoad_script,  // Load script text files
  SkLoad_binary,  // Load compiled code binary
  SkLoad_newer    // Load whichever is newest - binary or scripts
  };


//---------------------------------------------------------------------------------------
// Stores info for a SkookumScript script library. Each library contains a subset of the
// class hierarchy and is layered one over top of the other in a sequence specified by a
// project. Members with the same names are replaced with the content of successive
// overlays. This is essentially like layers in a graphics program - but with code.
struct SkOverlay
  {
  // Nested Structures

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    enum eFlag
      {
      Flag_none       = 0x0,
      Flag_applied    = 1 << 0,

      Flag__any       = Flag_none
      };

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Enumerated constants
    enum
      {
      Path_depth_any = -1
      };


  // Data Members

    // Load order - seq 1 is applied first then 2, 3, ..., n
    uint m_sequence;

    // Name of overlay - usually project or library name
    AString m_name;

    // Relative directory path where the class hierarchy is located (used for display and
    // saving/loading from config file)
    AString m_path;
    
    // Qualified directory path where the class hierarchy is located (used for actual
    // hierarchy file access)
    AString m_path_qual;

    // Nesting depth of subclass directories until flattening. This is primarily to deal
    // with the limitations of MAX_PATH.
    // - from any (-1), Object (0), Mind(1), Master(2), etc.
    // 
    // For example:
    // 
    //   -1 or 3
    //     Path: /Object/ClassA/ClassB/ClassC
    //     Object
    //       ClassA
    //         ClassB
    //           ClassC
    //       
    //   2
    //     Path: /Object/ClassA/ClassB.ClassC
    //     Object
    //       ClassA
    //         ClassB
    //         ClassB.ClassC
    //   1
    //     Path: /Object/ClassB.ClassC
    //     Object
    //       ClassA
    //       ClassA.ClassB
    //       ClassB.ClassC
    //   0
    //     Path: /ClassB.ClassC
    //     Object
    //     Object.ClassA
    //     ClassA.ClassB
    //     ClassB.ClassC
    int32_t m_path_depth;

    // Toggle indicating whether this overlay layer is to be applied or not
    bool m_apply_b;

    // Toggle indicating whether this overlay is editable
    bool m_editable_b;

  // Methods

    SkOverlay() : m_sequence(0u), m_path_depth(Path_depth_any), m_apply_b(false), m_editable_b(true) {}
    SkOverlay(uint sequence, const AString & name, const AString & path, const AString & path_qual, bool apply_b) :
      m_sequence(sequence), m_name(name), m_path(path), m_path_qual(path_qual), m_path_depth(Path_depth_any), m_apply_b(apply_b), m_editable_b(true) {}
    SkOverlay(const SkOverlay & overlay) :
      m_sequence(overlay.m_sequence), m_name(overlay.m_name), m_path(overlay.m_path), m_path_qual(overlay.m_path_qual), m_path_depth(overlay.m_path_depth), m_apply_b(overlay.m_apply_b), m_editable_b(overlay.m_editable_b) {}

    AString get_path_class(const SkClass & cls) const;
    AString get_path_member(const SkMemberInfo & member) const;

  // Comparison Methods

    bool operator==(const SkOverlay & overlay) const  { return m_sequence == overlay.m_sequence; }
    bool operator<(const SkOverlay & overlay) const   { return m_sequence < overlay.m_sequence; }

  };  // SkOverlay
 

//---------------------------------------------------------------------------------------
class SkConsoleBase
  {
  public:

    // Methods

    virtual void  log_append(const SkPrintInfo & info) = 0;
    virtual void  status_update() = 0;
    virtual bool  browse_folder(AString * path_p, const char * message_p, const char * path_start_p) = 0;
    virtual void  progress_dot(bool completed = false) = 0;
    virtual AFile get_ini_compiled_file_query(bool loading_b = true) = 0;
    virtual AFile get_ini_file_proj_startup() = 0;

    virtual void  on_error(uint error_count = 1u) = 0;
    virtual void  on_compile_complete() = 0;
    virtual void  on_reparse(SkClass * class_p = nullptr) = 0;

  };

//---------------------------------------------------------------------------------------
class SkCompiler :
  public SkRuntimeBase
  {
  friend class SkConsole;
  friend class SkClassSettings;

  public:

  // Nested Structures

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    enum eFlag
      {
      // User set flags
      Flag_show_preparse_errors  = 1 << 0,
      Flag_ensure_bindings       = 1 << 1,
      Flag_save_compiled         = 1 << 2,
      Flag_save_manifest         = 1 << 3,
      Flag_evaluate_scripts      = 1 << 4,

      // Internal Flags
      Flag_compile_only          = 1 << 16,
      Flag_pre_load_init         = 1 << 17,
      Flag_bindings_bound        = 1 << 18,
      Flag_object_ids_validate   = 1 << 19,  // Validate object IDs with deferred validation
      Flag_misordered_flattened  = 1 << 20,  // Mid-processing misordered flattened directories

      Flag__none    = 0x0,
      Flag__default = Flag__none
      };


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    enum eInit
      {
      Init_phased,    // Initialize compiler and perform compilation in phases in the background allowing other tasks in this app to be processed concurrently.
      Init_immediate  // Initialize compiler and perform all compilation phases immediately
      };

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // AEx<SkCompiler> exception id values
    enum
      {
      ErrId_nonexistent_dir = AErrId_last,  // Specified directory does not exist
      ErrId_invalid_dir                     // File is in incorrect directory location
      };


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Normal phases are:
    //   [A] determine newest -> [B] or [C]
    //     (or)
    //   [B] preparse -> parse -> [save compiled] -> [bind atomics] -> idle
    //     (or)
    //   [C] load compiled -> [bind atomics] -> idle
    enum ePhase
      {
      Phase_determine_newest,  // [Start point A] Determine whichever is newest: compiled binary (Phase_load_compiled) or scripts (Phase_preparse)
      Phase_preparse,          // [Start point B] Build class hierarchy, preparse source files, and fully parse data members
      Phase_parse,             // Parse source files - methods and coroutines
      Phase_save_compiled,
      Phase_load_compiled,     // [Start point C] 
      Phase_bind_atomics,
      Phase_idle
      };

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Used with reparse_class()
    enum eReparse
      {
      Reparse_class_ctor  = 1 << 0,  // Call class constructor(s) after successful parse
      Reparse_recurse     = 1 << 1,  // Recurse subclasses
      Reparse_warn        = 1 << 2,  // Warn if the primary class (or its subclasses) have 1 or more active instances

      Reparse__default    = Reparse_class_ctor | Reparse_recurse | Reparse_warn,
      Reparse__simple     = 0x0
      };


  // Public Class Data Members

    static SkCompiler * ms_compiler_p;

  // Common Methods

    static void initialize();
    static void deinitialize();
    static bool is_initialized();

    SkCompiler(SkConsoleBase * console_p = nullptr);
    ~SkCompiler();

  // Methods

    ePhase get_phase() const                                        { return m_phase; }

    void phases_init(eInit init_type = Init_phased);
    bool execute_file(AFile * file_p, eSkLocale locale = SkLocale_runtime);

    void calc_member_count();
    void enable_flags(uint32_t flags, bool enable = true)           { if (enable) { m_flags |= flags; } else { m_flags &= ~flags; } }
    void enable_ensure_atomics(bool enable_b = true);
    void enable_compiled_save(bool enable_b = true);
    void enable_evaluation(bool enable_b = true);
    bool is_compiled_fresh() const                                  { return const_cast<SkCompiler *>(this)->determine_compiled_fresh(false); }
    bool is_flags(uint32_t flags) const                             { return (m_flags & flags) != 0u; }
    bool is_idle() const                                            { return m_phase == Phase_idle; }

    // Overridden from SkRuntimeBase

      virtual bool             is_binary_hierarchy_existing();
      virtual void             on_binary_hierarchy_path_changed();
      virtual SkBinaryHandle * get_binary_hierarchy();
      virtual SkBinaryHandle * get_binary_class_group(const SkClass & cls);
      virtual void             release_binary(SkBinaryHandle * handle_p);

      #if defined(A_SYMBOL_STR_DB_AGOG)  
        virtual SkBinaryHandle * get_binary_symbol_table();
      #endif

  // Settings Methods

    eSkLoad get_ini_code_load();
    AString get_ini_project_name();
    bool    get_ini_project_editable();
    bool    get_ini_project_can_make_editable();
    AString get_ini_startup_mind();
    void    set_ini_startup_mind(const AString & mind_class_str);

    void set_load_type(eSkLoad load_type);
    void load_settings();

    bool    use_builtin_actor() const            { return m_use_builtin_actor; }
    ASymbol get_custom_actor_class_name() const  { return m_custom_actor_class_name; }

    // Replace with same from SkookumRuntime

      AIni & get_ini_main()                                     { return m_ini_main; }
      AIni & get_ini_project()                                  { return m_ini_proj; }
      AIni & get_ini_project_default()                          { return m_ini_proj_default; }
      AIni & get_ini_ide()                                      { return m_ini_ide; }

      bool   get_ini_strict();
      AFile  get_ini_compiled_file();
      AFile  get_ini_compiled_file_query(bool loading_b = true);
      void   set_ini_compiled_file(const AFile & file);
      void   set_ini_strict(bool strict);

      AFile get_ini_file_main();

      //---------------------------------------------------------------------------------------
      // Gets the path for the default project settings.
      // 
      // Note:
      //   When the runtime is connected to the IDE, the project file used by the runtime is
      //   loaded by the IDE if it is different than the project already loaded.
      // 
      // Returns: default project file path
      AFile get_ini_file_proj_default();

      AFile get_ini_file_proj_startup();

      AFile get_ini_file_proj_last();

      AFile get_ini_file_user();

  // File and Overlay Methods

    void load_overlays();
    void save_overlays();
    void set_overlays(APArray<SkOverlay> & overlays);
    bool have_any_editable_overlays() const;

    const APArrayFree<SkOverlay> & get_overlays() const         { return m_overlays; }

    SkOverlay *      find_member_file(const SkMemberInfo & member_info, AFile * file_p = nullptr, SkOverlay::eFlag match = SkOverlay::Flag_applied);
    SkOverlay *      find_file_overlay(const AFile & file, SkMemberInfo * info_p = nullptr, bool * topmost_p = nullptr);
    static eSkMember parse_file_member_type(const AFile & file, SkQualifier * ident_p = nullptr, bool * class_member_p = nullptr);
    static bool      parse_file_member(const AFile & file, SkMemberInfo * info_p);
    static AFile     make_qualified(const AFile & rel_file)     { return SkCompiler::ms_compiler_p->m_ini_proj.make_qualified(rel_file); }

  // Recompilation

    void              reparse();
    bool              reparse_if_stale();
    uint32_t          reparse_class(SkClass * class_p, uint32_t flags = Reparse__default);
    SkParser::eResult reparse_file_method(AFile * file_p, const ASymbol & name, SkClassUnaryBase * scope_p);
    SkParser::eResult reparse_file_coroutine(AFile * file_p, const ASymbol & name, SkClass * scope_p);
    void              load_and_validate_object_ids(SkClass * class_p = nullptr, eAHierarchy hierarchy = AHierarchy__all, eAVerbosity print_level = AVerbosity_full);
    SkClass *         get_bind_name_class() const;

  protected:
  // Internal Methods

    void phase_start(ePhase phase);
    void phase_next();
    void phases_completed();
    void progress(bool completed_pass = false);
    void update_msg_queue();
    void update_results(SkParser::eResult result);

    void     apply_overlay_files(AFunctionArgBase<AFile *> * file_func_p, const SkClass & cls, bool recurse = true, bool print_info = true);
    void     apply_overlay_files_recurse(AFunctionArgBase<AFile *> * file_func_p, const SkClass & cls, bool recurse);
    bool     determine_compiled_fresh(bool auto_progress = true);
    void     compiled_load();
    bool     compiled_save();
    void     compiled_save_demand_loaded(SkClass * class_p, AFile * file_p, AString * bin_manifest_str_p = nullptr);
    void     compiled_copy();
    void     parse_entry_newest(ADirEntry * entry_p);
    void     preparse_files();
    void     preparse_files_overlays(const SkClass & cls, bool recurse = true);
    void     preparse_file(AFile * file_p);
    void     preparse_file_method(AFile * file_p, const ASymbol & name, SkClassUnaryBase * scope_p);
    void     preparse_file_coroutine(AFile * file_p, const ASymbol & name, SkClassUnaryBase * scope_p);
    void     parse_files();
    void     parse_files_overlays(const SkClass & cls, bool recurse = true);
    void     parse_directory_class(ADirectory * dir_p);
    void     parse_directory_classes_flattened();
    void     parse_file(AFile * file_p);
    void     parse_file_class_meta(AFile * file_p, SkClass * class_p);
    bool     parse_file_object_ids(AFile * file_p, SkClass * class_p);
    void     parse_file_data_members(AFile * file_p, SkClassUnaryBase * scope_p);
    void     parse_file_method(AFile * file_p, const ASymbol & name, SkClassUnaryBase * scope_p);
    void     parse_file_coroutine(AFile * file_p, const ASymbol & name, SkClassUnaryBase * scope_p);
    void     parse_complete();
    uint32_t reparse_class_prep(SkClass * class_p, AString * class_strs_p, uint32_t flags = Reparse__default);
    uint32_t load_object_ids(SkClass * class_p, eAVerbosity print_level = AVerbosity_full);
    uint32_t load_object_ids_recurse(SkClass * class_p, eAHierarchy hierarchy = AHierarchy__all, eAVerbosity print_level = AVerbosity_full);
    uint32_t load_object_ids_generated(eAVerbosity print_level = AVerbosity_full);

    void bind_atomics();

    const char *     map_text_file(AFile * file_p, uint32_t * length_p);
    static SkClass * parse_file_class(const AFile & file);
    static void      reparse_warn();

    // Command-line Arguments

      void cmd_args_execute(const AString & cmd_str);
      bool cmd_arg_parse(const AString & cmd_str, uint32_t idx_begin, uint32_t * idx_end_p);
      void cmd_args_help();

  // Event Methods

    void on_idle();

  // Data Members

    // See eFlag
    uint32_t m_flags;

    // Reference to project file and IDE user preferences file
    AIni m_ini_main;
    AIni m_ini_ide;

    SkConsoleBase * m_console_p;

    uint32_t m_classes;
    uint32_t m_methods;
    uint32_t m_coroutines;
    uint32_t m_data_members;
    uint32_t m_pre_errors;  // Pre-parse errors
    uint32_t m_errors;
    uint32_t m_warnings;
    uint32_t m_progress_count;
    eInit    m_init_type;
    ePhase   m_phase;
    ePhase   m_phase_start;
    ATimer   m_timer;

    // Current overlay being parsed
    SkOverlay * m_current_overlay_p;

    // Project related ini - Script overlay locations & output info
    mutable AIni m_ini_proj;
    mutable AIni m_ini_proj_default; // Default ini file - might be the same as m_ini_proj

    // Flattened directories that reference classes that have not yet bee initialized.
    // See: SkCompiler::preparse_files() and 
    APArray<ADirectory> m_flattened_dirs;

    APArrayFree<SkOverlay> m_overlays;

    // Handy cached variables
    bool              m_use_builtin_actor;
    ASymbol           m_custom_actor_class_name;
    mutable SkClass * m_bind_name_class_p;

  };  // SkCompiler


//=======================================================================================
// Inline Methods
//=======================================================================================


#endif  // __SKCOMPILER_HPP

