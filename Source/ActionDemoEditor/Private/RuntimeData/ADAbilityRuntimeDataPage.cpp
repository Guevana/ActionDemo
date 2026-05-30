#include "RuntimeData/ADAbilityRuntimeDataPage.h"

#include "Abilities/GameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpec.h"
#include "Styling/AppStyle.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Widgets/Views/STableRow.h"

#define LOCTEXT_NAMESPACE "FADAbilityRuntimeDataPage"

namespace
{
	static FSlateColor GetAbilityTextColor(const TSharedPtr<FADAbilityRuntimeRow>& Row)
	{
		return Row.IsValid() && Row->bIsActive
			? FSlateColor(FLinearColor(0.1f, 0.75f, 1.0f, 1.0f))
			: FSlateColor::UseForeground();
	}

	class SADAbilityRuntimeRowWidget : public SMultiColumnTableRow<TSharedPtr<FADAbilityRuntimeRow>>
	{
	public:
		SLATE_BEGIN_ARGS(SADAbilityRuntimeRowWidget) {}
			SLATE_ARGUMENT(TSharedPtr<FADAbilityRuntimeRow>, Row)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTable)
		{
			Row = InArgs._Row;
			SMultiColumnTableRow<TSharedPtr<FADAbilityRuntimeRow>>::Construct(
				FSuperRowType::FArguments().Padding(FMargin(4.0f, 2.0f)),
				OwnerTable);
		}

		virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override
		{
			FText Text = FText::GetEmpty();

			if (Row.IsValid())
			{
				if (ColumnName == TEXT("Ability"))
				{
					Text = FText::FromString(Row->AbilityName);
				}
				else if (ColumnName == TEXT("Level"))
				{
					Text = FText::AsNumber(Row->Level);
				}
				else if (ColumnName == TEXT("Input"))
				{
					Text = FText::FromString(Row->InputText);
				}
				else if (ColumnName == TEXT("State"))
				{
					Text = FText::FromString(Row->GetStateText());
				}
				else if (ColumnName == TEXT("Tags"))
				{
					Text = FText::FromString(Row->AbilityTags);
				}
			}

			return SNew(STextBlock)
				.Text(Text)
				.ColorAndOpacity(GetAbilityTextColor(Row))
				.Font(FAppStyle::GetFontStyle(Row.IsValid() && Row->bIsActive ? "PropertyWindow.BoldFont" : "PropertyWindow.NormalFont"));
		}

	private:
		TSharedPtr<FADAbilityRuntimeRow> Row;
	};
}

FString FADAbilityRuntimeRow::GetStateText() const
{
	return bIsActive
		? FString::Printf(TEXT("Active (%d)"), ActiveCount)
		: TEXT("Inactive");
}

bool FADAbilityRuntimeRow::MatchesFilter(const FString& InFilterText) const
{
	return InFilterText.IsEmpty()
		|| AbilityName.Contains(InFilterText, ESearchCase::IgnoreCase)
		|| AbilityTags.Contains(InFilterText, ESearchCase::IgnoreCase)
		|| GetStateText().Contains(InFilterText, ESearchCase::IgnoreCase);
}

FName FADAbilityRuntimeDataPage::GetId() const
{
	return TEXT("Ability");
}

FText FADAbilityRuntimeDataPage::GetDisplayName() const
{
	return LOCTEXT("AbilityPageTitle", "Ability");
}

bool FADAbilityRuntimeDataPage::SupportsActor(const AActor* Actor) const
{
	return GetAbilitySystemComponent(Actor) != nullptr;
}

TSharedRef<SWidget> FADAbilityRuntimeDataPage::BuildWidget()
{
	StatusText = LOCTEXT("AbilityWaitingForActor", "Select a PIE/SIE actor with an Ability System Component.");

	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("Brushes.Panel"))
		.Padding(8.0f)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSearchBox)
				.HintText(LOCTEXT("AbilitySearchHint", "Search abilities"))
				.OnTextChanged_Raw(this, &FADAbilityRuntimeDataPage::HandleSearchTextChanged)
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
				.OnGenerateRow_Raw(this, &FADAbilityRuntimeDataPage::GenerateRow)
				.HeaderRow
				(
					SNew(SHeaderRow)
					+ SHeaderRow::Column(TEXT("Ability"))
					.DefaultLabel(LOCTEXT("AbilityColumn", "Ability"))
					.FillWidth(0.34f)
					+ SHeaderRow::Column(TEXT("Level"))
					.DefaultLabel(LOCTEXT("LevelColumn", "Level"))
					.FillWidth(0.10f)
					+ SHeaderRow::Column(TEXT("Input"))
					.DefaultLabel(LOCTEXT("InputColumn", "Input"))
					.FillWidth(0.12f)
					+ SHeaderRow::Column(TEXT("State"))
					.DefaultLabel(LOCTEXT("StateColumn", "State"))
					.FillWidth(0.16f)
					+ SHeaderRow::Column(TEXT("Tags"))
					.DefaultLabel(LOCTEXT("TagsColumn", "Tags"))
					.FillWidth(0.28f)
				)
			]
		];
}

void FADAbilityRuntimeDataPage::Refresh(const TWeakObjectPtr<AActor>& SelectedActor)
{
	AllRows.Reset();

	AActor* Actor = SelectedActor.Get();
	if (!IsValid(Actor))
	{
		StatusText = LOCTEXT("AbilityNoActor", "Select a PIE/SIE actor with an Ability System Component.");
		RebuildFilteredRows();
		return;
	}

	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent(Actor);
	if (AbilitySystemComponent == nullptr)
	{
		StatusText = LOCTEXT("AbilityNoASC", "Selected actor does not have a valid Ability System Component.");
		RebuildFilteredRows();
		return;
	}

	FScopedAbilityListLock AbilityListLock(*AbilitySystemComponent);
	for (const FGameplayAbilitySpec& Spec : AbilitySystemComponent->GetActivatableAbilities())
	{
		FRowPtr Row = MakeShared<FADAbilityRuntimeRow>();
		Row->AbilityName = GetAbilityDisplayName(Spec.Ability);
		Row->AbilityTags = Spec.Ability != nullptr ? Spec.Ability->GetAssetTags().ToStringSimple() : FString();
		Row->InputText = Spec.InputID == INDEX_NONE ? TEXT("None") : FString::FromInt(Spec.InputID);
		Row->Level = Spec.Level;
		Row->ActiveCount = Spec.ActiveCount;
		Row->bIsActive = Spec.IsActive();
		AllRows.Add(Row);
	}

	AllRows.Sort([](const FRowPtr& Left, const FRowPtr& Right)
	{
		if (!Left.IsValid() || !Right.IsValid())
		{
			return Left.IsValid();
		}

		if (Left->bIsActive != Right->bIsActive)
		{
			return Left->bIsActive;
		}

		return Left->AbilityName < Right->AbilityName;
	});

	const int32 ActiveAbilityCount = AllRows.FilterByPredicate([](const FRowPtr& Row)
	{
		return Row.IsValid() && Row->bIsActive;
	}).Num();

	StatusText = FText::Format(
		LOCTEXT("AbilityRowCount", "{0} abilities from {1}, {2} active"),
		FText::AsNumber(AllRows.Num()),
		FText::FromString(Actor->GetActorNameOrLabel()),
		FText::AsNumber(ActiveAbilityCount));

	RebuildFilteredRows();
}

UAbilitySystemComponent* FADAbilityRuntimeDataPage::GetAbilitySystemComponent(const AActor* Actor) const
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

FString FADAbilityRuntimeDataPage::GetAbilityDisplayName(const UGameplayAbility* Ability) const
{
	if (Ability == nullptr)
	{
		return TEXT("None");
	}

	FString AbilityName = Ability->GetClass()->GetName();
	AbilityName.RemoveFromEnd(TEXT("_C"));
	return AbilityName;
}

void FADAbilityRuntimeDataPage::RebuildFilteredRows()
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

void FADAbilityRuntimeDataPage::HandleSearchTextChanged(const FText& NewText)
{
	FilterText = NewText.ToString();
	RebuildFilteredRows();
}

FText FADAbilityRuntimeDataPage::GetStatusText() const
{
	if (!FilterText.IsEmpty())
	{
		return FText::Format(
			LOCTEXT("AbilityFilteredStatus", "{0} / {1} abilities"),
			FText::AsNumber(FilteredRows.Num()),
			FText::AsNumber(AllRows.Num()));
	}

	return StatusText;
}

TSharedRef<ITableRow> FADAbilityRuntimeDataPage::GenerateRow(FRowPtr Row, const TSharedRef<STableViewBase>& OwnerTable) const
{
	return SNew(SADAbilityRuntimeRowWidget, OwnerTable)
		.Row(Row);
}

#undef LOCTEXT_NAMESPACE
