#include "GameTime.h"
#include <chrono>
#include <ctime> 
#include <stdio.h>
#include <math.h>

using namespace std::chrono;

float SettableTime::globalDeltaTime = 0;
const float& GlobalDeltaTime = SettableTime::globalDeltaTime;

uint16_t GameTime::GetMSTimeOfPC()
{
    using namespace std::chrono;

    auto now = system_clock::now();
    double seconds = duration<double>(now.time_since_epoch()).count();

    // 전체 초에서 정수부만 추출
    uint64_t totalSec = static_cast<uint64_t>(seconds);

    // 정수부 2자리 (0~99)
    uint16_t secPart = totalSec % 100;

    // 소수부 두 자리 (0~99)
    double fractional = seconds - static_cast<double>(totalSec);
    uint16_t fracPart = static_cast<uint16_t>(fractional * 100);

    // 두 값을 이어붙임
    uint16_t result = secPart * 100 + fracPart;

    return result;
}