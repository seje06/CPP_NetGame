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
	if (id != NetworkClientManager::GetInstance()->playerId) return; //������ �����ڰ� �ƴϸ� ����

	if (GetAsyncKeyState(VK_LEFT) & 0x8000 && !inputInfo.isPressedLeft)
	{
		inputInfo.isPressedLeft = true;
		//ChangeMoveState(true, true);
		bool paramData[2];
		paramData[0] = true;
		paramData[1] = true;
		WirteRPCData(NetworkClientManager::GetInstance()->stageDataOutputStrm, "ChangeMoveState", paramData, 2); //������ ��û�ϱ� ���� ��Ʈ���� ������ �����
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
		if (MapManager::GetInstance()->map[pos.y][(int)(pos.x)] == 1 && (int)pos.x == MAP_WIDTH - 1) pos.x++;
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
		if (MapManager::GetInstance()->map[pos.y][(int)(pos.x)] == 1 && (int)pos.x == MAP_WIDTH - 1) pos.x--;
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
	fixedTime = (int)time + (int)(NetworkServerManager::GetInstance()->playerInfoMap[id].currentTimeDif * 100); //�ð��� ������������ ������Ʈ
#else
	fixedTime = (int)time - (int)(NetworkClientManager::GetInstance()->currentTimeDif * 100); // �ð��� Ŭ��������� ������Ʈ
#endif
	float timeToAdd = (int)GameTime::GetMSTimeOfPC() - fixedTime; //�Լ� ȣ���� ��û�� �ð����κ��� ���� �ð�
	if (timeToAdd > 5000) timeToAdd -= 10000;
	else if (timeToAdd < -5000) timeToAdd += 10000; // ����
	timeToAdd /= 100.0f; // ����ȭ

	//������ ���� �б�
	/*inputStrm->Read(&isMovingL,1);
	inputStrm->Read(&isMovingR, 1);*/
	inputStrm->Read(&pos, sizeof(Pos));
	inputStrm->Read(&dir, 1);
	inputStrm->Read(&aniIndex, 1);
	inputStrm->Read(&isHitting, 1);
	inputStrm->Read(&hitTime, 4);
	inputStrm->Read(&hp, 4);
	inputStrm->Read(&isDie, 1);
}

void Player::WirteReplicatedData(OutputMemoryStream* outputStrm)
{
#if SERVER
	EINetObjType objType = EINetObjType::Player;
	outputStrm->Write(&objType, sizeof(EINetObjType)); //������ ai�� �÷��̾� ������ �Ѵ� ������ �����ϱ� Ÿ���� �˷�����Ѵ�.

	outputStrm->Write(&id, sizeof(ULONG)); //��Ŷ�� ������ �ȴٸ� ���� �÷��̾��� Ŭ�е��� �����Ͱ� ���ϱ� �����͸� ���� id�� �־�����Ѵ�.
	//Ŭ��� �ڱ�͸� ���� �״� ���� ���ص� �ȴ�
#endif									  
	EDataType dataType = EDataType::Replicated;
	outputStrm->Write(&dataType, sizeof(EDataType)); // ReplicatedŸ���̶�� �˷��ֱ�

	//������ ���� ����
	/*outputStrm->Write(&isMovingL, 1);
	outputStrm->Write(&isMovingR, 1);*/
	outputStrm->Write(&pos, sizeof(Pos));
	outputStrm->Write(&dir, 1);
	outputStrm->Write(&aniIndex, 1);
	outputStrm->Write(&isHitting, 1);
	outputStrm->Write(&hitTime, 4);
	outputStrm->Write(&hp, 4);
	outputStrm->Write(&isDie, 1);

}

void Player::ReadRPCData(InputMemoryStream* inputStrm)
{
	float fixedTime = 0;
	uint16_t time;
	inputStrm->Read(&time, 2);
#if SERVER
	fixedTime = (int)time + (int)(NetworkServerManager::GetInstance()->playerInfoMap[id].currentTimeDif*100); //�ð��� ������������ ������Ʈ
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
	else if (str._Equal("Jump")) //��Ƽĳ��Ʈ
	{
		if (jumpInfo.isJumpUp || jumpInfo.isJumpDown) return;
		
		Jump();
		NetMulticast("Jump"); //������ Ŭ��鿡�� �Ѹ�
	}
	else if (str._Equal("ActiveBullet")) //��Ƽĳ��Ʈ
	{
		if (gun->reloadTime < 0.3f) return;

		gun->ActiveBullet();
		NetMulticast("ActiveBullet"); //������ Ŭ��鿡�� �Ѹ�
	}

	delete[] funcName;
}

void Player::WirteRPCData(OutputMemoryStream* outputStrm, std::string funcName, void* paramData, int paramSize)
{
#if SERVER
	auto networkManager = NetworkServerManager::GetInstance();
	std::lock_guard<std::mutex> lk(networkManager->playerInfoMap[id].m); //stageOutputStrm�� ���Ӿ����忡�� ������ ���ν����忡�� ������ ū�� ���� ������ ���� ���ش�.

	EINetObjType objType = EINetObjType::Player;
	outputStrm->Write(&objType, sizeof(EINetObjType)); //������ ai�� �÷��̾� ������ �Ѵ� ������ �����ϱ� Ÿ���� �˷�����Ѵ�.

	outputStrm->Write(&id, sizeof(ULONG)); //��Ŷ�� ������ �ȴٸ� ���� �÷��̾��� Ŭ�е��� �����Ͱ� ���ϱ� �����͸� ���� id�� �־�����Ѵ�.
										   //Ŭ��� �ڱ�͸� ���� �״� ���� ���ص� �ȴ�
#else//Ŭ��
	std::lock_guard<std::mutex> lk(NetworkClientManager::GetInstance()->m); //stageOutputStrm�� ���Ӿ����忡�� ������ ���ν����忡�� ������ ū�� ���� ������ ���� ���ش�.
#endif									
	EDataType dataType = EDataType::RPC;
	outputStrm->Write(&dataType, sizeof(EDataType)); // RPCŸ���̶�� �˷��ֱ�

	uint16_t time = GameTime::GetMSTimeOfPC();
	outputStrm->Write(&time, sizeof(uint16_t)); //�Լ��� ���� �� �ð�
	uint8_t size = funcName.size() + 1; //�ι��ڵ� ���� �ؾ��ؼ� ũ�� 1����
	outputStrm->Write(&size, sizeof(uint8_t)); // ���ڿ� ���� ���� ����.
	outputStrm->Write(funcName.c_str(), size); // �ι��ڵ� ���Ե�.
	if (paramData != nullptr) outputStrm->Write(paramData, paramSize); //���ڰ� �־��ֱ�
}

void Player::NetMulticast(std::string funcName, void* paramData, int paramSize)
{
#if SERVER
	for (auto& playerId : NetworkServerManager::GetInstance()->gameInfoMap[NetworkServerManager::GetInstance()->playerInfoMap[id].gameId].playerIdArray)// �ش� �÷��̾ �ִ� ������ ��� �÷��̾� id
	{
		//�� �÷��̾��� stageOutputStrm�� �ش��ϴ� �÷��̾��� rpc �߰� 
		WirteRPCData(NetworkServerManager::GetInstance()->playerInfoMap[playerId].stageDataOutputStrm, funcName, paramData, paramSize);
	}
	//std::cout << "Player ID : " << id << ", NetMulticast : " << funcName << std::endl;
#endif	
}

