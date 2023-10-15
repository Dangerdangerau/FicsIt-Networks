﻿#pragma once

#include "ModuleSystem/FINModuleSystemModule.h"
#include "ModuleSystem/FINModuleSystemPanel.h"
#include "Network/Signals/FINSignalSender.h"
#include "Buildables/FGBuildable.h"
#include "FINModuleBase.generated.h"

UCLASS()
class AFINModuleBase : public AFGBuildable, public IFINModuleSystemModule, public IFINSignalSender {
	GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly)
    FVector2D ModuleSize;

	UPROPERTY(EditDefaultsOnly, Replicated)
    FName ModuleName;

	UPROPERTY(BlueprintReadOnly, SaveGame)
    FVector ModulePos;

	UPROPERTY(BlueprintReadOnly, SaveGame)
    UFINModuleSystemPanel* ModulePanel = nullptr;

	// Begin AActor
	virtual void EndPlay(EEndPlayReason::Type reason) override;
	// End AActor

	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override;
	// End IFGSaveInterface

	// Begin IFINModuleSystemModule
	virtual void setPanel_Implementation(UFINModuleSystemPanel* Panel, int X, int Y, int Rot) override;
	virtual void getModuleSize_Implementation(int& Width, int& Height) const override;
	virtual FName getName_Implementation() const override;
	// End IFINModuleSystemModule

	// Begin IFINSignalSender
	virtual UObject* GetSignalSenderOverride_Implementation() override;
	// End IFINSignalSender
};
