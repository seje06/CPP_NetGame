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

//template<typename T>
//void GameManager::SetCurrentScene(SCENE_ID id)
//{
//	if(sceneManagers[(int)this->id]) delete sceneManagers[(int)this->id];
//	sceneManagers[(int)this->id] = nullptr;
//
//	this->id = id;
//	if (sceneManagers[(int)id] == nullptr)
//	{
//		sceneManagers[(int)id] = new T();
//	}
//	sceneManagers[(int)id]->Init();
//}
//template void GameManager::SetCurrentScene<LogoScene>(SCENE_ID);
//template void GameManager::SetCurrentScene<StageManager>(SCENE_ID);


void GameManager::ManageScene()
{
	std::lock_guard<std::mutex> lk(m); //씬 변경중에 다른 스레드에서 사용할수 있기에 락

	if (deltaTime == nullptr) return;
	/*if (true)
	{
		int nextId = ((int)id) + 1;
		if (SCENE_MAXIMUM <= nextId)
		{
			nextId -= 1;
		}
		else
		{
			id = (SCENE_ID)nextId;
			sceneManagers[(int)id]->Init();
		}
	}*/

	/*if (GetAsyncKeyState(VK_RETURN) & 0x8000)
	{
		int nextId = ((int)id) + 1;
		if (SCENE_MAXIMUM <= nextId)
		{
			nextId -= 1;
		}
		else
		{
			SetCurrentScene((SCENE_ID)nextId);
		}
	}*/

	sceneManagers[(int)id]->Progress(*deltaTime);
#if SERVER
	return;
#endif
	sceneManagers[(int)id]->Render();
	//switch (id)
	//{
	//case SCENE_ID::LOGO:
	//	//ProgressLogo();
	//	break;
	//case SCENE_ID::STAGE:
	//	sceneManagers[(int)id]->Progress();
	//	RenderStage();
	//	break;
	//default:
	//	break;
	//}
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
