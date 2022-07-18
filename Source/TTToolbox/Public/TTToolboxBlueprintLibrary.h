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
#include "IKRigDefinition.h"
#include "TTToolboxBlueprintLibrary.generated.h"

// forward declarations
class USkeleton;
class UIKRigDefinition;


// Helper stucture that is exposed to Blueprints to be independent from the ik rig implementation.
// Additionally, this can be reused in data tables to store the bone chains.
USTRUCT(Blueprintable)
struct TTTOOLBOX_API FBoneChain_BP
{
	GENERATED_BODY()

	FBoneChain_BP() = default;

	FBoneChain_BP(const FBoneChain& BoneChain)
		: ChainName(BoneChain.ChainName)
		, StartBone(BoneChain.StartBone.BoneName)
		, EndBone(BoneChain.EndBone.BoneName)
		, IKGoalName(BoneChain.IKGoalName)
	{}

	// stores the bone chain name that will be shown in the ik rig asset editor window
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BoneChain)
	FName ChainName = NAME_None;

	// stores the beginning bone name of the bone chain
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BoneChain)
	FName StartBone = NAME_None;

	// stores the ending bone name of the bone chain
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BoneChain)
	FName EndBone = NAME_None;

	// stores the ik goal name that is visible in the ik rig asset editor window
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IK)
	FName IKGoalName = NAME_None;
};


USTRUCT(Blueprintable)
struct TTTOOLBOX_API FTTNewBone_BP
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TTToolbox")
	FName NewBoneName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TTToolbox")
	FName ParentBone = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TTToolbox")
	FName ConstraintBone = NAME_None;
};

USTRUCT(Blueprintable)
struct TTTOOLBOX_API FTTConstraintBone_BP
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TTToolbox")
	FName ModifiedBone = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TTToolbox")
	FName ConstraintBone = NAME_None;
};

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

	// AnimMontage functions

	UFUNCTION(BlueprintCallable, Category = "TTToolbox")
	static bool CopyAnimMontageCurves(UAnimMontage* SourceAnimMontage, UAnimMontage* TargetAnimMontage);

	// AnimSequence functions

	// forces animation sequence recompression, which will also reconstraint the virtual bones
	UFUNCTION(BlueprintCallable, Category = "TTToolbox")
	static void RequestAnimationRecompress(USkeleton* Skeleton);

	// IK Rig functions

	UFUNCTION(BlueprintCallable, Category = "TTToolbox")
  static bool DumpIKChains(const UIKRigDefinition* IKRigDefinition);

  UFUNCTION(BlueprintCallable, Category = "TTToolbox")
  static bool AddIKBoneChains(UIKRigDefinition* IKRigDefinition, const TArray<FBoneChain_BP>& BoneChains);

  UFUNCTION(BlueprintCallable, Category = "TTToolbox")
  static bool SetIKBoneChainGoal(UIKRigDefinition* IKRigDefinition, const FName& ChainName, const FName& GoalName);
};
