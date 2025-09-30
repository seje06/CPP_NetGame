#include "EndSceneManager.h"
#include "../StageRelation/MapManager.h"
#include "BufferManager.h"
#include "Enums.h"
#include "NetGameBuildInfo.h"
#include "NetworkManager.h"
#include "iostream"

void EndSceneManager::Init()
{
}

void EndSceneManager::Progress(float _deltaTime)
{
	timer += _deltaTime;

	if (timer > 5 && gameId != -1)
	{
#if SERVER
		timer = 0;
		for (auto playerId : NetworkServerManager::GetInstance()->gameInfoMap[gameId].playerIdArray)
		{
			NetworkServerManager::GetInstance()->playerInfoMap.erase(playerId);
		}
		std::cout<< "Game Id : "<<gameId<<", End Game "<<std::endl;
		NetworkServerManager::GetInstance()->gameInfoMap.erase(gameId);
#endif
		
	}
}

void EndSceneManager::Render()
{
	int cx = MAP_WIDTH / 2 - 4;
	int cy = MAP_HEIGHT / 2;
	BufferManager::GetInstance()->WriteBuffer(cx, cy - 1, "================", (int)COLOR::YELLOW);
	if (NetworkClientManager::GetInstance()->gameEndType == EGameEndType::Win) 	BufferManager::GetInstance()->WriteBuffer(cx, cy, "  You Win!!  ", (int)COLOR::LIGHTGREEN);
	else if (NetworkClientManager::GetInstance()->gameEndType == EGameEndType::Lose) 	BufferManager::GetInstance()->WriteBuffer(cx, cy, "  You Lose....  ", (int)COLOR::LIGHTGREEN);
	BufferManager::GetInstance()->WriteBuffer(cx, cy + 1, "================", (int)COLOR::YELLOW);
}
