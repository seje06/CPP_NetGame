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
	//처음에 패킷 사이즈 넣어주는 시늉하기. 시늉인 이유는 데이터를 다넣고 나서야 패킷 크기를 알수 있는데, 패킷앞에 넣어주려면 자리를 확보해야함.
	uint16_t tempPacketSize = outputStrm.GetLength();
	outputStrm.Write(&tempPacketSize, sizeof(uint16_t));

	uint16_t packetSize;
	memcpy(&packetSize, inBuffer, 2); // 패킷사이즈 직접 꺼내기. 인풋스트림 초기화할때 읽을 패킷의 크기를 전달해줘야해서 일단 크기를 알아낸다.

	InputMemoryStream inputStrm(inBuffer, packetSize); //인풋 스트림 초기화
	inputStrm.Read(&packetSize, sizeof(uint16_t)); // 패킷사이즈 타입 만큼 mHead 올리기
	
	EPacketType pakcetType;
	inputStrm.Read(&pakcetType, sizeof(EPacketType)); // 패킷 타입 꺼내기
	if (pakcetType == EPacketType::Client) // 서버에서 클라로 넣어준거면
	{
		EPacketType serverPacketType = EPacketType::Server;
		outputStrm.Write(&serverPacketType, sizeof(EPacketType)); //서버타입 넣어주기
	}
	else if (pakcetType == EPacketType::Server) // 클라에서 서버로 넣어준거면
	{
		EPacketType clientPacketType = EPacketType::Client;
		outputStrm.Write(&clientPacketType, sizeof(EPacketType)); //서버타입 넣어주기
	}

	EHeaderType headerType;
	inputStrm.Read(&headerType, sizeof(EHeaderType)); // 헤더 타입 꺼내기
	switch (headerType)
	{
		case EHeaderType::InitReliable: //초기화용 신뢰성 패킷인지
		{
			ProcessOnInitReliable(pakcetType, inUdpSocket, inSocketAdr,  packetSize, outputStrm, inputStrm);

			
			break;
		}
		case EHeaderType::Reliable: //신뢰성 패킷인가
		{
			ProcessOnReliable(pakcetType, inUdpSocket, inSocketAdr,packetSize, outputStrm, inputStrm);

			
			
			break;
		}
		case EHeaderType::UnReliable: //비신뢰인가
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
	if (inPacketType != EPacketType::Server) return; //패킷 타입이 안맞으면 리턴

	EHeaderType clientHeaderType = EHeaderType::InitReliable;
	outputStrm.Write(&clientHeaderType, sizeof(EHeaderType)); // 신뢰성헤더정보 클라 버퍼에 쓰기

	EPacketType clientPacketType = EPacketType::Client; // 클라에게

	NetReliableHeader header;
	inputStrm.Read(&header, sizeof(NetReliableHeader)); //신뢰성 헤더 꺼내기
	

	NetReliableHeader clientHeader;
	clientHeader.packetNum = header.packetNum + 1; // 초기화용 신뢰패킷에선 따로 검사하진않고 패킷번호 1더해서 클라에게 넘겨줘야함
	clientHeader.playerId = header.playerId;

	if (playerInfoMap.find(header.playerId) == playerInfoMap.end()) // 해당 아이디가 없을시, 플레이어 정보 생성하고 응답 패킷발송.
	{
		*playerInfoMap[header.playerId].sockAddr = *inSocketAdr;
		cout << "New Player came in, Id : " << header.playerId << endl;

		outputStrm.Write(&clientHeader, sizeof(NetReliableHeader)); //신뢰성헤더 쓰기
		
		ELoginResultType loginRes = ELoginResultType::Success;
		outputStrm.Write(&loginRes, sizeof(ELoginResultType)); //로그인 성공 데이터 넣어주기

		playerInfoMap[header.playerId].currentPacketNum = header.packetNum + 1; //플레이어 정보에도 패킷번호 저장
		
		currentPlayerCount++;
		currentPlayerCountInLogo++;

		// 현재 게임 id에 대해 만들어진 게임이 없으면 만들고 게임매니저 할당받기 
		if (gameInfoMap.find(currentGameCount) == gameInfoMap.end())
		{
			cout << "Game Id : " << currentGameCount<<", ";
			gameInfoMap[currentGameCount].gameManager = GameMaker::MakeGame();
		}
		gameInfoMap[currentGameCount].playerIdArray[currentPlayerCountInLogo - 1] = header.playerId; //게임 정보에 해당 게임에 들어가는 플레이어 아이디 저장
		gameInfoMap[currentGameCount].currentPlayerCountInGame = currentPlayerCountInLogo;

		playerInfoMap[header.playerId].gameId = currentGameCount; // 각 플레이어정보에 게임 아이디 저장

		if (currentPlayerCountInLogo == LOGO_ACCEPTABLE_MAX_COUNT) //로고에 수용인원 다 차면
		{
			gameInfoMap[currentGameCount].gameManager->SetCurrentScene<StageManager>(SCENE_ID::STAGE); //해당 게임의 씬을 스테이지로 변경
			cout <<"Game Id : " << currentGameCount<< ", Stage is Started" << endl;

			int playerColor = int(COLOR::LIGHTBLUE);
			for (auto& id : gameInfoMap[currentGameCount].playerIdArray)
			{
				
				playerInfoMap[id].sceneType = ESceneType::STAGE; // 각 플레이어정보에 씬 타입 저장

				auto player = new Player();
				player->Init(0, PLAYER_MAX_HP, false, false);
				player->id = id;
				player->color = (COLOR)playerColor++;
				shared_ptr<Player> playerPtr(player);
				auto objGroup = gameInfoMap[currentGameCount].gameManager->GetObjects();
				(*objGroup->playerMap)[id] = playerPtr; //해당 게임에 플레이어 캐릭 생성후 넣기
				playerInfoMap[id].playerCharac = playerPtr; // 플레이어 정보에 플레이어 캐릭 넣기
			}

			currentGameCount++; //게임 id 올리기
			currentPlayerCountInLogo = 0; // 스테이지로 올렸으니 로고인원 초기화
		}
		else
		{

		}
	}
	else //아이디가 이미 있을시 아이디의 주인이 보낸건지 확인한다.
	{
		if (playerInfoMap[header.playerId].sockAddr->GetIP4Ref() == inSocketAdr->GetIP4Ref()) // 주인이 맞다면(같은 ip인지 확인. 사설 ip까진 확인하지 않음)
		{
			//클라가 응답패킷을 못받았기에 다시 보내준다.
			inUdpSocket->SendTo(playerInfoMap[header.playerId].ackBuffer, 1500, *inSocketAdr);
			*playerInfoMap[header.playerId].sockAddr = *inSocketAdr;
			std::cout << "Player Id : " << header.playerId << ", reLogin. Packet num : "<< (int)header.packetNum << std::endl;
			return;
		}
		else
		{
			outputStrm.Write(&clientHeader, sizeof(NetReliableHeader)); //신뢰성헤더 쓰기

			ELoginResultType loginRes = ELoginResultType::Failed;
			outputStrm.Write(&loginRes, sizeof(ELoginResultType));

			//마지막에 패킷 맨앞부분에 패킷 크기 덮어쓰기. memcpy를 직접하는 이유는 write를 쓰게되면 mHead때문에 뒤에 들어감.
			uint16_t outputPacketSize = outputStrm.GetLength();
			memcpy(outputStrm.GetBufferPtr(), &outputPacketSize, sizeof(uint16_t));

			inUdpSocket->SendTo(outputStrm.GetBufferPtr(), 1500, *inSocketAdr);
			return;
		}
	}

	//마지막에 패킷 맨앞부분에 패킷 크기 덮어쓰기. memcpy를 직접하는 이유는 write를 쓰게되면 mHead때문에 뒤에 들어감.
	uint16_t outputPacketSize = outputStrm.GetLength();
	memcpy(outputStrm.GetBufferPtr(), &outputPacketSize, sizeof(uint16_t));
	
	inUdpSocket->SendTo(outputStrm.GetBufferPtr(), 1500, *inSocketAdr); 
	
	//응답을 위한 패킷을 해당 플레이어 정보에 저장.
	memcpy(playerInfoMap[header.playerId].ackBuffer, outputStrm.GetBufferPtr(), outputPacketSize);

	
}

void NetworkServerManager::ProcessOnReliable(EPacketType inPacketType, UDPSocket* inUdpSocket, SocketAddress* inSocketAdr, uint16_t packetSize, OutputMemoryStream& outputStrm, InputMemoryStream& inputStrm)
{
	if (inPacketType != EPacketType::Server) return; //패킷 타입이 안맞으면 리턴

	EHeaderType clientHeaderType = EHeaderType::Reliable;
	outputStrm.Write(&clientHeaderType, sizeof(EHeaderType)); // 헤더타입 클라 버퍼에 쓰기

	NetReliableHeader header;
	inputStrm.Read(&header, sizeof(NetReliableHeader)); //신뢰성 헤더 꺼내기
	
	if (playerInfoMap.find(header.playerId) == playerInfoMap.end()) return; //만약 없는 아이디면 게임이 제거 된 것이기에 리턴.

	if (++playerInfoMap[header.playerId].currentPacketNum != header.packetNum) //만약 전에 왔던 패킷이면
	{
		--playerInfoMap[header.playerId].currentPacketNum;
		//저장해놓은 패킷 재송신
		inUdpSocket->SendTo(playerInfoMap[header.playerId].ackBuffer, 1500, *playerInfoMap[header.playerId].sockAddr);
		return;
	}

	if (gameInfoMap[playerInfoMap[header.playerId].gameId].gameManager->id == ESceneType::End) return; //만약 EndScene이면 재전송 말곤 패킷을 더이상 보낼 필요 없기에 리턴.

	 //해당 패킷이 처음 왔을때
	
	NetReliableHeader clientHeader;  //클라에게 보낼 헤더 설정
	clientHeader.packetNum = header.packetNum + 1;

	playerInfoMap[header.playerId].currentPacketNum = header.packetNum + 1; // 플레이어 정보에 패킷번호 갱신
	if (header.timeSteb == 0 && playerInfoMap[header.playerId].updatedTDCount < 65000) //클라로부터 스텝 0
	{
		float timeDifs = (int)GameTime::GetMSTimeOfPC() - (int)header.time;
		if (timeDifs < -5000) timeDifs = timeDifs + 10000;
		else if (timeDifs > 5000) timeDifs = timeDifs - 10000; //이상한 값이 나오면 보정


		clientHeader.timeSteb = 1;
		clientHeader.timeDifs = timeDifs;
	}
	else if (header.timeSteb == 2 && playerInfoMap[header.playerId].updatedTDCount < 65000) //스텝 2
	{
		clientHeader.timeSteb = 3; // 클라에게 해당 타입 스텝 과정 끝이라고 알릴 값
		//pc시간차이 갱신 값을 정상화 한다. ex) 2580 -> 25.80
		if (playerInfoMap[header.playerId].currentTimeDif == 0)playerInfoMap[header.playerId].currentTimeDif = ((float)header.timeDifc) / 100;
		else
		{
			//timeDif의 평균값을 넣어준다
			playerInfoMap[header.playerId].currentTimeDif = (playerInfoMap[header.playerId].currentTimeDif * playerInfoMap[header.playerId].updatedTDCount + ((float)header.timeDifc) / 100) / (playerInfoMap[header.playerId].updatedTDCount + 1);
		}
		clientHeader.timeDifc = playerInfoMap[header.playerId].currentTimeDif * 100;
	}

	clientHeader.sceneType = playerInfoMap[header.playerId].sceneType; //어느 씬에서 보낸건지 넣어주기
	clientHeader.playerCount = gameInfoMap[playerInfoMap[header.playerId].gameId].currentPlayerCountInGame; // 플레이어가 속한 게임의 사람 수
	
	//outputStrm.Write(&playerInfoMap[header.playerId].sceneType, 1); //클라용 버퍼에 플레이어의 씬 타입 쓰기

	if (clientHeader.sceneType == ESceneType::LOGO) //로고 일 경우
	{
		outputStrm.Write(&clientHeader, sizeof(NetReliableHeader)); // 클라용 버퍼에 신뢰용 헤더 정보 쓰기
	}
	else //스테이지 일 경우
	{
		//게임이 승 또는 패로 끝났을경우 클라에게 정보 알려주기
		if (gameInfoMap[playerInfoMap[header.playerId].gameId].gameEndType == EGameEndType::Lose || gameInfoMap[playerInfoMap[header.playerId].gameId].gameEndType == EGameEndType::Win)
		{
			if(gameInfoMap[playerInfoMap[header.playerId].gameId].gameEndType == EGameEndType::Lose) clientHeader.gameEndType = EGameEndType::Lose;
			else clientHeader.gameEndType = EGameEndType::Win;

		}
		auto stageManager = dynamic_cast<StageManager*>(gameInfoMap[playerInfoMap[header.playerId].gameId].gameManager->sceneManagers[(int)ESceneType::STAGE]);
		if (stageManager)clientHeader.stageLevel = stageManager->currentStage; // 스테이지 레벨 보내기
		outputStrm.Write(&clientHeader, sizeof(NetReliableHeader)); // 클라용 버퍼에 신뢰용 헤더 정보 쓰기

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

		//락 스코프
		{
			std::lock_guard<std::mutex> lk(playerInfoMap[header.playerId].m); //stageOutputStrm은 게임쓰레드에서 쓰는중 메인스레드에서 읽으면 큰일 나기 때문에 락을 해준다.
			auto stageOutputStrm = playerInfoMap[header.playerId].stageDataOutputStrm;
			if (stageOutputStrm->GetLength() != 0)
				outputStrm.Write(stageOutputStrm->GetBufferPtr(), stageOutputStrm->GetLength()); //스테이지에서 얻은 데이터 추가하기
			stageOutputStrm->Init(); // 그리고 초기화
		}

		for (auto& playerId : gameInfoMap[playerInfoMap[header.playerId].gameId].playerIdArray)
		{
			playerInfoMap[playerId].playerCharac->WirteReplicatedData(&outputStrm); // 해당 플레이어가 속한 게임의 모든 플레이어의 replicated 데이터 쓰기
		}
		auto enemies = gameInfoMap[playerInfoMap[header.playerId].gameId].gameManager->GetObjects()->enemies;
		for (int i = 0; i < ENEMY_COUNT; i++)
		{
			enemies[i]->WirteReplicatedData(&outputStrm); // 해당 플레이어가 속한 게임의 모든 AI의 replicated 데이터 쓰기
		}
	}

	//마지막에 패킷 맨앞부분에 패킷 크기 덮어쓰기. memcpy를 직접하는 이유는 write를 쓰게되면 mHead때문에 뒤에 들어감.
	uint16_t outputPacketSize = outputStrm.GetLength();
	memcpy(outputStrm.GetBufferPtr(), &outputPacketSize, sizeof(uint16_t));

	inUdpSocket->SendTo(outputStrm.GetBufferPtr(), 1500, *inSocketAdr); // 패킷전송

	//응답을 위한 패킷을 해당 플레이어 정보에 저장.
	memcpy(playerInfoMap[header.playerId].ackBuffer, outputStrm.GetBufferPtr(), outputStrm.GetLength());
	
	//게임이 승 또는 패로 끝났을경우 게임 끝 씬으로 전환
	if (clientHeader.gameEndType == EGameEndType::Lose || clientHeader.gameEndType == EGameEndType::Win)
	{
		//gameInfoMap[playerInfoMap[header.playerId].gameId].gameManager->SetCurrentScene<EndSceneManager>(ESceneType::End);
		gameInfoMap[playerInfoMap[header.playerId].gameId].endPlayerCount++;
		if (gameInfoMap[playerInfoMap[header.playerId].gameId].endPlayerCount == LOGO_ACCEPTABLE_MAX_COUNT) // 해당 게임의 플레이어 전원 endScene 패킷을 받는 중인지
		{
			gameInfoMap[playerInfoMap[header.playerId].gameId].gameManager->SetCurrentScene<EndSceneManager>(ESceneType::End);

			auto endScene = dynamic_cast<EndSceneManager*>(gameInfoMap[playerInfoMap[header.playerId].gameId].gameManager->sceneManagers[(int)ESceneType::End]);
			endScene->gameId = playerInfoMap[header.playerId].gameId;
		}
	}
}

void NetworkServerManager::ProcessOnUnReliable(EPacketType inPacketType, UDPSocket* inUdpSocket, SocketAddress* inSocketAdr, uint16_t packetSize, OutputMemoryStream& outputStrm, InputMemoryStream& inputStrm)
{
	if (inPacketType != EPacketType::Server) return; //패킷 타입이 안맞으면 리턴

}


/////////////////////////            Client Code          /////////////////////////

NetworkClientManager::NetworkClientManager()
{
	stageDataOutputStrm = new OutputMemoryStream();
}

void NetworkClientManager::SendLoginPacket(unsigned long inPlayerId,  UDPSocket* inUdpSocket, SocketAddress* inSocketAdr)
{
	OutputMemoryStream outputStrm;
	//처음에 패킷 사이즈 넣어주는 시늉하기. 시늉인 이유는 데이터를 다넣고 나서야 패킷 크기를 알수 있는데, 패킷앞에 넣어주려면 자리를 확보해야함.
	uint16_t tempPacketSize = outputStrm.GetLength();
	outputStrm.Write(&tempPacketSize, sizeof(uint16_t));

	EPacketType serverPacketType = EPacketType::Server;
	outputStrm.Write(&serverPacketType, sizeof(EPacketType)); // 보낼 패킷타입을 서버 버퍼에 쓰기

	EHeaderType serverHeaderType = EHeaderType::InitReliable;
	outputStrm.Write(&serverHeaderType, sizeof(EHeaderType)); // 보낼 헤더타입을 서버 버퍼에 쓰기

	currentPacketNum = 1;
	NetReliableHeader serverHeader; //서버에게 보낼 헤더
	serverHeader.packetNum = currentPacketNum;
	serverHeader.playerId = inPlayerId;
	playerId = inPlayerId;
	outputStrm.Write(&serverHeader, sizeof(NetReliableHeader));

	//마지막에 패킷 맨앞부분에 패킷 크기 덮어쓰기. memcpy를 직접하는 이유는 write를 쓰게되면 mHead때문에 뒤에 들어감.
	uint16_t outputPacketSize = outputStrm.GetLength();
	memcpy(outputStrm.GetBufferPtr(), &outputPacketSize, sizeof(uint16_t));

	inUdpSocket->SendTo(outputStrm.GetBufferPtr(), 1500, *inSocketAdr);//보내기
	recentSendTime = GameTime::GetMSTimeOfPC();

	//응답을 위한 패킷을 해당 플레이어 정보에 저장.
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
	if (inPacketType != EPacketType::Client) return; //패킷 타입이 안맞으면 리턴

	NetReliableHeader header;
	inputStrm.Read(&header, sizeof(NetReliableHeader)); // 헤더 읽기

	if (header.playerId != playerId) //내가 보낸 playerID가 아닐경우 받지않는다 
	{
		return;
	}

	if (header.packetNum != ++currentPacketNum) // 해당 패킷이 처음 온게 아니면 받지않는다
	{
		--currentPacketNum;
		return;
	}

	ELoginResultType loginRes;
	inputStrm.Read(&loginRes, sizeof(ELoginResultType)); // 로그인 결과 읽기

	if (loginRes == ELoginResultType::Failed) //로그인 실패시
	{
		// id 재입력, 헤더 재설정 후 다시 보내기 
		std::cout << "로그인 실패!. 다른 아이디로 다시 입력 해주세요." << std::endl;
		SendLoginPacket(Login(), inUdpSocket, inSocketAdr);
		return;
	}

	isFinishInit = true;

	playerId = header.playerId;
	gameManager = GameMaker::MakeGame();
	
	EHeaderType serverHeaderType = EHeaderType::Reliable;
	outputStrm.Write(&serverHeaderType, sizeof(EHeaderType)); // 보낼 헤더타입을 서버 버퍼에 쓰기

	NetReliableHeader serverHeader; // 서버에게 보낼 헤더
	serverHeader.packetNum = ++currentPacketNum;
	serverHeader.time = GameTime::GetMSTimeOfPC();
	recentCheckedTime = serverHeader.time;
	serverHeader.timeSteb = 0; // 초기화 신뢰용이아닌 신뢰용으로 처음 보낼때는 0으로 
	serverHeader.playerId = playerId;
	serverHeader.sceneType = ESceneType::LOGO;

	outputStrm.Write(&serverHeader, sizeof(NetReliableHeader)); // 보낼 헤더 쓰기
	//outputStrm.Write(&currentScene, sizeof(ESceneType)); // 현재 씬 타입 쓰기

	//마지막엔 패킷 맨앞부분에 패킷 크기 덮어쓰기. memcpy를 직접하는 이유는 write를 쓰게되면 mHead때문에 뒤에 들어감.
	uint16_t outputPacketSize = outputStrm.GetLength();
	memcpy(outputStrm.GetBufferPtr(), &outputPacketSize, sizeof(uint16_t));

	inUdpSocket->SendTo(outputStrm.GetBufferPtr(), 1500, *inSocketAdr);//보내기
	recentSendTime = GameTime::GetMSTimeOfPC();

	//응답을 위한 패킷을 해당 플레이어 정보에 저장.
	memcpy(recentSendedBuffer, outputStrm.GetBufferPtr(), outputStrm.GetLength());
}

void NetworkClientManager::ProcessOnReliable(EPacketType inPacketType, UDPSocket* inUdpSocket, SocketAddress* inSocketAdr, uint16_t packetSize, OutputMemoryStream& outputStrm, InputMemoryStream& inputStrm)
{
	if (inPacketType != EPacketType::Client) return; //패킷 타입이 안맞으면 리턴

	NetReliableHeader header;
	inputStrm.Read(&header, sizeof(NetReliableHeader)); // 헤더 읽기
	//ESceneType sceneType;
	//inputStrm.Read(&sceneType, sizeof(ESceneType)); // 현재 있어야할 씬 읽기

	if (!isFinishInit) //초기화 통신이 완료되지않았는데 여기로 왔다는건 해당 아이디로 돌아가는 플레이어가 아직 서버에 살아있다는것. 즉 재접속이니 게임을 만들어줘야한다
	{
		//게임 생성
		gameManager = GameMaker::MakeGame();
		currentPacketNum = header.packetNum;
		isFinishInit = true;
	}
	else if (header.packetNum != ++currentPacketNum) // 초기화됐는데도 해당 패킷이 처음 온게 아니면 받지않는다
	{
		--currentPacketNum;
		return;
	}

	if (gameManager->id == ESceneType::End) return; //현재 끝 씬이면 패킷을 받지않는다.

	currentPlayerCount = header.playerCount;

	EHeaderType serverHeaderType = EHeaderType::Reliable;
	outputStrm.Write(&serverHeaderType, sizeof(EHeaderType)); // 보낼 헤더타입을 서버 버퍼에 쓰기

	NetReliableHeader serverHeader; // 서버에게 보낼 헤더
	serverHeader.playerId = playerId;
	serverHeader.packetNum = ++currentPacketNum;
	serverHeader.time = GameTime::GetMSTimeOfPC();

	//serverHeader.time = GameTime::GetMSTimeOfPC();
	if (header.timeSteb == 1)
	{
		serverHeader.timeSteb = 2;

		float ping = (int)GameTime::GetMSTimeOfPC() - (int)recentCheckedTime;
		if (ping < -5000) ping = ping + 10000;
		else if (ping > 5000) ping = ping - 10000; //이상한 값이 나오면 보정
		serverHeader.timeDifc = header.timeDifs - ping / 2; // 하프핑을 빼서 컴터 시간차이를 구한다(정확할순 없음).
	}
	else if (header.timeSteb == 3)
	{
		serverHeader.timeSteb = 0;
		currentTimeDif = (float)header.timeDifc / 100.f; //클라가 보낸 시간차이를 서버가 평균화 해서 보내줌
	}
	recentCheckedTime = serverHeader.time; //보낼 떄 마다 컴터 시간 저장하기


	if (header.sceneType == ESceneType::LOGO)
	{
		
	}
	else
	{
		//게임 끝이라는 신호 오면 게임 끝내기
		if (header.gameEndType == EGameEndType::Win || header.gameEndType == EGameEndType::Lose)
		{
			gameEndType = header.gameEndType;
			gameManager->SetCurrentScene<EndSceneManager>(ESceneType::End);
			return;
		}

		auto stageManager = dynamic_cast<StageManager*>(gameManager->sceneManagers[(int)ESceneType::STAGE]);
		if (stageManager)stageManager->currentStage = header.stageLevel; // 스테이지 레벨 동기화

		if (currentScene == ESceneType::LOGO) //아직 클라는 로고라 스테이지로 넘어가기
		{
			gameManager->SetCurrentScene<StageManager>(ESceneType::STAGE);
		}
		
		while (inputStrm.GetRemainingDataSize() != 0) //데이터가 남지 않을때까지 돌린다
		{
			EINetObjType objType;
			inputStrm.Read(&objType, 1);
			auto objGroup = gameManager->GetObjects();

			if (objType == EINetObjType::Player)
			{
				PlayerID pID;
				inputStrm.Read(&pID, sizeof(PlayerID));

				if (objGroup->playerMap->find(pID) == objGroup->playerMap->end()) //해당 플레이어가 초기화 되어있지 않다면
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
				else //리플리케이티드
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
				else //리플리케이티드
				{
					objGroup->enemies[enemyId]->ReadReplicatedData(&inputStrm, header.time);
				}
			}
		}


	}
	currentScene = header.sceneType;
	serverHeader.sceneType = currentScene;
	outputStrm.Write(&serverHeader, sizeof(NetReliableHeader)); // 보낼 헤더 쓰기

	//락 스코프
	{
		std::lock_guard<std::mutex> lk(m); //stageOutputStrm은 게임쓰레드에서 쓰는중 메인스레드에서 읽으면 큰일 나기 때문에 락을 해준다.

		if (stageDataOutputStrm->GetLength() != 0)outputStrm.Write(stageDataOutputStrm->GetBufferPtr(), stageDataOutputStrm->GetLength()); //스테이지에서 쌓은 데이터들 추가해주기. RPC만 있을예정
		stageDataOutputStrm->Init();// 그리고 초기화

	}

	//마지막에 패킷 맨앞부분에 패킷 크기 덮어쓰기. memcpy를 직접하는 이유는 write를 쓰게되면 mHead때문에 뒤에 들어감.
	uint16_t outputPacketSize = outputStrm.GetLength();
	memcpy(outputStrm.GetBufferPtr(), &outputPacketSize, sizeof(uint16_t));

	inUdpSocket->SendTo(outputStrm.GetBufferPtr(), 1500, *inSocketAdr); // 보내기
	recentSendTime = GameTime::GetMSTimeOfPC();

	//응답을 위한 패킷을 해당 플레이어 정보에 저장.
	memcpy(recentSendedBuffer, outputStrm.GetBufferPtr(), outputStrm.GetLength());
}

void NetworkClientManager::ProcessOnUnReliable(EPacketType inPacketType, UDPSocket* inUdpSocket, SocketAddress* inSocketAdr, uint16_t packetSize, OutputMemoryStream& outputStrm, InputMemoryStream& inputStrm)
{
	if (inPacketType != EPacketType::Client) return; //패킷 타입이 안맞으면 리턴

}


