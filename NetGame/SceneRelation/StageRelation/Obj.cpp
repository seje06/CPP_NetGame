#include "Obj.h"
#include"GameTime.h"
#include"MapManager.h"

void Obj::ProcessGravity( int objHeight, int weight, float _deltaTime)
{
	int preY = pos.y;
	int nextY;


	//���� ���°� �ƴҽ�, obj �߹� üũ �� ���� ���� 
	if (MapManager::GetInstance()->map[pos.y + objHeight][(int)pos.x] != 1 && (!jumpInfo.isJumpUp && !jumpInfo.isJumpDown))
	{
		jumpInfo.isJumpDown = true;
		jumpInfo.isJumpUp = false;
		jumpInfo.startJumpY = preY;
	}

	//���� �����϶� ó��
	if (jumpInfo.isJumpUp || jumpInfo.isJumpDown)
	{
		jumpInfo.time += _deltaTime * weight;
		float freeFallDis = -0.5 * G * jumpInfo.time * jumpInfo.time;
		float jumpDis = Vo * jumpInfo.time;

		//����������
		if (jumpInfo.isJumpUp)
		{
			nextY = jumpInfo.startJumpY - (jumpDis + freeFallDis);

			//���Ϸ� ��ȯ��
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
		//�߶���
		if (jumpInfo.isJumpDown)
		{
			nextY = jumpInfo.startJumpY - (freeFallDis);
			//player->pos.y = (int)nextY;

			//�߹� �� �� ó��
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