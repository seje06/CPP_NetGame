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
#pragma comment(lib, "ws2_32.lib")  // 링크 설정

#define EndMain(log)  {std::cout <<"main : " << __LINE__ << " line, Log : "<< log <<endl; WSACleanup(); return 0;}

using namespace std;

int main()
{
    system("mode con cols=130 lines=30");

    WSADATA wsaData;  // 스택에 구조체 생성
    int res_wsaStart = WSAStartup(MAKEWORD(2, 2), &wsaData);  // 윈도우 소켓 DLL 사용 시작
    //실패시 
    if(res_wsaStart != 0) EndMain("WSAStartup failed!");

    UDPSocketPtr udpSocket = SocketUtil::CreateUDPSocket(SocketAddressFamily::INET); //UDP소켓 생성
    if (udpSocket == nullptr) EndMain("");
    udpSocket->SetNonBlockingMode(true); // 논블로킹으로 설정
#if SERVER
    SocketAddressPtr socketAdressPtr = SocketAddressFactory::CreateIPv4FromString(string("127.0.0.1:1111"));
#else
    SocketAddressPtr socketAdressPtr = SocketAddressFactory::CreateIPv4FromString(string("127.0.0.1:1111"));
#endif
    if (socketAdressPtr == nullptr) EndMain("");
#if SERVER
    //서버소켓주소 생성후 소켓에 바인딩
    if (udpSocket->Bind(*socketAdressPtr) != NO_ERROR) EndMain("");
#endif
    //데이터설정
    char packetMem[1500]; //데이터가 받아질 변수
    int packetSize = sizeof(packetMem);
    SocketAddress fromAddress; //여기로 보낸곳의 주소
 

#if SERVER
#else //클라이언트 일때
    NetworkClientManager::GetInstance()->SendLoginPacket(Login(), udpSocket.get(), socketAdressPtr.get());
#endif
    //소켓에서 데이터 받기
    while (true)
    {
#if SERVER
#else //클라이언트 일때
        if (NetworkClientManager::GetInstance()->gameManager && NetworkClientManager::GetInstance()->gameManager->id == SCENE_ID::End)
        {
            auto endScene = dynamic_cast<EndSceneManager*>(NetworkClientManager::GetInstance()->gameManager->sceneManagers[(int)ESceneType::End]);
            if (endScene->timer > 5) break;
        }

        if (NetworkClientManager::GetInstance()->recentSendTime >= 0)
        {
            float timeDifs = (int)GameTime::GetMSTimeOfPC() - (int)NetworkClientManager::GetInstance()->recentSendTime;
            if (timeDifs < -5000) timeDifs = timeDifs + 10000; //이상한 값이 나오면 보정
            if (timeDifs >= 10)
            {
                NetworkClientManager::GetInstance()->ReSendPacket(0, udpSocket.get(), socketAdressPtr.get()); //0.1초마다 패킷을 다시보냄
            }
        }
#endif
        
        int readByteCount = udpSocket->ReceiveFrom(packetMem, packetSize, fromAddress);
        if (readByteCount == 0 || readByteCount == -WSAECONNRESET) continue;
        else if (readByteCount < 0) EndMain("");

        
        //패킷 처리
#if SERVER    
        NetworkServerManager::GetInstance()->ProcessPacket(packetMem, udpSocket.get(), &fromAddress);
#else //클라이언트 일때
        NetworkClientManager::GetInstance()->ProcessPacket(packetMem, udpSocket.get(), &fromAddress);

#endif
     
    }

    GameMaker::canRunning = false; //스레드 정리하기 위해 false설정
    for (auto& t : GameMaker::gameThreadVec) 
    {
        if (t.joinable()) t.join();   // 스레드 정리
    }

    EndMain("");
}