#pragma once

#include "CoreMinimal.h"
#include "RuntimeData/IADRuntimeDataPage.h"
#include "Widgets/SCompoundWidget.h"

class SHorizontalBox;
class SWidgetSwitcher;
class UWorld;
class UAbilitySystemComponent;
template<typename OptionType>
class SComboBox;

struct FADRuntimeActorOption
{
	TWeakObjectPtr<AActor> Actor;
	FString Label;
};

class SADRuntimeDataPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SADRuntimeDataPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	using FActorOptionPtr = TSharedPtr<FADRuntimeActorOption>;

	EActiveTimerReturnType HandleRefreshTimer(double InCurrentTime, float InDeltaTime);
	void RefreshActorOptions();
	void RefreshActivePage();
	UWorld* FindRuntimeWorld() const;
	UAbilitySystemComponent* GetAbilitySystemComponent(const AActor* Actor) const;
	bool IsInspectableActor(const AActor* Actor) const;
	bool IsSelectedActorStillAvailable() const;
	FString BuildActorLabel(const AActor* Actor) const;

	TSharedRef<SWidget> BuildActorCombo();
	TSharedRef<SWidget> BuildNavigationBar();
	TSharedRef<SWidget> BuildPageSwitcher();
	TSharedRef<SWidget> GenerateActorComboRow(FActorOptionPtr Option) const;
	FText GetSelectedActorText() const;
	FText GetRuntimeWorldStatusText() const;
	void HandleActorSelectionChanged(FActorOptionPtr NewSelection, ESelectInfo::Type SelectInfo);
	void SelectPage(FName PageId);
	FReply HandlePageButtonClicked(FName PageId);

	TArray<FActorOptionPtr> ActorOptions;
	TSharedPtr<SComboBox<FActorOptionPtr>> ActorComboBox;
	TWeakObjectPtr<AActor> SelectedActor;

	TArray<TSharedRef<IADRuntimeDataPage>> Pages;
	FName ActivePageId;
	TSharedPtr<SWidgetSwitcher> PageSwitcher;
	TSharedPtr<SHorizontalBox> NavigationBar;
};
