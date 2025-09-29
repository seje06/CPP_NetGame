#include "LoginManager.h"

unsigned long Login()
{
	std::cout << "id를 입력 해 주세요. 형식 : 0~4294967295" << std::endl;

	unsigned long playerId;
	std::cin >> playerId;

	return playerId;
}
