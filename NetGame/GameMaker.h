#pragma once
#include <vector>
#include <thread>
#include <memory>
#include <atomic>

class GameManager;
class GameTime;

class GameMaker
{
public:
    static std::shared_ptr<GameManager> MakeGame();
private:
    static void ProcessGame(std::shared_ptr<GameManager> gameManager, GameTime* time);

public:
    static std::vector<std::thread> gameThreadVec;
    static std::atomic<bool> canRunning;
};
