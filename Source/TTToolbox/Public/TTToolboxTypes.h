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
#include "Animation/BlendProfile.h"
#include "TTToolboxTypes.generated.h"

// forward declarations
struct FBoneChain;

// Helper stucture that is exposed to Blueprints to be independent from the ik rig implementation.
// Additionally, this can be reused in data tables to store the bone chains.
USTRUCT(Blueprintable)
struct TTTOOLBOX_API FBoneChain_BP
{
	GENERATED_BODY()

	FBoneChain_BP() = default;

	FBoneChain_BP(const FBoneChain& BoneChain);

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

USTRUCT(Blueprintable)
struct TTTOOLBOX_API FTTConstraintBones_BP
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TTToolbox")
	TArray<FTTConstraintBone_BP> ConstraintBones;
};

USTRUCT(Blueprintable)
struct TTTOOLBOX_API FTTBlendProfile_BP
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TTToolbox")
	EBlendProfileMode BlendProfileMode = EBlendProfileMode::BlendMask;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TTToolbox")
	TMap<FName, float> BlendValues;
};

USTRUCT(Blueprintable)
struct TTTOOLBOX_API FTTMontageSlotGroup
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TTToolbox")
	FName GroupName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TTToolbox")
	TArray<FName> SlotNames;
};

