#pragma once
#include"StageRelation.h"
#include"StageEnums.h"
#include <cstdint>
//#include"MapManager.h"
//#include"Time.h"
class Obj
{
public:
	const char*** shape[2];
	JumpeInfo jumpInfo;
	Pos pos;
	float speed;
	int weight;
	uint8_t dir;
	uint8_t aniIndex;
	COLOR color;
protected:
	void Init(const char*** shape[2], Pos pos, COLOR color, float speed)
	{
		this->shape[0] = shape[0];
		this->shape[1] = shape[1];
		this->pos = pos;
		this->color = color;
		this->speed = speed;

		jumpInfo.isJumpDown = true;
		jumpInfo.isJumpUp = false;
		jumpInfo.startJumpY = pos.y;
		jumpInfo.time = 0;

		dir = (int)DIR::RIGHT;
		aniIndex = 0;
	}

public:
	virtual ~Obj()
	{

	}

	void ProcessGravity( int objHeight, int weight, float _deltaTime);
};

