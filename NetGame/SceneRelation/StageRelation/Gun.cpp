#include "Gun.h"
#include"GameTime.h"
#include "MapManager.h"

Gun::Gun(Pos& ownerPos, uint8_t& ownerDir, float speed, int ownerHeight,float reloadTime)
{
	pos = &ownerPos;
	dir = &ownerDir;
	this->speed = speed;
	this->ownerHeight = ownerHeight;
	this->reloadTime = reloadTime;
	for (auto& bullet : bullets) bullet = nullptr;

	Init();
}

void Gun::Init()
{
	for (int i = 0; i < BULLET_COUNT; i++)
	{
		bullets[i] = new Bullet();
		bullets[i]->isActive = false;
		bullets[i]->pos = *pos;
		bullets[i]->prePos = bullets[i]->pos;
	}
}

void Gun::ActiveBullet()
{
	reloadTime = 0;
	for (int i = 0; i < BULLET_COUNT; i++)
	{
		if (!bullets[i]->isActive)
		{
			bullets[i]->isActive = true;
			//int addX = player->obj.dir == 0 ? -1 : 1;
			bullets[i]->pos.x = pos->x;
			bullets[i]->pos.y = pos->y + ownerHeight / 2;
			bullets[i]->dir = *dir;
			break;
		}
	}
}

void Gun::ControllBullet(float _deltaTime)
{
	reloadTime += _deltaTime;
	
	for (int i = 0; i < BULLET_COUNT; i++)
	{
		if (bullets[i]->isActive)
		{
			bullets[i]->prePos = bullets[i]->pos;

			if (bullets[i]->dir == 0) bullets[i]->pos.x -= speed * _deltaTime;
			else bullets[i]->pos.x += speed * _deltaTime;

			if (bullets[i]->pos.x > MAP_WIDTH || bullets[i]->pos.x < 0)  bullets[i]->isActive = false;
		}
	}
}
