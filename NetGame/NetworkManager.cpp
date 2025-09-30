#include "NetworkManager.h"
#include "MyGamePCH.h"
#include "GameTime.h"
#include "Utill.h"
#include "GameManager.h"
#include "GameMaker.h"
#include "SceneRelation/StageRelation/Character/Player/Player.h"
#include "SceneRelation/StageRelation/Character/Enemy/Enemy.h"
#include "SceneRelation/StageRelation/Character/Enemy/EnemiesManager.h"
#include "IManageable.h"
#include "LoginManager.h"
#include "SceneRelation/StageRelation/StageManager.h"
#include "SceneRelation/EndSceneRelation/EndSceneManager.h"

using namespace std;

void NetworkManager::ProcessPacket(char* inBuffer, UDPSocket* inUdpSocket, SocketAddress* inSocketAdr)
{
	OutputMemoryStream outputStrm;
	//ó���� ��Ŷ ������ �־��ִ� �ô��ϱ�. �ô��� ������ �����͸� �ٳְ� ������ ��Ŷ ũ�⸦ �˼� �ִµ�, ��Ŷ�տ� �־��ַ��� �ڸ��� Ȯ���ؾ���.
	uint16_t tempPacketSize = outputStrm.GetLength();
	outputStrm.Write(&tempPacketSize, sizeof(uint16_t));

	uint16_t packetSize;
	memcpy(&packetSize, inBuffer, 2); // ��Ŷ������ ���� ������. ��ǲ��Ʈ�� �ʱ�ȭ�Ҷ� ���� ��Ŷ�� ũ�⸦ ����������ؼ� �ϴ� ũ�⸦ �˾Ƴ���.

	InputMemoryStream inputStrm(inBuffer, packetSize); //��ǲ ��Ʈ�� �ʱ�ȭ
	inputStrm.Read(&packetSize, sizeof(uint16_t)); // ��Ŷ������ Ÿ�� ��ŭ mHead �ø���
	
	EPacketType pakcetType;
	inputStrm.Read(&pakcetType, sizeof(EPacketType)); // ��Ŷ Ÿ�� ������
	if (pakcetType == EPacketType::Client) // �������� Ŭ��� �־��ذŸ�
	{
		EPacketType serverPacketType = EPacketType::Server;
		outputStrm.Write(&serverPacketType, sizeof(EPacketType)); //����Ÿ�� �־��ֱ�
	}
	else if (pakcetType == EPacketType::Server) // Ŭ�󿡼� ������ �־��ذŸ�
	{
		EPacketType clientPacketType = EPacketType::Client;
		outputStrm.Write(&clientPacketType, sizeof(EPacketType)); //����Ÿ�� �־��ֱ�
	}

	EHeaderType headerType;
	inputStrm.Read(&headerType, sizeof(EHeaderType)); // ��� Ÿ�� ������
	switch (headerType)
	{
		case EHeaderType::InitReliable: //�ʱ�ȭ�� �ŷڼ� ��Ŷ����
		{
			ProcessOnInitReliable(pakcetType, inUdpSocket, inSocketAdr,  packetSize, outputStrm, inputStrm);

			
			break;
		}
		case EHeaderType::Reliable: //�ŷڼ� ��Ŷ�ΰ�
		{
			ProcessOnReliable(pakcetType, inUdpSocket, inSocketAdr,packetSize, outputStrm, inputStrm);

			
			
			break;
		}
		case EHeaderType::UnReliable: //��ŷ��ΰ�
		{
			ProcessOnUnReliable(pakcetType, inUdpSocket, inSocketAdr,  packetSize, outputStrm, inputStrm);

			break;
		}
	}

	
}




/////////////////////////            ServerCode          /////////////////////////


NetworkServerManager::PlayerInfo_s::PlayerInfo_s()
{
	sockAddr = new SocketAddress();
	stageDataOutputStrm = new OutputMemoryStream();
	ackBuffer = new char[1500];
}

NetworkServerManager::PlayerInfo_s::~PlayerInfo_s()
{
	delete[] ackBuffer;
	delete sockAddr;
	delete stageDataOutputStrm;
}

void NetworkServerManager::ReSendPacket(PlayerID _id, UDPSocket* _udpSocket, SocketAddress* _socketAdr)
{
}

void NetworkServerManager::ProcessOnInitReliable(EPacketType inPacketType, UDPSocket* inUdpSocket, SocketAddress* inSocketAdr,  uint16_t packetSize, OutputMemoryStream& outputStrm, InputMemoryStream& inputStrm)
{
	if (inPacketType != EPacketType::Server) return; //��Ŷ Ÿ���� �ȸ����� ����

	EHeaderType clientHeaderType = EHeaderType::InitReliable;
	outputStrm.Write(&clientHeaderType, sizeof(EHeaderType)); // �ŷڼ�������� Ŭ�� ���ۿ� ����

	EPacketType clientPacketType = EPacketType::Client; // Ŭ�󿡰�

	NetReliableHeader header;
	inputStrm.Read(&header, sizeof(NetReliableHeader)); //�ŷڼ� ��� ������
	

	NetReliableHeader clientHeader;
	clientHeader.packetNum = header.packetNum + 1; // �ʱ�ȭ�� �ŷ���Ŷ���� ���� �˻������ʰ� ��Ŷ��ȣ 1���ؼ� Ŭ�󿡰� �Ѱ������
	clientHeader.playerId = header.playerId;

	if (playerInfoMap.find(header.playerId) == playerInfoMap.end()) // �ش� ���̵� ������, �÷��̾� ���� �����ϰ� ���� ��Ŷ�߼�.
	{
		*playerInfoMap[header.playerId].sockAddr = *inSocketAdr;
		cout << "New Player came in, Id : " << header.playerId << endl;

		outputStrm.Write(&clientHeader, sizeof(NetReliableHeader)); //�ŷڼ���� ����
		
		ELoginResultType loginRes = ELoginResultType::Success;
		outputStrm.Write(&loginRes, sizeof(ELoginResultType)); //�α��� ���� ������ �־��ֱ�

		playerInfoMap[header.playerId].currentPacketNum = header.packetNum + 1; //�÷��̾� �������� ��Ŷ��ȣ ����
		
		currentPlayerCount++;
		currentPlayerCountInLogo++;

		// ���� ���� id�� ���� ������� ������ ������ ����� ���ӸŴ��� �Ҵ�ޱ� 
		if (gameInfoMap.find(currentGameCount) == gameInfoMap.end())
		{
			cout << "Game Id : " << currentGameCount<<", ";
			gameInfoMap[currentGameCount].gameManager = GameMaker::MakeGame();
		}
		gameInfoMap[currentGameCount].playerIdArray[currentPlayerCountInLogo - 1] = header.playerId; //���� ������ �ش� ���ӿ� ���� �÷��̾� ���̵� ����
		gameInfoMap[currentGameCount].currentPlayerCountInGame = currentPlayerCountInLogo;

		playerInfoMap[header.playerId].gameId = currentGameCount; // �� �÷��̾������� ���� ���̵� ����

		if (currentPlayerCountInLogo == LOGO_ACCEPTABLE_MAX_COUNT) //�ΰ� �����ο� �� ����
		{
			gameInfoMap[currentGameCount].gameManager->SetCurrentScene<StageManager>(SCENE_ID::STAGE); //�ش� ������ ���� ���������� ����
			cout <<"Game Id : " << currentGameCount<< ", Stage is Started" << endl;

			int playerColor = int(COLOR::LIGHTBLUE);
			for (auto& id : gameInfoMap[currentGameCount].playerIdArray)
			{
				
				playerInfoMap[id].sceneType = ESceneType::STAGE; // �� �÷��̾������� �� Ÿ�� ����

				auto player = new Player();
				player->Init(0, PLAYER_MAX_HP, false, false);
				player->id = id;
				player->color = (COLOR)playerColor++;
				shared_ptr<Player> playerPtr(player);
				auto objGroup = gameInfoMap[currentGameCount].gameManager->GetObjects();
				(*objGroup->playerMap)[id] = playerPtr; //�ش� ���ӿ� �÷��̾� ĳ�� ������ �ֱ�
				playerInfoMap[id].playerCharac = playerPtr; // �÷��̾� ������ �÷��̾� ĳ�� �ֱ�
			}

			currentGameCount++; //���� id �ø���
			currentPlayerCountInLogo = 0; // ���������� �÷����� �ΰ��ο� �ʱ�ȭ
		}
		else
		{

		}
	}
	else //���̵� �̹� ������ ���̵��� ������ �������� Ȯ���Ѵ�.
	{
		if (playerInfoMap[header.playerId].sockAddr->GetIP4Ref() == inSocketAdr->GetIP4Ref()) // ������ �´ٸ�(���� ip���� Ȯ��. ���� ip�� �޷��ִ� �缳ip�� ���� �ֱ⿡ ������ �ֱ���
		{
			//Ŭ�� ������Ŷ�� ���޾ұ⿡ �ٽ� �����ش�.
			inUdpSocket->SendTo(playerInfoMap[header.playerId].ackBuffer, 1500, *inSocketAdr);
			std::cout << "Player Id : " << header.playerId << " resent packet, packet num : "<< header.packetNum << std::endl;
			return;
		}
		else
		{
			outputStrm.Write(&clientHeader, sizeof(NetReliableHeader)); //�ŷڼ���� ����

			ELoginResultType loginRes = ELoginResultType::Failed;
			outputStrm.Write(&loginRes, sizeof(ELoginResultType));

			//�������� ��Ŷ �Ǿպκп� ��Ŷ ũ�� �����. memcpy�� �����ϴ� ������ write�� ���ԵǸ� mHead������ �ڿ� ��.
			uint16_t outputPacketSize = outputStrm.GetLength();
			memcpy(outputStrm.GetBufferPtr(), &outputPacketSize, sizeof(uint16_t));

			inUdpSocket->SendTo(outputStrm.GetBufferPtr(), 1500, *inSocketAdr);
			return;
		}
	}

	//�������� ��Ŷ �Ǿպκп� ��Ŷ ũ�� �����. memcpy�� �����ϴ� ������ write�� ���ԵǸ� mHead������ �ڿ� ��.
	uint16_t outputPacketSize = outputStrm.GetLength();
	memcpy(outputStrm.GetBufferPtr(), &outputPacketSize, sizeof(uint16_t));
	
	inUdpSocket->SendTo(outputStrm.GetBufferPtr(), 1500, *inSocketAdr); 
	
	//������ ���� ��Ŷ�� �ش� �÷��̾� ������ ����.
	memcpy(playerInfoMap[header.playerId].ackBuffer, outputStrm.GetBufferPtr(), outputPacketSize);

	
}

void NetworkServerManager::ProcessOnReliable(EPacketType inPacketType, UDPSocket* inUdpSocket, SocketAddress* inSocketAdr, uint16_t packetSize, OutputMemoryStream& outputStrm, InputMemoryStream& inputStrm)
{
	if (inPacketType != EPacketType::Server) return; //��Ŷ Ÿ���� �ȸ����� ����

	EHeaderType clientHeaderType = EHeaderType::Reliable;
	outputStrm.Write(&clientHeaderType, sizeof(EHeaderType)); // ���Ÿ�� Ŭ�� ���ۿ� ����

	NetReliableHeader header;
	inputStrm.Read(&header, sizeof(NetReliableHeader)); //�ŷڼ� ��� ������
	
	if (playerInfoMap.find(header.playerId) == playerInfoMap.end()) return; //���� ���� ���̵�� ������ ���� �� ���̱⿡ ����.

	if (++playerInfoMap[header.playerId].currentPacketNum != header.packetNum) //���� ���� �Դ� ��Ŷ�̸�
	{
		--playerInfoMap[header.playerId].currentPacketNum;
		//�����س��� ��Ŷ ��۽�
		inUdpSocket->SendTo(playerInfoMap[header.playerId].ackBuffer, 1500, *playerInfoMap[header.playerId].sockAddr);
		return;
	}

	if (gameInfoMap[playerInfoMap[header.playerId].gameId].gameManager->id == ESceneType::End) return; //���� EndScene�̸� ������ ���� ��Ŷ�� ���̻� ���� �ʿ� ���⿡ ����.

	 //�ش� ��Ŷ�� ó�� ������
	
	NetReliableHeader clientHeader;  //Ŭ�󿡰� ���� ��� ����
	clientHeader.packetNum = header.packetNum + 1;

	playerInfoMap[header.playerId].currentPacketNum = header.packetNum + 1; // �÷��̾� ������ ��Ŷ��ȣ ����
	if (header.timeSteb == 0 && playerInfoMap[header.playerId].updatedTDCount < 65000) //Ŭ��κ��� ���� 0
	{
		float timeDifs = (int)GameTime::GetMSTimeOfPC() - (int)header.time;
		if (timeDifs < -5000) timeDifs = timeDifs + 10000;
		else if (timeDifs > 5000) timeDifs = timeDifs - 10000; //�̻��� ���� ������ ����


		clientHeader.timeSteb = 1;
		clientHeader.timeDifs = timeDifs;
	}
	else if (header.timeSteb == 2 && playerInfoMap[header.playerId].updatedTDCount < 65000) //���� 2
	{
		clientHeader.timeSteb = 3; // Ŭ�󿡰� �ش� Ÿ�� ���� ���� ���̶�� �˸� ��
		//pc�ð����� ���� ���� ����ȭ �Ѵ�. ex) 2580 -> 25.80
		if (playerInfoMap[header.playerId].currentTimeDif == 0)playerInfoMap[header.playerId].currentTimeDif = ((float)header.timeDifc) / 100;
		else
		{
			//timeDif�� ��հ��� �־��ش�
			playerInfoMap[header.playerId].currentTimeDif = (playerInfoMap[header.playerId].currentTimeDif * playerInfoMap[header.playerId].updatedTDCount + ((float)header.timeDifc) / 100) / (playerInfoMap[header.playerId].updatedTDCount + 1);
		}
		clientHeader.timeDifc = playerInfoMap[header.playerId].currentTimeDif * 100;
	}

	clientHeader.sceneType = playerInfoMap[header.playerId].sceneType; //��� ������ �������� �־��ֱ�
	clientHeader.playerCount = gameInfoMap[playerInfoMap[header.playerId].gameId].currentPlayerCountInGame; // �÷��̾ ���� ������ ��� ��
	
	//outputStrm.Write(&playerInfoMap[header.playerId].sceneType, 1); //Ŭ��� ���ۿ� �÷��̾��� �� Ÿ�� ����

	if (clientHeader.sceneType == ESceneType::LOGO) //�ΰ� �� ���
	{
		outputStrm.Write(&clientHeader, sizeof(NetReliableHeader)); // Ŭ��� ���ۿ� �ŷڿ� ��� ���� ����
	}
	else //�������� �� ���
	{
		//������ �� �Ǵ� �з� ��������� Ŭ�󿡰� ���� �˷��ֱ�
		if (gameInfoMap[playerInfoMap[header.playerId].gameId].gameEndType == EGameEndType::Lose || gameInfoMap[playerInfoMap[header.playerId].gameId].gameEndType == EGameEndType::Win)
		{
			if(gameInfoMap[playerInfoMap[header.playerId].gameId].gameEndType == EGameEndType::Lose) clientHeader.gameEndType = EGameEndType::Lose;
			else clientHeader.gameEndType = EGameEndType::Win;

		}
		auto stageManager = dynamic_cast<StageManager*>(gameInfoMap[playerInfoMap[header.playerId].gameId].gameManager->sceneManagers[(int)ESceneType::STAGE]);
		if (stageManager)clientHeader.stageLevel = stageManager->currentStage; // �������� ���� ������
		outputStrm.Write(&clientHeader, sizeof(NetReliableHeader)); // Ŭ��� ���ۿ� �ŷڿ� ��� ���� ����

		while (inputStrm.GetRemainingDataSize() != 0)
		{
			EDataType dataType;
			inputStrm.Read(&dataType, sizeof(EDataType));
			if (dataType == EDataType::RPC) //RPC
			{
				playerInfoMap[header.playerId].playerCharac->ReadRPCData(&inputStrm);
			}
			else //Replicated
			{
				playerInfoMap[header.playerId].playerCharac->ReadReplicatedData(&inputStrm, header.time);
			}
		}

		//�� ������
		{
			std::lock_guard<std::mutex> lk(playerInfoMap[header.playerId].m); //stageOutputStrm�� ���Ӿ����忡�� ������ ���ν����忡�� ������ ū�� ���� ������ ���� ���ش�.
			auto stageOutputStrm = playerInfoMap[header.playerId].stageDataOutputStrm;
			if (stageOutputStrm->GetLength() != 0)
				outputStrm.Write(stageOutputStrm->GetBufferPtr(), stageOutputStrm->GetLength()); //������������ ���� ������ �߰��ϱ�
			stageOutputStrm->Init(); // �׸��� �ʱ�ȭ
		}

		for (auto& playerId : gameInfoMap[playerInfoMap[header.playerId].gameId].playerIdArray)
		{
			playerInfoMap[playerId].playerCharac->WirteReplicatedData(&outputStrm); // �ش� �÷��̾ ���� ������ ��� �÷��̾��� replicated ������ ����
		}
		auto enemies = gameInfoMap[playerInfoMap[header.playerId].gameId].gameManager->GetObjects()->enemies;
		for (int i = 0; i < ENEMY_COUNT; i++)
		{
			enemies[i]->WirteReplicatedData(&outputStrm); // �ش� �÷��̾ ���� ������ ��� AI�� replicated ������ ����
		}
	}

	//�������� ��Ŷ �Ǿպκп� ��Ŷ ũ�� �����. memcpy�� �����ϴ� ������ write�� ���ԵǸ� mHead������ �ڿ� ��.
	uint16_t outputPacketSize = outputStrm.GetLength();
	memcpy(outputStrm.GetBufferPtr(), &outputPacketSize, sizeof(uint16_t));

	inUdpSocket->SendTo(outputStrm.GetBufferPtr(), 1500, *inSocketAdr); // ��Ŷ����

	//������ ���� ��Ŷ�� �ش� �÷��̾� ������ ����.
	memcpy(playerInfoMap[header.playerId].ackBuffer, outputStrm.GetBufferPtr(), outputStrm.GetLength());
	
	//������ �� �Ǵ� �з� ��������� ���� �� ������ ��ȯ
	if (clientHeader.gameEndType == EGameEndType::Lose || clientHeader.gameEndType == EGameEndType::Win)
	{
		//gameInfoMap[playerInfoMap[header.playerId].gameId].gameManager->SetCurrentScene<EndSceneManager>(ESceneType::End);
		gameInfoMap[playerInfoMap[header.playerId].gameId].endPlayerCount++;
		if (gameInfoMap[playerInfoMap[header.playerId].gameId].endPlayerCount == LOGO_ACCEPTABLE_MAX_COUNT) // �ش� ������ �÷��̾� ���� endScene ��Ŷ�� �޴� ������
		{
			gameInfoMap[playerInfoMap[header.playerId].gameId].gameManager->SetCurrentScene<EndSceneManager>(ESceneType::End);

			auto endScene = dynamic_cast<EndSceneManager*>(gameInfoMap[playerInfoMap[header.playerId].gameId].gameManager->sceneManagers[(int)ESceneType::End]);
			endScene->gameId = playerInfoMap[header.playerId].gameId;
		}
	}
}

void NetworkServerManager::ProcessOnUnReliable(EPacketType inPacketType, UDPSocket* inUdpSocket, SocketAddress* inSocketAdr, uint16_t packetSize, OutputMemoryStream& outputStrm, InputMemoryStream& inputStrm)
{
	if (inPacketType != EPacketType::Server) return; //��Ŷ Ÿ���� �ȸ����� ����

}


/////////////////////////            Client Code          /////////////////////////

NetworkClientManager::NetworkClientManager()
{
	stageDataOutputStrm = new OutputMemoryStream();
}

void NetworkClientManager::SendLoginPacket(unsigned long inPlayerId,  UDPSocket* inUdpSocket, SocketAddress* inSocketAdr)
{
	OutputMemoryStream outputStrm;
	//ó���� ��Ŷ ������ �־��ִ� �ô��ϱ�. �ô��� ������ �����͸� �ٳְ� ������ ��Ŷ ũ�⸦ �˼� �ִµ�, ��Ŷ�տ� �־��ַ��� �ڸ��� Ȯ���ؾ���.
	uint16_t tempPacketSize = outputStrm.GetLength();
	outputStrm.Write(&tempPacketSize, sizeof(uint16_t));

	EPacketType serverPacketType = EPacketType::Server;
	outputStrm.Write(&serverPacketType, sizeof(EPacketType)); // ���� ��ŶŸ���� ���� ���ۿ� ����

	EHeaderType serverHeaderType = EHeaderType::InitReliable;
	outputStrm.Write(&serverHeaderType, sizeof(EHeaderType)); // ���� ���Ÿ���� ���� ���ۿ� ����

	currentPacketNum = 1;
	NetReliableHeader serverHeader; //�������� ���� ���
	serverHeader.packetNum = currentPacketNum;
	serverHeader.playerId = inPlayerId;
	playerId = inPlayerId;
	outputStrm.Write(&serverHeader, sizeof(NetReliableHeader));

	//�������� ��Ŷ �Ǿպκп� ��Ŷ ũ�� �����. memcpy�� �����ϴ� ������ write�� ���ԵǸ� mHead������ �ڿ� ��.
	uint16_t outputPacketSize = outputStrm.GetLength();
	memcpy(outputStrm.GetBufferPtr(), &outputPacketSize, sizeof(uint16_t));

	inUdpSocket->SendTo(outputStrm.GetBufferPtr(), 1500, *inSocketAdr);//������
	recentSendTime = GameTime::GetMSTimeOfPC();

	//������ ���� ��Ŷ�� �ش� �÷��̾� ������ ����.
	memcpy(recentSendedBuffer, outputStrm.GetBufferPtr(), outputStrm.GetLength());
}

void NetworkClientManager::ReSendPacket(PlayerID _id, UDPSocket* _udpSocket, SocketAddress* _socketAdr)
{
	recentSendTime = GameTime::GetMSTimeOfPC();
	_udpSocket->SendTo(recentSendedBuffer, 1500, *_socketAdr);
}

NetworkClientManager::~NetworkClientManager()
{
	delete[] recentSendedBuffer;
	delete stageDataOutputStrm;
}

void NetworkClientManager::ProcessOnInitReliable(EPacketType inPacketType, UDPSocket* inUdpSocket, SocketAddress* inSocketAdr, uint16_t packetSize, OutputMemoryStream& outputStrm, InputMemoryStream& inputStrm)
{
	if (inPacketType != EPacketType::Client) return; //��Ŷ Ÿ���� �ȸ����� ����

	NetReliableHeader header;
	inputStrm.Read(&header, sizeof(NetReliableHeader)); // ��� �б�

	if (header.playerId != playerId) //���� ���� playerID�� �ƴҰ�� �����ʴ´� 
	{
		return;
	}

	if (header.packetNum != ++currentPacketNum) // �ش� ��Ŷ�� ó�� �°� �ƴϸ� �����ʴ´�
	{
		--currentPacketNum;
		return;
	}

	ELoginResultType loginRes;
	inputStrm.Read(&loginRes, sizeof(ELoginResultType)); // �α��� ��� �б�

	if (loginRes == ELoginResultType::Failed) //�α��� ���н�
	{
		// id ���Է�, ��� �缳�� �� �ٽ� ������ 
		std::cout << "�α��� ����!. �ٸ� ���̵�� �ٽ� �Է� ���ּ���." << std::endl;
		SendLoginPacket(Login(), inUdpSocket, inSocketAdr);
		return;
	}

	isFinishInit = true;

	playerId = header.playerId;
	gameManager = GameMaker::MakeGame();
	
	EHeaderType serverHeaderType = EHeaderType::Reliable;
	outputStrm.Write(&serverHeaderType, sizeof(EHeaderType)); // ���� ���Ÿ���� ���� ���ۿ� ����

	NetReliableHeader serverHeader; // �������� ���� ���
	serverHeader.packetNum = ++currentPacketNum;
	serverHeader.time = GameTime::GetMSTimeOfPC();
	recentCheckedTime = serverHeader.time;
	serverHeader.timeSteb = 0; // �ʱ�ȭ �ŷڿ��̾ƴ� �ŷڿ����� ó�� �������� 0���� 
	serverHeader.playerId = playerId;
	serverHeader.sceneType = ESceneType::LOGO;

	outputStrm.Write(&serverHeader, sizeof(NetReliableHeader)); // ���� ��� ����
	//outputStrm.Write(&currentScene, sizeof(ESceneType)); // ���� �� Ÿ�� ����

	//�������� ��Ŷ �Ǿպκп� ��Ŷ ũ�� �����. memcpy�� �����ϴ� ������ write�� ���ԵǸ� mHead������ �ڿ� ��.
	uint16_t outputPacketSize = outputStrm.GetLength();
	memcpy(outputStrm.GetBufferPtr(), &outputPacketSize, sizeof(uint16_t));

	inUdpSocket->SendTo(outputStrm.GetBufferPtr(), 1500, *inSocketAdr);//������
	recentSendTime = GameTime::GetMSTimeOfPC();

	//������ ���� ��Ŷ�� �ش� �÷��̾� ������ ����.
	memcpy(recentSendedBuffer, outputStrm.GetBufferPtr(), outputStrm.GetLength());
}

void NetworkClientManager::ProcessOnReliable(EPacketType inPacketType, UDPSocket* inUdpSocket, SocketAddress* inSocketAdr, uint16_t packetSize, OutputMemoryStream& outputStrm, InputMemoryStream& inputStrm)
{
	if (inPacketType != EPacketType::Client) return; //��Ŷ Ÿ���� �ȸ����� ����
	if (gameManager->id == ESceneType::End) return; //���� �� ���̸� ��Ŷ�� �����ʴ´�.

	NetReliableHeader header;
	inputStrm.Read(&header, sizeof(NetReliableHeader)); // ��� �б�
	//ESceneType sceneType;
	//inputStrm.Read(&sceneType, sizeof(ESceneType)); // ���� �־���� �� �б�

	if (!isFinishInit) //�ʱ�ȭ ����� �Ϸ�����ʾҴµ� ����� �Դٴ°� �ش� ���̵�� ���ư��� �÷��̾ ���� ������ ����ִٴ°�. �� �������̴� ������ ���������Ѵ�
	{
		//���� ����
		gameManager = GameMaker::MakeGame();
	}
	else if (header.packetNum != ++currentPacketNum) // �ʱ�ȭ�ƴµ��� �ش� ��Ŷ�� ó�� �°� �ƴϸ� �����ʴ´�
	{
		--currentPacketNum;
		return;
	}

	currentPlayerCount = header.playerCount;

	EHeaderType serverHeaderType = EHeaderType::Reliable;
	outputStrm.Write(&serverHeaderType, sizeof(EHeaderType)); // ���� ���Ÿ���� ���� ���ۿ� ����

	NetReliableHeader serverHeader; // �������� ���� ���
	serverHeader.playerId = playerId;
	serverHeader.packetNum = ++currentPacketNum;
	serverHeader.time = GameTime::GetMSTimeOfPC();

	//serverHeader.time = GameTime::GetMSTimeOfPC();
	if (header.timeSteb == 1)
	{
		serverHeader.timeSteb = 2;

		float ping = (int)GameTime::GetMSTimeOfPC() - (int)recentCheckedTime;
		if (ping < -5000) ping = ping + 10000;
		else if (ping > 5000) ping = ping - 10000; //�̻��� ���� ������ ����
		serverHeader.timeDifc = header.timeDifs - ping / 2; // �������� ���� ���� �ð����̸� ���Ѵ�(��Ȯ�Ҽ� ����).
	}
	else if (header.timeSteb == 3)
	{
		serverHeader.timeSteb = 0;
		currentTimeDif = (float)header.timeDifc / 100.f; //Ŭ�� ���� �ð����̸� ������ ���ȭ �ؼ� ������
	}
	recentCheckedTime = serverHeader.time; //���� �� ���� ���� �ð� �����ϱ�


	if (header.sceneType == ESceneType::LOGO)
	{
		
	}
	else
	{
		//���� ���̶�� ��ȣ ���� ���� ������
		if (header.gameEndType == EGameEndType::Win || header.gameEndType == EGameEndType::Lose)
		{
			gameEndType = header.gameEndType;
			gameManager->SetCurrentScene<EndSceneManager>(ESceneType::End);
			return;
		}

		auto stageManager = dynamic_cast<StageManager*>(gameManager->sceneManagers[(int)ESceneType::STAGE]);
		if (stageManager)stageManager->currentStage = header.stageLevel; // �������� ���� ����ȭ

		if (currentScene == ESceneType::LOGO) //���� Ŭ��� �ΰ�� ���������� �Ѿ��
		{
			gameManager->SetCurrentScene<StageManager>(ESceneType::STAGE);
		}
		
		while (inputStrm.GetRemainingDataSize() != 0) //�����Ͱ� ���� ���������� ������
		{
			EINetObjType objType;
			inputStrm.Read(&objType, 1);
			auto objGroup = gameManager->GetObjects();

			if (objType == EINetObjType::Player)
			{
				PlayerID pID;
				inputStrm.Read(&pID, sizeof(PlayerID));

				if (objGroup->playerMap->find(pID) == objGroup->playerMap->end()) //�ش� �÷��̾ �ʱ�ȭ �Ǿ����� �ʴٸ�
				{
					Player* player = new Player();
					player->Init(0, PLAYER_MAX_HP, false, false);
					player->id = pID;
					shared_ptr<Player> playerPtr(player);
					(*objGroup->playerMap)[pID] = playerPtr;
				}

				EDataType dataType;
				inputStrm.Read(&dataType, sizeof(EDataType));
				if (dataType == EDataType::RPC)
				{
					(*objGroup->playerMap)[pID]->ReadRPCData(&inputStrm);
				}
				else //���ø�����Ƽ��
				{
					(*objGroup->playerMap)[pID]->ReadReplicatedData(&inputStrm, header.time);
				}
			}
			else if (objType == EINetObjType::AI)
			{
				uint8_t enemyId;
				inputStrm.Read(&enemyId, 1);
				EDataType dataType;
				inputStrm.Read(&dataType, sizeof(EDataType));
				if (dataType == EDataType::RPC)
				{
					objGroup->enemies[enemyId]->ReadRPCData(&inputStrm);
				}
				else //���ø�����Ƽ��
				{
					objGroup->enemies[enemyId]->ReadReplicatedData(&inputStrm, header.time);
				}
			}
		}


	}
	currentScene = header.sceneType;
	serverHeader.sceneType = currentScene;
	outputStrm.Write(&serverHeader, sizeof(NetReliableHeader)); // ���� ��� ����

	//�� ������
	{
		std::lock_guard<std::mutex> lk(m); //stageOutputStrm�� ���Ӿ����忡�� ������ ���ν����忡�� ������ ū�� ���� ������ ���� ���ش�.

		if (stageDataOutputStrm->GetLength() != 0)outputStrm.Write(stageDataOutputStrm->GetBufferPtr(), stageDataOutputStrm->GetLength()); //������������ ���� �����͵� �߰����ֱ�. RPC�� ��������
		stageDataOutputStrm->Init();// �׸��� �ʱ�ȭ

	}

	//�������� ��Ŷ �Ǿպκп� ��Ŷ ũ�� �����. memcpy�� �����ϴ� ������ write�� ���ԵǸ� mHead������ �ڿ� ��.
	uint16_t outputPacketSize = outputStrm.GetLength();
	memcpy(outputStrm.GetBufferPtr(), &outputPacketSize, sizeof(uint16_t));

	inUdpSocket->SendTo(outputStrm.GetBufferPtr(), 1500, *inSocketAdr); // ������
	recentSendTime = GameTime::GetMSTimeOfPC();

	//������ ���� ��Ŷ�� �ش� �÷��̾� ������ ����.
	memcpy(recentSendedBuffer, outputStrm.GetBufferPtr(), outputStrm.GetLength());
}

void NetworkClientManager::ProcessOnUnReliable(EPacketType inPacketType, UDPSocket* inUdpSocket, SocketAddress* inSocketAdr, uint16_t packetSize, OutputMemoryStream& outputStrm, InputMemoryStream& inputStrm)
{
	if (inPacketType != EPacketType::Client) return; //��Ŷ Ÿ���� �ȸ����� ����

}


