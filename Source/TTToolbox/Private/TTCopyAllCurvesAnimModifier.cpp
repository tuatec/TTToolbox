// The MIT License (MIT)
// ---------------------
// 
// Copyright 2022 Achim Turan (https://www.instagram.com/tuatec/)
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
// associated documentation files (the "Software"), to deal in the Software without restriction,
// including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all copies or
// substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
// THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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

  if (ReplaceExistingCurves)
  {
    UAnimationBlueprintLibrary::RemoveAllCurveData(TargetSequence);
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
