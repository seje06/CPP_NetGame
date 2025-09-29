#include "Obj.h"
#include"GameTime.h"
#include"MapManager.h"

void Obj::ProcessGravity( int objHeight, int weight, float _deltaTime)
{
	int preY = pos.y;
	int nextY;


	//점핑 상태가 아닐시, obj 발밑 체크 후 상태 수정 
	if (MapManager::GetInstance()->map[pos.y + objHeight][(int)pos.x] != 1 && (!jumpInfo.isJumpUp && !jumpInfo.isJumpDown))
	{
		jumpInfo.isJumpDown = true;
		jumpInfo.isJumpUp = false;
		jumpInfo.startJumpY = preY;
	}

	//점핑 상태일때 처리
	if (jumpInfo.isJumpUp || jumpInfo.isJumpDown)
	{
		jumpInfo.time += _deltaTime * weight;
		float freeFallDis = -0.5 * G * jumpInfo.time * jumpInfo.time;
		float jumpDis = Vo * jumpInfo.time;

		//위로점프시
		if (jumpInfo.isJumpUp)
		{
			nextY = jumpInfo.startJumpY - (jumpDis + freeFallDis);

			//낙하로 전환시
			if (nextY > preY)
			{
				jumpInfo.isJumpUp = false;
				jumpInfo.isJumpDown = true;
				jumpInfo.startJumpY = preY;
				jumpInfo.time = 0;
				freeFallDis = -0.5 * G * jumpInfo.time * jumpInfo.time;
			}
			else
			{

			}
		}
		//추락시
		if (jumpInfo.isJumpDown)
		{
			nextY = jumpInfo.startJumpY - (freeFallDis);
			//player->pos.y = (int)nextY;

			//발밑 땅 상세 처리
			int yDif = nextY - preY;
			for (int i = 0; i < 1; i++)
			{
				for (int h = 0; h < yDif; h++)
				{

					if (MapManager::GetInstance()->map[preY + h + objHeight][(int)pos.x + i] == 1)
					{
						jumpInfo.isJumpUp = false;
						jumpInfo.isJumpDown = false;
						jumpInfo.time = 0;
						nextY = preY + h;
						//player->pos.y = (int)nextY;
						break;
					}

				}
			}
		}

		pos.y = (int)nextY;
	}
}