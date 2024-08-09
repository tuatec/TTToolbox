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
#include "Engine/AssetManager.h"

#include "Rendering/SkeletalMeshModel.h"

#include "Commandlets/CompressAnimationsCommandlet.h"

#include "Rig/IKRigDefinition.h"
#include "RigEditor/IKRigController.h"
#include "Rigs/RigHierarchyController.h"

#include "AssetRegistry/ARFilter.h"
#include "AssetRegistry/AssetRegistryModule.h"

#include "ControlRigBlueprint.h"
#include "ControlRig.h"

#include "Animation/BlendProfile.h"

#if WITH_EDITOR
#include "HAL/PlatformApplicationMisc.h"
#endif

// function prototypes
static FString FVectorToString(const FVector& Vector);
static TArray<USkeletalMesh*> getAllSkeletalMeshes(USkeleton* Skeleton);

// helper variables
static const FName gs_rootBoneName("root");


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
  dumpString += "(";

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

  dumpString += ")";

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

  // check if the source and target bones exists already in the skeleton
  bool boneMissingInSkeleton = false;
  if (Skeleton->GetReferenceSkeleton().FindBoneIndex(SourceBoneName) == INDEX_NONE)
  {
    UE_LOG(LogTemp, Error, TEXT("Skeleton \"%s\" does not provide the SourceBone \"%s\". Adding the virtual bone \"%s\" is impossible."),
      *Skeleton->GetPathName(), *SourceBoneName.ToString(), *VirtualBoneName.ToString());
    boneMissingInSkeleton = true;
  }

  if (Skeleton->GetReferenceSkeleton().FindBoneIndex(TargetBoneName) == INDEX_NONE)
  {
    UE_LOG(LogTemp, Error, TEXT("Skeleton \"%s\" does not provide the TargetBone \"%s\". Adding the virtual bone \"%s\" is impossible."),
      *Skeleton->GetPathName(), *TargetBoneName.ToString(), *VirtualBoneName.ToString());
    boneMissingInSkeleton = true;
  }

  if (boneMissingInSkeleton)
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
  TArray<FName> curveNames;
  Skeleton->GetCurveMetaDataNames(curveNames);

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
  TArray<FName> skeletonCurveNames;
  Skeleton->GetCurveMetaDataNames(skeletonCurveNames);

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

bool UTTToolboxBlueprintLibrary::HasSkeletonCurve(USkeleton* Skeleton, const FName& SkeletonCurveName)
{
    // check input arguments
    if (!IsValid(Skeleton))
    {
        UE_LOG(LogTemp, Error, TEXT("Called \"HasSkeletonCurve\" with invalid \"Skeleton\"."));
        return false;
    }

    if (SkeletonCurveName.IsNone())
    {
        UE_LOG(LogTemp, Error, TEXT("Called \"HasSkeletonCurve\" with invalid \"SkeletonCurveName\" (\"None\")."));
        return false;
    }

    // is the SkeletonCurveName already present?
    return Skeleton->GetCurveMetaData(SkeletonCurveName) != nullptr;
}

bool UTTToolboxBlueprintLibrary::DumpSkeletonBlendProfile(USkeleton* Skeleton)
{
    // check input arguments
    if (!IsValid(Skeleton))
    {
        UE_LOG(LogTemp, Error, TEXT("Called \"DumpSkeletonBlendProfile\" with invalid \"Skeleton\"."));
        return false;
    }

    // convert blend profiles to a string
    FString dumpString = "(";
    uint32 count = 0;
    FString enumString; // temporary variable for retrieving the enum name
    for (auto& blendProfile : Skeleton->BlendProfiles)
    {
        if (!blendProfile)
        {
            UE_LOG(LogTemp, Error, TEXT("Found invalid blend profile while dumping. Please create an issue here https://github.com/tuatec/TTToolbox/issues"));
            continue;
        }

        if (count > 0)
        {
            dumpString += ",(";
        }
        else
        {
            dumpString += "(";
        }

        // name
        dumpString += "\"";
#if ENGINE_MAJOR_VERSION ==	5 &&  ENGINE_MINOR_VERSION < 1
        dumpString += blendProfile->GetName();
#elif ENGINE_MAJOR_VERSION ==	5 &&  ENGINE_MINOR_VERSION >= 1
        dumpString += blendProfile.GetName();
#endif
        dumpString += "\", ";

        if (blendProfile)
        {
            dumpString += "(";
            dumpString += "BlendProfileMode=";
            // in 5.1 this was working fine but now it is deprecated and Visual Studio
            // is using the wrong overload for this enum class 'EBlendProfileMode'.
            //dumpString += UEnum::GetValueAsString<EBlendProfileMode>(blendProfile->GetMode()).Replace(TEXT("EBlendProfileMode::"), TEXT(""));
            // But the other overload with only the enum type and the FString is working fine :)
            UEnum::GetValueAsString<EBlendProfileMode>(blendProfile->GetMode(), enumString);
            dumpString += enumString.Replace(TEXT("EBlendProfileMode::"), TEXT(""));

            dumpString += ",BlendValues=(";
            uint32 boneCount = 0;
            for (auto& bone : blendProfile->ProfileEntries)
            {
                if (boneCount > 0)
                {
                    dumpString += ",";
                }

                dumpString += "(\"";
                dumpString += bone.BoneReference.BoneName.ToString();
                dumpString += "\", ";

                dumpString += FString::SanitizeFloat(bone.BlendScale);
                dumpString += ")";

                boneCount++;
            }

            dumpString += ")";
        }

        dumpString += "))";

        count++;
    }
    dumpString += ")";

    // print dump string to the output log
    UE_LOG(LogTemp, Log, TEXT("%s"), *dumpString);

#if WITH_EDITOR
    FPlatformApplicationMisc::ClipboardCopy(*dumpString);
#endif

    return true;
}

bool UTTToolboxBlueprintLibrary::AddSkeletonBlendProfile(USkeleton* Skeleton, const FName& BlendProfileName, const FTTBlendProfile_BP& BlendProfile, bool Overwrite)
{
    // check input arguments
    if (!IsValid(Skeleton))
    {
        UE_LOG(LogTemp, Error, TEXT("Called \"AddSkeletonBlendProfile\" with invalid \"Skeleton\"."));
        return false;
    }

    if (BlendProfileName.IsNone())
    {
        UE_LOG(LogTemp, Error, TEXT("Called \"AddSkeletonBlendProfile\" with invalid \"BlendProfileName\" (\"None\")."));
        return false;
    }

    // try to find a blend profile with the same name
    auto blendProfile = Skeleton->GetBlendProfile(BlendProfileName);
    if (blendProfile && !Overwrite)
    { // if a blend profile was found and does not need to be overwriten, nothing is to do here
        UE_LOG(LogTemp, Error, TEXT("The blend profile \"%s\" did already exist in Skeleton \"%s\" in case you want to overwrite the values set \"Overwrite\" to true."), *BlendProfileName.ToString(), *Skeleton->GetPathName());
        return false;
    }

    // in case a blend profile was not found and a not existing one needs to be overwriten,
    // a new blend profile is created
    if (!blendProfile)
    {
        blendProfile = Skeleton->CreateNewBlendProfile(BlendProfileName);
    }

    // fill out blend profile with it's values
    blendProfile->Mode = BlendProfile.BlendProfileMode;

    blendProfile->ProfileEntries.Empty(BlendProfile.BlendValues.Num());
    for (auto& blendEntry : BlendProfile.BlendValues)
    {
        int32 boneIndex = Skeleton->GetReferenceSkeleton().FindBoneIndex(blendEntry.Key);
        if (boneIndex == INDEX_NONE)
        {
            UE_LOG(LogTemp, Error, TEXT("The bone name \"%s\" did not exist in Skeleton \"%s\" while trying to add the blend profile \"%s\"."),
                   *blendEntry.Key.ToString(), *Skeleton->GetPathName(), *BlendProfileName.ToString());
            continue;
        }

        blendProfile->SetBoneBlendScale(blendEntry.Key, blendEntry.Value, false, true);
    }

    return true;
}

bool UTTToolboxBlueprintLibrary::AddSkeletonCurve(USkeleton* Skeleton, const FName& SkeletonCurveName)
{
    // check input arguments
    if (!IsValid(Skeleton))
    {
        UE_LOG(LogTemp, Error, TEXT("Called \"AddSkeletonCurve\" with invalid \"Skeleton\"."));
        return false;
    }

    if (SkeletonCurveName.IsNone())
    {
        UE_LOG(LogTemp, Error, TEXT("Called \"AddSkeletonCurve\" with invalid \"SkeletonCurveName\" (\"None\")."));
        return false;
    }

    // add the SkeletonCurveName
    return Skeleton->AddCurveMetaData(SkeletonCurveName);
}

bool UTTToolboxBlueprintLibrary::AddSkeletonSlotGroup(USkeleton* Skeleton, const FTTMontageSlotGroup& SlotGroup)
{
    // check input arguments
    if (!IsValid(Skeleton))
    {
        UE_LOG(LogTemp, Error, TEXT("Called \"DumpGroupsAndSlots\" with invalid \"Skeleton\"."));
        return false;
    }

    if (SlotGroup.GroupName.IsNone())
    {
        UE_LOG(LogTemp, Error, TEXT("Called \"AddSkeletonSlotGroup\" with invalid \"SlotGroup.GroupName\" (\"None\")."));
        return false;
    }

    auto slotGroup = Skeleton->FindAnimSlotGroup(SlotGroup.GroupName);
    if (!slotGroup)
    {
        (void)Skeleton->AddSlotGroupName(SlotGroup.GroupName); // do not process the return value or raise any warning
        slotGroup = Skeleton->FindAnimSlotGroup(SlotGroup.GroupName);
    }

    for(int32 ii = 0; ii < SlotGroup.SlotNames.Num(); ii++)
    {
        if (SlotGroup.SlotNames[ii].IsNone())
        {
            UE_LOG(LogTemp, Error, TEXT("During the call of \"AddSkeletonSlotGroup\" the slot group \"%s\" did contain a invalid slot name (\"None\") at index %i."), *SlotGroup.GroupName.ToString(), ii);
            continue;
        }

        slotGroup->SlotNames.AddUnique(SlotGroup.SlotNames[ii]);
    }

    Skeleton->Modify();

    return true;
}

bool UTTToolboxBlueprintLibrary::DumpGroupsAndSlots(USkeleton* Skeleton)
{
    // check input arguments
    if (!IsValid(Skeleton))
    {
        UE_LOG(LogTemp, Error, TEXT("Called \"DumpGroupsAndSlots\" with invalid \"Skeleton\"."));
        return false;
    }

    // convert blend profiles to a string
    // ((GroupName="G",SlotNames=("S1","S2")))
    FString dumpString = "(";
    uint32 count = 0;
    for (auto& group : Skeleton->GetSlotGroups())
    {
        if (count > 0)
        {
            dumpString += ",(";
        }
        else
        {
            dumpString += "(";
        }

        // name
        dumpString += "GroupName=\"";
        dumpString += group.GroupName.ToString();
        dumpString += "\"";

        uint32 slotCount = 0;
        if (group.SlotNames.Num() > 0)
        {
            dumpString += ",SlotNames=(";
        }
        for (auto& slot : group.SlotNames)
        {
            if (slotCount > 0)
            {
                dumpString += ",";
            }

            dumpString += "\"";
            dumpString += slot.ToString();
            dumpString += "\"";

            slotCount++;
        }

        if (group.SlotNames.Num() > 0)
        {
            dumpString += ")";
        }

        dumpString += ")";

        count++;
    }
    dumpString += ")";

    // print dump string to the output log
    UE_LOG(LogTemp, Log, TEXT("%s"), *dumpString);

#if WITH_EDITOR
    FPlatformApplicationMisc::ClipboardCopy(*dumpString);
#endif

    return true;
}

//! @todo @ffs check if the engine class could be used here
struct CSkeletonReferencePose
{
  CSkeletonReferencePose(const FReferenceSkeleton& ReferenceSkeleton)
    : m_referenceSkeleton(ReferenceSkeleton)
  {
    m_localSpacePoses.SetNumZeroed(m_referenceSkeleton.GetNum());
    for (int32 ii = 0; ii < m_referenceSkeleton.GetNum(); ++ii)
    {
      m_localSpacePoses[ii] = m_referenceSkeleton.GetRefBonePose()[ii];
    }

    calculateWorldSpaceTransforms();
  }

  enum class EBonePoseSpaces : uint8
  {
    // Local (bone) space 
    Local,
    // World (component) space
    World
  };

  void SetBonePose(const FName& BoneName, const FTransform& Transform, EBonePoseSpaces Space = EBonePoseSpaces::Local)
  {
    int32 boneIndex = m_referenceSkeleton.FindBoneIndex(BoneName);
    if (boneIndex == INDEX_NONE)
    {
      UE_LOG(LogTemp, Error, TEXT("The bone name \"%s\" is not present to calculate the local and world transforms. Please create an issue here https://github.com/tuatec/TTToolbox/issues."), *BoneName.ToString());
      return;
    }

    if (Space == EBonePoseSpaces::Local)
    {
      m_localSpacePoses[boneIndex] = Transform;
    }
    else
    {
      const int32 parentIndex = m_referenceSkeleton.GetParentIndex(boneIndex);
      const FTransform ParentTransformWS = parentIndex != INDEX_NONE ? m_worldSpacePoses[parentIndex] : FTransform::Identity;
      m_localSpacePoses[boneIndex] = Transform.GetRelativeTransform(ParentTransformWS);
    }

    calculateWorldSpaceTransforms();
  }

  const FTransform& GetRefBonePose(const FName& BoneName, EBonePoseSpaces Space = EBonePoseSpaces::Local)
  {
    int32 boneIndex = m_referenceSkeleton.FindBoneIndex(BoneName);
    if (boneIndex == INDEX_NONE)
    {
      return FTransform::Identity;
    }

    return Space == EBonePoseSpaces::Local ? m_localSpacePoses[boneIndex] : m_worldSpacePoses[boneIndex];
  }

private:
  void calculateWorldSpaceTransforms()
  {
    TArray<bool> processed;
    processed.SetNumZeroed(m_localSpacePoses.Num());
    m_worldSpacePoses.SetNum(m_localSpacePoses.Num());
    for (int32 ii = 0; ii < m_referenceSkeleton.GetNum(); ++ii)
    {
      const int32 ParentIndex = m_referenceSkeleton.GetParentIndex(ii);
      if (ParentIndex != INDEX_NONE)
      {
        //ensure(Processed[ii]);
        m_worldSpacePoses[ii] = m_localSpacePoses[ii] * m_worldSpacePoses[ParentIndex];
      }
      else
      {
        m_worldSpacePoses[ii] = m_localSpacePoses[ii];
      }

      processed[ii] = true;
    }
  }

  const FReferenceSkeleton& m_referenceSkeleton;
  TArray<FTransform> m_localSpacePoses;

  TArray<FTransform> m_worldSpacePoses;
};

bool UTTToolboxBlueprintLibrary::AddUnweightedBone(const TArray<FTTNewBone_BP>& NewBones, USkeleton* Skeleton)
{
  if(!IsValid(Skeleton))
  {
    UE_LOG(LogTemp, Error, TEXT("Invalid input. AddUnweightedBone was called with an invalid skeleton asset. Adding unweighted bones will be aborted."));
    return false;
  }

  if (NewBones.Num() <= 0)
  {
    UE_LOG(LogTemp, Error, TEXT("Invalid input. No new bones were given to AddUnweightedBone. Adding unweighted bones will be aborted."));
    return false;
  }

  bool errorsOccured = false;
  bool foundParent = false;
  for (auto& newBone : NewBones)
  {
    if (Skeleton->GetReferenceSkeleton().FindBoneIndex(newBone.NewBoneName) != INDEX_NONE)
    {
      UE_LOG(LogTemp, Error, TEXT("The unweighted bone \"%s\" already exists in the skeleton \"%s\"."), *newBone.NewBoneName.ToString(), *Skeleton->GetPathName());
      errorsOccured = true;
    }

    if (Skeleton->GetReferenceSkeleton().FindBoneIndex(newBone.ParentBone) != INDEX_NONE)
    {
      foundParent = true;
      UE_LOG(LogTemp, Display, TEXT("The following bone seems to be a parent bone \"%s\" for the new unweighted bone chain."), *newBone.ParentBone.ToString());
    }
    else
    {
      bool boneIsANewBone = false;
      for (auto& boneToTest : NewBones)
      {
        if (newBone.ParentBone == boneToTest.NewBoneName)
        {
          boneIsANewBone = true;
          break;
        }
      }

      if (!boneIsANewBone)
      {
        UE_LOG(LogTemp, Error, TEXT("ParentBone \"%s\" for child bone \"%s\" not found. Adding the unweighted bones is impossible as no correct parent bone setup exists."), *newBone.ParentBone.ToString(), *newBone.NewBoneName.ToString());
        errorsOccured = true;
      }
    }
  }

  if (!foundParent)
  {
    UE_LOG(LogTemp, Error, TEXT("Invalid input. No parent bone found for the new unweighted bones. Please check you configuration. Adding unweighted bones will be aborted."));
    return false;
  }

  if (errorsOccured)
  {
    UE_LOG(LogTemp, Error, TEXT("Invalid input. At least one error occured, for details see the error message(s) above. Adding unweighted bones will be aborted."));
    return false;
  }

  TArray<USkeletalMesh*> skeletalMeshes = getAllSkeletalMeshes(Skeleton);
  if (skeletalMeshes.IsEmpty())
  {
    UE_LOG(LogTemp, Error, TEXT("Aborting adding unweighted bones as no skeletal meshes found that are connected to \"%s\""), *Skeleton->GetPathName());
    return false;
  }

  // Sadly, the implementation does have some issues with wrong bone indices, 
  // see https://github.com/tuatec/TTToolbox/issues/5#issuecomment-1184052765 for the details.
  // That's why all virtual bones get removed (same state if a skeletal mesh is imported through an fbx file)
  // and later added again. This step needs to be done anyways as the bone tree needs to be regenerated, 
  // sadly again there is no public API that can trigger this. BUT! It is possible to trigger the regeneration process
  // through adding a virtual bone.
  // Long story short, adding virtual bones makes it possible to introduce unweighted bones in a save way. ;-)
  const auto savedVirtualBones = Skeleton->GetVirtualBones();
  {
    TArray<FName> virtualBoneNamesToDelete;
    for (auto& virtualBone : savedVirtualBones) {
      virtualBoneNamesToDelete.Add(virtualBone.VirtualBoneName);
    }
    if (virtualBoneNamesToDelete.Num() > 0) {
      Skeleton->RemoveVirtualBones(virtualBoneNamesToDelete);
    }
  }

  uint32 modifiedSkeletalMeshes = 0;
  //! @todo @ffs release renderer ressources
  //! It seems to be that the renderer does not need to be flushed.
  //! Still wondering a lot why it creates assertions. Mabye a future version of Unreal needs it
  //for (auto skeletalMesh : skeletalMeshes)
  //{
  //  skeletalMesh->FlushRenderState();
  //}

  for (auto skeletalMesh : skeletalMeshes)
  {
    if (Skeleton == skeletalMesh->GetSkeleton())
    {
      //! @todo @ffs release renderer ressources
      //skeletalMesh->FlushRenderState();
      //skeletalMesh->ReleaseResources();
      //skeletalMesh->ReleaseResourcesFence.Wait();
  
      for (auto& newBone : NewBones)
      {
        { // add bones to the reference skeleton of the skeletal mesh
          int32 parentBoneIndex = INDEX_NONE;
          FMeshBoneInfo meshParentBoneInfo;
          for (int32 ii = 0; ii < skeletalMesh->GetRefSkeleton().GetRawBoneNum(); ii++)
          {
            const FMeshBoneInfo meshBoneInfo = skeletalMesh->GetRefSkeleton().GetRawRefBoneInfo()[ii];
            if (meshBoneInfo.Name == newBone.ParentBone)
            {
              meshParentBoneInfo = meshBoneInfo;
              parentBoneIndex = ii;
              break;
            }
          }
  
          if (parentBoneIndex == INDEX_NONE)
          {
            UE_LOG(LogTemp, Error, TEXT("parent bone \"%s\" of the new bone \"%s\" not found in reference skeleton skipping..."), *newBone.ParentBone.ToString(), *newBone.NewBoneName.ToString());
            continue;
          }
  
          FReferenceSkeletonModifier referenceSkeletonModifier(skeletalMesh->GetRefSkeleton(), Skeleton);
          const FMeshBoneInfo newFMeshBoneInfo(newBone.NewBoneName, newBone.NewBoneName.ToString(), parentBoneIndex);
          referenceSkeletonModifier.Add(newFMeshBoneInfo, FTransform::Identity);
        }
  
        int32 LODIdx = 0;
        for (FSkeletalMeshLODModel& skeletalMeshLODModel : skeletalMesh->GetImportedModel()->LODModels)
        {
          int32 parentBoneIndex = skeletalMesh->GetRefSkeleton().FindBoneIndex(newBone.ParentBone);
          if (parentBoneIndex == INDEX_NONE)
          {
            UE_LOG(LogTemp, Warning, TEXT("During LOD adaption the parent bone \"%s\" was not present in the skeletal mesh \"%s\""),
              *newBone.ParentBone.ToString(), *skeletalMesh->GetPathName());
            continue;
          }

          int32 newBoneIndex = skeletalMesh->GetRefSkeleton().FindBoneIndex(newBone.NewBoneName);
          if (newBoneIndex == INDEX_NONE)
          {
            UE_LOG(LogTemp, Warning, TEXT("During LOD adaption the new bone \"%s\" was not present in the skeletal mesh \"%s\""),
              *newBone.NewBoneName.ToString(), *skeletalMesh->GetPathName());
            continue;
          }
        
          skeletalMeshLODModel.RequiredBones.Add(newBoneIndex);
#if ENGINE_MAJOR_VERSION ==	5 &&  ENGINE_MINOR_VERSION <= 3
          if (skeletalMesh->IsLODImportedDataBuildAvailable(LODIdx) && !skeletalMesh->IsLODImportedDataEmpty(LODIdx))
#elif ENGINE_MAJOR_VERSION ==	5 &&  ENGINE_MINOR_VERSION > 3
          if (skeletalMesh->HasMeshDescription(LODIdx))
#endif
          {
            FSkeletalMeshImportData skeletalMeshImportData;
#if ENGINE_MAJOR_VERSION ==	5 &&  ENGINE_MINOR_VERSION <= 3
            skeletalMesh->LoadLODImportedData(LODIdx, skeletalMeshImportData);
#elif ENGINE_MAJOR_VERSION ==	5 &&  ENGINE_MINOR_VERSION > 3
            if (const FMeshDescription* MeshDescription = skeletalMesh->GetMeshDescription(LODIdx))
            {
              skeletalMeshImportData = FSkeletalMeshImportData::CreateFromMeshDescription(*MeshDescription);
            }
#endif
        
            skeletalMeshImportData.RefBonesBinary[parentBoneIndex].NumChildren++;
            const SkeletalMeshImportData::FJointPos NewRootPos = { FTransform3f::Identity, 1.f, 100.f, 100.f, 100.f };
            const SkeletalMeshImportData::FBone bone = { newBone.NewBoneName.ToString(), 0, /*NumChildren*/0, parentBoneIndex, NewRootPos };
            skeletalMeshImportData.RefBonesBinary.Add(bone);

#if ENGINE_MAJOR_VERSION ==	5 &&  ENGINE_MINOR_VERSION <= 3
            skeletalMesh->SaveLODImportedData(LODIdx, skeletalMeshImportData);
#elif ENGINE_MAJOR_VERSION ==	5 &&  ENGINE_MINOR_VERSION > 3
            skeletalMesh->CommitMeshDescription(LODIdx);
#endif
          }
          else
          {
            for (auto& skelMeshSection : skeletalMeshLODModel.Sections)
            {
              skelMeshSection.BoneMap.Add(newBoneIndex);
            }
          }
        }
      }
  
      //! @todo @ffs release renderer ressources
      //skeletalMesh->PostEditChange();
      //skeletalMesh->InitResources();
  
      // the mesh got new bones and now it is necessary to merge those bones into the USkeleton asset as well
      if (!(Skeleton->MergeAllBonesToBoneTree(skeletalMesh)))
      {
        UE_LOG(LogTemp, Error, TEXT("The final step of merging all bones for the skeletal mesh \"%s\"into the bone failed. Please create an issue here https://github.com/tuatec/TTToolbox/issues."), *(skeletalMesh->GetPathName()));
      }
  
      for (auto& newBone : NewBones)
      {
        // constraint bone within the reference pose
        int32 constrainBoneIndex = skeletalMesh->GetRefSkeleton().FindBoneIndex(newBone.ConstraintBone);
        if (constrainBoneIndex == INDEX_NONE)
        {
          UE_LOG(LogTemp, Warning, TEXT("constraint bone \"%s\" was not found in the reference skeleton of skeleton asset \"%s\" applying identity transform."), *newBone.ConstraintBone.ToString(), *(Skeleton->GetPathName()));
        }
        else if (constrainBoneIndex >= skeletalMesh->GetRefSkeleton().GetRefBonePose().Num())
        {
          UE_LOG(LogTemp, Warning, TEXT("constraint bone \"%s\" index is not valid."), *newBone.ConstraintBone.ToString());
        }
        else
        {
          CSkeletonReferencePose skeletonReferencePose(skeletalMesh->GetRefSkeleton());
          const FTransform worldTransform = skeletonReferencePose.GetRefBonePose(newBone.ConstraintBone, CSkeletonReferencePose::EBonePoseSpaces::World);
          skeletonReferencePose.SetBonePose(newBone.NewBoneName, worldTransform, CSkeletonReferencePose::EBonePoseSpaces::World);
          const FTransform newBoneTransform = skeletonReferencePose.GetRefBonePose(newBone.NewBoneName);
  
          FReferenceSkeletonModifier referenceSkeletonModifier(skeletalMesh->GetRefSkeleton(), Skeleton);
          referenceSkeletonModifier.UpdateRefPoseTransform(skeletalMesh->GetRefSkeleton().FindBoneIndex(newBone.NewBoneName), newBoneTransform);
        }
      }

      // through caching reasons the USkeleton has internally a mapping table between skeletal meshes and the skeleton,
      // as new bones were added this table is not valid anymore ==> force rebuilding of that table!
      // Sadly none of these methods is exposed for plugin developers :(
      // - USkeleton::HandleVirtualBoneChanges
      // - USkeleton::RebuildLinkup
      // - USkeleton::RemoveLinkup
      //
      // But happily adding and removing virtual bones call internall USkeleton::HandleVirtualBoneChanges,
      // which should rebuild the mapping table ;-)
      FName virtualBoneName = *(NewBones[0].ParentBone.ToString() + "_delete_me");
      if (!Skeleton->AddNewVirtualBone(NewBones[0].ParentBone, NewBones[0].ParentBone, virtualBoneName))
      {
        UE_LOG(LogTemp, Error, TEXT("failed to add dirty virtual bone hack to force the rebuild of the bone mapping table of skeleton <todo-name>"));
      }
      Skeleton->RemoveVirtualBones({ virtualBoneName });
      
      skeletalMesh->PostEditChange();
      //skeletalMesh->InitResources();
      skeletalMesh->Modify();
      modifiedSkeletalMeshes++;
    }
  }

  // finally readd the virtual bones again to savely store everything
  if (savedVirtualBones.Num() > 0)
  {
    for (auto& virtualBone : savedVirtualBones)
    {
      if (!UTTToolboxBlueprintLibrary::AddVirtualBone(virtualBone.VirtualBoneName, virtualBone.SourceBoneName, virtualBone.TargetBoneName, Skeleton))
      {
        UE_LOG(LogTemp, Error, TEXT("Internal error! Failed to add virtual bone \"%s\" again please raise a issue here: https://github.com/tuatec/TTToolbox/issues."), *virtualBone.VirtualBoneName.ToString());
      }
    }
  }

  if (modifiedSkeletalMeshes > 0)
  {
    Skeleton->Modify();
  }

  return true;
}

void UTTToolboxBlueprintLibrary::RequestAnimationRecompress(USkeleton* Skeleton)
{
  // check input arguments
  if (!IsValid(Skeleton))
  {
    UE_LOG(LogTemp, Error, TEXT("Called \"RequestAnimationRecompress\" with invalid skeleton."));
    return;
  }

  TArray<FAssetData> assets;
  IAssetRegistry& assetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
  assetRegistry.GetAssetsByClass(UAnimSequence::StaticClass()->GetClassPathName(), assets);

  for (auto& asset : assets)
  {
    if (auto animSequence = Cast<UAnimSequence>(asset.GetAsset()))
    {
      if (animSequence->GetSkeleton()->GetFName() == Skeleton->GetFName())
      {
        animSequence->BeginCacheDerivedDataForCurrentPlatform();
      }
    }
  }
}

void UTTToolboxBlueprintLibrary::RequestAnimSequencesRecompression(TArray<UAnimSequence*> AnimSequences)
{
  for (auto animSequence : AnimSequences)
  {
    if (!IsValid(animSequence))
    {
      //! @todo error message
    }
    else
    {
      animSequence->BeginCacheDerivedDataForCurrentPlatform();
    }
  }
}

// the reason why we not call the official function "UAnimationBlueprintLibrary::SetAnimationInterpolationType"
// is that it does not give us the feedback that is needed, no return value ...
// But we like to use the error node of TTToolbox and don't want to check if the AnimSequence is valid
// it should be covered by the function and not by the caller. 
// Additionally, we want to show that the AnimSequence has changed ==> so we call AnimSequence->Modify() of course as well!
// Also investing these ~10 lines of code will result in a better UX in the end. ;-)
bool UTTToolboxBlueprintLibrary::SetAnimSequenceInterpolation(UAnimSequence* AnimSequence, EAnimInterpolationType AnimInterpolationType)
{
  // check input arguments
  if (!IsValid(AnimSequence))
  {
    UE_LOG(LogTemp, Error, TEXT("Called \"SetAnimSequenceInterpolation\" with invalid AnimSequence."));
    return false;
  }

  AnimSequence->Interpolation = AnimInterpolationType;
  AnimSequence->Modify();

  return true;
}

bool UTTToolboxBlueprintLibrary::ConstraintBonesForSkeletonPose(const TArray<FTTConstraintBone_BP>& ConstraintBones, USkeleton* Skeleton)
{
  //! @todo @ffs implement
  return false;
}

bool UTTToolboxBlueprintLibrary::AddRootBone(USkeleton* Skeleton)
{
  // check input arguments
  if (!IsValid(Skeleton))
  {
    UE_LOG(LogTemp, Error, TEXT("Called \"AddRootBone\" with invalid Skeleton."));
    return false;
  }

  // check if root bone already exists
  if (Skeleton->GetReferenceSkeleton().FindBoneIndex("root") != INDEX_NONE)
  {
    UE_LOG(LogTemp, Error, TEXT("root bone already exists in \"%s\""), *(Skeleton->GetPathName()));
    return false;
  }

  // adapt all meshes to register the new root bone
  TArray<USkeletalMesh*> skeletalMeshes = getAllSkeletalMeshes(Skeleton);
  if (skeletalMeshes.IsEmpty())
  {
    UE_LOG(LogTemp, Error, TEXT("During the call of \"AddRootBone\" no skeletal meshes found that are connected to the skeleton \"s\""), *(Skeleton->GetPathName()));
    return false;
  }

  // Sadly, the implementation does have some issues with wrong bone indices, 
  // see https://github.com/tuatec/TTToolbox/issues/5#issuecomment-1184052765 for the details.
  // That's why all virtual bones get removed (same state if a skeletal mesh is imported through an fbx file)
  // and later added again. This step needs to be done anyways as the bone tree needs to be regenerated, 
  // sadly again there is no public API that can trigger this. BUT! It is possible to trigger the regeneration process
  // through adding a virtual bone.
  // Long story short, adding virtual bones makes it possible to introduce unweighted bones in a save way. ;-)
  const auto savedVirtualBones = Skeleton->GetVirtualBones();
  {
    TArray<FName> virtualBoneNamesToDelete;
    for (auto& virtualBone : savedVirtualBones) {
      virtualBoneNamesToDelete.Add(virtualBone.VirtualBoneName);
    }
    if (virtualBoneNamesToDelete.Num() > 0) {
      Skeleton->RemoveVirtualBones(virtualBoneNamesToDelete);
    }
  }

  uint32 modifiedSkeletalMeshes = 0;
  for (auto skeletalMesh : skeletalMeshes)
  {
    if (Skeleton == skeletalMesh->GetSkeleton())
    {
      { // add root bone
        FReferenceSkeleton referenceSkeleton;
        {
          FReferenceSkeletonModifier referenceSkeletonModifier(referenceSkeleton, skeletalMesh->GetSkeleton());

          const FMeshBoneInfo meshRootBoneInfo(gs_rootBoneName, gs_rootBoneName.ToString(), INDEX_NONE);
          referenceSkeletonModifier.Add(meshRootBoneInfo, FTransform::Identity);

          // increase parent bone indices to sucessfully register the root bone
          for (int32 ii = 0; ii < skeletalMesh->GetSkeleton()->GetReferenceSkeleton().GetRawBoneNum(); ii++)
          {
            FMeshBoneInfo meshBoneInfo = skeletalMesh->GetSkeleton()->GetReferenceSkeleton().GetRawRefBoneInfo()[ii];
            meshBoneInfo.ParentIndex++;
            const auto boneRefPoseTransform = skeletalMesh->GetSkeleton()->GetReferenceSkeleton().GetRawRefBonePose()[ii];
            referenceSkeletonModifier.Add(meshBoneInfo, boneRefPoseTransform);
          }
        }

        skeletalMesh->SetRefSkeleton(referenceSkeleton);
      }

      // reset all bone transforms and reset retarget pose
      //! @todo clarify if this is still needed in 5.3 for adding root bones to the Mixamo Skeleton?
      //skeletalMesh->GetRetargetBasePose().Empty();
      skeletalMesh->CalculateInvRefMatrices();

      uint32 LODIndex = 0;
      for (FSkeletalMeshLODModel& skeletalMeshLODModel : skeletalMesh->GetImportedModel()->LODModels)
      {
        // increase active bone indices to sucessfully register the new root bone
        for (auto& activeBoneIndex : skeletalMeshLODModel.ActiveBoneIndices)
        {
          activeBoneIndex++;
        }
        // insert root bone
        skeletalMeshLODModel.ActiveBoneIndices.Insert(0, 0);

        // increase required bone (unweighted bones) indices to sucessfully register the new root bone
        for (auto& requiredBoneIndex : skeletalMeshLODModel.RequiredBones)
        {
          requiredBoneIndex++;
        }
        // insert root bone
        skeletalMeshLODModel.RequiredBones.Insert(0, 0);

        // update bone references used by the skin weights
        for (auto& skinWeightsProfile : skeletalMeshLODModel.SkinWeightProfiles)
        {
          FImportedSkinWeightProfileData& importedSkinWeightProfileData = skeletalMeshLODModel.SkinWeightProfiles.FindChecked(skinWeightsProfile.Key);

          // increase bone skin weight indices to sucessfully register the root bone
          for (auto& skinWeight : importedSkinWeightProfileData.SkinWeights)
          {
            for (int32 ii = 0; ii < MAX_TOTAL_INFLUENCES; ii++)
            {
              if (skinWeight.InfluenceWeights[ii] > 0)
              {
                skinWeight.InfluenceBones[ii]++;
              }
            }
          }

          // increase source model influence bone indices to sucessfully register the root bone
          for (auto& sourceModelInfluence : importedSkinWeightProfileData.SourceModelInfluences)
          {
            if (sourceModelInfluence.Weight > 0)
            {
              sourceModelInfluence.BoneIndex++;
            }
          }
        }

        // adapt LOD sections
#if ENGINE_MAJOR_VERSION ==	5 &&  ENGINE_MINOR_VERSION <= 3
        if (skeletalMesh->IsLODImportedDataBuildAvailable(LODIndex) && !skeletalMesh->IsLODImportedDataEmpty(LODIndex))
#elif ENGINE_MAJOR_VERSION ==	5 &&  ENGINE_MINOR_VERSION > 3
        if (skeletalMesh->HasMeshDescription(LODIndex))
#endif
        {
          FSkeletalMeshImportData skeletalMeshImportData;
#if ENGINE_MAJOR_VERSION ==	5 &&  ENGINE_MINOR_VERSION <= 3
          skeletalMesh->LoadLODImportedData(LODIndex, skeletalMeshImportData);
#elif ENGINE_MAJOR_VERSION ==	5 &&  ENGINE_MINOR_VERSION > 3
          if (const FMeshDescription* MeshDescription = skeletalMesh->GetMeshDescription(LODIndex))
          {
            skeletalMeshImportData = FSkeletalMeshImportData::CreateFromMeshDescription(*MeshDescription);
          }
#endif

          // increase parent indices to sucessfully add the new root bone
          int32 numRootBoneChilds = 0;
          for (auto& referenceBoneBinary : skeletalMeshImportData.RefBonesBinary)
          {
            if (referenceBoneBinary.ParentIndex == INDEX_NONE)
            {
              numRootBoneChilds += referenceBoneBinary.NumChildren;
            }
            referenceBoneBinary.ParentIndex++;
          }

          const SkeletalMeshImportData::FJointPos rootBonePosition = { FTransform3f::Identity, 1.f, 100.f, 100.f, 100.f };
          const SkeletalMeshImportData::FBone rootBone = { gs_rootBoneName.ToString(), 0, numRootBoneChilds, INDEX_NONE, rootBonePosition };
          skeletalMeshImportData.RefBonesBinary.Insert(rootBone, 0);

          // increase bone influences to sucessfully add the new root bone
          for (auto& influence : skeletalMeshImportData.Influences)
          {
            influence.BoneIndex++;
          }

          if (skeletalMeshImportData.MorphTargets.Num() > 0)
          {
            //! @todo @ffs is it possible to support morph targets?
            UE_LOG(LogTemp, Warning, TEXT("MorphTargets are currently not supported."));
          }

          if (skeletalMeshImportData.AlternateInfluences.Num() > 0)
          {
            //! @todo @ffs is it possible to support alternate influences?
            UE_LOG(LogTemp, Warning, TEXT("AlternateInfluences are currently not supported."));
          }

#if ENGINE_MAJOR_VERSION ==	5 &&  ENGINE_MINOR_VERSION <= 3
          skeletalMesh->SaveLODImportedData(LODIndex, skeletalMeshImportData);
#elif ENGINE_MAJOR_VERSION ==	5 &&  ENGINE_MINOR_VERSION > 3
          skeletalMesh->CommitMeshDescription(LODIndex);
#endif
        }
        else
        {
          for (auto& LODSection : skeletalMeshLODModel.Sections)
          {
            // increase bone indices to sucessfully register the new root bone
            for (auto& boneIndex : LODSection.BoneMap)
            {
              boneIndex++;
            }
          }
        }

        ++LODIndex;
      }

      if (modifiedSkeletalMeshes == 0)
      {
        if (!(Skeleton->RecreateBoneTree(skeletalMesh)))
        {
          UE_LOG(LogTemp, Error, TEXT("Final step of recreating the bone tree failed for skeleton asset \"%s\". Please raise a issue here: https://github.com/tuatec/TTToolbox/issues."), *(skeletalMesh->GetPathName()));
        }
      }
      else
      {
        // the mesh got new bones and now it is necessary to merge those bones into the USkeleton asset as well
        if (!(Skeleton->MergeAllBonesToBoneTree(skeletalMesh)))
        {
          UE_LOG(LogTemp, Error, TEXT("The final step of merging all bones for the skeletal mesh \"%s\"into the bone failed. Please create an issue here https://github.com/tuatec/TTToolbox/issues."), *(skeletalMesh->GetPathName()));
        }
      }

      // through caching reasons the USkeleton has internally a mapping table between skeletal meshes and the skeleton,
      // as new bones were added this table is not valid anymore ==> force rebuilding of that table!
      // Sadly none of these methods is exposed for plugin developers :(
      // - USkeleton::HandleVirtualBoneChanges
      // - USkeleton::RebuildLinkup
      // - USkeleton::RemoveLinkup
      //
      // But happily adding and removing virtual bones call internall USkeleton::HandleVirtualBoneChanges,
      // which should rebuild the mapping table ;-)
      FName virtualBoneName = *(gs_rootBoneName.ToString() + "_delete_me");
      if (!Skeleton->AddNewVirtualBone(gs_rootBoneName, gs_rootBoneName, virtualBoneName))
      {
        UE_LOG(LogTemp, Error, TEXT("failed to add dirty virtual bone hack to force the rebuild of the bone mapping table of skeleton \"%s\""), *Skeleton->GetPathName());
      }
      Skeleton->RemoveVirtualBones({ virtualBoneName });

      skeletalMesh->PostEditChange();
      skeletalMesh->Modify();
      modifiedSkeletalMeshes++;
    }
  }

  // finally readd the virtual bones again to savely store everything
  if (savedVirtualBones.Num() > 0)
  {
    for (auto& virtualBone : savedVirtualBones)
    {
      if (!UTTToolboxBlueprintLibrary::AddVirtualBone(virtualBone.VirtualBoneName, virtualBone.SourceBoneName, virtualBone.TargetBoneName, Skeleton))
      {
        UE_LOG(LogTemp, Error, TEXT("Internal error! Failed to add virtual bone \"%s\" again please raise a issue here: https://github.com/tuatec/TTToolbox/issues."), *virtualBone.VirtualBoneName.ToString());
      }
    }
  }

  if (modifiedSkeletalMeshes > 0)
  {
    Skeleton->Modify();
  }

  return true;
}

bool UTTToolboxBlueprintLibrary::UpdateControlRigBlueprintPreviewMesh(UControlRigBlueprint* ControlRigBlueprint, USkeletalMesh* SkeletalMesh)
{
    if (!IsValid(ControlRigBlueprint))
    {
        UE_LOG(LogTemp, Error, TEXT("Called \"UpdateControlRigBlueprintPreviewMesh\" with invalid \"ControlRigBlueprint\"."));
        return false;
    }

    if (!IsValid(SkeletalMesh))
    {
        UE_LOG(LogTemp, Error, TEXT("Called \"UpdateControlRigBlueprintPreviewMesh\" with invalid \"SkeletalMesh\"."));
        return false;
    }

    ControlRigBlueprint->SetPreviewMesh(SkeletalMesh, true);

    URigHierarchyController* controller = ControlRigBlueprint->GetHierarchyController();
    check(controller);

    controller->ImportBones(SkeletalMesh->GetSkeleton(), NAME_None, true, true, /*bSelectBones*/false, true, true);
    controller->ImportCurves(SkeletalMesh->GetSkeleton(), NAME_None, false, true, true);

    return true;
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
    const FAnimationCurveIdentifier curveId(sourceCurve.GetName(), ERawCurveTrackTypes::RCT_Float);
    targetController.AddCurve(curveId);
    targetController.SetCurveKeys(curveId, sourceCurve.FloatCurve.GetConstRefOfKeys());
  }

  // modify the TargetAnimMontage
  TargetAnimMontage->Modify();

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

  auto ikRigController = UIKRigController::GetController(IKRigDefinition);
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

#if ENGINE_MAJOR_VERSION ==	5 &&  ENGINE_MINOR_VERSION < 1
    ikRigController->AddRetargetChain(boneChain.ChainName, boneChain.StartBone, boneChain.EndBone);
#elif ENGINE_MAJOR_VERSION ==	5 && (ENGINE_MINOR_VERSION == 1 || ENGINE_MINOR_VERSION == 2)
    ikRigController->AddRetargetChain({ boneChain.ChainName, boneChain.StartBone, boneChain.EndBone });
#elif ENGINE_MAJOR_VERSION ==	5 && (ENGINE_MINOR_VERSION == 3)
    ikRigController->AddRetargetChain({ boneChain.ChainName, boneChain.StartBone, boneChain.EndBone, boneChain.IKGoalName });
#elif ENGINE_MAJOR_VERSION ==	5 && (ENGINE_MINOR_VERSION == 4)
    ikRigController->AddRetargetChain(boneChain.ChainName, boneChain.StartBone, boneChain.EndBone, boneChain.IKGoalName);
#endif

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
  auto ikRigController = UIKRigController::GetController(IKRigDefinition);
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

static TArray<USkeletalMesh*> getAllSkeletalMeshes(USkeleton* Skeleton)
{
  check(IsValid(Skeleton));

  TArray<USkeletalMesh*> skeletalMeshes;

  FARFilter filter;
  filter.ClassPaths.Add(USkeletalMesh::StaticClass()->GetClassPathName());
  filter.bRecursiveClasses = true;
  const FString skeletonString = FAssetData(Skeleton).GetExportTextName();
  filter.TagsAndValues.Add(USkeletalMesh::GetSkeletonMemberName(), skeletonString);
  FAssetRegistryModule& assetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

  TArray<FAssetData> assets;
  assetRegistryModule.Get().GetAssets(filter, assets);

  for (auto& asset : assets)
  {
    if (auto skeletalMesh = Cast<USkeletalMesh>(asset.GetAsset()))
    {
      skeletalMeshes.Add(skeletalMesh);
    }
  }

  return skeletalMeshes;
}
