#include "Player.h"
#include "../../MapManager.h"
#include "../../StageEnums.h"
#include "GameTime.h"
#include "MemoryStream.h"
#include "NetGameBuildInfo.h"
#include "NetworkEnums.h"
#include "NetworkManager.h"
#include "GameManager.h"


using namespace std;


void Player::Controll(float _deltaTime)
{
#if SERVER
	MoveL(_deltaTime);
	MoveR(_deltaTime);
	ProcessGravity( PLAYER_HEIGHT, 3, _deltaTime);
#endif
	gun->ControllBullet(_deltaTime);
}

void Player::CheckPlayerInput(float _deltaTime)
{
	if (isDie) return;
	if (id != NetworkClientManager::GetInstance()->playerId) return; //본인이 소유자가 아니면 리턴

	if (GetAsyncKeyState(VK_LEFT) & 0x8000 && !inputInfo.isPressedLeft)
	{
		inputInfo.isPressedLeft = true;
		//ChangeMoveState(true, true);
		bool paramData[2];
		paramData[0] = true;
		paramData[1] = true;
		WirteRPCData(NetworkClientManager::GetInstance()->stageDataOutputStrm, "ChangeMoveState", paramData, 2); //서버에 요청하기 위해 스트림에 데이터 써놓기
	}
	else if (!(GetAsyncKeyState(VK_LEFT) & 0x8000) && inputInfo.isPressedLeft)
	{
		inputInfo.isPressedLeft = false;
		//ChangeMoveState(true, false);
		bool paramData[2];
		paramData[0] = true;
		paramData[1] = false;
		WirteRPCData(NetworkClientManager::GetInstance()->stageDataOutputStrm, "ChangeMoveState", paramData, 2);
	}

	if (GetAsyncKeyState(VK_RIGHT) & 0x8000 && !inputInfo.isPressedRight)
	{
		inputInfo.isPressedRight = true;
		//ChangeMoveState(false, true);
		bool paramData[2];
		paramData[0] = false;
		paramData[1] = true;
		WirteRPCData(NetworkClientManager::GetInstance()->stageDataOutputStrm, "ChangeMoveState", paramData, 2);

	}
	else if (!(GetAsyncKeyState(VK_RIGHT) & 0x8000) && inputInfo.isPressedRight)
	{
		inputInfo.isPressedRight = false;
		//ChangeMoveState(false, false);
		bool paramData[2];
		paramData[0] = false;
		paramData[1] = false;
		WirteRPCData(NetworkClientManager::GetInstance()->stageDataOutputStrm, "ChangeMoveState", paramData, 2);
	}

	if (GetAsyncKeyState(VK_SPACE) & 0x8000 && !inputInfo.isPressedSpace)
	{
		inputInfo.isPressedSpace = true;
		//Jump();
		WirteRPCData(NetworkClientManager::GetInstance()->stageDataOutputStrm, "Jump");
	}
	else if (!(GetAsyncKeyState(VK_SPACE) & 0x8000) && inputInfo.isPressedSpace)
	{
		inputInfo.isPressedSpace = false;
	}

	if (GetAsyncKeyState(VK_LCONTROL) & 0x8000 && !inputInfo.isPressedLeftCon)
	{
		inputInfo.isPressedLeftCon = true;
		//gun->ActiveBullet();
		WirteRPCData(NetworkClientManager::GetInstance()->stageDataOutputStrm, "ActiveBullet");
	}
	else if (!(GetAsyncKeyState(VK_LCONTROL) & 0x8000) && inputInfo.isPressedLeftCon)
	{
		inputInfo.isPressedLeftCon = false;
	}
}

void Player::ChangeMoveState(bool isL, bool value)
{
	if (isL)
	{
		isMovingL = value;
	}
	else
	{
		isMovingR = value;
	}
}




void Player::MoveL(float _deltaTime)
{
	if (isMovingL)
	{
		float preX = pos.x;
		dir = (int)DIR::LEFT;
		pos.x -= _deltaTime * speed;
		if ((int)preX != (int)pos.x) aniIndex = aniIndex + 1 >= PLAYER_ANI_LENGTH ? 0 : aniIndex + 1;
		if (pos.x < 0 || pos.x >= MAP_WIDTH) 
		{
			pos.x = preX;
			return;
		}
		if (MapManager::GetInstance()->map[pos.y][(int)(pos.x)] == 1 && ((int)pos.x == MAP_WIDTH - 1 || (int)pos.x == 0)) pos.x++;
	}
}

void Player::MoveR(float _deltaTime)
{
	if (isMovingR)
	{
		float preX = pos.x;
		dir = (int)DIR::RIGHT;
		pos.x += _deltaTime * speed;
		if ((int)preX != (int)pos.x) aniIndex = aniIndex + 1 >= PLAYER_ANI_LENGTH ? 0 : aniIndex + 1;
		if (pos.x < 0 || pos.x >= MAP_WIDTH)
		{
			pos.x = preX;
			return;
		}
		if (MapManager::GetInstance()->map[pos.y][(int)(pos.x)] == 1 && ((int)pos.x == MAP_WIDTH - 1 || (int)pos.x == 0)) pos.x--;
	}
}

void Player::Jump()
{
	jumpInfo.isJumpUp = true;
	jumpInfo.isJumpDown = false;
	jumpInfo.startJumpY = pos.y;
}

void Player::ReadReplicatedData(InputMemoryStream* inputStrm, uint16_t time)
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

	//복제된 변수 읽기
	/*inputStrm->Read(&isMovingL,1);
	inputStrm->Read(&isMovingR, 1);*/
	inputStrm->Read(&pos, sizeof(Pos));
	inputStrm->Read(&dir, 1);
	inputStrm->Read(&aniIndex, 1);
	inputStrm->Read(&isHitting, 1);
	inputStrm->Read(&hitTime, 4);
	inputStrm->Read(&hp, 4);
	inputStrm->Read(&isDie, 1);
	inputStrm->Read(&color, 1);
}

void Player::WirteReplicatedData(OutputMemoryStream* outputStrm)
{
#if SERVER
	EINetObjType objType = EINetObjType::Player;
	outputStrm->Write(&objType, sizeof(EINetObjType)); //서버는 ai와 플레이어 데이터 둘다 보낼수 있으니까 타입을 알려줘야한다.

	outputStrm->Write(&id, sizeof(ULONG)); //패킷을 보내게 된다면 여러 플레이어의 클론들의 데이터가 들어가니까 데이터를 쓸때 id를 넣어줘야한다.
	//클라는 자기것만 보낼 테니 굳이 안해도 된다
#endif									  
	EDataType dataType = EDataType::Replicated;
	outputStrm->Write(&dataType, sizeof(EDataType)); // Replicated타입이라고 알려주기

	//복제할 변수 쓰기
	/*outputStrm->Write(&isMovingL, 1);
	outputStrm->Write(&isMovingR, 1);*/
	outputStrm->Write(&pos, sizeof(Pos));
	outputStrm->Write(&dir, 1);
	outputStrm->Write(&aniIndex, 1);
	outputStrm->Write(&isHitting, 1);
	outputStrm->Write(&hitTime, 4);
	outputStrm->Write(&hp, 4);
	outputStrm->Write(&isDie, 1);
	outputStrm->Write(&color, 1);

}

void Player::ReadRPCData(InputMemoryStream* inputStrm)
{
	float fixedTime = 0;
	uint16_t time;
	inputStrm->Read(&time, 2);
#if SERVER
	fixedTime = (int)time + (int)(NetworkServerManager::GetInstance()->playerInfoMap[id].currentTimeDif*100); //시간을 서버기준으로 업데이트
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

#if SERVER
	//std::cout << "Player ID : "<< id << ", RPC : " << funcName << std::endl;
#endif

	if (str._Equal("ChangeMoveState")) 
	{
		bool isL;
		bool value;
		inputStrm->Read(&isL, 1);
		inputStrm->Read(&value, 1);

		ChangeMoveState(isL, value);
	}
	else if (str._Equal("Jump")) //멀티캐스트
	{
		if (jumpInfo.isJumpUp || jumpInfo.isJumpDown) return;
		
		Jump();
		NetMulticast("Jump"); //서버가 클라들에게 뿌림
	}
	else if (str._Equal("ActiveBullet")) //멀티캐스트
	{
		if (gun->reloadTime < 0.3f) return;

		gun->ActiveBullet();
		NetMulticast("ActiveBullet"); //서버가 클라들에게 뿌림
	}

	delete[] funcName;
}

void Player::WirteRPCData(OutputMemoryStream* outputStrm, std::string funcName, void* paramData, int paramSize)
{
#if SERVER
	auto networkManager = NetworkServerManager::GetInstance();
	std::lock_guard<std::mutex> lk(networkManager->playerInfoMap[id].m); //stageOutputStrm은 게임쓰레드에서 쓰는중 메인스레드에서 읽으면 큰일 나기 때문에 락을 해준다.

	EINetObjType objType = EINetObjType::Player;
	outputStrm->Write(&objType, sizeof(EINetObjType)); //서버는 ai와 플레이어 데이터 둘다 보낼수 있으니까 타입을 알려줘야한다.

	outputStrm->Write(&id, sizeof(ULONG)); //패킷을 보내게 된다면 여러 플레이어의 클론들의 데이터가 들어가니까 데이터를 쓸때 id를 넣어줘야한다.
										   //클라는 자기것만 보낼 테니 굳이 안해도 된다
#else//클라
	std::lock_guard<std::mutex> lk(NetworkClientManager::GetInstance()->m); //stageOutputStrm은 게임쓰레드에서 쓰는중 메인스레드에서 읽으면 큰일 나기 때문에 락을 해준다.
#endif									
	EDataType dataType = EDataType::RPC;
	outputStrm->Write(&dataType, sizeof(EDataType)); // RPC타입이라고 알려주기

	uint16_t time = GameTime::GetMSTimeOfPC();
	outputStrm->Write(&time, sizeof(uint16_t)); //함수가 실행 될 시간
	uint8_t size = funcName.size() + 1; //널문자도 포함 해야해서 크기 1증가
	outputStrm->Write(&size, sizeof(uint8_t)); // 문자열 길이 정보 쓰기.
	outputStrm->Write(funcName.c_str(), size); // 널문자도 포함됨.
	if (paramData != nullptr) outputStrm->Write(paramData, paramSize); //인자값 넣어주기
}

void Player::NetMulticast(std::string funcName, void* paramData, int paramSize)
{
#if SERVER
	for (auto& playerId : NetworkServerManager::GetInstance()->gameInfoMap[NetworkServerManager::GetInstance()->playerInfoMap[id].gameId].playerIdArray)// 해당 플레이어가 있는 게임의 모든 플레이어 id
	{
		//각 플레이어의 stageOutputStrm에 해당하는 플레이어의 rpc 추가 
		WirteRPCData(NetworkServerManager::GetInstance()->playerInfoMap[playerId].stageDataOutputStrm, funcName, paramData, paramSize);
	}
	//std::cout << "Player ID : " << id << ", NetMulticast : " << funcName << std::endl;
#endif	
}

