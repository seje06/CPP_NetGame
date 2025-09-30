#include <iostream>
#include <winsock2.h>  
#include <ws2tcpip.h>
#include <thread>
#include <atomic>
#include "MyGamePCH.h"
#include "GameMaker.h"
#include "NetworkManager.h"
#include "NetGameBuildInfo.h"
#include "LoginManager.h"
#include "GameTime.h"
#include <cstddef>
#include "Enums.h"
#include "SceneRelation/EndSceneRelation/EndSceneManager.h"
#include "GameManager.h"
#include <memory>
#pragma comment(lib, "ws2_32.lib")  // ��ũ ����

#define EndMain(log)  {std::cout <<"main : " << __LINE__ << " line, Log : "<< log <<endl; WSACleanup(); return 0;}

using namespace std;

int main()
{
    system("mode con cols=130 lines=30");

    WSADATA wsaData;  // ���ÿ� ����ü ����
    int res_wsaStart = WSAStartup(MAKEWORD(2, 2), &wsaData);  // ������ ���� DLL ��� ����
    //���н� 
    if(res_wsaStart != 0) EndMain("WSAStartup failed!");

    UDPSocketPtr udpSocket = SocketUtil::CreateUDPSocket(SocketAddressFamily::INET); //UDP���� ����
    if (udpSocket == nullptr) EndMain("");
    udpSocket->SetNonBlockingMode(true); // ����ŷ���� ����
#if SERVER
    SocketAddressPtr socketAdressPtr = SocketAddressFactory::CreateIPv4FromString(string("127.0.0.1:1111"));
#else
    SocketAddressPtr socketAdressPtr = SocketAddressFactory::CreateIPv4FromString(string("127.0.0.1:1111"));
#endif
    if (socketAdressPtr == nullptr) EndMain("");
#if SERVER
    //���������ּ� ������ ���Ͽ� ���ε�
    if (udpSocket->Bind(*socketAdressPtr) != NO_ERROR) EndMain("");
#endif
    //�����ͼ���
    char packetMem[1500]; //�����Ͱ� �޾��� ����
    int packetSize = sizeof(packetMem);
    SocketAddress fromAddress; //����� �������� �ּ�
 

#if SERVER
#else //Ŭ���̾�Ʈ �϶�
    NetworkClientManager::GetInstance()->SendLoginPacket(Login(), udpSocket.get(), socketAdressPtr.get());
#endif
    //���Ͽ��� ������ �ޱ�
    while (true)
    {
#if SERVER
#else //Ŭ���̾�Ʈ �϶�
        if (NetworkClientManager::GetInstance()->gameManager && NetworkClientManager::GetInstance()->gameManager->id == SCENE_ID::End)
        {
            auto endScene = dynamic_cast<EndSceneManager*>(NetworkClientManager::GetInstance()->gameManager->sceneManagers[(int)ESceneType::End]);
            if (endScene->timer > 5) break;
        }

        if (NetworkClientManager::GetInstance()->recentSendTime >= 0)
        {
            float timeDifs = (int)GameTime::GetMSTimeOfPC() - (int)NetworkClientManager::GetInstance()->recentSendTime;
            if (timeDifs < -5000) timeDifs = timeDifs + 10000; //�̻��� ���� ������ ����
            if (timeDifs >= 10)
            {
                NetworkClientManager::GetInstance()->ReSendPacket(0, udpSocket.get(), socketAdressPtr.get()); //0.1�ʸ��� ��Ŷ�� �ٽú���
            }
        }
#endif
        
        int readByteCount = udpSocket->ReceiveFrom(packetMem, packetSize, fromAddress);
        if (readByteCount == 0 || readByteCount == -WSAECONNRESET) continue;
        else if (readByteCount < 0) EndMain("");

        
        //��Ŷ ó��
#if SERVER    
        NetworkServerManager::GetInstance()->ProcessPacket(packetMem, udpSocket.get(), &fromAddress);
#else //Ŭ���̾�Ʈ �϶�
        NetworkClientManager::GetInstance()->ProcessPacket(packetMem, udpSocket.get(), &fromAddress);

#endif
     
    }

    GameMaker::canRunning = false; //������ �����ϱ� ���� false����
    for (auto& t : GameMaker::gameThreadVec) 
    {
        if (t.joinable()) t.join();   // ������ ����
    }

    EndMain("");
}