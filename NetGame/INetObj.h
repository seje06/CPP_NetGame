#pragma once
#include "NetworkEnums.h"
#include "string"
#include <cstdint>
/// <summary>
/// ���� �ؾ��� �Լ���
///virtual void ReadReplicatedData(class InputMemoryStream* inputStrm, uint16_t time) abstract;
///virtual void WirteReplicatedData(class OutputMemoryStream* outputStrm) abstract;
///virtual void ReadRPCData(class InputMemoryStream* inputStrm) abstract;
///virtual void WirteRPCData(class OutputMemoryStream* outputStrm, std::string funcName, void* paramData = nullptr, int paramSize = 0) abstract;
///virtual void NetMulticast(std::string funcName, void* paramData = nullptr, int paramSize = 0) abstract; //���� ������
/// </summary>
class INetObj
{
public:
	virtual void ReadReplicatedData(class InputMemoryStream* inputStrm, uint16_t time) abstract;
	virtual void WirteReplicatedData(class OutputMemoryStream* outputStrm) abstract;
	virtual void ReadRPCData(class InputMemoryStream* inputStrm) abstract;
	virtual void WirteRPCData(class OutputMemoryStream* outputStrm, std::string funcName, void* paramData = nullptr, int paramSize = 0) abstract;
	virtual void NetMulticast(std::string funcName, void* paramData = nullptr, int paramSize = 0) abstract; //���� ������


	virtual ~INetObj() = default;
};