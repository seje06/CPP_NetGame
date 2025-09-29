#pragma once
#include <map>
#include <vector>
#include <memory>

class INetObj;
class Enemy;
class Player;

struct ObjectGroup
{
	std::map<unsigned long, std::shared_ptr<Player>>* playerMap = nullptr;
	std::shared_ptr<Enemy>* enemies = nullptr;
};

class IManageable
{
public:
	virtual void FindObjects(ObjectGroup& out_objGroup) abstract;

	virtual ~IManageable() = default;
};