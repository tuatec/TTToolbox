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

#include "TTToolboxBlueprintLibrary.h"

// Unreal Engine includes
#include "Engine/SkeletalMeshSocket.h"
#include "IKRigDefinition.h"
#include "RigEditor/IKRigController.h"

#if WITH_EDITOR
#include "HAL/PlatformApplicationMisc.h"
#endif

// function prototypes
static FString FVectorToString(const FVector& Vector);


bool UTTToolboxBlueprintLibrary::DumpVirtualBones(USkeleton* Skeleton)
{
  // check input arguments
  if (!IsValid(Skeleton))
  {
    UE_LOG(LogTemp, Error, TEXT("Called \"DumpVirtualBones\" with invalid skeleton."));
    return false;
  }

  if (Skeleton->GetVirtualBones().Num() <= 0)
  {
    UE_LOG(LogTemp, Error, TEXT("\"%s\" does not contain any virtual bones."), *(Skeleton->GetFullName()));
    return false;
  }

  // prepare string for virtual bones
  FString dumpString;
  if (Skeleton->GetVirtualBones().Num() > 1)
  {
    dumpString += "(";
  }

  uint32 count = 0;
  for (auto& virtualBone : Skeleton->GetVirtualBones())
  {
    if (count > 0)
    {
      dumpString += ",";
    }

    dumpString += "(";

    dumpString += "VirtualBoneName=\"";
    dumpString += virtualBone.VirtualBoneName.ToString();
    dumpString += "\",";

    dumpString += "SourceBoneName=\"";
    dumpString += virtualBone.SourceBoneName.ToString();
    dumpString += "\",";

    dumpString += "TargetBoneName=\"";
    dumpString += virtualBone.TargetBoneName.ToString();
    dumpString += "\"";

    dumpString += ")";

    count++;
  }

  if (Skeleton->GetVirtualBones().Num() > 1)
  {
    dumpString += ")";
  }

  // dump virtual bones
  UE_LOG(LogTemp, Log, TEXT("%s"), *dumpString);

  // copy virtual bones to the clipboard
#if WITH_EDITOR
  FPlatformApplicationMisc::ClipboardCopy(*dumpString);
#endif

  return true;
}

bool UTTToolboxBlueprintLibrary::AddVirtualBone(
  const FName& VirtualBoneName,
  const FName& SourceBoneName,
  const FName& TargetBoneName,
  USkeleton* Skeleton)
{
  // check input arguments
  if (!IsValid(Skeleton))
  {
    UE_LOG(LogTemp, Error, TEXT("Called \"AddVirtualBone\" with invalid skeleton."));
    return false;
  }

  bool isValidVirtualBone = true;
  isValidVirtualBone &= VirtualBoneName != NAME_None;
  if (VirtualBoneName == NAME_None)
  {
    UE_LOG(LogTemp, Error, TEXT("Called AddVirtualBone with invalid VirtualBoneName."));
  }

  isValidVirtualBone &= TargetBoneName != NAME_None;
  if (TargetBoneName == NAME_None)
  {
    UE_LOG(LogTemp, Error, TEXT("Called AddVirtualBone with invalid TargetBoneName."));
  }

  isValidVirtualBone &= SourceBoneName != NAME_None;
  if (SourceBoneName == NAME_None)
  {
    UE_LOG(LogTemp, Error, TEXT("Called AddVirtualBone with invalid SourceBoneName."));
  }

  if (!isValidVirtualBone)
  {
    return false;
  }

  // check if virtual bone already exists
  for (auto& virtualBone : Skeleton->GetVirtualBones())
  {
    if (virtualBone.VirtualBoneName == VirtualBoneName &&
        virtualBone.SourceBoneName == SourceBoneName &&
        virtualBone.TargetBoneName == TargetBoneName)
    {
      UE_LOG(LogTemp, Error, TEXT("virtual bone: %s, source = %s, target = %s already exists in skeleton \"%s\"."),
            *VirtualBoneName.ToString(), *SourceBoneName.ToString(), *TargetBoneName.ToString(), *(Skeleton->GetFullName()));
      return false;
    }
  }

  // try to add virtual bone
  FName newVirtualBoneName = VirtualBoneName;
  if (!Skeleton->AddNewVirtualBone(SourceBoneName, TargetBoneName, newVirtualBoneName))
  {
    UE_LOG(LogTemp, Error, TEXT("Failed to add virtual bone in skeleton \"%s\"."), *(Skeleton->GetFullName()));
    return false;
  }
  Skeleton->RenameVirtualBone(newVirtualBoneName, VirtualBoneName);

  // mark skeleton as dirty
  Skeleton->Modify();

  return true;
}

bool UTTToolboxBlueprintLibrary::DumpSockets(USkeleton* Skeleton)
{
  // check input arguments
  if (!IsValid(Skeleton))
  {
    UE_LOG(LogTemp, Error, TEXT("Called \"DumpSockets\" with invalid skeleton."));
    return false;
  }

  if (Skeleton->Sockets.Num() <= 0)
  {
    UE_LOG(LogTemp, Error, TEXT("\"%s\" does not contain any sockets."), *(Skeleton->GetFullName()));
    return false;
  }

  // prepare string for sockets
  FString dumpString;
  if (Skeleton->Sockets.Num() > 1)
  {
    dumpString += "(";
  }

  uint32 count = 0;
  for (auto socket : Skeleton->Sockets)
  {
    if (IsValid(socket))
    {
      if (count > 0)
      {
        dumpString += ",";
      }

      dumpString += "(";

      dumpString += "BoneName=\"";
      dumpString += socket->BoneName.ToString();
      dumpString += "\",";

      dumpString += "SocketName=\"";
      dumpString += socket->SocketName.ToString();
      dumpString += "\",";

      dumpString += "RelativeTransform=(Rotation=(";

      const auto rotation = socket->RelativeRotation.Quaternion();
      FString rotationString = "X=";
      rotationString += FString::SanitizeFloat(rotation.X);
      rotationString += ",Y=";
      rotationString += FString::SanitizeFloat(rotation.Y);
      rotationString += ",Z=";
      rotationString += FString::SanitizeFloat(rotation.Z);
      rotationString += ",W=";
      rotationString += FString::SanitizeFloat(rotation.W);
      dumpString += rotationString;

      dumpString += "),Translation=(";
      dumpString += FVectorToString(socket->RelativeLocation);

      dumpString += "),Scale3D=(";
      dumpString += FVectorToString(socket->RelativeScale);
      dumpString += ")))";

      count++;
    }
  }

  if (Skeleton->Sockets.Num() > 1)
  {
    dumpString += ")";
  }

  // dump sockets
  UE_LOG(LogTemp, Log, TEXT("%s"), *dumpString);

  // copy sockets to the clipboard
#if WITH_EDITOR
  FPlatformApplicationMisc::ClipboardCopy(*dumpString);
#endif

  return true;
}

bool UTTToolboxBlueprintLibrary::AddSocket(const FName& BoneName, const FName& SocketName, const FTransform& RelativeTransform, USkeleton* Skeleton)
{
  // check input arguments
  if (!IsValid(Skeleton))
  {
    UE_LOG(LogTemp, Error, TEXT("Called \"AddSocket\" with invalid skeleton."));
    return false;
  }

  if (BoneName == NAME_None)
  {
    UE_LOG(LogTemp, Error, TEXT("Called \"AddSocket\" with invalid bone name."));
    return false;
  }

  if (SocketName == NAME_None)
  {
    UE_LOG(LogTemp, Error, TEXT("Called \"AddSocket\" with invalid socket name."));
    return false;
  }

  if (UTTToolboxBlueprintLibrary::HasSocket(SocketName, Skeleton))
  {
    UE_LOG(LogTemp, Error, TEXT("\"%s\" does already contain the socket \"%s\"."), *(Skeleton->GetFullName()), *SocketName.ToString());
    return false;
  }

  // introduce the socket to the skeleton
  auto socket = NewObject<USkeletalMeshSocket>(Skeleton);
  socket->BoneName = BoneName;
  socket->SocketName = SocketName;
  socket->RelativeLocation = RelativeTransform.GetLocation();
  socket->RelativeRotation = RelativeTransform.GetRotation().Rotator();
  socket->RelativeScale = RelativeTransform.GetScale3D();
  Skeleton->Sockets.Add(socket);

  // notify the editor that the skeleton was changed
  Skeleton->Modify();

  return true;
}

bool UTTToolboxBlueprintLibrary::HasSocket(const FName& SocketName, USkeleton* Skeleton)
{
  // check input arguments
  if (!IsValid(Skeleton))
  {
    UE_LOG(LogTemp, Error, TEXT("Called \"HasSocket\" with invalid skeleton."));
    return false;
  }

  if (SocketName == NAME_None)
  {
    UE_LOG(LogTemp, Error, TEXT("Called \"HasSocket\" with invalid socket name."));
    return false;
  }

  // check if socket is already present
  for (auto socket : Skeleton->Sockets)
  {
    if (IsValid(socket))
    {
      if (socket->SocketName == SocketName)
      {
        return true;
      }
    }
  }

  return false;
}

bool UTTToolboxBlueprintLibrary::DumpSkeletonCurveNames(USkeleton* Skeleton)
{
  // check input arguments
  if (!IsValid(Skeleton))
  {
    UE_LOG(LogTemp, Error, TEXT("Called \"DumpSkeletonCurveNames\" with invalid skeleton."));
    return false;
  }

  // get curves names
  auto curveMapping = Skeleton->GetSmartNameContainer(USkeleton::AnimCurveMappingName);
  if (!curveMapping)
  {
    UE_LOG(LogTemp, Error, TEXT("Failed to get curve mapping in \"DumpSkeletonCurveNames\". Please contact the author of this plugin."));
    return false;
  }

  // get curve names
  TArray<FName> curveNames;
  curveMapping->Iterate([&curveNames](const FSmartNameMappingIterator& Iterator)
    {
      FName curveName;
      if (Iterator.GetName(curveName))
      {
        curveNames.Add(curveName);
      }
      else
      {
        UE_LOG(LogTemp, Error, TEXT("An error occured in \"DumpSkeletonCurveNames\" while getting a curve name. Please contact the author of this plugin."));
      }
    }
  );

  // prepare dump string
  FString dumpString = "(";

  uint32 count = 0;
  for (auto curveName : curveNames)
  {
    if (count > 0)
    {
      dumpString += ",";
    }

    dumpString += "\"";
    dumpString += curveName.ToString();
    dumpString += "\"";

    count++;
  }

  dumpString += ")";

  // dump curve names
  UE_LOG(LogTemp, Log, TEXT("%s"), *dumpString);

#if WITH_EDITOR
  FPlatformApplicationMisc::ClipboardCopy(*dumpString);
#endif

  return true;
}

bool UTTToolboxBlueprintLibrary::CheckForMissingCurveNames(const TArray<FName>& CurveNamesToCheck, USkeleton* Skeleton)
{
  // check input arguments
  if (!IsValid(Skeleton))
  {
    UE_LOG(LogTemp, Error, TEXT("Called \"CheckForMissingCurveNames\" with invalid skeleton."));
    return false;
  }

  // get curve names from target skeleton
  auto curveMapping = Skeleton->GetSmartNameContainer(USkeleton::AnimCurveMappingName);
  if (!curveMapping)
  {
    UE_LOG(LogTemp, Error, TEXT("Failed to get curve mapping in \"DumpSkeletonCurveNames\". Please contact the author of this plugin."));
    return false;
  }

  TArray<FName> skeletonCurveNames;
  curveMapping->Iterate([&skeletonCurveNames](const FSmartNameMappingIterator& Iterator)
    {
      FName curveName;
      if (Iterator.GetName(curveName))
      {
        skeletonCurveNames.Add(curveName);
      }
      else
      {
        UE_LOG(LogTemp, Error, TEXT("An error occured in \"CheckForMissingCurveNames\" while getting a curve name. Please contact the author of this plugin."));
      }
    }
  );

  // check if curves are missing in the target skeleton
  bool hasNoMissingCurveNames = true;
  for (auto& curveName : CurveNamesToCheck)
  {
    if (skeletonCurveNames.Find(curveName) == INDEX_NONE)
    {
      if (hasNoMissingCurveNames)
      {
        UE_LOG(LogTemp, Error, TEXT("The following curves are missing in skeleton \"%s\":"), *(Skeleton->GetFullName()));
      }

      UE_LOG(LogTemp, Error, TEXT("  %s"), *curveName.ToString());

      hasNoMissingCurveNames = false;
    }
  }

  return hasNoMissingCurveNames;
}

static FName retrieveContainerNameForCurve(const UAnimSequenceBase* AnimaSequenceBase, const FName& CurveName)
{
  checkf(AnimaSequenceBase != nullptr, TEXT("Invalid Animation Sequence ptr"));
  const FName Names[(int32)ESmartNameContainerType::SNCT_MAX] = { USkeleton::AnimCurveMappingName, USkeleton::AnimTrackCurveMappingName };
  for (int32 Index = 0; Index < (int32)ESmartNameContainerType::SNCT_MAX; ++Index)
  {
    const FSmartNameMapping* CurveMapping = AnimaSequenceBase->GetSkeleton()->GetSmartNameContainer(Names[Index]);
    if (CurveMapping && CurveMapping->Exists(CurveName))
    {
      return Names[Index];
    }
  }

  return NAME_None;
}

bool UTTToolboxBlueprintLibrary::CopyAnimMontageCurves(UAnimMontage* SourceAnimMontage, UAnimMontage* TargetAnimMontage)
{
  // check input arguments
  if (!IsValid(SourceAnimMontage) || !IsValid(TargetAnimMontage))
  {
    UE_LOG(LogTemp, Error, TEXT("Called \"CopyAnimMontageCurves\" with invalid SourceAnimMontage or TargetAnimMontage."));
    return false;
  }

  // curves should be copied over so all existing curves need to be removed
  auto& targetController = TargetAnimMontage->GetController();
  targetController.RemoveAllCurvesOfType(ERawCurveTrackTypes::RCT_Float);

  for (auto& sourceCurve : SourceAnimMontage->GetCurveData().FloatCurves)
  {
    FSmartName curveSmartName;

    const FName containerName = retrieveContainerNameForCurve(TargetAnimMontage, sourceCurve.Name.DisplayName);
    if (!TargetAnimMontage->GetSkeleton()->GetSmartNameByName(containerName, sourceCurve.Name.DisplayName, curveSmartName))
    {
      UE_LOG(LogTemp, Error, TEXT("Failed to get smart name for curve %s"), *sourceCurve.Name.DisplayName.ToString());
      continue;
    }

    const FAnimationCurveIdentifier curveId(curveSmartName, ERawCurveTrackTypes::RCT_Float);
    targetController.AddCurve(curveId);
    targetController.SetCurveKeys(curveId, sourceCurve.FloatCurve.GetConstRefOfKeys());
  }

  // modify the TargetAnimMontage

  return true;
}

bool UTTToolboxBlueprintLibrary::DumpIKChains(const UIKRigDefinition* IKRigDefinition)
{
  // check input arguments
  if (!IsValid(IKRigDefinition))
  {
    UE_LOG(LogTemp, Error, TEXT("Called \"DumpIKChains\" with invalid IKRigDefinition."));
    return false;
  }

  if (IKRigDefinition->GetRetargetChains().Num() <= 0)
  {
    UE_LOG(LogTemp, Error, TEXT("Called \"DumpIKChains\" with invalid IKRigDefinition %s, which did not provide any IK chains."), *(IKRigDefinition->GetFullName()));
    return false;
  }

  // prepare the dump string
  FString dumpString;
  if (IKRigDefinition->GetRetargetChains().Num() > 1)
  {
    dumpString += "(";
  }

  // iterate over all IK chains
  uint32 count = 0;
  for (auto& boneChain : IKRigDefinition->GetRetargetChains())
  {
    if (count > 0)
    {
      dumpString += ",";
    }

    dumpString += "(";

    dumpString += "ChainName=\"";
    dumpString += boneChain.ChainName.ToString();
    dumpString += "\",";

    dumpString += "StartBone=\"";
    dumpString += boneChain.StartBone.BoneName.ToString();
    dumpString += "\",";

    dumpString += "EndBone=\"";
    dumpString += boneChain.EndBone.BoneName.ToString();
    dumpString += "\",";

    dumpString += "IKGoalName=\"";
    dumpString += boneChain.IKGoalName.ToString();
    dumpString += "\"";

    dumpString += ")";

    count++;
  }

  if (IKRigDefinition->GetRetargetChains().Num() > 1)
  {
    dumpString += ")";
  }

  // print the IK chains to the log
  UE_LOG(LogTemp, Log, TEXT("%s"), *dumpString);

  // store the IK chains in the clipboard
#if WITH_EDITOR
  FPlatformApplicationMisc::ClipboardCopy(*dumpString);
#endif

  return true;
}

bool UTTToolboxBlueprintLibrary::AddIKBoneChains(UIKRigDefinition* IKRigDefinition, const TArray<FBoneChain_BP>& BoneChains)
{
  // check input arguments
  if (!IsValid(IKRigDefinition))
  {
    UE_LOG(LogTemp, Error, TEXT("Called \"AddIKBoneChains\" with invalid IKRigDefinition."));
    return false;
  }

  auto ikRigController = UIKRigController::GetIKRigController(IKRigDefinition);
  if (!IsValid(ikRigController))
  {
    UE_LOG(LogTemp, Error, TEXT("During getting the IKRigController for %s in \"AddIKBoneChains\" failed."), *(IKRigDefinition->GetFullName()));
    return false;
  }

  // delete all existing retarget chains
  {
    //auto retargetChains = ikRigController->GetRetargetChains();
    for (auto& retargetChain : ikRigController->GetRetargetChains())
    {
      if (!ikRigController->RemoveRetargetChain(retargetChain.ChainName))
      {
        UE_LOG(LogTemp, Error, TEXT("Removing %s retarget chain in \"AddIKBoneChains\" failed."), *(IKRigDefinition->GetFullName()));
      }
    }
  }

  // add the new IK chains
  for (auto& boneChain : BoneChains)
  {
    if (IKRigDefinition->GetRetargetChainByName(boneChain.ChainName))
    {
      UE_LOG(LogTemp, Error, TEXT("Adding %s retarget chain in \"AddIKBoneChains\" failed."), *(IKRigDefinition->GetFullName()));
      continue;
    }

    ikRigController->AddRetargetChain(boneChain.ChainName, boneChain.StartBone, boneChain.EndBone);
  }

  return true;
}

bool UTTToolboxBlueprintLibrary::SetIKBoneChainGoal(UIKRigDefinition* IKRigDefinition, const FName& ChainName, const FName& GoalName)
{
  // check input arguments
  if (!IsValid(IKRigDefinition))
  {
    UE_LOG(LogTemp, Error, TEXT("Called \"SetIKBoneChainGoal\" with invalid IKRigDefinition."));
    return false;
  }

  // get the IK rig controller
  auto ikRigController = UIKRigController::GetIKRigController(IKRigDefinition);
  if (!IsValid(ikRigController))
  {
    UE_LOG(LogTemp, Error, TEXT("During getting the IKRigController for %s in \"SetIKBoneChainGoal\" failed."), *(IKRigDefinition->GetFullName()));
    return false;
  }

  // check if the ik chain is present in the IKRigDefinition
  if (!ikRigController->GetGoal(GoalName))
  {
    return false;
  }

  // set the IK goal within the IK chain
  return ikRigController->SetRetargetChainGoal(ChainName, GoalName);
}

// helper function implementations
FString FVectorToString(const FVector& Vector)
{
  FString str = "X=";
  str += FString::SanitizeFloat(Vector.X);
  str += ",Y=";
  str += FString::SanitizeFloat(Vector.Y);
  str += ",Z=";
  str += FString::SanitizeFloat(Vector.Z);

  return str;
}
