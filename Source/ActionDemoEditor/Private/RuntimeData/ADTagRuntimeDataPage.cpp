#include "RuntimeData/ADTagRuntimeDataPage.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "Styling/AppStyle.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Widgets/Views/STableRow.h"

#define LOCTEXT_NAMESPACE "FADTagRuntimeDataPage"

namespace
{
	class SADTagRuntimeRowWidget : public SMultiColumnTableRow<TSharedPtr<FADTagRuntimeRow>>
	{
	public:
		SLATE_BEGIN_ARGS(SADTagRuntimeRowWidget) {}
			SLATE_ARGUMENT(TSharedPtr<FADTagRuntimeRow>, Row)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTable)
		{
			Row = InArgs._Row;
			SMultiColumnTableRow<TSharedPtr<FADTagRuntimeRow>>::Construct(
				FSuperRowType::FArguments().Padding(FMargin(4.0f, 2.0f)),
				OwnerTable);
		}

		virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override
		{
			FText Text = FText::GetEmpty();

			if (Row.IsValid())
			{
				if (ColumnName == TEXT("Tag"))
				{
					Text = FText::FromString(Row->GetDisplayName());
				}
				else if (ColumnName == TEXT("Count"))
				{
					Text = FText::AsNumber(Row->Count);
				}
			}

			return SNew(STextBlock)
				.Text(Text)
				.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"));
		}

	private:
		TSharedPtr<FADTagRuntimeRow> Row;
	};
}

FString FADTagRuntimeRow::GetDisplayName() const
{
	return Count > 1
		? FString::Printf(TEXT("%s (%d)"), *TagName, Count)
		: TagName;
}

bool FADTagRuntimeRow::MatchesFilter(const FString& InFilterText) const
{
	return InFilterText.IsEmpty()
		|| TagName.Contains(InFilterText, ESearchCase::IgnoreCase);
}

FName FADTagRuntimeDataPage::GetId() const
{
	return TEXT("Tag");
}

FText FADTagRuntimeDataPage::GetDisplayName() const
{
	return LOCTEXT("TagPageTitle", "Tag");
}

bool FADTagRuntimeDataPage::SupportsActor(const AActor* Actor) const
{
	return GetAbilitySystemComponent(Actor) != nullptr;
}

TSharedRef<SWidget> FADTagRuntimeDataPage::BuildWidget()
{
	StatusText = LOCTEXT("TagWaitingForActor", "Select a PIE/SIE actor with an Ability System Component.");

	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("Brushes.Panel"))
		.Padding(8.0f)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSearchBox)
				.HintText(LOCTEXT("TagSearchHint", "Search tags"))
				.OnTextChanged_Raw(this, &FADTagRuntimeDataPage::HandleSearchTextChanged)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 8.0f, 0.0f, 6.0f)
			[
				SNew(STextBlock)
				.Text_Lambda([this]()
				{
					return GetStatusText();
				})
				.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSeparator)
			]

			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			.Padding(0.0f, 6.0f, 0.0f, 0.0f)
			[
				SAssignNew(ListView, SListView<FRowPtr>)
				.ListItemsSource(&FilteredRows)
				.SelectionMode(ESelectionMode::None)
				.OnGenerateRow_Raw(this, &FADTagRuntimeDataPage::GenerateRow)
				.HeaderRow
				(
					SNew(SHeaderRow)
					+ SHeaderRow::Column(TEXT("Tag"))
					.DefaultLabel(LOCTEXT("TagColumn", "Tag"))
					.FillWidth(0.82f)
					+ SHeaderRow::Column(TEXT("Count"))
					.DefaultLabel(LOCTEXT("CountColumn", "Count"))
					.FillWidth(0.18f)
				)
			]
		];
}

void FADTagRuntimeDataPage::Refresh(const TWeakObjectPtr<AActor>& SelectedActor)
{
	AllRows.Reset();

	AActor* Actor = SelectedActor.Get();
	if (!IsValid(Actor))
	{
		StatusText = LOCTEXT("TagNoActor", "Select a PIE/SIE actor with an Ability System Component.");
		RebuildFilteredRows();
		return;
	}

	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent(Actor);
	if (AbilitySystemComponent == nullptr)
	{
		StatusText = LOCTEXT("TagNoASC", "Selected actor does not have a valid Ability System Component.");
		RebuildFilteredRows();
		return;
	}

	FGameplayTagContainer OwnedTags;
	AbilitySystemComponent->GetOwnedGameplayTags(OwnedTags);

	for (const FGameplayTag& Tag : OwnedTags)
	{
		FRowPtr Row = MakeShared<FADTagRuntimeRow>();
		Row->TagName = Tag.ToString();
		Row->Count = AbilitySystemComponent->GetTagCount(Tag);
		AllRows.Add(Row);
	}

	AllRows.Sort([](const FRowPtr& Left, const FRowPtr& Right)
	{
		return Left.IsValid() && Right.IsValid() && Left->TagName < Right->TagName;
	});

	StatusText = FText::Format(
		LOCTEXT("TagRowCount", "{0} tags from {1}"),
		FText::AsNumber(AllRows.Num()),
		FText::FromString(Actor->GetActorNameOrLabel()));

	RebuildFilteredRows();
}

UAbilitySystemComponent* FADTagRuntimeDataPage::GetAbilitySystemComponent(const AActor* Actor) const
{
	if (Actor == nullptr)
	{
		return nullptr;
	}

	if (const IAbilitySystemInterface* AbilityActor = Cast<IAbilitySystemInterface>(Actor))
	{
		return AbilityActor->GetAbilitySystemComponent();
	}

	return nullptr;
}

void FADTagRuntimeDataPage::RebuildFilteredRows()
{
	FilteredRows.Reset();

	for (const FRowPtr& Row : AllRows)
	{
		if (Row.IsValid() && Row->MatchesFilter(FilterText))
		{
			FilteredRows.Add(Row);
		}
	}

	if (ListView.IsValid())
	{
		ListView->RequestListRefresh();
	}
}

void FADTagRuntimeDataPage::HandleSearchTextChanged(const FText& NewText)
{
	FilterText = NewText.ToString();
	RebuildFilteredRows();
}

FText FADTagRuntimeDataPage::GetStatusText() const
{
	if (!FilterText.IsEmpty())
	{
		return FText::Format(
			LOCTEXT("TagFilteredStatus", "{0} / {1} tags"),
			FText::AsNumber(FilteredRows.Num()),
			FText::AsNumber(AllRows.Num()));
	}

	return StatusText;
}

TSharedRef<ITableRow> FADTagRuntimeDataPage::GenerateRow(FRowPtr Row, const TSharedRef<STableViewBase>& OwnerTable) const
{
	return SNew(SADTagRuntimeRowWidget, OwnerTable)
		.Row(Row);
}

#undef LOCTEXT_NAMESPACE
