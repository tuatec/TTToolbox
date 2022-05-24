

#pragma once

#include "CoreMinimal.h"
#include "AnimationModifier.h"
#include "TTCopyAllCurvesAnimModifier.generated.h"

UCLASS()
class TTTOOLBOX_API UTTCopyAllCurvesAnimModifier : public UAnimationModifier
{
	GENERATED_BODY()
private: // methods
	void OnApply_Implementation(UAnimSequence* TargetSequence) override;

public: // members
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	UAnimSequence* SourceSequence = nullptr;
};
