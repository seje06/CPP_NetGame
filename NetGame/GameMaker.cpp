#include "GameMaker.h"
#include "GameTime.h"
#include "BufferManager.h"
#include "SceneRelation/LogoRelation/LogoScene.h"
#include "SceneRelation/StageRelation/StageManager.h"
#include "NetGameBuildInfo.h"
#include "GameManager.h"
#include <iostream>

using namespace std;

shared_ptr<GameManager> GameMaker::MakeGame()
{
	std::shared_ptr<GameManager> gameManager;
#if SERVER
	gameManager = std::make_shared<GameManager>();
	std::cout << "Server MakeGame" << std::endl;
#else
	gameManager = shared_ptr<GameManager>(GameManager::GetInstance(),[](GameManager*) {}); // 싱글톤이 해당 포인터에 주인이니까, 커스텀 델리터 줘서 스마트 포인터가 삭제 안하게끔 하기
//	std::cout << "Client MakeGame" << std::endl;
#endif
	GameTime* gameTime = new GameTime();

	gameManager->Init(SCENE_ID::LOGO, gameTime->deltaTime);
	gameManager->SetCurrentScene<LogoScene>(SCENE_ID::LOGO);

	gameThreadVec.push_back(std::thread(&GameMaker::ProcessGame, gameManager, gameTime));
	
	return gameManager;
}

void GameMaker::ProcessGame(std::shared_ptr<GameManager> gameManager, GameTime* gameTime)
{
	int frameSpeed = 2;
	ULONGLONG time = GetTickCount64();

#if SERVER
#else
	BufferManager::GetInstance()->InitBuffer();
#endif

	while (true)
	{
		if (time + 100 / frameSpeed <= GetTickCount64())
		{
			if (!GameMaker::canRunning || gameManager.use_count() == 1)
			{
				delete gameTime;
				return;
			}

			//GameManager::GetInstance()->deltaTime = (GetTickCount64() - time) / 1000.0;
			gameTime->deltaTime = (GetTickCount64() - time) / 1000.0;
			time = GetTickCount64();

#if SERVER
#else
			BufferManager::GetInstance()->ClearBuffer();
#endif
			gameManager->ManageScene();
#if SERVER
#else
			BufferManager::GetInstance()->FlipBuffer();
#endif
		}
	}
#if SERVER
#else
	BufferManager::GetInstance()->ReleaseBuffer();
#endif
}

std::vector<std::thread> GameMaker::gameThreadVec;
atomic<bool> GameMaker:: canRunning = true;
