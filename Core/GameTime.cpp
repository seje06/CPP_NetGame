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

    // ��ü �ʿ��� �����θ� ����
    uint64_t totalSec = static_cast<uint64_t>(seconds);

    // ������ 2�ڸ� (0~99)
    uint16_t secPart = totalSec % 100;

    // �Ҽ��� �� �ڸ� (0~99)
    double fractional = seconds - static_cast<double>(totalSec);
    uint16_t fracPart = static_cast<uint16_t>(fractional * 100);

    // �� ���� �̾����
    uint16_t result = secPart * 100 + fracPart;

    return result;
}