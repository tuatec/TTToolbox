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

#include "IKRig_ConstraintBones.h"

#define LOCTEXT_NAMESPACE "UIKRig_BoneConstrainer"

UIKRig_ConstraintBones::UIKRig_ConstraintBones() {}
UIKRig_ConstraintBones::~UIKRig_ConstraintBones() {}

void UIKRig_ConstraintBones::Initialize(const FIKRigSkeleton& IKRigSkeleton)
{
	m_constraintBones.Reserve(ConstraintBones.Num());

	bool errorsOccurred = false;

	//! @todo @ffs sort bone indices
	for (auto& constraint : ConstraintBones)
	{
		int32 constraintBone = IKRigSkeleton.GetBoneIndexFromName(constraint.ConstraintBone);
		int32 modifiedBone = IKRigSkeleton.GetBoneIndexFromName(constraint.ModifiedBone);

		if (constraintBone == INDEX_NONE)
		{
			errorsOccurred = true;
			UE_LOG(LogTemp, Error, TEXT("Failed get get bone index for ConstraintBone %s"), *constraint.ConstraintBone.ToString());
			continue;
		}

		if (modifiedBone == INDEX_NONE)
		{
			errorsOccurred = true;
			UE_LOG(LogTemp, Error, TEXT("Failed get get bone index for ModifiedBone %s"), *constraint.ModifiedBone.ToString());
			continue;
		}

			m_constraintBones.Add(
				{
					constraintBone,
					modifiedBone
			});
	}

	if (errorsOccurred)
	{
		UE_LOG(LogTemp, Error, TEXT("Some constraint bones could not be set up, no constraining will be done. Please check the error messages above."));
		m_constraintBones.Empty();
	}
}

void UIKRig_ConstraintBones::Solve(FIKRigSkeleton& IKRigSkeleton, const FIKRigGoalContainer& Goals)
{
	if (m_constraintBones.Num() <= 0)
	{
		// nothing to do here as no constraint bones are configured
		return;
	}

	for (auto& constraintBone : m_constraintBones)
	{
		IKRigSkeleton.CurrentPoseGlobal[constraintBone.ModifiedBone] = IKRigSkeleton.CurrentPoseGlobal[constraintBone.ConstraintBone];
		IKRigSkeleton.PropagateGlobalPoseBelowBone(constraintBone.ModifiedBone);
	}
}

#if WITH_EDITOR

FText UIKRig_ConstraintBones::GetNiceName() const
{
	return FText(LOCTEXT("SolverName", "Constraint Bones"));
}

bool UIKRig_ConstraintBones::IsBoneAffectedBySolver(const FName& BoneName, const FIKRigSkeleton& IKRigSkeleton) const
{
	for (auto& constraintBone : ConstraintBones)
	{
		if (constraintBone.ModifiedBone == BoneName)
		{
			return true;
		}
	}

	return false;
}

bool UIKRig_ConstraintBones::GetWarningMessage(FText& OutWarningMessage) const
{
	if (ConstraintBones.Num() <= 0)
	{
		OutWarningMessage = LOCTEXT("NoConstraintBones", "Missing constraint bones.");
		return true;
	}

	return false;
}

bool UIKRig_ConstraintBones::IsGoalConnected(const FName& GoalName) const
{
	// always return true as we don't want to connect IK goals for this solver ;)
	return true;
}

#endif

#undef LOCTEXT_NAMESPACE
