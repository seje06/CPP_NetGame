#pragma once
//#include"Enemy.h"
#include"Utill.h"
//#include"Time.h"
//#include"../Player/Player.h"

#include <memory>
#include <map>
#include <vector>
#include <cstdint>

#define ENEMY_COUNT 10

class INetObj;
class Player;
class Enemy;

class EnemiesManager
{
public:
	//Enemy* enemies[ENEMY_COUNT];
	std::shared_ptr<Enemy> enemies[ENEMY_COUNT];
	float spawnTime;
	int deathCount;

	void Spawn(float _deltaTime);
	void Controll(std::map<unsigned long, std::shared_ptr<Player>>& playerMap, float _deltaTime);
	Player* GetNearestPlayer(std::map<unsigned long, std::shared_ptr<Player>>& playerMap, Enemy* enemy);
	void Init();
	bool IsAllDie();
};
//EnemiesManager* EnemiesManager::instance = nullptr;
