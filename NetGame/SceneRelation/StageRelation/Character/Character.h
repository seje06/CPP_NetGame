#pragma once
#include"../Obj.h"

class Character: public Obj
{
public:
	float hitTime = 0;
	int hp = 0;
	bool isDie = false;
	bool isHitting = false;

protected:
	void Init(float hitTime,int hp, bool isDie, bool isHitting)
	{
		this->hitTime = hitTime;
		this->hp = hp;
		this->isDie = isDie;
		this->isHitting = isHitting;
	}
public:
	virtual ~Character() override
	{
		
	}
};

