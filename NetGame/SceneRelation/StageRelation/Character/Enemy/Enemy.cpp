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
	fixedTime = (int)time + (int)(NetworkServerManager::GetInstance()->playerInfoMap[id].currentTimeDif * 100); //시간을 서버기준으로 업데이트
#else
	fixedTime = (int)time - (int)(NetworkClientManager::GetInstance()->currentTimeDif * 100); // 시간을 클라기준으로 업데이트
#endif
	float timeToAdd = (int)GameTime::GetMSTimeOfPC() - fixedTime; //함수 호출을 요청한 시간으로부터 지난 시간
	if (timeToAdd > 5000) timeToAdd -= 10000;
	else if (timeToAdd < -5000) timeToAdd += 10000; // 보정
	timeToAdd /= 100.0f; // 정상화

	//복제된 변수들 읽기
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
	outputStrm->Write(&objType, sizeof(EINetObjType)); //서버는 ai와 플레이어 데이터 둘다 보낼수 있으니까 타입을 알려줘야한다.

	outputStrm->Write(&id, sizeof(uint8_t)); //패킷을 보내게 된다면 여러 AI들의 데이터가 들어가니까 데이터를 쓸때 id를 넣어줘야한다.
#endif									  
	EDataType dataType = EDataType::Replicated;
	outputStrm->Write(&dataType, sizeof(EDataType)); // Replicated타입이라고 알려주기

	//복제할 변수들 쓰기
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
	fixedTime = (int)time + (int)(NetworkServerManager::GetInstance()->playerInfoMap[id].currentTimeDif * 100); //시간을 서버기준으로 업데이트
#else
	fixedTime = (int)time - (int)(NetworkClientManager::GetInstance()->currentTimeDif * 100); // 시간을 클라기준으로 업데이트
#endif
	float timeToAdd = (int)GameTime::GetMSTimeOfPC() - fixedTime; //함수 호출을 요청한 시간으로부터 지난 시간
	if (timeToAdd > 5000) timeToAdd -= 10000;
	else if (timeToAdd < -5000) timeToAdd += 10000; // 보정
	timeToAdd /= 100.0f; // 정상화

	uint8_t funcSize;
	inputStrm->Read(&funcSize, 1);
	char* funcName = new char[funcSize];
	inputStrm->Read(funcName, funcSize); // 함수명 읽기. 널문자 포함
	string str(funcName);


	delete[] funcName;
}

void Enemy::WirteRPCData(OutputMemoryStream* outputStrm, std::string funcName, void* paramData, int paramSize)
{
#if SERVER
	EINetObjType objType = EINetObjType::AI;
	outputStrm->Write(&objType, sizeof(EINetObjType)); //서버는 ai와 플레이어 데이터 둘다 보낼수 있으니까 타입을 알려줘야한다.

	outputStrm->Write(&id, sizeof(uint8_t)); //패킷을 보내게 된다면 여러 AI의 데이터가 들어가니까 데이터를 쓸때 id를 넣어줘야한다.
#endif	
	//클라는 AI로 RPC할거 같진 않다

	EDataType dataType = EDataType::RPC;
	outputStrm->Write(&dataType, sizeof(EDataType)); // RPC타입이라고 알려주기
	uint16_t time = GameTime::GetMSTimeOfPC();
	outputStrm->Write(&time, sizeof(uint16_t)); //함수가 실행 될 시간
	uint8_t size = funcName.size() + 1; //널문자도 포함 해야해서 크기 1증가
	outputStrm->Write(&size, sizeof(uint8_t)); // 문자열 길이 정보 쓰기.
	outputStrm->Write(funcName.c_str(), size); // 널문자도 포함됨.
	if (paramData != nullptr) outputStrm->Write(paramData, paramSize); //인자값 넣어주기 
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


	for (auto& playerId : gameInfo->playerIdArray)// 해당 게임의 모든 플레이어 id
	{
		auto networkManager = NetworkServerManager::GetInstance();
		std::lock_guard<std::mutex> lk(networkManager->playerInfoMap[playerId].m); //stageOutputStrm은 게임쓰레드에서 쓰는중 메인스레드에서 읽으면 큰일 나기 때문에 락을 해준다.

		WirteRPCData(NetworkServerManager::GetInstance()->playerInfoMap[playerId].stageDataOutputStrm, funcName); //각 플레이어의 stageOutputStrm에다가 해당하는 AI의 rpc 추가 
	}
#endif
}
