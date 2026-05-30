#pragma once

#include "IADRuntimeDataPage.h"

#include "Widgets/Views/SListView.h"

class AActor;
class UAbilitySystemComponent;

struct FADAbilityRuntimeRow
{
	FString AbilityName;
	FString AbilityTags;
	FString InputText;
	int32 Level = 0;
	int32 ActiveCount = 0;
	bool bIsActive = false;

	FString GetStateText() const;
	bool MatchesFilter(const FString& InFilterText) const;
};

class FADAbilityRuntimeDataPage : public IADRuntimeDataPage
{
public:
	virtual FName GetId() const override;
	virtual FText GetDisplayName() const override;
	virtual bool SupportsActor(const AActor* Actor) const override;
	virtual TSharedRef<SWidget> BuildWidget() override;
	virtual void Refresh(const TWeakObjectPtr<AActor>& SelectedActor) override;

private:
	using FRowPtr = TSharedPtr<FADAbilityRuntimeRow>;

	UAbilitySystemComponent* GetAbilitySystemComponent(const AActor* Actor) const;
	FString GetAbilityDisplayName(const class UGameplayAbility* Ability) const;
	void RebuildFilteredRows();
	void HandleSearchTextChanged(const FText& NewText);
	FText GetStatusText() const;
	TSharedRef<ITableRow> GenerateRow(FRowPtr Row, const TSharedRef<STableViewBase>& OwnerTable) const;

	TArray<FRowPtr> AllRows;
	TArray<FRowPtr> FilteredRows;
	TSharedPtr<SListView<FRowPtr>> ListView;
	FString FilterText;
	FText StatusText;
};
