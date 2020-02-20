#include "Misc/AutomationTest.h"
#include "SkookumScriptRuntime.h"

#if WITH_EDITOR

BEGIN_DEFINE_SPEC(FSkookumScriptRuntimeGeneratorSpec, "SkookumScript.Runtime.Generator", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ApplicationContextMask)
FSkookumScriptRuntime* SkRuntime;
TSet<FName> EngineSkipClasses;
TSet<FName> ProjectSkipClasses;
END_DEFINE_SPEC(FSkookumScriptRuntimeGeneratorSpec)

bool ClassDoesNotExistCpp(FName ClassName)
{
  // Stubs may still exist, just verify that they are stubs
  if (SkClass* C = SkBrain::get_class(ASymbol::create(AString(*ClassName.ToString()))))
  {
    const uint32_t DataC = C->get_instance_data().get_length();
    const uint32_t MethodC = C->get_instance_methods().get_length();
    return DataC == 0 && MethodC == 0;
  }
  return true;
}

void FSkookumScriptRuntimeGeneratorSpec::Define()
{
  Describe("SkipClasses", [this]()
  {
    BeforeEach([this]()
    {
      SkRuntime = static_cast<FSkookumScriptRuntime *>(FModuleManager::Get().GetModule("SkookumScriptRuntime"));

      // All of the classes we should be skipping
      EngineSkipClasses = SkRuntime->get_runtime_generator()->get_target(FSkookumScriptGeneratorBase::ClassScope_engine)->m_skip_classes;
      ProjectSkipClasses = SkRuntime->get_runtime_generator()->get_target(FSkookumScriptGeneratorBase::ClassScope_project)->m_skip_classes;
    });

    It("should not exist in any BP Generated class", [this]()
    {
      // The path to our sk overlays
      const FString OverlayPath = SkRuntime->get_runtime_generator()->m_overlay_path;

      // All files in the overlay
      TArray<FString> BPGenerated;
      IFileManager::Get().FindFiles(BPGenerated, *(OverlayPath / TEXT("*.sk")), true, false);
      for (const FString& Filename : BPGenerated)
      {
        int32 FirstPeriod;
        Filename.FindChar('.', FirstPeriod);
        const FName ClassName = FName(*Filename.Left(FirstPeriod));
         
        TestFalse("SkipClasses", EngineSkipClasses.Contains(ClassName) || ProjectSkipClasses.Contains(ClassName));
      }
    });

    It("should not exist in any C++ Generated class", [this]()
    {
      for (const FName& ClassName : EngineSkipClasses)
      {
        TestTrue(FString::Printf(TEXT("%s does not exist"), *ClassName.ToString()), ClassDoesNotExistCpp(ClassName));
      }

      for (const FName& ClassName : ProjectSkipClasses)
      {
        TestTrue(FString::Printf(TEXT("%s does not exist"), *ClassName.ToString()), ClassDoesNotExistCpp(ClassName));
      }
    });
  });
}
#endif