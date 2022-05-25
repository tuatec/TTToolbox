#include "TTToolboxBlueprintLibrary.h"

// Unreal Engine includes
#include "Engine/SkeletalMeshSocket.h"

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
