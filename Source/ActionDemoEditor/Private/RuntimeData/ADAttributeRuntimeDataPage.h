#pragma once

#include "IADRuntimeDataPage.h"

#include "Widgets/Views/SListView.h"

class AActor;
class UAbilitySystemComponent;

struct FADAttributeRuntimeRow
{
	FString AttributeSetName;
	FString AttributeName;
	float CurrentValue = 0.0f;
	float BaseValue = 0.0f;

	bool MatchesFilter(const FString& InFilterText) const;
};

class FADAttributeRuntimeDataPage : public IADRuntimeDataPage
{
public:
	virtual FName GetId() const override;
	virtual FText GetDisplayName() const override;
	virtual bool SupportsActor(const AActor* Actor) const override;
	virtual TSharedRef<SWidget> BuildWidget() override;
	virtual void Refresh(const TWeakObjectPtr<AActor>& SelectedActor) override;

private:
	using FRowPtr = TSharedPtr<FADAttributeRuntimeRow>;

	UAbilitySystemComponent* GetAbilitySystemComponent(const AActor* Actor) const;
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
