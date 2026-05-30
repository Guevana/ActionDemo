#include "Modules/ModuleManager.h"

#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "RuntimeData/SADRuntimeDataPanel.h"
#include "Styling/AppStyle.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "FActionDemoEditorModule"

namespace ActionDemoEditor
{
	static const FName RuntimeDataPanelTabName(TEXT("ActionDemoRuntimeDataPanel"));
}

class FActionDemoEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
			ActionDemoEditor::RuntimeDataPanelTabName,
			FOnSpawnTab::CreateRaw(this, &FActionDemoEditorModule::SpawnRuntimeDataPanelTab))
			.SetDisplayName(LOCTEXT("RuntimeDataPanelTabTitle", "Runtime Data Panel"))
			.SetTooltipText(LOCTEXT("RuntimeDataPanelTooltip", "Inspect runtime ActionDemo actor data during PIE or SIE."))
			.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"))
			.SetMenuType(ETabSpawnerMenuType::Hidden);

		UToolMenus::RegisterStartupCallback(
			FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FActionDemoEditorModule::RegisterMenus));
	}

	virtual void ShutdownModule() override
	{
		UToolMenus::UnRegisterStartupCallback(this);
		UToolMenus::UnregisterOwner(this);

		if (FSlateApplication::IsInitialized())
		{
			FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(ActionDemoEditor::RuntimeDataPanelTabName);
		}
	}

private:
	TSharedRef<SDockTab> SpawnRuntimeDataPanelTab(const FSpawnTabArgs& SpawnTabArgs)
	{
		return SNew(SDockTab)
			.TabRole(ETabRole::NomadTab)
			[
				SNew(SADRuntimeDataPanel)
			];
	}

	void RegisterMenus()
	{
		FToolMenuOwnerScoped OwnerScoped(this);

		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		FToolMenuSection& Section = Menu->FindOrAddSection("ActionDemo", LOCTEXT("ActionDemoMenuSection", "ActionDemo"));

		Section.AddMenuEntry(
			"OpenActionDemoRuntimeDataPanel",
			LOCTEXT("OpenRuntimeDataPanel", "Runtime Data Panel"),
			LOCTEXT("OpenRuntimeDataPanelTooltip", "Open the ActionDemo runtime data inspector."),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"),
			FUIAction(FExecuteAction::CreateRaw(this, &FActionDemoEditorModule::OpenRuntimeDataPanel)));
	}

	void OpenRuntimeDataPanel()
	{
		FGlobalTabmanager::Get()->TryInvokeTab(ActionDemoEditor::RuntimeDataPanelTabName);
	}
};

IMPLEMENT_MODULE(FActionDemoEditorModule, ActionDemoEditor)

#undef LOCTEXT_NAMESPACE
