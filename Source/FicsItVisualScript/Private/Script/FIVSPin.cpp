#include "Script/FIVSPin.h"


bool UFIVSPin::VisitAllConnected(TSet<UFIVSPin*>& Searches, TFunction<bool(UFIVSPin*)> Callback) {
	if (Searches.Contains(this)) return true;
	Searches.Add(this);
	if (!Callback(this)) return false;
	for (UFIVSPin* Pin : GetConnections()) {
		if (!Pin->VisitAllConnected(Searches, Callback)) {
			return false;
		}
	}
	return true;
}

UFIVSPin* UFIVSPin::FindConnected() {
	EFIVSPinType PinType = (GetPinType() & FIVS_PIN_DATA) != 0 ? FIVS_PIN_OUTPUT : FIVS_PIN_INPUT;
	for (UFIVSPin* Pin : GetAllConnected()) {
		if (Pin->IsA<UFIVSWildcardPin>()) continue;
		if (Pin->GetPinType() & PinType) return Pin;
	}
	return nullptr;
}

UFIVSPin* UFIVSPin::FindNextConnected(EFIVSPinType PinType) {
	UFIVSPin* NextPin = nullptr;
	for (UFIVSPin* connection : GetConnections()) {
		TSet<UFIVSPin*> Searches{this};
		connection->VisitAllConnected(Searches, [&](UFIVSPin* Pin) {
			if (Pin->IsA<UFIVSWildcardPin>()) return true;
			if ((Pin->GetPinType() & PinType) == PinType) {
				NextPin = Pin;
				return false;
			}
			return true;
		});
		if (NextPin) break;
	}
	return NextPin;
}

void UFIVSPin::AddConnection(UFIVSPin* Pin) {
	if (!CanConnect(Pin) || !Pin->CanConnect(this)) return;
	ConnectedPins.Add(Pin);
	Pin->ConnectedPins.Add(this);
}

void UFIVSPin::AddConnections(TArray<UFIVSPin*> Pins) {
	for (UFIVSPin* pin : Pins) {
		AddConnection(pin);
	}
}

void UFIVSPin::RemoveConnection(UFIVSPin* Pin) {
	if (ConnectedPins.Contains(Pin)) ConnectedPins.Remove(Pin);
	if (Pin->ConnectedPins.Contains(this)) Pin->ConnectedPins.Remove(this);
}

bool FFIVSFullPinType::CanConnect(const FFIVSFullPinType& Other) const {
	const FFIVSFullPinType* Output = this;
	const FFIVSFullPinType* Input = &Other;
	if (Output->PinType & FIVS_PIN_INPUT && Input->PinType & FIVS_PIN_OUTPUT) {
		Swap(Output, Input);
	}
	if (!(Output->PinType & FIVS_PIN_OUTPUT && Input->PinType & FIVS_PIN_INPUT)) return false;

	bool bWouldFail = true;
	if (Output->PinType & FIVS_PIN_DATA) {
		bWouldFail = false;
		if (!(Input->PinType & FIVS_PIN_DATA)) bWouldFail = true;
		else if (!Output->DataType.IsA(Input->DataType) && Output->PinType != (FIVS_PIN_DATA_INPUT | FIVS_PIN_EXEC_OUTPUT) && Input->PinType != (FIVS_PIN_DATA_INPUT | FIVS_PIN_EXEC_OUTPUT) /* Exclude wildcard pins with no yet defined state */) bWouldFail = true;
	}
	if (bWouldFail && Output->PinType & FIVS_PIN_EXEC) {
		bWouldFail = false;
		if (!(Input->PinType & FIVS_PIN_EXEC)) bWouldFail = true;
	}
	if (bWouldFail) return false;
	return true;
}

EFIVSPinType UFIVSPin::GetPinType() {
	return FIVS_PIN_NONE;
}

FFIVSPinDataType UFIVSPin::GetPinDataType() {
	return FFIVSPinDataType(FIR_NIL);
}

const TArray<UFIVSPin*>& UFIVSPin::GetConnections() {
	return ConnectedPins;
}

bool UFIVSPin::CanConnect(UFIVSPin* Pin) {
	if (ConnectedPins.Contains(Pin) || Pin == this) return false;

	UFIVSPin* OutputPin = this;
	UFIVSPin* InputPin = Pin;
	EFIVSPinType OutputPinType = GetPinType();
	EFIVSPinType InputPinType = Pin->GetPinType();
	FFIVSPinDataType OutputPinDataType = GetPinDataType();
	FFIVSPinDataType InputPinDataType = Pin->GetPinDataType();

	// Swap Input and Output Pins if it makes sense to swap them
	if (OutputPinType & FIVS_PIN_INPUT && InputPinType & FIVS_PIN_OUTPUT) {
		Swap(OutputPinType, InputPinType);
		Swap(OutputPinDataType, InputPinDataType);
		Swap(OutputPin, InputPin);
	}
	if (!(OutputPinType & FIVS_PIN_OUTPUT && InputPinType & FIVS_PIN_INPUT)) return false;

	if (!FFIVSFullPinType(OutputPinType, OutputPinDataType).CanConnect(FFIVSFullPinType(InputPinType, InputPinDataType))) return false;

	bool bThisHasInput = false;
	bool bPinHasInput = false;
	bool bThisHasOutput = false;
	bool bPinHasOutput = false;
	OutputPin->VisitAllConnected([&](UFIVSPin* Pin) {
		if (Cast<UFIVSWildcardPin>(Pin)) return true;
		if (Pin->GetPinType() & FIVS_PIN_INPUT) {
			bThisHasOutput = true;
		}
		if (Pin->GetPinType() & FIVS_PIN_OUTPUT) {
			bThisHasInput = true;
		}
		return true;
	});
	InputPin->VisitAllConnected([&](UFIVSPin* Pin) {
		if (Cast<UFIVSWildcardPin>(Pin)) return true;
		if (Pin->GetPinType() & FIVS_PIN_INPUT) {
			bPinHasOutput = true;
		}
		if (Pin->GetPinType() & FIVS_PIN_OUTPUT) {
			bPinHasInput = true;
		}
		return true;
	});

	if (OutputPinType & FIVS_PIN_DATA) {
		if (bThisHasInput && bPinHasInput) return false;
	} else if (OutputPinType & FIVS_PIN_EXEC) {
		if (bThisHasOutput && bPinHasOutput) return false;
	}
	return true;
}

TArray<UFIVSPin*> UFIVSPin::RemoveAllConnections() {
	TArray<UFIVSPin*> Connections = GetConnections();
	for (UFIVSPin* Connection : Connections) {
		RemoveConnection(Connection);
	}
	return Connections;
}

EFIVSPinType UFIVSGenericPin::GetPinType() {
	return PinType;
}

FFIVSPinDataType UFIVSGenericPin::GetPinDataType() {
	return PinDataType;
}

UFIVSGenericPin* UFIVSGenericPin::Create(FFIVSPinDataType DataType, EFIVSPinType PinType, const FString& Name, const FText& DisplayName) {
	UFIVSGenericPin* Pin = NewObject<UFIVSGenericPin>();
	Pin->Name = Name;
	Pin->DisplayName = DisplayName;
	Pin->PinDataType = DataType;
	Pin->PinType = PinType;
	return Pin;
}

EFIVSPinType UFIVSWildcardPin::GetPinType() {
	TSet<UFIVSPin*> Connected = GetAllConnected();
	EFIVSPinType Type = (EFIVSPinType)(FIVS_PIN_EXEC | FIVS_PIN_DATA);
	for (UFIVSPin* Pin : Connected) {
		if (Cast<UFIVSWildcardPin>(Pin)) continue;
		EFIVSPinType ConnectedPinType = Pin->GetPinType();
		if (ConnectedPinType & FIVS_PIN_DATA) {
			if (ConnectedPinType & FIVS_PIN_OUTPUT) {
				return (EFIVSPinType)(FIVS_PIN_DATA | FIVS_PIN_OUTPUT);
			}
			Type = FIVS_PIN_DATA;
		}
		if (ConnectedPinType & FIVS_PIN_EXEC) {
			if (ConnectedPinType & FIVS_PIN_INPUT) {
				return (EFIVSPinType)(FIVS_PIN_EXEC | FIVS_PIN_INPUT);
			}
			Type = FIVS_PIN_EXEC;
		}
	}
	return (EFIVSPinType)(Type | FIVS_PIN_INPUT | FIVS_PIN_OUTPUT);
}

FFIVSPinDataType UFIVSWildcardPin::GetPinDataType() {
	FFIVSPinDataType Type = FFIVSPinDataType(FFIRExtendedValueType(FIR_ANY));
	VisitAllConnected([&](UFIVSPin* Pin) {
		if (Cast<UFIVSWildcardPin>(Pin)) return true;
		Type = Pin->GetPinDataType();
		return false;
	});
	return Type;
}

bool UFIVSWildcardPin::CanConnect(UFIVSPin* Pin) {
	return UFIVSPin::CanConnect(Pin);
}
