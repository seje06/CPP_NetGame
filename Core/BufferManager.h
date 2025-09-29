#pragma once
#include<Windows.h>
#include"Utill.h"

#define BufferWidth 160	
#define BufferHeight 160	

class BufferManager:public Singletone<BufferManager>
{
private:
	HANDLE hBuffer[2];
	int screenIndex;
public:
	void InitBuffer();
	void WriteBuffer(int x, int y, const char* shape, int color);
	void ReleaseBuffer();
	void FlipBuffer();
	void ClearBuffer();
};
//BufferManager* BufferManager::instance = nullptr;