#include "Enemy.h"
#include "GameTime.h"
#include "MemoryStream.h"
#include "NetGameBuildInfo.h"
#include "NetworkEnums.h"
#include "NetworkManager.h"
#include "GameManager.h"
#include "IManageable.h"

void Enemy::ReadReplicatedData(InputMemoryStream* inputStrm, uint16_t time)
{
	float fixedTime = 0;
#if SERVER
	fixedTime = (int)time + (int)(NetworkServerManager::GetInstance()->playerInfoMap[id].currentTimeDif * 100); //�ð��� ������������ ������Ʈ
#else
	fixedTime = (int)time - (int)(NetworkClientManager::GetInstance()->currentTimeDif * 100); // �ð��� Ŭ��������� ������Ʈ
#endif
	float timeToAdd = (int)GameTime::GetMSTimeOfPC() - fixedTime; //�Լ� ȣ���� ��û�� �ð����κ��� ���� �ð�
	if (timeToAdd > 5000) timeToAdd -= 10000;
	else if (timeToAdd < -5000) timeToAdd += 10000; // ����
	timeToAdd /= 100.0f; // ����ȭ

	//������ ������ �б�
	inputStrm->Read(&jumpInfo, sizeof(JumpeInfo));
	inputStrm->Read(&pos, sizeof(Pos));
	inputStrm->Read(&speed, sizeof(float));
	inputStrm->Read(&dir, sizeof(uint8_t));
	inputStrm->Read(&aniIndex, sizeof(uint8_t));
	inputStrm->Read(&color, sizeof(COLOR));
	inputStrm->Read(&hitTime, sizeof(float));
	inputStrm->Read(&hp, sizeof(int));
	inputStrm->Read(&isDie, sizeof(bool));
	inputStrm->Read(&isHitting, sizeof(bool));
	inputStrm->Read(&type, sizeof(ENEMY_TYPE));
	inputStrm->Read(&isActive, sizeof(bool));
}

void Enemy::WirteReplicatedData(OutputMemoryStream* outputStrm)
{
#if SERVER
	EINetObjType objType = EINetObjType::AI;
	outputStrm->Write(&objType, sizeof(EINetObjType)); //������ ai�� �÷��̾� ������ �Ѵ� ������ �����ϱ� Ÿ���� �˷�����Ѵ�.

	outputStrm->Write(&id, sizeof(uint8_t)); //��Ŷ�� ������ �ȴٸ� ���� AI���� �����Ͱ� ���ϱ� �����͸� ���� id�� �־�����Ѵ�.
#endif									  
	EDataType dataType = EDataType::Replicated;
	outputStrm->Write(&dataType, sizeof(EDataType)); // ReplicatedŸ���̶�� �˷��ֱ�

	//������ ������ ����
	outputStrm->Write(&jumpInfo, sizeof(JumpeInfo));
	outputStrm->Write(&pos, sizeof(Pos));
	outputStrm->Write(&speed, sizeof(float));
	outputStrm->Write(&dir, sizeof(uint8_t));
	outputStrm->Write(&aniIndex, sizeof(uint8_t));
	outputStrm->Write(&color, sizeof(COLOR));
	outputStrm->Write(&hitTime, sizeof(float));
	outputStrm->Write(&hp, sizeof(int));
	outputStrm->Write(&isDie, sizeof(bool));
	outputStrm->Write(&isHitting, sizeof(bool));
	outputStrm->Write(&type, sizeof(ENEMY_TYPE));
	outputStrm->Write(&isActive, sizeof(bool));
}

void Enemy::ReadRPCData(InputMemoryStream* inputStrm)
{
	float fixedTime = 0;
	uint16_t time;
	inputStrm->Read(&time, 2);
#if SERVER
	fixedTime = (int)time + (int)(NetworkServerManager::GetInstance()->playerInfoMap[id].currentTimeDif * 100); //�ð��� ������������ ������Ʈ
#else
	fixedTime = (int)time - (int)(NetworkClientManager::GetInstance()->currentTimeDif * 100); // �ð��� Ŭ��������� ������Ʈ
#endif
	float timeToAdd = (int)GameTime::GetMSTimeOfPC() - fixedTime; //�Լ� ȣ���� ��û�� �ð����κ��� ���� �ð�
	if (timeToAdd > 5000) timeToAdd -= 10000;
	else if (timeToAdd < -5000) timeToAdd += 10000; // ����
	timeToAdd /= 100.0f; // ����ȭ

	uint8_t funcSize;
	inputStrm->Read(&funcSize, 1);
	char* funcName = new char[funcSize];
	inputStrm->Read(funcName, funcSize); // �Լ��� �б�. �ι��� ����
	string str(funcName);


	delete[] funcName;
}

void Enemy::WirteRPCData(OutputMemoryStream* outputStrm, std::string funcName, void* paramData, int paramSize)
{
#if SERVER
	EINetObjType objType = EINetObjType::AI;
	outputStrm->Write(&objType, sizeof(EINetObjType)); //������ ai�� �÷��̾� ������ �Ѵ� ������ �����ϱ� Ÿ���� �˷�����Ѵ�.

	outputStrm->Write(&id, sizeof(uint8_t)); //��Ŷ�� ������ �ȴٸ� ���� AI�� �����Ͱ� ���ϱ� �����͸� ���� id�� �־�����Ѵ�.
#endif	
	//Ŭ��� AI�� RPC�Ұ� ���� �ʴ�

	EDataType dataType = EDataType::RPC;
	outputStrm->Write(&dataType, sizeof(EDataType)); // RPCŸ���̶�� �˷��ֱ�
	uint16_t time = GameTime::GetMSTimeOfPC();
	outputStrm->Write(&time, sizeof(uint16_t)); //�Լ��� ���� �� �ð�
	uint8_t size = funcName.size() + 1; //�ι��ڵ� ���� �ؾ��ؼ� ũ�� 1����
	outputStrm->Write(&size, sizeof(uint8_t)); // ���ڿ� ���� ���� ����.
	outputStrm->Write(funcName.c_str(), size); // �ι��ڵ� ���Ե�.
	if (paramData != nullptr) outputStrm->Write(paramData, paramSize); //���ڰ� �־��ֱ� 
}

void Enemy::NetMulticast(std::string funcName, void* paramData, int paramSize)
{
#if SERVER
	GameManager* gameManager = nullptr;
	NetworkServerManager::GameInfo* gameInfo = nullptr;
	for (auto& gameInfoIt : NetworkServerManager::GetInstance()->gameInfoMap)
	{
		if (gameInfoIt.second.gameManager->GetObjects()->enemies[id].get() == this)
		{
			gameManager = gameInfoIt.second.gameManager.get();
			gameInfo = &gameInfoIt.second;
			break;
		}
	}
	if (!gameManager) return;


	for (auto& playerId : gameInfo->playerIdArray)// �ش� ������ ��� �÷��̾� id
	{
		auto networkManager = NetworkServerManager::GetInstance();
		std::lock_guard<std::mutex> lk(networkManager->playerInfoMap[playerId].m); //stageOutputStrm�� ���Ӿ����忡�� ������ ���ν����忡�� ������ ū�� ���� ������ ���� ���ش�.

		WirteRPCData(NetworkServerManager::GetInstance()->playerInfoMap[playerId].stageDataOutputStrm, funcName); //�� �÷��̾��� stageOutputStrm���ٰ� �ش��ϴ� AI�� rpc �߰� 
	}
#endif
}
