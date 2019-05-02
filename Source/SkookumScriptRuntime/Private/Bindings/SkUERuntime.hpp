// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

//=======================================================================================
// SkookumScript Plugin for Unreal Engine 4
//
// SkookumScript Runtime Hooks for Unreal - Input/Output Init/Update/Deinit Manager
//=======================================================================================

#pragma once

//=======================================================================================
// Includes
//=======================================================================================

#include "../SkookumScriptListenerManager.hpp"
#include "SkUEReflectionManager.hpp"

#include "HAL/Platform.h"  // Set up base types, etc for the platform

#include <SkookumScript/SkRuntimeBase.hpp>
#include <SkookumScript/SkParser.hpp>

//---------------------------------------------------------------------------------------

class ISkookumScriptRuntimeEditorInterface;
class SkUEBindingsInterface;

//=======================================================================================
// Global Structures
//=======================================================================================

//---------------------------------------------------------------------------------------
// SkookumScript Runtime Hooks for Unreal
// - Input/Output Init/Update/Deinit Manager
class SkUERuntime :
  public SkRuntimeBase
  {
  public:

    static SkUERuntime * get_singleton() { return static_cast<SkUERuntime *>(SkRuntimeBase::ms_singleton_p); }

  // Methods

    SkUERuntime();
    ~SkUERuntime() {}

    void startup();
    void shutdown();

    void ensure_static_ue_types_registered();

    // Script Loading / Binding

      const FString & get_compiled_path() const;
      bool            content_file_exists(const TCHAR * file_name_p, FString * folder_path_p) const;

      bool load_and_bind_compiled_scripts(bool is_hot_reload = false, bool ensure_atomics = true, SkClass ** ignore_classes_pp = nullptr, uint32_t ignore_count = 0u);
      bool load_compiled_scripts();
      void bind_compiled_scripts(bool is_hot_reload = false, bool ensure_atomics = true, SkClass ** ignore_classes_pp = nullptr, uint32_t ignore_count = 0u);

      void sync_all_reflected_from_sk();
      void sync_all_reflected_to_ue(bool is_final);

    // Overridden from SkRuntimeBase

      // Binary Serialization / Loading Overrides

        virtual bool             is_binary_hierarchy_existing() override;
        virtual void             on_binary_hierarchy_path_changed() override;
        virtual SkBinaryHandle * get_binary_hierarchy() override;
        virtual SkBinaryHandle * get_binary_class_group(const SkClass & cls) override;
        virtual void             release_binary(SkBinaryHandle * handle_p) override;

        #if defined(A_SYMBOL_STR_DB_AGOG)  
          virtual SkBinaryHandle * get_binary_symbol_table() override;
        #endif

      // Flow Methods

        virtual void on_bind_routines() override;
        virtual void on_initialization_level_changed(SkookumScript::eInitializationLevel from_level, SkookumScript::eInitializationLevel to_level);

      // Accessors

        bool                                   is_initialized() const             { return m_is_initialized; }
        bool                                   is_compiled_scripts_bound() const  { return m_is_compiled_scripts_bound; }
        bool                                   is_compiled_scripts_loaded() const { return m_is_compiled_scripts_loaded; }
        bool                                   have_game_module() const           { return m_have_game_module; }

        SkookumScriptListenerManager *         get_listener_manager()                 { return &m_listener_manager; }
        SkUEReflectionManager *                get_reflection_manager()               { return &m_reflection_manager; }
        const SkUEReflectionManager *          get_reflection_manager() const         { return &m_reflection_manager; }
        ISkookumScriptRuntimeEditorInterface * get_editor_interface() const           { return m_editor_interface_p; }
        SkUEBindingsInterface *                get_project_generated_bindings() const { return m_project_generated_bindings_p; }

        void                                   set_project_generated_bindings(SkUEBindingsInterface * game_bindings_p);
        void                                   set_editor_interface(ISkookumScriptRuntimeEditorInterface * editor_interface_p)  { m_editor_interface_p = editor_interface_p; }

  protected:

    // Data Members

      bool                m_is_initialized;
      bool                m_is_compiled_scripts_loaded; // If compiled binaries have ever been loaded
      bool                m_is_compiled_scripts_bound;  // If on_bind_routines() has been called at least once
      bool                m_have_game_module; // If set_project_generated_bindings() was called at least once

      mutable bool        m_compiled_file_b;
      mutable FString     m_compiled_path;

      SkookumScriptListenerManager m_listener_manager;
      SkUEReflectionManager        m_reflection_manager;

      SkUEBindingsInterface *                 m_project_generated_bindings_p;
      ISkookumScriptRuntimeEditorInterface *  m_editor_interface_p;

  };  // SkUERuntime

