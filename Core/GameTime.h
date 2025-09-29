#pragma once
#include <cstdint>

namespace SettableTime
{
	extern float globalDeltaTime;
}
extern const float& GlobalDeltaTime;

class GameTime
{
public:
	static uint16_t GetMSTimeOfPC();
public:
	float deltaTime = 0;
};