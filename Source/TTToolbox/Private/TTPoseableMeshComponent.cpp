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

#include "TTPoseableMeshComponent.h"

void UTTPoseableMeshComponent::SetBoneLocalTransformByName(const FName& BoneName, const FTransform& InTransform)
{
  if (!SkeletalMesh || !RequiredBones.IsValid())
  {
    return;
  }

  int32 boneIndex = GetBoneIndex(BoneName);
  if (boneIndex >= 0 && boneIndex < BoneSpaceTransforms.Num())
  {
    BoneSpaceTransforms[boneIndex] = InTransform;
    MarkRefreshTransformDirty();
  }
}

void UTTPoseableMeshComponent::UpdatePose()
{
  // Can't do anything without a SkeletalMesh
  if (!SkeletalMesh)
  {
    return;
  }

  // Do nothing more if no bones in skeleton.
  if (GetNumComponentSpaceTransforms() == 0)
  {
    return;
  }

  // We need the mesh space bone transforms now for renderer to get delta from ref pose:
  FillComponentSpaceTransforms();
  FinalizeBoneTransform();

  UpdateChildTransforms();
  UpdateBounds();
  MarkRenderTransformDirty();
  MarkRenderDynamicDataDirty();
}
