#pragma once
#include<Windows.h>
#include"StageRelation.h"
#include <cstdint>


#define BULLET_COUNT 10

class Gun
{
public:
	struct Bullet
	{
		Pos pos;
		Pos prePos;
		int dir;
		bool isActive;
	};
	Bullet* bullets[10] = {};

	const Pos* pos;
	const uint8_t* dir;
	float speed;
	float reloadTime;
	int ownerHeight;

	Gun(Pos& ownerPos, uint8_t& ownerDir, float speed,int ownerHeight,float reloadTime);

	void Init();
	void ActiveBullet(); //Player - RPC, Server - Multicast
	void ControllBullet(float _deltaTime);

	~Gun()
	{
		for (int i = 0; i < BULLET_COUNT; i++)
		{
			if (bullets[i] != nullptr) delete bullets[i];
			bullets[i] = nullptr;
		}
	}
};

