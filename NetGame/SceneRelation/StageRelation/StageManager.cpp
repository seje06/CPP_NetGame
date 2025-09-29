#include "StageManager.h"
#include"../../GameManager.h"
#include"BufferManager.h"
#include"Character/Enemy/EnemiesManager.h"
#include"MapManager.h"
#include "GameTime.h"
#include "NetGameBuildInfo.h"
#include "NetworkManager.h"
#include "Character/Enemy/Enemy.h"
#include "Character/Player/Player.h"
#include <cstdint>
using namespace std;

StageManager::StageManager()
{
	currentStage = 1;
	//player = nullptr;
#if SERVER

#else
	//GameManager::GetInstance()->AddSceneManager(this,SCENE_ID::STAGE);
#endif
	enemiesManager = new EnemiesManager();
}

void StageManager::Init()
{
	currentStage = 1;

	/*auto playerIt = playerMap.begin();
	while (playerIt != playerMap.end())
	{
		dynamic_cast<Player*>(playerIt->second.get())->Init(0, PLAYER_MAX_HP, false, false);
		playerIt++;
	}*/

	/*player = new Player();
	player->Init(0, PLAYER_MAX_HP, false, false);*/
	enemiesManager->Init();

}

void StageManager::Progress(float _deltaTime)
{
#if SERVER
	if ((currentStage == 5 && enemiesManager->IsAllDie()) || playerDeathCount == LOGO_ACCEPTABLE_MAX_COUNT) //게임 승리 또는 패배시 승패 정보 저장하고 리턴.
	{
		if (playerMap.begin() == playerMap.end()) return;
		auto gameId = NetworkServerManager::GetInstance()->playerInfoMap[playerMap.begin()->second->id].gameId;
		if (NetworkServerManager::GetInstance()->gameInfoMap.find(gameId) == NetworkServerManager::GetInstance()->gameInfoMap.end()) return;

		if (currentStage == 5 && enemiesManager->IsAllDie()) NetworkServerManager::GetInstance()->gameInfoMap[gameId].gameEndType = EGameEndType::Win;
		else NetworkServerManager::GetInstance()->gameInfoMap[gameId].gameEndType = EGameEndType::Lose;

		cout << "End Game" << endl;
		return;
	}
	StageLevelUp();
	if(currentStage!=5) enemiesManager->Spawn(_deltaTime);
#endif

	auto playerIt = playerMap.begin();
	while (playerIt != playerMap.end())
	{
		if (!playerIt->second) continue;
		auto iNetObj = playerIt->second.get();
		Player* player = dynamic_cast<Player*>(iNetObj);
#if SERVER
		
#else 
		//player = dynamic_cast<Player*>(playerMap[GameManager::GetInstance()->gameOwnerId].get());
		player->CheckPlayerInput(_deltaTime);
#endif
		player->Controll(_deltaTime);
		playerIt++;
	}

#if SERVER
	enemiesManager->Controll(playerMap, _deltaTime);
	SensePlayerHit(_deltaTime);
#endif
	SenseEnemyHit(_deltaTime);
}

void StageManager::Render()
{
	//맵
	for (int y = 0; y < MAP_HEIGHT; y++)
	{
		for (int x = 0; x < MAP_WIDTH; x++)
		{
			switch (MapManager::GetInstance()->map[y][x])
			{
			case 1:
				BufferManager::GetInstance()->WriteBuffer(x, y, "■", (int)COLOR::CYAN);
				break;
			default:
				break;
			}

		}
	}

	//스테이지 정보//


	Player* player = nullptr;
	auto playerIt = playerMap.begin();
	while (playerIt != playerMap.end())
	{
		auto iNetObj = playerIt->second.get();
		player = dynamic_cast<Player*>(iNetObj);
		if (!playerIt->second) continue;
		//플레이어
		for (int i = 0; i < PLAYER_HEIGHT; i++)
		{
			if(!player->isDie) BufferManager::GetInstance()->WriteBuffer((int)player->pos.x, player->pos.y + i, player->shape[player->dir][player->aniIndex][i], (int)player->color);
		}
		playerIt++;
	}

#if SERVER
	
#else 
	player = dynamic_cast<Player*>(playerMap[NetworkClientManager::GetInstance()->playerId].get());
#endif
	//플레이어 상태
	if(player)
	for (int i = 0; i < player->hp; i++)
	{
		BufferManager::GetInstance()->WriteBuffer(i + 1, 10, "♥", (int)COLOR::WHITE);
	}

	//총알
	int j = 0;
	for (auto it : playerMap)
	{
		if (!player) continue;
		player = it.second.get();
		for (int i = 0; i < BULLET_COUNT; i++)
		{
			if (player->gun->bullets[i]->isActive)
			{
				BufferManager::GetInstance()->WriteBuffer((int)player->gun->bullets[i]->pos.x, player->gun->bullets[i]->pos.y, "@", (int)COLOR::LIGHTMAGENTA);
			}
			else if(playerMap[NetworkClientManager::GetInstance()->playerId].get()== player)
			{
				BufferManager::GetInstance()->WriteBuffer(++j + 20, 10, "@", (int)COLOR::WHITE);
			}
		}
	}
	//const char* t = player->hitTime >= 1? "1":"0";
	//WriteBuffer(10, 10, t, WHITE);

	//적
	for (int i = 0; i < ENEMY_COUNT; i++)
	{
		Enemy* enemy = enemiesManager->enemies[i].get();
		if (enemy->isActive)
		{
			for (j = 0; j < PLAYER_HEIGHT; j++)
			{
				BufferManager::GetInstance()->WriteBuffer((int)enemy->pos.x, enemy->pos.y + j, enemy->shape[enemy->dir][enemy->aniIndex][j], (int)enemy->color);

			}
		}
	}
}


void StageManager::StageLevelUp()
{
	
	if (enemiesManager->deathCount % 5 == 0 && enemiesManager->deathCount != 0&& currentStage!=5)
	{
		currentStage++;
		enemiesManager->deathCount = 0;
		isPartialClear = true;
	}

	if (isPartialClear)
	{
		int random;
		isPartialClear = false;

		for (int i = 0; i < ENEMY_COUNT; i++)
		{
			Enemy* enemy = enemiesManager->enemies[i].get();
			enemy->isDie = false;
			enemy->hitTime = 0;
			enemy->isHitting = false;
		}

		switch (currentStage)
		{
		case 0:

			break;
		case 1:
			for (int i = 0; i < ENEMY_COUNT; i++)
			{
				enemiesManager->enemies[i]->type = ENEMY_TYPE::EASY;
			}
			break;
		case 2:
			for (int i = 0; i < ENEMY_COUNT; i++)
			{
				Enemy* enemy = enemiesManager->enemies[i].get();
				random = rand() % 3;
				if (random == 0) enemy->type = ENEMY_TYPE::EASY;
				else if (random == 1)enemy->type = ENEMY_TYPE::GENERAL;
				else enemy->type = ENEMY_TYPE::FREAKISH;
			}
			break;
		case 3:
			for (int i = 0; i < ENEMY_COUNT; i++)
			{
				Enemy* enemy = enemiesManager->enemies[i].get();
				random = rand() % 3;
				if (random == 0) enemy->type = ENEMY_TYPE::GENERAL;
				else if (random == 1)enemy->type = ENEMY_TYPE::HARD;
				else enemy->type = ENEMY_TYPE::FREAKISH;
			}
			break;
		case 4:
			for (int i = 0; i < ENEMY_COUNT; i++)
			{
				Enemy* enemy = enemiesManager->enemies[i].get();
				random = rand() % 3;
				if (random == 0) enemy->type = ENEMY_TYPE::HARD;
				else if (random == 1)enemy->type = ENEMY_TYPE::FREAKISH;
				else enemy->type = ENEMY_TYPE::HARD_FREAKISH;
			}
			break;
		case 5:
			break;
		}
		isPartialClear = false;
	}
}

void StageManager::SensePlayerHit(float _deltaTime)
{
	
	Player* player;
	auto playerIt = playerMap.begin();

	while (playerIt != playerMap.end())
	{
		auto iNetObj = playerIt->second.get();
		player = dynamic_cast<Player*>(iNetObj);

		if (player->isDie) 
		{
			playerIt++;
			continue;
		}

		if (player->isHitting)
		{
			player->hitTime += _deltaTime;
			if (player->hitTime >= 1) player->isHitting = false;
		}
		if (!player->isHitting)
		{
			for (int i = 0; i < ENEMY_COUNT; i++)
			{
				Enemy* enemy = enemiesManager->enemies[i].get();
				if (enemiesManager->enemies[i]->isActive)
				{
					if (enemy->pos.y <= player->pos.y + PLAYER_HEIGHT - 1 && enemy->pos.y + PLAYER_HEIGHT - 1 >= player->pos.y &&
						(int)enemy->pos.x == (int)player->pos.x)
					{
						player->isHitting = true;
						player->hp -= 1;
						player->hitTime = 0;
						if (player->hp <= 0)
						{
							player->isDie = true;
							playerDeathCount++;
						}
						break;
					}
				}
			}
		}

		playerIt++;
	}
}

void StageManager::SenseEnemyHit(float _deltaTime)
{
	Player* player;
	auto playerIt = playerMap.begin();

	while (playerIt != playerMap.end())
	{
		auto iNetObj = playerIt->second.get();
		player = dynamic_cast<Player*>(iNetObj);

		for (int i = 0; i < ENEMY_COUNT; i++)
		{
			Enemy* enemy = enemiesManager->enemies[i].get();
			if (enemy->isActive)
			{
#if SERVER
				if (enemy->isHitting)
				{
					enemy->hitTime += _deltaTime;
					if (enemy->hitTime >= 0)
					{
						enemy->hitTime = 0;
						enemy->isHitting = false;
					}
				}
#endif
				if (!enemy->isHitting)
				{
					for (int j = 0; j < BULLET_COUNT; j++)
					{
						if (player->gun->bullets[j]->isActive&&
							enemy->pos.y <= player->gun->bullets[j]->pos.y && enemy->pos.y + PLAYER_HEIGHT - 1 >= player->gun->bullets[j]->pos.y && 
							((player->gun->bullets[j]->dir == (int)DIR::LEFT&& (int)enemy->pos.x >= (int)player->gun->bullets[j]->pos.x&& (int)enemy->pos.x <= (int)player->gun->bullets[j]->prePos.x)||
								(player->gun->bullets[j]->dir == (int)DIR::RIGHT && (int)enemy->pos.x <= (int)player->gun->bullets[j]->pos.x && (int)enemy->pos.x >= (int)player->gun->bullets[j]->prePos.x)))
						{
							player->gun->bullets[j]->isActive = false;
#if SERVER
							enemy->isHitting = true;
							enemy->hp -= 1;
							enemy->hitTime = 0;
							if (enemy->hp <= 0)
							{
								enemy->isDie = true;
								enemy->isActive = false;
								enemiesManager->deathCount++;
							}
#endif
						}
					}
				}
			}
		}
		playerIt++;
	}
}

void StageManager::FindObjects(ObjectGroup& out_objGroup)
{
	out_objGroup.playerMap = &playerMap;
	out_objGroup.enemies = enemiesManager->enemies;
}

StageManager::~StageManager()
{
	delete enemiesManager;
	enemiesManager = nullptr;
}

