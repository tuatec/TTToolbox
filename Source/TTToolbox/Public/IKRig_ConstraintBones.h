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

// Unreal Engine includes
#include "CoreMinimal.h"
#include "Rig/Solvers/IKRigSolver.h"

#include "IKRig_ConstraintBones.generated.h"


USTRUCT(Blueprintable)
struct TTTOOLBOX_API FConstraintBone
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Constraint)
	FName ConstraintBone = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Constraint)
	FName ModifiedBone = NAME_None;
};

UCLASS(EditInlineNew)
class TTTOOLBOX_API UIKRig_ConstraintBones : public UIKRigSolver
{
	GENERATED_BODY()

public:
	UIKRig_ConstraintBones();
	~UIKRig_ConstraintBones();

protected: // exposed members
	UPROPERTY(EditAnywhere, Category = "Settings")
	TArray<FConstraintBone> ConstraintBones;

protected: // UIKRigSolver interface
	void Initialize(const FIKRigSkeleton& IKRigSkeleton) override;
	void Solve(FIKRigSkeleton& IKRigSkeleton, const FIKRigGoalContainer& Goals) override;

#if WITH_EDITOR
	FText GetNiceName() const override;
	bool IsBoneAffectedBySolver(const FName& BoneName, const FIKRigSkeleton& IKRigSkeleton) const override;
	bool GetWarningMessage(FText& OutWarningMessage) const override;
	bool IsGoalConnected(const FName& GoalName) const override;

	void AddGoal(const UIKRigEffectorGoal& NewGoal) override {}
	void RemoveGoal(const FName& GoalName) override {}
	void RenameGoal(const FName& OldName, const FName& NewName) override {}
	void SetGoalBone(const FName& GoalName, const FName& NewBoneName) override {}
#endif

private:
	struct CConstrainedBone
	{
		int32 ConstraintBone = INDEX_NONE;
		int32 ModifiedBone = INDEX_NONE;
	};
	TArray<CConstrainedBone> m_constraintBones;
};
