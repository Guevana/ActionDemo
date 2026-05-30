#pragma once

#include "CoreMinimal.h"

class AActor;
class SWidget;

class IADRuntimeDataPage
{
public:
	virtual ~IADRuntimeDataPage() = default;

	virtual FName GetId() const = 0;
	virtual FText GetDisplayName() const = 0;
	virtual bool SupportsActor(const AActor* Actor) const = 0;
	virtual TSharedRef<SWidget> BuildWidget() = 0;
	virtual void Refresh(const TWeakObjectPtr<AActor>& SelectedActor) = 0;
};
