#include "LogoScene.h"
#include "../StageRelation/MapManager.h"
#include "BufferManager.h"
#include "Enums.h"
#include "NetworkManager.h"
#include <string> 

LogoScene::LogoScene()
{

}

void LogoScene::Init()
{
	
}

void LogoScene::Progress(float _deltaTime)
{
	
	
}

void LogoScene::Render()
{
	// 간단 안내 렌더
	int cx = MAP_WIDTH / 2 - 8;
	int cy = MAP_HEIGHT / 2;
	BufferManager::GetInstance()->WriteBuffer(cx, cy - 1, "================", (int)COLOR::YELLOW);
	BufferManager::GetInstance()->WriteBuffer(cx, cy, "  Waiting Players...  ", (int)COLOR::LIGHTGREEN);
	BufferManager::GetInstance()->WriteBuffer(cx, cy + 1, "================", (int)COLOR::YELLOW);
	BufferManager::GetInstance()->WriteBuffer(cx, cy + 2, "Current Player Count : ", (int)COLOR::YELLOW);
	BufferManager::GetInstance()->WriteBuffer(cx+12, cy + 2, std::to_string((int)NetworkClientManager::GetInstance()->currentPlayerCount).c_str(), (int)COLOR::WHITE);
}
