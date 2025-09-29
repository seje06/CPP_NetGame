#pragma once
#include <typeinfo>
#include <mutex>

template<typename T>
class Singletone
{
protected:
	static T* instance;
public:
	static T* GetInstance()
	{
		static std::once_flag flag;
		std::call_once(flag, [] { instance = new T(); });

		return instance;
	}
protected:
	Singletone() = default;
};

template<typename T>
T* Singletone<T>::instance = nullptr;

template<typename T>
using Action = T(*)();