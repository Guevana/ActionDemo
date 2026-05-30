#include "RuntimeData/ADAttributeRuntimeDataPage.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AttributeSet.h"
#include "Styling/AppStyle.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Widgets/Views/STableRow.h"

#define LOCTEXT_NAMESPACE "FADAttributeRuntimeDataPage"

namespace
{
	static FText FormatAttributeValue(float Value)
	{
		FNumberFormattingOptions FormatOptions;
		FormatOptions.MinimumFractionalDigits = 0;
		FormatOptions.MaximumFractionalDigits = 3;
		return FText::AsNumber(Value, &FormatOptions);
	}

	class SADAttributeRuntimeRowWidget : public SMultiColumnTableRow<TSharedPtr<FADAttributeRuntimeRow>>
	{
	public:
		SLATE_BEGIN_ARGS(SADAttributeRuntimeRowWidget) {}
			SLATE_ARGUMENT(TSharedPtr<FADAttributeRuntimeRow>, Row)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTable)
		{
			Row = InArgs._Row;
			SMultiColumnTableRow<TSharedPtr<FADAttributeRuntimeRow>>::Construct(
				FSuperRowType::FArguments().Padding(FMargin(4.0f, 2.0f)),
				OwnerTable);
		}

		virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override
		{
			FText Text = FText::GetEmpty();

			if (Row.IsValid())
			{
				if (ColumnName == TEXT("AttributeSet"))
				{
					Text = FText::FromString(Row->AttributeSetName);
				}
				else if (ColumnName == TEXT("Attribute"))
				{
					Text = FText::FromString(Row->AttributeName);
				}
				else if (ColumnName == TEXT("Current"))
				{
					Text = FormatAttributeValue(Row->CurrentValue);
				}
				else if (ColumnName == TEXT("Base"))
				{
					Text = FormatAttributeValue(Row->BaseValue);
				}
			}

			return SNew(STextBlock)
				.Text(Text)
				.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"));
		}

	private:
		TSharedPtr<FADAttributeRuntimeRow> Row;
	};
}

bool FADAttributeRuntimeRow::MatchesFilter(const FString& InFilterText) const
{
	return InFilterText.IsEmpty()
		|| AttributeSetName.Contains(InFilterText, ESearchCase::IgnoreCase)
		|| AttributeName.Contains(InFilterText, ESearchCase::IgnoreCase);
}

FName FADAttributeRuntimeDataPage::GetId() const
{
	return TEXT("Attribute");
}

FText FADAttributeRuntimeDataPage::GetDisplayName() const
{
	return LOCTEXT("AttributePageTitle", "Attribute");
}

bool FADAttributeRuntimeDataPage::SupportsActor(const AActor* Actor) const
{
	return GetAbilitySystemComponent(Actor) != nullptr;
}

TSharedRef<SWidget> FADAttributeRuntimeDataPage::BuildWidget()
{
	StatusText = LOCTEXT("AttributeWaitingForActor", "Select a PIE/SIE actor with an Ability System Component.");

	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("Brushes.Panel"))
		.Padding(8.0f)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSearchBox)
				.HintText(LOCTEXT("AttributeSearchHint", "Search attributes"))
				.OnTextChanged_Raw(this, &FADAttributeRuntimeDataPage::HandleSearchTextChanged)
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
				.OnGenerateRow_Raw(this, &FADAttributeRuntimeDataPage::GenerateRow)
				.HeaderRow
				(
					SNew(SHeaderRow)
					+ SHeaderRow::Column(TEXT("AttributeSet"))
					.DefaultLabel(LOCTEXT("AttributeSetColumn", "Set"))
					.FillWidth(0.28f)
					+ SHeaderRow::Column(TEXT("Attribute"))
					.DefaultLabel(LOCTEXT("AttributeColumn", "Attribute"))
					.FillWidth(0.32f)
					+ SHeaderRow::Column(TEXT("Current"))
					.DefaultLabel(LOCTEXT("CurrentColumn", "Current"))
					.FillWidth(0.20f)
					+ SHeaderRow::Column(TEXT("Base"))
					.DefaultLabel(LOCTEXT("BaseColumn", "Base"))
					.FillWidth(0.20f)
				)
			]
		];
}

void FADAttributeRuntimeDataPage::Refresh(const TWeakObjectPtr<AActor>& SelectedActor)
{
	AllRows.Reset();

	AActor* Actor = SelectedActor.Get();
	if (!IsValid(Actor))
	{
		StatusText = LOCTEXT("AttributeNoActor", "Select a PIE/SIE actor with an Ability System Component.");
		RebuildFilteredRows();
		return;
	}

	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent(Actor);
	if (AbilitySystemComponent == nullptr)
	{
		StatusText = LOCTEXT("AttributeNoASC", "Selected actor does not have a valid Ability System Component.");
		RebuildFilteredRows();
		return;
	}

	for (const UAttributeSet* AttributeSet : AbilitySystemComponent->GetSpawnedAttributes())
	{
		if (AttributeSet == nullptr)
		{
			continue;
		}

		for (TFieldIterator<FProperty> PropertyIt(AttributeSet->GetClass(), EFieldIteratorFlags::IncludeSuper); PropertyIt; ++PropertyIt)
		{
			const FProperty* Property = *PropertyIt;
			if (!FGameplayAttribute::IsGameplayAttributeDataProperty(Property))
			{
				continue;
			}

			const FGameplayAttribute Attribute(const_cast<FProperty*>(Property));
			const FGameplayAttributeData* AttributeData = Attribute.GetGameplayAttributeData(AttributeSet);
			if (AttributeData == nullptr)
			{
				continue;
			}

			FRowPtr Row = MakeShared<FADAttributeRuntimeRow>();
			Row->AttributeSetName = AttributeSet->GetClass()->GetName();
			Row->AttributeName = Attribute.GetName();
			Row->CurrentValue = AttributeData->GetCurrentValue();
			Row->BaseValue = AttributeData->GetBaseValue();
			AllRows.Add(Row);
		}
	}

	StatusText = FText::Format(
		LOCTEXT("AttributeRowCount", "{0} attributes from {1}"),
		FText::AsNumber(AllRows.Num()),
		FText::FromString(Actor->GetActorNameOrLabel()));

	RebuildFilteredRows();
}

UAbilitySystemComponent* FADAttributeRuntimeDataPage::GetAbilitySystemComponent(const AActor* Actor) const
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

void FADAttributeRuntimeDataPage::RebuildFilteredRows()
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

void FADAttributeRuntimeDataPage::HandleSearchTextChanged(const FText& NewText)
{
	FilterText = NewText.ToString();
	RebuildFilteredRows();
}

FText FADAttributeRuntimeDataPage::GetStatusText() const
{
	if (!FilterText.IsEmpty())
	{
		return FText::Format(
			LOCTEXT("AttributeFilteredStatus", "{0} / {1} attributes"),
			FText::AsNumber(FilteredRows.Num()),
			FText::AsNumber(AllRows.Num()));
	}

	return StatusText;
}

TSharedRef<ITableRow> FADAttributeRuntimeDataPage::GenerateRow(FRowPtr Row, const TSharedRef<STableViewBase>& OwnerTable) const
{
	return SNew(SADAttributeRuntimeRowWidget, OwnerTable)
		.Row(Row);
}

#undef LOCTEXT_NAMESPACE
