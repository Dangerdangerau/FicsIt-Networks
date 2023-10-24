#pragma once

#include "FIVSScriptNode.h"
#include "FIVSNode_DownCast.generated.h"

UCLASS()
class UFIVSNode_DownCast : public UFIVSScriptNode {
	GENERATED_BODY()
private:
	UPROPERTY()
	UFIVSPin* ExecInput = nullptr;
	UPROPERTY()
	UFIVSPin* DataInput = nullptr;
	UPROPERTY()
	UFIVSPin* ExecOutput = nullptr;
	UPROPERTY()
	UFIVSPin* SuccessOutput = nullptr;
	UPROPERTY()
	UFIVSPin* DataOutput = nullptr;
	UPROPERTY()
	bool bPure = true;

	UPROPERTY()
	UFINClass* ToClass = nullptr;
	
public:
	// Begin UFIVSNode
	virtual void GetNodeActions(TArray<FFIVSNodeAction>& Actions) const override;
	virtual void InitPins() override;
	virtual void SerializeNodeProperties(FFIVSNodeProperties& Properties) const override;
	virtual void DeserializeNodeProperties(const FFIVSNodeProperties& Properties) override;
	// End UFIVSNode
	
	// Begin UFIVSScriptNode
	virtual FString GetNodeName() const override;
	virtual TArray<UFIVSPin*> PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;
	virtual UFIVSPin* ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;
	// End UFIVSScriptNode
};
