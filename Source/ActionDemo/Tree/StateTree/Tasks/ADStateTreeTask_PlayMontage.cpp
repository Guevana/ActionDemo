#include "Tree/StateTree/Tasks/ADStateTreeTask_PlayMontage.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Character/Base/ADCharacterBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "StateTreeExecutionContext.h"
#include "Tree/StateTree/ADStateTreeContextHelpers.h"

UADStateTreeTask_PlayMontage::UADStateTreeTask_PlayMontage(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bShouldCallTick = true;
}

EStateTreeRunStatus UADStateTreeTask_PlayMontage::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
	Super::EnterState(Context, Transition);
	ResetRuntimeState();

	if (Montage == nullptr || PlayRate <= 0.0f)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (StartSectionName != NAME_None && Montage->GetSectionIndex(StartSectionName) == INDEX_NONE)
	{
		return EStateTreeRunStatus::Failed;
	}

	UAnimInstance* AnimInstance = ResolveAnimInstance(Context);
	if (AnimInstance == nullptr)
	{
		return EStateTreeRunStatus::Failed;
	}

	const float PlayDuration = AnimInstance->Montage_Play(
		Montage,
		PlayRate,
		EMontagePlayReturnType::Duration,
		StartPosition,
		bStopAllMontages);

	if (PlayDuration <= 0.0f)
	{
		return EStateTreeRunStatus::Failed;
	}

	ActiveAnimInstance = AnimInstance;
	ActiveMontage = Montage;
	bMontageStarted = true;

	FOnMontageEnded MontageEndedDelegate;
	MontageEndedDelegate.BindUObject(this, &UADStateTreeTask_PlayMontage::HandleMontageEnded);
	AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, Montage);

	if (StartSectionName != NAME_None)
	{
		AnimInstance->Montage_JumpToSection(StartSectionName, Montage);
	}

	return bWaitForMontageEnd ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Succeeded;
}

EStateTreeRunStatus UADStateTreeTask_PlayMontage::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
	Super::Tick(Context, DeltaTime);

	if (!bWaitForMontageEnd)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	if (!bMontageStarted)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (bMontageEnded)
	{
		return bMontageInterrupted && bTreatInterruptedAsFailure ? EStateTreeRunStatus::Failed : EStateTreeRunStatus::Succeeded;
	}

	UAnimInstance* AnimInstance = ActiveAnimInstance.Get();
	UAnimMontage* PlayingMontage = ActiveMontage.Get();
	if (AnimInstance == nullptr || PlayingMontage == nullptr)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (!AnimInstance->Montage_IsActive(PlayingMontage))
	{
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Running;
}

void UADStateTreeTask_PlayMontage::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
	Super::ExitState(Context, Transition);

	UAnimInstance* AnimInstance = ActiveAnimInstance.Get();
	UAnimMontage* PlayingMontage = ActiveMontage.Get();
	if (AnimInstance != nullptr && PlayingMontage != nullptr)
	{
		if (bStopOnExit && bMontageStarted && !bMontageEnded && AnimInstance->Montage_IsActive(PlayingMontage))
		{
			AnimInstance->Montage_Stop(ExitBlendOutTime, PlayingMontage);
		}

		FOnMontageEnded EmptyDelegate;
		AnimInstance->Montage_SetEndDelegate(EmptyDelegate, PlayingMontage);
	}

	if (RunStatus == EStateTreeRunStatus::Failed)
	{
		const AActor* OwnerActor = GetOwnerActor(Context);
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[ActionDemo] PlayMontage Task failed. Owner=%s Montage=%s StartSection=%s"),
			*GetNameSafe(OwnerActor),
			*GetNameSafe(Montage),
			*StartSectionName.ToString());
	}

	ResetRuntimeState();
}

UAnimInstance* UADStateTreeTask_PlayMontage::ResolveAnimInstance(FStateTreeExecutionContext& Context) const
{
	AActor* OwnerActor = GetOwnerActor(Context);

	if (const AADCharacterBase* Character = ADStateTreeContextHelpers::ResolveCharacter(OwnerActor))
	{
		return Character->GetMesh() != nullptr ? Character->GetMesh()->GetAnimInstance() : nullptr;
	}

	if (OwnerActor != nullptr)
	{
		if (const USkeletalMeshComponent* SkeletalMeshComponent = OwnerActor->FindComponentByClass<USkeletalMeshComponent>())
		{
			return SkeletalMeshComponent->GetAnimInstance();
		}
	}

	return nullptr;
}

void UADStateTreeTask_PlayMontage::HandleMontageEnded(UAnimMontage* EndedMontage, bool bInterrupted)
{
	if (EndedMontage != ActiveMontage.Get())
	{
		return;
	}

	bMontageEnded = true;
	bMontageInterrupted = bInterrupted;
}

void UADStateTreeTask_PlayMontage::ResetRuntimeState()
{
	ActiveAnimInstance.Reset();
	ActiveMontage.Reset();
	bMontageStarted = false;
	bMontageEnded = false;
	bMontageInterrupted = false;
}
