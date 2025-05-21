#include <allegro5/allegro_audio.h>
#include <fstream>
#include <functional>
#include <sstream>
#include <string>
#include <algorithm>
#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Resources.hpp"
#include "LeaderBoardScene.hpp"
#include "PlayScene.hpp"
#include "UI/Component/ImageButton.hpp"
#include "UI/Component/Label.hpp"
#include "player_data.h"

void LeaderBoardScene::Initialize() {
    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;

    LoadLeaderboard();
    SortLeaderboard();

    AddNewObject(new Engine::Label("LEADERBOARD", "pirulen.ttf", 48, halfW, h / 8 + 20, 255, 255, 255, 255, 0.5, 0.5));

    float box_x = 50;
    float box_y = h / 8 - 30;
    float box_width = w - 100;

    float col_rank = box_x + 60;
    float col_name = box_x + box_width / 3 + 20;
    float col_score = box_x + (box_width / 3) * 2 - 20;
    float col_time = box_x + box_width - 90;

    AddNewObject(new Engine::Label("Rank", "pirulen.ttf", 28, col_rank, box_y + 130, 180, 180, 180, 255, 0.5, 0.5));
    AddNewObject(new Engine::Label("Player Name", "pirulen.ttf", 28, col_name, box_y + 130, 180, 180, 180, 255, 0.5, 0.5));
    AddNewObject(new Engine::Label("Score", "pirulen.ttf", 28, col_score, box_y + 130, 180, 180, 180, 255, 0.5, 0.5));
    AddNewObject(new Engine::Label("Time", "pirulen.ttf", 28, col_time, box_y + 130, 180, 180, 180, 255, 0.5, 0.5));

    int start = currentPage * entriesPerPage;
    int end = std::min(start + entriesPerPage, (int)leaderboard.size());

    for (int i = start; i < end; i++) {
        float row_y = box_y + 200 + (i - start) * 50;
        auto color = (leaderboard[i].uid == player_uid) ? std::make_tuple(255, 223, 0) : std::make_tuple(255, 255, 255);

        AddNewObject(new Engine::Label(std::to_string(i + 1), "pirulen.ttf", 24, col_rank, row_y,
            std::get<0>(color), std::get<1>(color), std::get<2>(color), 255, 0.5, 0.5));
        AddNewObject(new Engine::Label(leaderboard[i].name, "pirulen.ttf", 24, col_name, row_y,
            std::get<0>(color), std::get<1>(color), std::get<2>(color), 255, 0.5, 0.5));
        AddNewObject(new Engine::Label(std::to_string(leaderboard[i].score), "pirulen.ttf", 24, col_score, row_y,
            std::get<0>(color), std::get<1>(color), std::get<2>(color), 255, 0.5, 0.5));
        AddNewObject(new Engine::Label(leaderboard[i].time, "pirulen.ttf", 24, col_time, row_y,
            std::get<0>(color), std::get<1>(color), std::get<2>(color), 255, 0.5, 0.5));
    }

    Engine::ImageButton* btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", halfW - 200, halfH / 2 + 350, 400, 100);
    btn->SetOnClickCallback(std::bind(&LeaderBoardScene::BackOnClick, this, 2));
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label("Back", "pirulen.ttf", 36, halfW, halfH / 2 + 400, 0, 0, 0, 255, 0.5, 0.5));
}

void LeaderBoardScene::Terminate() {
    bgmInstance = std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE>();
    IScene::Terminate();
}

void LeaderBoardScene::BackOnClick(int stage) {
    Engine::GameEngine::GetInstance().ChangeScene("start");
}

void LeaderBoardScene::Update(float deltaTime) {}

void LeaderBoardScene::LoadLeaderboard() {
    leaderboard.clear();
    std::ifstream file("Data/scoreboard.txt");
    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string uid, name, scoreStr, time;
        std::getline(ss, uid, ',');
        std::getline(ss, name, ',');
        std::getline(ss, scoreStr, ',');
        std::getline(ss, time, ',');

        LeaderboardEntry entry;
        entry.uid = uid;
        entry.name = name;
        entry.score = std::stoi(scoreStr);
        entry.time = time;
        leaderboard.push_back(entry);
    }
    file.close();
}

void LeaderBoardScene::SortLeaderboard() {
    std::sort(leaderboard.begin(), leaderboard.end(), [](const LeaderboardEntry& a, const LeaderboardEntry& b) {
        return a.score > b.score;
    });
}
