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
        unsigned long    playerId;                      //�÷��̾� �ĺ� ��ȣ
        uint8_t   packetNum;                            //��Ŷ �ĺ� ��ȣ 0~255;
        uint8_t   timeSteb;                             //0=first, 1=second, 2=third, 3=end.   ������ Ŭ����� pc�ð����̸� �����ϱ� ����.
        uint16_t  time;                                 //Ŭ����pc�ð� 0~9999. õ�� �ڸ��� 10���ڸ���.     timeSteb : 0
        int  timeDifs;                                  //������ Ŭ�󿡰� ������ ������ ���� �ð�����. timeSteb : 1
        int  timeDifc;                                  //Ŭ�� �������� ������ ������ �ð�����. ���� ������ ���ȭ�� �ð����̸� ������.  timeSteb : 2, 3
        uint8_t playerCount = 0;                        //�ش� ���� ���� �÷��̾� ��
        ESceneType sceneType = ESceneType::LOGO;        //���� ��
        EGameEndType gameEndType = EGameEndType::None;  //���� �������� Ÿ��. none�� �ȳ����� ��. ������ Ŭ������ ����
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
    // to do ���������� �ƿ���Ʈ�� �����

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
        uint8_t currentPacketNum = 0;                 //�޾ƾ� �� ��Ŷ ��ȣ���� 1���� ��ȣ. ��Ŷ�� ������ ��� +2�Ͽ� ������. 0~255��
        uint16_t updatedTDCount = 0;                  //������ ���� �ð����̰� ���ŵ� Ƚ��. 65000�� ������. ������ �׸��д�.
        float currentTimeDif = 0;                     //Ŭ��� ������ ������ ��ǻ�� �ð�����. timeSteb : 2 �� ���� ������ ���ŵɰ�.
        void* ackBuffer;             //���� ��Ŷ ��ȣ���� 1ū ���� ��Ŷ�� ���� ���, �ش� ���۷� ��Ŷ�� �ٽ� ������.
        std::shared_ptr<Player> playerCharac = nullptr;   //�ΰ������ null�̰� �ΰ��ӿ��� �����Ѱ� ���� �־��ش�
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