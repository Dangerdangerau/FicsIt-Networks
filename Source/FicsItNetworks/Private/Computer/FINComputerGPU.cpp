﻿#include "Computer/FINComputerGPU.h"
#include "Graphics/FINScreenInterface.h"

AFINComputerGPU::AFINComputerGPU() {
	SetActorTickEnabled(true);
	PrimaryActorTick.SetTickFunctionEnable(true);
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void AFINComputerGPU::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AFINComputerGPU, ScreenPtr);
	DOREPLIFETIME(AFINComputerGPU, Screen);
}

void AFINComputerGPU::BeginPlay() {
	Super::BeginPlay();
	if (HasAuthority()) ScreenPtr = Screen.Get();
}

void AFINComputerGPU::TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction) {
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);

	if (HasAuthority() && (((bool)ScreenPtr) != Screen.IsValid())) {
		if (!ScreenPtr) ScreenPtr = Screen.Get();
		OnValidationChanged(Screen.IsValid(), ScreenPtr);
		ScreenPtr = Screen.Get();
		ForceNetUpdate();
	}
	if (bShouldCreate) {
		bShouldCreate = false;
		if (!ScreenPtr) return;
		if (!Widget.IsValid()) Widget = CreateWidget();
		Cast<IFINScreenInterface>(ScreenPtr)->SetWidget(Widget);
	}
	if (bScreenChanged) {
		bScreenChanged = false;
		ForceNetUpdate();
	}
}

void AFINComputerGPU::EndPlay(const EEndPlayReason::Type endPlayReason) {
	Super::EndPlay(endPlayReason);
	if (endPlayReason == EEndPlayReason::Destroyed) BindScreen(FFINNetworkTrace());
}

void AFINComputerGPU::BindScreen(const FFINNetworkTrace& screen) {
	if (IsValid(screen.GetUnderlyingPtr())) check(screen->GetClass()->ImplementsInterface(UFINScreenInterface::StaticClass()))
	if (Screen == screen) return;
	FFINNetworkTrace oldScreen = Screen;
	Screen = FFINNetworkTrace();
	if (oldScreen.IsValid()) Cast<IFINScreenInterface>(oldScreen.Get())->BindGPU(FFINNetworkTrace());
	if (!screen.IsValid()) DropWidget();
	Screen = screen;
	if (screen.IsValid()) Cast<IFINScreenInterface>(screen.Get())->BindGPU(screen / this);
	ScreenPtr = screen.Get();
	netSig_ScreenBound(oldScreen);
	bScreenChanged = true;
}

FFINNetworkTrace AFINComputerGPU::GetScreen() const {
	return Screen;
}

void AFINComputerGPU::RequestNewWidget() {
	bShouldCreate = true;
}

void AFINComputerGPU::DropWidget() {
	Widget.Reset();
	if (ScreenPtr) Cast<IFINScreenInterface>(ScreenPtr)->SetWidget(nullptr);
}

void AFINComputerGPU::OnValidationChanged_Implementation(bool bValid, UObject* newScreen) {
	if (!bValid) DropWidget();
	else RequestNewWidget();
}

TSharedPtr<SWidget> AFINComputerGPU::CreateWidget() {
	return nullptr;
}

int AFINComputerGPU::MouseToInt(const FPointerEvent& MouseEvent) {
	int mouseEvent = 0;
	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))	mouseEvent |= 0b0000000001;
	if (MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton))	mouseEvent |= 0b0000000010;
	if (MouseEvent.IsControlDown())								mouseEvent |= 0b0000000100;
	if (MouseEvent.IsShiftDown())								mouseEvent |= 0b0000001000;
	if (MouseEvent.IsAltDown())									mouseEvent |= 0b0000010000;
	if (MouseEvent.IsCommandDown())								mouseEvent |= 0b0000100000;
	return mouseEvent;
}

int AFINComputerGPU::InputToInt(const FInputEvent& KeyEvent) {
	int mouseEvent = 0;
	if (KeyEvent.IsControlDown())								mouseEvent |= 0b0000000100;
	if (KeyEvent.IsShiftDown())									mouseEvent |= 0b0000001000;
	if (KeyEvent.IsAltDown())									mouseEvent |= 0b0000010000;
	if (KeyEvent.IsCommandDown())								mouseEvent |= 0b0000100000;
	return mouseEvent;
}

void AFINComputerGPU::netFunc_bindScreen(FFINNetworkTrace NewScreen) {
	if (Cast<IFINScreenInterface>(NewScreen.GetUnderlyingPtr())) BindScreen(NewScreen);
}

FVector2D AFINComputerGPU::netFunc_getScreenSize() {
	return Widget->GetCachedGeometry().GetLocalSize();
}

void AFINComputerGPU::netSig_ScreenBound_Implementation(const FFINNetworkTrace& oldScreen) {}

void UFINScreenWidget::OnNewWidget() {
	if (Container.IsValid()) {
		if (Screen && Cast<IFINScreenInterface>(Screen)->GetWidget().IsValid()) {
			Container->SetContent(Cast<IFINScreenInterface>(Screen)->GetWidget().ToSharedRef());
		} else {
			Container->SetContent(SNew(SBox));
		}
	}
}

void UFINScreenWidget::OnNewGPU() {
	if (Screen && Cast<IFINScreenInterface>(Screen)->GetGPU()) {
		Cast<IFINScreenInterface>(Screen)->RequestNewWidget();
	} else if (Container.IsValid()) {
		Container->SetContent(SNew(SBox));
	}
}

void UFINScreenWidget::SetScreen(UObject* NewScreen) {
	Screen = NewScreen;
	if (Screen) {
		if (Container.IsValid()) {
			Cast<IFINScreenInterface>(Screen)->RequestNewWidget();
		}
	}
}

UObject* UFINScreenWidget::GetScreen() {
	return Screen;
}

void UFINScreenWidget::Focus() {
	if (this->Screen) {
		TSharedPtr<SWidget> widget = Cast<IFINScreenInterface>(this->Screen)->GetWidget();
		if (widget.IsValid()) {
			FSlateApplication::Get().SetKeyboardFocus(widget);
		}
	}
}

void UFINScreenWidget::ReleaseSlateResources(bool bReleaseChildren) {
	Super::ReleaseSlateResources(bReleaseChildren);

	/*if (Screen) {
		IFINGPUInterface* GPU = Cast<IFINGPUInterface>(Cast<IFINScreenInterface>(Screen)->GetGPU());
		if (GPU) GPU->DropWidget();
		OnNewWidget();
	}
	Container.Reset();*/
}

TSharedRef<SWidget> UFINScreenWidget::RebuildWidget() {
	Container = SNew(SBox).HAlign(HAlign_Fill).VAlign(VAlign_Fill);
	OnNewWidget();
	return Container.ToSharedRef();
}
