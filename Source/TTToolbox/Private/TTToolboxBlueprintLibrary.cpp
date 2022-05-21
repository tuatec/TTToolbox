#include "TTToolboxBlueprintLibrary.h"

#if WITH_EDITOR
#include "HAL/PlatformApplicationMisc.h"
#endif

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
