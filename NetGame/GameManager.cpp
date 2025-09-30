#include "GameManager.h"
#include "IManageable.h"
#include "SceneRelation/LogoRelation/LogoScene.h"
#include "SceneRelation/StageRelation/StageManager.h"

void GameManager::Init(SCENE_ID id, float& _deltaTime)
{
	this->id = id;
	deltaTime = &_deltaTime;
	//if (sceneManagers[(int)id] != nullptr) sceneManagers[(int)id]->Init();
}

void GameManager::ManageScene()
{
	std::lock_guard<std::mutex> lk(m); //�� �����߿� �ٸ� �����忡�� ����Ҽ� �ֱ⿡ ��

	if (deltaTime == nullptr) return;

	sceneManagers[(int)id]->Progress(*deltaTime);
#if SERVER
	return;
#endif
	sceneManagers[(int)id]->Render();
}

ObjectGroup* GameManager::GetObjects()
{
	if (objectGroup == nullptr) 
	{
		objectGroup = new ObjectGroup();
		IManageable* manageable = dynamic_cast<IManageable*>(sceneManagers[(int)id]);
		if (manageable == nullptr)
		{
			delete objectGroup;
		}

		manageable->FindObjects(*objectGroup);
	}

	return objectGroup;
}
