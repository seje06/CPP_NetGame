#pragma once

#ifndef MyGame_LinkingContext_h
#define MyGame_LinkingContext_h

class Obj;

class LinkingContext
{
public:

	LinkingContext() :
		mNextNetworkId(1)
	{
	}

	uint32_t GetNetworkId(Obj* inGameObject, bool inShouldCreateIfNotFound)
	{
		auto it = mGameObjectToNetworkIdMap.find(inGameObject);
		if (it != mGameObjectToNetworkIdMap.end())
		{
			return it->second;
		}
		else if (inShouldCreateIfNotFound)
		{
			uint32_t newNetworkId = mNextNetworkId++;
			AddGameObject(inGameObject, newNetworkId);
			return newNetworkId;
		}
		else
		{
			return 0;
		}
	}

	Obj* GetGameObject(uint32_t inNetworkId) const
	{
		auto it = mNetworkIdToGameObjectMap.find(inNetworkId);
		if (it != mNetworkIdToGameObjectMap.end())
		{
			return it->second;
		}
		else
		{
			return nullptr;
		}
	}

	void AddGameObject(Obj* inGameObject, uint32_t inNetworkId)
	{
		mNetworkIdToGameObjectMap[inNetworkId] = inGameObject;
		mGameObjectToNetworkIdMap[inGameObject] = inNetworkId;
	}

	void RemoveGameObject(Obj* inGameObject)
	{
		uint32_t networkId = mGameObjectToNetworkIdMap[inGameObject];
		mGameObjectToNetworkIdMap.erase(inGameObject);
		mNetworkIdToGameObjectMap.erase(networkId);
	}

private:
	std::unordered_map< uint32_t, Obj* > mNetworkIdToGameObjectMap;
	std::unordered_map< const Obj*, uint32_t > mGameObjectToNetworkIdMap;

	uint32_t mNextNetworkId;
};

#endif