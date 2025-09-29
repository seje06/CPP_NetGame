#pragma once
//#include"Enums.h"
#include"../SceneManager.h";


class LogoScene: public SceneManager
{
public:
	LogoScene();
	virtual void Init() override;
	virtual void Progress(float _deltaTime) override;
	virtual void Render() override;
};
