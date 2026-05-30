#pragma once

#include "IADRuntimeDataPage.h"

#include "Widgets/Views/SListView.h"

class AActor;
class UAbilitySystemComponent;

struct FADTagRuntimeRow
{
	FString TagName;
	int32 Count = 0;

	FString GetDisplayName() const;
	bool MatchesFilter(const FString& InFilterText) const;
};

class FADTagRuntimeDataPage : public IADRuntimeDataPage
{
public:
	virtual FName GetId() const override;
	virtual FText GetDisplayName() const override;
	virtual bool SupportsActor(const AActor* Actor) const override;
	virtual TSharedRef<SWidget> BuildWidget() override;
	virtual void Refresh(const TWeakObjectPtr<AActor>& SelectedActor) override;

private:
	using FRowPtr = TSharedPtr<FADTagRuntimeRow>;

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
