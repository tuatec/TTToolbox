


#include "TTCopyAllCurvesAnimModifier.h"

// Unreal Engine includes
#include "Animation/AnimSequence.h"

void UTTCopyAllCurvesAnimModifier::OnApply_Implementation(UAnimSequence* TargetSequence)
{
  // check input arguments
  if (!IsValid(TargetSequence) || !IsValid(SourceSequence))
  {
    UE_LOG(LogTemp, Error, TEXT("Called \"OnApply_Implementation\" without valid \"TargetSequence\" or valid \"SourceSequence\"."));
    return;
  }

  // copy all curves to the target anim sequence
  for (auto& sourceCurve : SourceSequence->GetCurveData().FloatCurves)
  {
    // remove the curve if it already exists ==> overwrite
    if (UAnimationBlueprintLibrary::DoesCurveExist(TargetSequence, sourceCurve.Name.DisplayName, ERawCurveTrackTypes::RCT_Float))
    {
      UAnimationBlueprintLibrary::RemoveCurve(TargetSequence, sourceCurve.Name.DisplayName);
    }

    // introduce the curve
    UAnimationBlueprintLibrary::AddCurve(TargetSequence, sourceCurve.Name.DisplayName);

    // prepare needed objects to set the curve keys
    const FName containerName = UAnimationBlueprintLibrary::RetrieveContainerNameForCurve(TargetSequence, sourceCurve.Name.DisplayName);
    if (containerName == NAME_None)
    {
      UE_LOG(LogTemp, Error, TEXT("Failed to get container name for curve %s aborting copy curves to %s"), *sourceCurve.Name.DisplayName.ToString(), *TargetSequence->GetName());
      return;
    }

    const FSmartName curveSmartName = UAnimationBlueprintLibrary::RetrieveSmartNameForCurve(TargetSequence, sourceCurve.Name.DisplayName, containerName);
    const FAnimationCurveIdentifier curveId(curveSmartName, ERawCurveTrackTypes::RCT_Float);

    // transfer curve keys
    TargetSequence->GetController().SetCurveKeys(curveId, sourceCurve.FloatCurve.GetConstRefOfKeys());
  }
}
