#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

/**
 * 项目级原生标签声明。
 * 后续新增核心战斗标签时，优先在此处集中维护。
 */
namespace ADGameplayTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Action_Airborne);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Dead);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Hit_React);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Cancel_Active);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Attack_Light_01);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Attack_Light_02);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Attack_Light_03);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Target_Lock);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Hit_React);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Hit_Confirm);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Target_Locked);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Target_Unlocked);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Invincible);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Damage);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Attack_Light);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Dodge);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Target_Lock);
}
