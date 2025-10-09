#pragma once
#include<Windows.h>
#include"Utill.h"
#include "Enums.h"
#include"SceneRelation/SceneManager.h"
#include "NetGameBuildInfo.h"
#include <memory>
#include <mutex>

#if SERVER
class GameManager
#else 
class GameManager : public Singletone<GameManager>
#endif
{

public:
	SCENE_ID id;
	SceneManager* sceneManagers[SCENE_MAXIMUM];
	std::mutex m;
private:
	float* deltaTime = nullptr;
	class ObjectGroup* objectGroup = nullptr;
	
public:
	GameManager()
	{
		for (int i = 0; i < SCENE_MAXIMUM; i++) sceneManagers[i] = nullptr;
	}

	void Init(SCENE_ID id, float& _deltaTime);

	/*void SetSceneManager(SceneManager* manager, SCENE_ID id)
	{
		sceneManagers[(int)id] = manager;
	}*/

	template<typename T = SceneManager>
	void SetCurrentScene(SCENE_ID id)
	{
		std::lock_guard<std::mutex> lk(m); //씬 변경중에 다른 스레드에서 사용할수 있기에 락

		if (this->id == id && sceneManagers[(int)this->id]) return;

		if (sceneManagers[(int)this->id]) delete sceneManagers[(int)this->id];
		sceneManagers[(int)this->id] = nullptr;

		this->id = id;
		if (sceneManagers[(int)id] == nullptr)
		{
			sceneManagers[(int)id] = new T();
		}
		sceneManagers[(int)id]->Init();
	}

	void ManageScene();

	class ObjectGroup* GetObjects();

	~GameManager()
	{
		for (int i = 0; i < SCENE_MAXIMUM; i++) 
		{
			if (sceneManagers[i]) delete sceneManagers[i];
			sceneManagers[i] = nullptr;
		}
		if(objectGroup) delete objectGroup;
		objectGroup = nullptr;
		deltaTime = nullptr;
		//if (deltaTime) delete deltaTime;
	}
};
//GameManager* GameManager::instance = nullptr;
