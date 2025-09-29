#pragma once
#include"Utill.h"

#define SCENE_MAXIMUM 3

class SceneManager
{
public:
	virtual void Init() {}
	virtual void Progress(float _deltaTime) {}
	virtual void Render() {}

	virtual ~SceneManager() = default;
};
