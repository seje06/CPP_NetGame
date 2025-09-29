#include "EnemiesManager.h"
#include "GameTime.h"
#include "map"
#include "INetObj.h"
#include <cmath>
#include "../Player/Player.h"
#include "Enemy.h"

using namespace std;

void EnemiesManager::Spawn(float _deltaTime)
{
	spawnTime += _deltaTime;
	if (spawnTime >= 2)
	{
		spawnTime = 0;
		for (int i = 0; i < ENEMY_COUNT; i++)
		{
			if (!enemies[i]->isActive)
			{
				int random = rand() % 2;
				enemies[i]->pos.x = random == 0 ? 1 : 29;

				enemies[i]->isActive = true;
				switch (enemies[i]->type)
				{
				case ENEMY_TYPE::EASY:
					enemies[i]->color = COLOR::WHITE;
					enemies[i]->speed = 2;
					enemies[i]->hp = 3;
					break;
				case ENEMY_TYPE::GENERAL:
					enemies[i]->color = COLOR::YELLOW;
					enemies[i]->speed = 4;
					enemies[i]->hp = 4;
					break;
				case ENEMY_TYPE::HARD:
					enemies[i]->color = COLOR::BLUE;
					enemies[i]->speed = 6;
					enemies[i]->hp = 5;
					break;
				case ENEMY_TYPE::FREAKISH:
					enemies[i]->color = COLOR::BROWN;
					enemies[i]->speed = 4;
					enemies[i]->hp = 4;
					break;
				case ENEMY_TYPE::HARD_FREAKISH:
					enemies[i]->color = COLOR::RED;
					enemies[i]->speed = 6;
					enemies[i]->hp = 5;
					break;
				}

				break;
			}
		}
	}
}

void EnemiesManager::Controll(map<unsigned long, shared_ptr<Player>>& playerMap, float _deltaTime)
{
	for (int i = 0; i < ENEMY_COUNT; i++)
	{
		if (enemies[i]->isActive)
		{
			Player* player = GetNearestPlayer(playerMap, enemies[i].get());
			if (!player) continue;
			// x贸府
			float xDif = (player->pos.x - enemies[i]->pos.x);

			if (xDif > 0)
			{
				enemies[i]->dir = (int)DIR::RIGHT;
				enemies[i]->pos.x += _deltaTime * enemies[i]->speed;
				if (enemies[i]->pos.x == (int)enemies[i]->pos.x)
					enemies[i]->aniIndex = enemies[i]->aniIndex + 1 >= ENEMY_ANI_LENGTH ? 0 : enemies[i]->aniIndex + 1;

			}
			else if (xDif < 0)
			{
				enemies[i]->dir = (int)DIR::LEFT;
				enemies[i]->pos.x -= _deltaTime * enemies[i]->speed;
				if (enemies[i]->pos.x == (int)enemies[i]->pos.x)
					enemies[i]->aniIndex = enemies[i]->aniIndex + 1 >= ENEMY_ANI_LENGTH ? 0 : enemies[i]->aniIndex + 1;
			}

			if ((!enemies[i]->jumpInfo.isJumpUp && !enemies[i]->jumpInfo.isJumpDown))
			{
				int random = 0;
				//jump贸府
				switch (enemies[i]->type)
				{
				case ENEMY_TYPE::EASY:
					break;
				case ENEMY_TYPE::GENERAL:
					break;
				case ENEMY_TYPE::HARD:
					break;
				case ENEMY_TYPE::FREAKISH:
					random = rand() % 2;
					if (random == 1)
					{
						enemies[i]->jumpInfo.isJumpUp = true;
						enemies[i]->jumpInfo.isJumpDown = false;
						enemies[i]->jumpInfo.startJumpY = enemies[i]->pos.y;
					}
					break;
				case ENEMY_TYPE::HARD_FREAKISH:
					random = rand() % 2;
					if (random == 1)
					{
						enemies[i]->jumpInfo.isJumpUp = true;
						enemies[i]->jumpInfo.isJumpDown = false;
						enemies[i]->jumpInfo.startJumpY = enemies[i]->pos.y;
					}
					break;
				}
			}
			enemies[i]->ProcessGravity(ENEMY_HEIGHT, 3, _deltaTime);
		}
	}
}

Player* EnemiesManager::GetNearestPlayer(map<unsigned long, shared_ptr<Player>>& playerMap, Enemy* enemy)
{
	Player* res_player = nullptr;

	auto playerIt = playerMap.begin();
	while (playerIt != playerMap.end())
	{
		if (playerIt->second->isDie)
		{

		}
		else if (res_player != nullptr)
		{
			auto iNetObj = playerIt->second.get();
			Player* player = dynamic_cast<Player*>(iNetObj);
			if (fabs(res_player->pos.x - enemy->pos.x) > fabs(player->pos.x - enemy->pos.x)) res_player = player;
		}
		else
		{
			auto iNetObj = playerIt->second.get();
			res_player = dynamic_cast<Player*>(iNetObj);
		}
		playerIt++;
	}

	return res_player;
}

void EnemiesManager::Init()
{
	for (int i = 0; i < ENEMY_COUNT;i++)
	{
		enemies[i] = make_shared<Enemy>();

		enemies[i]->isActive = false;
		
		enemies[i]->Init(0, 3, false, false, ENEMY_TYPE::EASY);

		enemies[i]->id = i;
	}
}

bool EnemiesManager::IsAllDie()
{
	for (int i = 0; i < ENEMY_COUNT; i++)
	{
		if (enemies[i]->isActive) return false;
	}
	return true;
}
