#include "LoginManager.h"

unsigned long Login()
{
	std::cout << "id�� �Է� �� �ּ���. ���� : 0~4294967295" << std::endl;

	unsigned long playerId;
	std::cin >> playerId;

	return playerId;
}
