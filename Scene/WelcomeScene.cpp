#include <fstream>
#include <sstream>
#include <iostream>
#include <functional>
#include "WelcomeScene.hpp"
#include "Engine/AudioHelper.hpp"
#include "player_data.h"
#include "Engine/GameEngine.hpp"
#include "Engine/Resources.hpp"
#include "UI/Component/ImageButton.hpp"
#include "UI/Component/Label.hpp"
#include "StartScene.h"

void WelcomeScene::LoadPlayerHistory(const std::string &path, const std::string &uid)
{
    filteredEntries.clear();
    std::ifstream file(path);
    if (!file)
    {
        std::cout << "Leaderboard file not found.\n";
        return;
    }

    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream ss(line);
        std::string file_uid, file_name, score_str, time;
        if (std::getline(ss, file_uid, ',') &&
            std::getline(ss, file_name, ',') &&
            std::getline(ss, score_str, ',') &&
            std::getline(ss, time))
        {
            if (file_uid == uid)
            {
                LeaderboardEntry entry;
                entry.uid = file_uid;
                entry.name = file_name;
                entry.score = std::stoi(score_str);
                entry.time = time;
                filteredEntries.push_back(entry);
            }
        }
    }
}

void WelcomeScene::OnContinueClicked()
{
    Engine::GameEngine::GetInstance().ChangeScene("start");
}

void WelcomeScene::Initialize()
{
    Engine::Point screen = Engine::GameEngine::GetInstance().GetScreenSize();
    int cx = screen.x / 2;

    bgmInstance = AudioHelper::PlaySample("select.ogg", true, AudioHelper::BGMVolume);

    LoadPlayerHistory("Data/scoreboard.txt", player_uid);

    AddNewObject(new Engine::Label(std::string("Welcome back ") + nameInput + "!", "pirulen.ttf", 48, cx, 50, 146, 161, 185, 255, 0.5, 0.5));
    AddNewObject(new Engine::Label("Take a look at your previous records", "pirulen.ttf", 24, cx, 120, 180, 180, 180, 255, 0.5, 0.5));

    float col_name = screen.x / 4;
    float col_score = screen.x / 2;
    float col_time = 3 * screen.x / 4;

    AddNewObject(new Engine::Label("Name", "pirulen.ttf", 24, col_name, 190, 255, 255, 255, 255, 0.5, 0.5));
    AddNewObject(new Engine::Label("Score", "pirulen.ttf", 24, col_score, 190, 255, 255, 255, 255, 0.5, 0.5));
    AddNewObject(new Engine::Label("Time", "pirulen.ttf", 24, col_time, 190, 255, 255, 255, 255, 0.5, 0.5));

    if (filteredEntries.empty())
    {
        AddNewObject(new Engine::Label("No Record Found", "pirulen.ttf", 24, cx, screen.y / 2, 255, 0, 0, 255, 0.5, 0.5));
    }
    else
    {
        int y = 240;
        for (const auto &entry : filteredEntries)
        {
            AddNewObject(new Engine::Label(entry.name, "pirulen.ttf", 20, col_name, y, 200, 200, 200, 255, 0.5, 0.5));
            AddNewObject(new Engine::Label(std::to_string(entry.score), "pirulen.ttf", 20, col_score, y, 200, 200, 200, 255, 0.5, 0.5));
            AddNewObject(new Engine::Label(entry.time, "pirulen.ttf", 20, col_time, y, 200, 200, 200, 255, 0.5, 0.5));
            y += 40;
        }
    }

    auto *btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", cx - 200, 650, 400, 100);
    btn->SetOnClickCallback(std::bind(&WelcomeScene::OnContinueClicked, this));
    AddNewControlObject(btn);
    AddNewObject(btn);
    AddNewObject(new Engine::Label("CONTINUE", "pirulen.ttf", 36, cx, 700, 225, 225, 225, 255, 0.5, 0.5));
}

void WelcomeScene::Draw() const
{
    IScene::Draw(); // All rendering done via AddNewObject, nothing extra to draw manually
}

void WelcomeScene::Terminate()
{
    AudioHelper::StopSample(bgmInstance);
    bgmInstance = nullptr;
}
