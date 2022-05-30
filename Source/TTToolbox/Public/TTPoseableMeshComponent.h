

#pragma once

#include "CoreMinimal.h"
#include "Components/PoseableMeshComponent.h"
#include "TTPoseableMeshComponent.generated.h"

/**
 * 
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class TTTOOLBOX_API UTTPoseableMeshComponent : public UPoseableMeshComponent
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Components|PoseableMesh")
	void SetBoneLocalTransformByName(const FName& BoneName, const FTransform& InTransform);

	UFUNCTION(BlueprintCallable, Category = "Components|PoseableMesh")
	void UpdatePose();
};
