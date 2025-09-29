#pragma once

#include"../SceneManager.h";
#include"NetworkEnums.h"

class EndSceneManager : public SceneManager
{
public:
	float timer = 0;
	long long gameId = -1;
public:
	virtual void Init() override;
	virtual void Progress(float _deltaTime) override;
	virtual void Render() override;
};