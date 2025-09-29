#pragma once
#include "Enums.h"
#include "NetworkEnums.h"
#include <unordered_map>
#include <vector>
#include <memory>
#include "Utill.h"
#include <cstdint>
#include <stddef.h>
#include <shared_mutex>
#define LOGO_ACCEPTABLE_MAX_COUNT 3


typedef unsigned long PlayerID;
typedef unsigned long GameID;

typedef SCENE_ID ESceneType;

class INetObj;
class GameManager;
class OutputMemoryStream;
class InputMemoryStream;
class UDPSocket;
class SocketAddress;
class Player;



class NetworkManager
{
    
protected:
    // client -> server //
    // server -> client //
    struct NetReliableHeader
    {
        unsigned long    playerId;                      //플레이어 식별 번호
        uint8_t   packetNum;                            //패킷 식별 번호 0~255;
        uint8_t   timeSteb;                             //0=first, 1=second, 2=third, 3=end.   서버가 클라와의 pc시간차이를 예측하기 위함.
        uint16_t  time;                                 //클라의pc시간 0~9999. 천의 자리는 10의자리초.     timeSteb : 0
        int  timeDifs;                                  //서버가 클라에게 보내는 하프핑 포함 시간차이. timeSteb : 1
        int  timeDifc;                                  //클라가 서버에게 보내는 예측된 시간차이. 이후 서버가 평균화된 시간차이를 보낸다.  timeSteb : 2, 3
        uint8_t playerCount = 0;                        //해당 게임 안의 플레이어 수
        ESceneType sceneType = ESceneType::LOGO;        //현재 씬
        EGameEndType gameEndType = EGameEndType::None;  //게임 끝났을때 타입. none은 안끝났단 뜻. 서버가 클라한테 보냄
    };

public:

    virtual void ReSendPacket(PlayerID _id, class UDPSocket* _udpSocket, class SocketAddress* _socketAdr) abstract;
    void ProcessPacket(char* inBuffer, class UDPSocket* inUdpSocket,class SocketAddress* inSocketAdr);
    virtual ~NetworkManager() = default;
protected:

    virtual void ProcessOnInitReliable(EPacketType inPacketType, class UDPSocket* inUdpSocket, class SocketAddress* inSocketAdr,
         uint16_t packetSize, OutputMemoryStream& outputStrm, InputMemoryStream& inputStrm) abstract;
    virtual void ProcessOnReliable(EPacketType inPacketType, class UDPSocket* inUdpSocket, class SocketAddress* inSocketAdr,
         uint16_t packetSize, OutputMemoryStream& outputStrm, InputMemoryStream& inputStrm) abstract;
    virtual void ProcessOnUnReliable(EPacketType inPacketType, class UDPSocket* inUdpSocket, class SocketAddress* inSocketAdr,
         uint16_t packetSize, OutputMemoryStream& outputStrm, InputMemoryStream& inputStrm) abstract;

public:
    unsigned long currentPlayerCount = 0;
    // to do 스테이지용 아웃스트림 만들기

protected:

    
};  


class NetworkServerManager :public Singletone<NetworkServerManager>, public NetworkManager
{
public:
    struct GameInfo
    {
        std::shared_ptr<GameManager> gameManager;
        PlayerID playerIdArray[LOGO_ACCEPTABLE_MAX_COUNT];
        uint8_t currentPlayerCountInGame = 0;
        EGameEndType gameEndType = EGameEndType::None;
        uint8_t endPlayerCount = 0;
    };
    struct PlayerInfo_s
    {
        PlayerInfo_s();
        ~PlayerInfo_s();
        ESceneType sceneType = ESceneType::LOGO;
        uint8_t currentPacketNum = 0;                 //받아야 할 패킷 번호보다 1작은 번호. 패킷을 보내는 경우 +2하여 보낸다. 0~255값
        uint16_t updatedTDCount = 0;                  //예측된 컴터 시간차이가 갱신된 횟수. 65000이 넘으면. 갱신을 그만둔다.
        float currentTimeDif = 0;                     //클라와 서버의 예측된 컴퓨터 시간차이. timeSteb : 2 를 받을 때마다 갱신될것.
        void* ackBuffer;             //현재 패킷 번호보다 1큰 값이 패킷에 들어올 경우, 해당 버퍼로 패킷을 다시 보낸다.
        std::shared_ptr<Player> playerCharac = nullptr;   //로고씬에선 null이고 인게임에선 생성한걸 여기 넣어준다
        SocketAddress* sockAddr;
        unsigned long gameId = 0;
        OutputMemoryStream* stageDataOutputStrm;
        std::mutex m;
    };
public:
    virtual void ReSendPacket(PlayerID _id,class UDPSocket* _udpSocket, class SocketAddress* _socketAdr) override;
protected:
    virtual void ProcessOnInitReliable(EPacketType inPacketType, class UDPSocket* inUdpSocket, class SocketAddress* inSocketAdr,
       uint16_t packetSize, OutputMemoryStream& outputStrm, InputMemoryStream& inputStrm) override;
    virtual void ProcessOnReliable(EPacketType inPacketType, class UDPSocket* inUdpSocket, class SocketAddress* inSocketAdr,
         uint16_t packetSize, OutputMemoryStream& outputStrm, InputMemoryStream& inputStrm) override;
    virtual void ProcessOnUnReliable(EPacketType inPacketType, class UDPSocket* inUdpSocket, class SocketAddress* inSocketAdr,
         uint16_t packetSize, OutputMemoryStream& outputStrm, InputMemoryStream& inputStrm) override;
public:
    std::unordered_map<PlayerID, PlayerInfo_s> playerInfoMap;
    std::unordered_map<GameID, GameInfo> gameInfoMap;
private:
    unsigned long currentGameCount = 0;

    

    uint8_t currentPlayerCountInLogo = 0;
};


class NetworkClientManager :public Singletone<NetworkClientManager>, public NetworkManager
{
public:
    NetworkClientManager();

    void SendLoginPacket(unsigned long inPlayerId, UDPSocket* inUdpSocket, SocketAddress* inSocketAdr);

    virtual void ReSendPacket(PlayerID _id, class UDPSocket* _udpSocket, class SocketAddress* _socketAdr) override;

    ~NetworkClientManager();
protected:
    virtual void ProcessOnInitReliable(EPacketType inPacketType, class UDPSocket* inUdpSocket, class SocketAddress* inSocketAdr,
         uint16_t packetSize, OutputMemoryStream& outputStrm, InputMemoryStream& inputStrm) override;
    virtual void ProcessOnReliable(EPacketType inPacketType, class UDPSocket* inUdpSocket, class SocketAddress* inSocketAdr,
         uint16_t packetSize, OutputMemoryStream& outputStrm, InputMemoryStream& inputStrm) override;
    virtual void ProcessOnUnReliable(EPacketType inPacketType, class UDPSocket* inUdpSocket, class SocketAddress* inSocketAdr,
         uint16_t packetSize, OutputMemoryStream& outputStrm, InputMemoryStream& inputStrm) override;

public:
    uint8_t currentPacketNum = 0;
    ESceneType currentScene = ESceneType::LOGO;
    EGameEndType gameEndType = EGameEndType::None;

    int recentSendTime=-1;
    PlayerID playerId;
    bool isFinishInit = false;

    OutputMemoryStream* stageDataOutputStrm;
    float currentTimeDif = 0;

    std::shared_ptr<GameManager> gameManager;
    std::mutex m;
private:
    void* recentSendedBuffer = new char[1500];
    uint16_t recentCheckedTime;
     
    
};