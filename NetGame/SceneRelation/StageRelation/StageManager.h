#pragma once
//#include"GameManager.h"
//#include"BufferManager.h"
//#include"MapManager.h"
//#include"Character/Player/Player.h"
//#include"EnemiesManager.h"
#include "../SceneManager.h"
#include "IManageable.h"
#include <map>
#include <memory>

class StageManager:public SceneManager, public IManageable
{
private:
	//Player* player;
	int currentStage;
	
	std::map<unsigned long, std::shared_ptr<class Player>> playerMap;
	class EnemiesManager* enemiesManager = nullptr;

	bool isPartialClear;
	int playerDeathCount = 0;
public:
	StageManager();
	virtual void Init() override;
	virtual void Progress(float _deltaTime) override;
	virtual void Render() override;
	void StageLevelUp();

	void SensePlayerHit(float _deltaTime);
	void SenseEnemyHit(float _deltaTime);


	virtual void FindObjects(class ObjectGroup& out_objGroup) override;

	~StageManager();
};

