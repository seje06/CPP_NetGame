#pragma once
#include"../Character.h"
#include "INetObj.h"
#include <cstdint>

#define ENEMY_ANI_LENGTH 2
#define ENEMY_HEIGHT 3

class Enemy : public Character, INetObj
{
public:

	ENEMY_TYPE type;
	bool isActive;
	uint8_t id = 0;

	void Init(float hitTime, int hp, bool isDie, bool isHitting, ENEMY_TYPE type)
	{
		const char*** shape[2];
		shape[0] = new const char** [ENEMY_ANI_LENGTH];
		shape[1] = new const char** [ENEMY_ANI_LENGTH];
		for (int i = 0; i < ENEMY_ANI_LENGTH; i++)
		{
			shape[0][i] = new const char* [ENEMY_HEIGHT];
			shape[1][i] = new const char* [ENEMY_HEIGHT];
		}
		shape[0][0][0] = "¢¸";
		shape[0][0][1] = "¦À";
		shape[0][0][2] = "¥Ë";

		shape[0][1][0] = "¢¸";
		shape[0][1][1] = "¦À";
		shape[0][1][2] = "¦­";

		shape[1][0][0] = "¢º";
		shape[1][0][1] = "¦À";
		shape[1][0][2] = "¥Ë";

		shape[1][1][0] = "¢º";
		shape[1][1][1] = "¦À";
		shape[1][1][2] = "¦­";

		Pos pos;
		pos.x = 10; pos.y = 25;
		Obj::Init(shape, pos, COLOR::GREEN, 10);

		Character::Init(hitTime, hp, isDie, isHitting);
		this->type = type;
		isActive = false;
	}

	virtual ~Enemy() override
	{
		for (int i = 0; i < 2; i++)
		{
			if (shape[i] != nullptr)
			{
				for (int j = 0; j < ENEMY_ANI_LENGTH; j++)
				{
					if (shape[i][j] != nullptr)
					{
						delete[] shape[i][j];
					}
				}
				delete[] shape[i];
			}
		}
	}


	virtual void ReadReplicatedData(class InputMemoryStream* inputStrm, uint16_t time) override;
	virtual void WirteReplicatedData(class OutputMemoryStream* outputStrm) override;
	virtual void ReadRPCData(class InputMemoryStream* inputStrm) override;
	virtual void WirteRPCData(class OutputMemoryStream* outputStrm, std::string funcName, void* paramData = nullptr, int paramSize = 0) override;
	virtual void NetMulticast(std::string funcName, void* paramData = nullptr, int paramSize = 0) override;
};

