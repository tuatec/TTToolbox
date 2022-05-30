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
