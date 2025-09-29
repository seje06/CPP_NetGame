#pragma once
#include "Enums.h"
#include <iostream>
#include <string>
using namespace std;

#define G 9.81
#define Vo 10

struct Pos
{
	float x = 0;
	int y = 0;
};
struct JumpeInfo
{
	float time = 0;
	int startJumpY = 0;
	bool isJumpUp = false;
	bool isJumpDown = false;
};




