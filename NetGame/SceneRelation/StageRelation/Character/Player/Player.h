#pragma once
#include"../Character.h"
#include "../../Gun.h"
#include "INetObj.h"

#define PLAYER_ANI_LENGTH 2
#define PLAYER_HEIGHT 3
#define PLAYER_MAX_HP 10


class Player : public Character, public INetObj
{
private:
	struct InputInfo
	{
		bool isPressedLeft = false;
		bool isPressedRight = false;
		bool isPressedSpace = false;
		bool isPressedLeftCon = false;
	};

public:
	ULONG id = 0;
	Gun* gun = nullptr;

	bool isMovingL = false; //replicated
	bool isMovingR = false; //replicated

	InputInfo inputInfo;

	void Init(float hitTime, int hp, bool isDie, bool isHitting)
	{
		const char*** shape[2];
		shape[0] = new const char**[PLAYER_ANI_LENGTH];
		shape[1] = new const char** [PLAYER_ANI_LENGTH];
		for (int i = 0; i < PLAYER_ANI_LENGTH; i++)
		{
			shape[0][i] = new const char* [PLAYER_HEIGHT];
			shape[1][i] = new const char* [PLAYER_HEIGHT];
		}

		shape[0][0][0] = "〣";
		shape[0][0][1] = "自";
		shape[0][0][2] = "瓦";
		shape[0][1][0] = "〣";
		shape[0][1][1] = "自";
		shape[0][1][2] = "早";
		shape[1][0][0] = "〢";
		shape[1][0][1] = "肌";
		shape[1][0][2] = "瓦";
		shape[1][1][0] = "〢";
		shape[1][1][1] = "肌";
		shape[1][1][2] = "早";

		//Pos pos;
		pos.x = 10; pos.y = 25;
		Obj::Init(shape, pos, COLOR::GREEN, 10);

		Character::Init(hitTime,hp, isDie,isHitting);
		gun = new Gun(pos, dir, speed, PLAYER_HEIGHT, 1);
	}

	virtual ~Player() override
	{
		for (int i = 0; i < 2; i++)
		{
			if (shape[i] != nullptr)
			{
				for (int j = 0; j < PLAYER_ANI_LENGTH; j++)
				{
					if (shape[i][j] != nullptr)
					{
						delete[] shape[i][j];
						shape[i][j] = nullptr;
					}

				}
				delete[] shape[i];
				shape[i] = nullptr;
			}
		}
	}

	void Controll(float _deltaTime);
	void CheckPlayerInput(float _deltaTime);

	void ChangeMoveState(bool isL, bool value); //Player - RPC

	void MoveL(float _deltaTime);
	void MoveR(float _deltaTime);
	void Jump(); //Player - RPC, Server - Multicast

	virtual void ReadReplicatedData(class InputMemoryStream* inputStrm, uint16_t time) override;
	virtual void WirteReplicatedData(class OutputMemoryStream* outputStrm) override;
	virtual void ReadRPCData(class InputMemoryStream* inputStrm) override;
	virtual void WirteRPCData(class OutputMemoryStream* outputStrm, std::string funcName, void* paramData = nullptr, int paramSize = 0) override;
	virtual void NetMulticast(std::string funcName, void* paramData = nullptr, int paramSize = 0) override;
};

