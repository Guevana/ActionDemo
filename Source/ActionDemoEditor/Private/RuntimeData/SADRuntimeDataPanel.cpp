#include "RuntimeData/SADRuntimeDataPanel.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "RuntimeData/ADAttributeRuntimeDataPage.h"
#include "RuntimeData/ADAbilityRuntimeDataPage.h"
#include "RuntimeData/ADTagRuntimeDataPage.h"
#include "Styling/AppStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SADRuntimeDataPanel"

void SADRuntimeDataPanel::Construct(const FArguments& InArgs)
{
	Pages.Add(MakeShared<FADAttributeRuntimeDataPage>());
	Pages.Add(MakeShared<FADTagRuntimeDataPage>());
	Pages.Add(MakeShared<FADAbilityRuntimeDataPage>());
	ActivePageId = Pages[0]->GetId();

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("Brushes.Panel"))
		.Padding(10.0f)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					BuildActorCombo()
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(8.0f, 0.0f, 0.0f, 0.0f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(this, &SADRuntimeDataPanel::GetRuntimeWorldStatusText)
					.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 10.0f, 0.0f, 6.0f)
			[
				BuildNavigationBar()
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSeparator)
			]

			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			.Padding(0.0f, 8.0f, 0.0f, 0.0f)
			[
				BuildPageSwitcher()
			]
		]
	];

	RefreshActorOptions();
	RefreshActivePage();
	RegisterActiveTimer(0.1f, FWidgetActiveTimerDelegate::CreateSP(this, &SADRuntimeDataPanel::HandleRefreshTimer));
}

EActiveTimerReturnType SADRuntimeDataPanel::HandleRefreshTimer(double InCurrentTime, float InDeltaTime)
{
	RefreshActorOptions();
	RefreshActivePage();
	return EActiveTimerReturnType::Continue;
}

void SADRuntimeDataPanel::RefreshActorOptions()
{
	UWorld* RuntimeWorld = FindRuntimeWorld();

	TArray<FActorOptionPtr> NewOptions;
	if (RuntimeWorld != nullptr)
	{
		for (TActorIterator<AActor> ActorIt(RuntimeWorld); ActorIt; ++ActorIt)
		{
			AActor* Actor = *ActorIt;
			if (!IsInspectableActor(Actor))
			{
				continue;
			}

			FActorOptionPtr Option = MakeShared<FADRuntimeActorOption>();
			Option->Actor = Actor;
			Option->Label = BuildActorLabel(Actor);
			NewOptions.Add(Option);
		}

		NewOptions.Sort([](const FActorOptionPtr& Left, const FActorOptionPtr& Right)
		{
			return Left.IsValid() && Right.IsValid() && Left->Label < Right->Label;
		});
	}

	bool bOptionsChanged = NewOptions.Num() != ActorOptions.Num();
	if (!bOptionsChanged)
	{
		for (int32 Index = 0; Index < NewOptions.Num(); ++Index)
		{
			if (NewOptions[Index]->Actor != ActorOptions[Index]->Actor || NewOptions[Index]->Label != ActorOptions[Index]->Label)
			{
				bOptionsChanged = true;
				break;
			}
		}
	}

	if (bOptionsChanged)
	{
		ActorOptions = MoveTemp(NewOptions);
		if (ActorComboBox.IsValid())
		{
			ActorComboBox->RefreshOptions();
		}
	}

	if (!IsSelectedActorStillAvailable())
	{
		SelectedActor.Reset();
	}
}

void SADRuntimeDataPanel::RefreshActivePage()
{
	for (const TSharedRef<IADRuntimeDataPage>& Page : Pages)
	{
		if (Page->GetId() == ActivePageId)
		{
			Page->Refresh(SelectedActor);
			return;
		}
	}
}

UWorld* SADRuntimeDataPanel::FindRuntimeWorld() const
{
	if (GEngine == nullptr)
	{
		return nullptr;
	}

	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		UWorld* World = WorldContext.World();
		if (World != nullptr && (World->WorldType == EWorldType::PIE || World->WorldType == EWorldType::Game))
		{
			return World;
		}
	}

	return nullptr;
}

UAbilitySystemComponent* SADRuntimeDataPanel::GetAbilitySystemComponent(const AActor* Actor) const
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

bool SADRuntimeDataPanel::IsInspectableActor(const AActor* Actor) const
{
	return IsValid(Actor) && GetAbilitySystemComponent(Actor) != nullptr;
}

bool SADRuntimeDataPanel::IsSelectedActorStillAvailable() const
{
	AActor* Actor = SelectedActor.Get();
	if (!IsInspectableActor(Actor))
	{
		return false;
	}

	for (const FActorOptionPtr& Option : ActorOptions)
	{
		if (Option.IsValid() && Option->Actor == Actor)
		{
			return true;
		}
	}

	return false;
}

FString SADRuntimeDataPanel::BuildActorLabel(const AActor* Actor) const
{
	if (Actor == nullptr)
	{
		return FString();
	}

	return FString::Printf(TEXT("%s (%s)"), *Actor->GetActorNameOrLabel(), *Actor->GetClass()->GetName());
}

TSharedRef<SWidget> SADRuntimeDataPanel::BuildActorCombo()
{
	return SAssignNew(ActorComboBox, SComboBox<FActorOptionPtr>)
		.OptionsSource(&ActorOptions)
		.OnGenerateWidget(this, &SADRuntimeDataPanel::GenerateActorComboRow)
		.OnSelectionChanged(this, &SADRuntimeDataPanel::HandleActorSelectionChanged)
		[
			SNew(STextBlock)
			.Text(this, &SADRuntimeDataPanel::GetSelectedActorText)
			.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
		];
}

TSharedRef<SWidget> SADRuntimeDataPanel::BuildNavigationBar()
{
	SAssignNew(NavigationBar, SHorizontalBox);

	for (const TSharedRef<IADRuntimeDataPage>& Page : Pages)
	{
		const FName PageId = Page->GetId();
		const FText PageDisplayName = Page->GetDisplayName();

		NavigationBar->AddSlot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 6.0f, 0.0f)
			[
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "Button")
				.OnClicked(this, &SADRuntimeDataPanel::HandlePageButtonClicked, PageId)
				[
					SNew(STextBlock)
					.Text(PageDisplayName)
					.ColorAndOpacity_Lambda([this, PageId]()
					{
						return ActivePageId == PageId
							? FSlateColor(FLinearColor(0.1f, 0.75f, 1.0f, 1.0f))
							: FSlateColor::UseForeground();
					})
				]
			];
	}

	return NavigationBar.ToSharedRef();
}

TSharedRef<SWidget> SADRuntimeDataPanel::BuildPageSwitcher()
{
	SAssignNew(PageSwitcher, SWidgetSwitcher);

	for (const TSharedRef<IADRuntimeDataPage>& Page : Pages)
	{
		PageSwitcher->AddSlot()
		[
			Page->BuildWidget()
		];
	}

	SelectPage(ActivePageId);
	return PageSwitcher.ToSharedRef();
}

TSharedRef<SWidget> SADRuntimeDataPanel::GenerateActorComboRow(FActorOptionPtr Option) const
{
	return SNew(STextBlock)
		.Text(FText::FromString(Option.IsValid() ? Option->Label : FString()))
		.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"));
}

FText SADRuntimeDataPanel::GetSelectedActorText() const
{
	if (AActor* Actor = SelectedActor.Get())
	{
		return FText::FromString(BuildActorLabel(Actor));
	}

	if (FindRuntimeWorld() == nullptr)
	{
		return LOCTEXT("NoRuntimeWorld", "Waiting for PIE/SIE runtime world");
	}

	return LOCTEXT("NoActorSelected", "Select an actor");
}

FText SADRuntimeDataPanel::GetRuntimeWorldStatusText() const
{
	if (FindRuntimeWorld() == nullptr)
	{
		return LOCTEXT("RuntimeWorldInactive", "PIE/SIE inactive");
	}

	return FText::Format(LOCTEXT("RuntimeWorldActive", "{0} inspectable actors"), FText::AsNumber(ActorOptions.Num()));
}

void SADRuntimeDataPanel::HandleActorSelectionChanged(FActorOptionPtr NewSelection, ESelectInfo::Type SelectInfo)
{
	SelectedActor = NewSelection.IsValid() ? NewSelection->Actor : nullptr;
	RefreshActivePage();
}

void SADRuntimeDataPanel::SelectPage(FName PageId)
{
	ActivePageId = PageId;

	if (!PageSwitcher.IsValid())
	{
		return;
	}

	for (int32 PageIndex = 0; PageIndex < Pages.Num(); ++PageIndex)
	{
		if (Pages[PageIndex]->GetId() == ActivePageId)
		{
			PageSwitcher->SetActiveWidgetIndex(PageIndex);
			return;
		}
	}
}

FReply SADRuntimeDataPanel::HandlePageButtonClicked(FName PageId)
{
	SelectPage(PageId);
	RefreshActivePage();
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
