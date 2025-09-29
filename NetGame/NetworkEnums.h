#pragma once
#include <cstdint>

// client -> server //
// server -> client //
enum class EHeaderType : uint8_t
{
    InitReliable,
    Reliable,
    UnReliable
};

enum class EPacketType : uint8_t
{
    Server,
    Client
};

enum class EDataType : uint8_t
{
    RPC,
    Replicated
};

// server -> client //
enum class ELoginResultType : uint8_t
{
    Success,
    Failed
};

enum class EINetObjType : uint8_t
{
    Player,
    AI
};

enum class EGameEndType : uint8_t
{
    None,
    Win,
    Lose
};

