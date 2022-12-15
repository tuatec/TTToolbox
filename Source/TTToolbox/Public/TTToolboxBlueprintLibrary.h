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

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TTToolboxTypes.h"
#include "TTToolboxBlueprintLibrary.generated.h"

// forward declarations
class USkeleton;
class UIKRigDefinition;
class UControlRig;
class UControlRigBlueprint;


UCLASS()
class TTTOOLBOX_API UTTToolboxBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	// virtual bone functions

	UFUNCTION(BlueprintCallable, Category = "TTToolbox")
	static bool DumpVirtualBones(USkeleton* Skeleton);

	UFUNCTION(BlueprintCallable, Category = "TTToolbox")
	static bool AddVirtualBone(const FName& VirtualBoneName, const FName& SourceBoneName, const FName& TargetBoneName, USkeleton* Skeleton);


	// socket functions

	UFUNCTION(BlueprintCallable, Category = "TTToolbox")
	static bool DumpSockets(USkeleton* Skeleton);

	UFUNCTION(BlueprintCallable, Category = "TTToolbox")
	static bool AddSocket(const FName& BoneName, const FName& SocketName, const FTransform& RelativeTransform, USkeleton* Skeleton);

	UFUNCTION(BlueprintCallable, Category = "TTToolbox")
	static bool HasSocket(const FName& SocketName, USkeleton* Skeleton);


	// skeleton functions

	// dumps all available skeleton curve names to the console and makes them available in the clipboard as well
	UFUNCTION(BlueprintCallable, Category = "TTToolbox")
	static bool DumpSkeletonCurveNames(USkeleton* Skeleton);

	// checks if the given 'CurveNamesToCheck' are available in the given 'Skeleton' and prints the missing curves to the console
	UFUNCTION(BlueprintCallable, Category = "TTToolbox")
	static bool CheckForMissingCurveNames(const TArray<FName>& CurveNamesToCheck, USkeleton* Skeleton);

	// returns true if the given 'SkeletonCurveName' exists in the specified 'Skeleton', otherwise false.
	UFUNCTION(BlueprintCallable, Category = "TTToolbox")
	static bool HasSkeletonCurve(USkeleton* Skeleton, const FName& SkeletonCurveName);

	UFUNCTION(BlueprintCallable, Category = "TTToolbox")
	static bool DumpSkeletonBlendProfile(USkeleton* Skeleton);

	// will add a new 'BlendProfile' to the given 'Skeleton' with the 'BlendProfileName'. If 'Overwrite' is set to true it will overwrite the already existing blend values otherwise returns with false.
	UFUNCTION(BlueprintCallable, Category = "TTToolbox")
	static bool AddSkeletonBlendProfile(USkeleton* Skeleton, const FName& BlendProfileName, const FTTBlendProfile_BP& BlendProfile, bool Overwrite = false);

	// adds the given 'SkeletonCurveName' to the specified 'Skeleton' and returns if successful, false if the given 'SkeletonCurveName' already exists.
	UFUNCTION(BlueprintCallable, Category = "TTToolbox")
	static bool AddSkeletonCurve(USkeleton* Skeleton, const FName& SkeletonCurveName);

	// dumps all groups and montages slots for the given 'Skeleton'. Returns true on success, false otherwise.
	UFUNCTION(BlueprintCallable, Category = "TTToolbox")
	static bool DumpGroupsAndSlots(USkeleton* Skeleton);

	// adds the given 'SlotGroup' to the specified 'Skeleton'. Returns true on success, false otherwise.
	UFUNCTION(BlueprintCallable, Category = "TTToolbox")
	static bool AddSkeletonSlotGroup(USkeleton* Skeleton, const FTTMontageSlotGroup& SlotGroup);

	// adds the fiven 'NewBones' to the given 'Skeleton' and it's connected skeletal meshes.
	// NOTE: Sadly Unreal Engine does come with lot's of assertions and it is very hard to implement this feature in a save way,
	// the function removes all virtual bones and adds them after again after the unweighted bones are added to the skeletal meshes.
	UFUNCTION(BlueprintCallable, Category = "TTToolbox")
	static bool AddUnweightedBone(const TArray<FTTNewBone_BP>& NewBones, USkeleton* Skeleton);

	//! @todo @ffs implement this feature
	UFUNCTION(BlueprintCallable, Category = "TTToolbox")
	static bool ConstraintBonesForSkeletonPose(const TArray<FTTConstraintBone_BP>& ConstraintBones, USkeleton* Skeleton);

	// adds the root bone to the given 'Skeleton' and it's connected skeletal meshes (needed for Mixamo based characters)
	// NOTE: Sadly Unreal Engine does come with lot's of assertions and it is very hard to implement this feature in a save way,
	// the function removes all virtual bones and adds them after again after the root bone was added to the skeletal meshes.
	UFUNCTION(BlueprintCallable, Category = "TTToolbox")
	static bool AddRootBone(USkeleton* Skeleton);

	// ControlRig functions

	// updates the given 'ControlRigBlueprint' with the specified 'SkeletalMesh'. Returns true on success, false otherwise.
	UFUNCTION(BlueprintCallable, Category = "TTToolbox")
	static bool UpdateControlRigBlueprintPreviewMesh(UControlRigBlueprint* ControlRigBlueprint, USkeletalMesh* SkeletalMesh);

	// AnimMontage functions

	UFUNCTION(BlueprintCallable, Category = "TTToolbox")
	static bool CopyAnimMontageCurves(UAnimMontage* SourceAnimMontage, UAnimMontage* TargetAnimMontage);

	// AnimSequence functions

	// forces animation sequence recompression, which will also reconstraint the virtual bones
	UFUNCTION(BlueprintCallable, Category = "TTToolbox")
	static void RequestAnimationRecompress(USkeleton* Skeleton);

	// forces animation sequence recompression for the given 'AnimSequences', which will also reconstraint the virtual bones.
	UFUNCTION(BlueprintCallable, Category = "TTToolbox")
	static void RequestAnimSequencesRecompression(TArray<UAnimSequence*> AnimSequences);

	// sets the interpolation mode for the given 'AnimSequence'.
	UFUNCTION(BlueprintCallable, Category = "TTToolbox")
	static bool SetAnimSequenceInterpolation(UAnimSequence* AnimSequence, EAnimInterpolationType AnimInterpolationType);

	// IK Rig functions

	UFUNCTION(BlueprintCallable, Category = "TTToolbox")
  static bool DumpIKChains(const UIKRigDefinition* IKRigDefinition);

  UFUNCTION(BlueprintCallable, Category = "TTToolbox")
  static bool AddIKBoneChains(UIKRigDefinition* IKRigDefinition, const TArray<FBoneChain_BP>& BoneChains);

  UFUNCTION(BlueprintCallable, Category = "TTToolbox")
  static bool SetIKBoneChainGoal(UIKRigDefinition* IKRigDefinition, const FName& ChainName, const FName& GoalName);
};
