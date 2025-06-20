#include <algorithm>
#include <iostream>
#include <allegro5/allegro.h>
#include <cmath>
#include <fstream>
#include <functional>
#include <memory>
#include <queue>
#include <string>
#include <vector>

#include "Enemy/Enemy.hpp"
#include "Enemy/SoldierEnemy.hpp"
#include "Enemy/TankEnemy.hpp"
#include "Enemy/SuperTankEnemy.hpp"
#include "Enemy/ExtremeTankEnemy.hpp"
#include "Enemy/PlaneEnemy.hpp"
#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Group.hpp"
#include "Engine/LOG.hpp"
#include "Engine/Resources.hpp"
#include "Engine/Collider.hpp"
#include "PlayScene.hpp"
#include "Turret/Bom.hpp"
#include "Turret/LaserTurret.hpp"
#include "Turret/MachineGunTurret.hpp"
#include "Turret/TurretButton.hpp"
#include "UI/Animation/DirtyEffect.hpp"
#include "UI/Animation/Plane.hpp"
#include "UI/Component/Label.hpp"
#include "player_data.h"
#include "UI/Animation/ExplosionEffect.hpp"
#include "Turret/Shovel.hpp"

bool PlayScene::DebugMode = false;
const std::vector<Engine::Point> PlayScene::directions = {Engine::Point(-1, 0), Engine::Point(0, -1), Engine::Point(1, 0), Engine::Point(0, 1)};
const int PlayScene::MapWidth = 20, PlayScene::MapHeight = 13;
const int PlayScene::BlockSize = 64;
const float PlayScene::DangerTime = 7.61;
const Engine::Point PlayScene::SpawnGridPoint = Engine::Point(-1, 0);
const Engine::Point PlayScene::EndGridPoint = Engine::Point(MapWidth, MapHeight - 1);
const std::vector<int> PlayScene::code = {
    ALLEGRO_KEY_UP, ALLEGRO_KEY_UP, ALLEGRO_KEY_DOWN, ALLEGRO_KEY_DOWN,
    ALLEGRO_KEY_LEFT, ALLEGRO_KEY_RIGHT, ALLEGRO_KEY_LEFT, ALLEGRO_KEY_RIGHT,
    ALLEGRO_KEY_B, ALLEGRO_KEY_A, ALLEGRO_KEY_LSHIFT, ALLEGRO_KEY_ENTER};
Engine::Point PlayScene::GetClientSize()
{
    return Engine::Point(MapWidth * BlockSize, MapHeight * BlockSize);
}
void PlayScene::Initialize()
{
    mapState.clear();
    keyStrokes.clear();
    ticks = 0;
    deathCountDown = -1;
    lives = 10;
    money = 200;
    SpeedMult = 1;
    // Add groups from bottom to top.
    AddNewObject(TileMapGroup = new Group());
    AddNewObject(GroundEffectGroup = new Group());
    AddNewObject(DebugIndicatorGroup = new Group());
    AddNewObject(TowerGroup = new Group());
    AddNewObject(EnemyGroup = new Group());
    AddNewObject(BulletGroup = new Group());
    AddNewObject(EffectGroup = new Group());
    // Should support buttons.
    AddNewControlObject(UIGroup = new Group());
    ReadMap();
    ReadEnemyWave();
    mapDistance = CalculateBFSDistance();
    ConstructUI();
    imgTarget = new Engine::Image("play/target.png", 0, 0);
    imgTarget->Visible = false;
    preview = nullptr;
    UIGroup->AddNewObject(imgTarget);
    // Preload Lose Scene
    deathBGMInstance = Engine::Resources::GetInstance().GetSampleInstance("astronomia.ogg");
    Engine::Resources::GetInstance().GetBitmap("lose/benjamin-happy.png");
    // Start BGM.
    bgmId = AudioHelper::PlayBGM("play.ogg");
}
void PlayScene::Terminate()
{
    AudioHelper::StopBGM(bgmId);
    AudioHelper::StopSample(deathBGMInstance);
    deathBGMInstance = std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE>();
    IScene::Terminate();
}
void PlayScene::Update(float deltaTime)
{
    matchTime += deltaTime;
    // If we use deltaTime directly, then we might have Bullet-through-paper problem.
    // Reference: Bullet-Through-Paper
    if (SpeedMult == 0)
        deathCountDown = -1;
    else if (deathCountDown != -1)
        SpeedMult = 1;
    // Calculate danger zone.
    std::vector<float> reachEndTimes;
    for (auto &it : EnemyGroup->GetObjects())
    {
        reachEndTimes.push_back(dynamic_cast<Enemy *>(it)->reachEndTime);
    }
    // Can use Heap / Priority-Queue instead. But since we won't have too many enemies, sorting is fast enough.
    std::sort(reachEndTimes.begin(), reachEndTimes.end());
    float newDeathCountDown = -1;
    int danger = lives;
    for (auto &it : reachEndTimes)
    {
        if (it <= DangerTime)
        {
            danger--;
            if (danger <= 0)
            {
                // Death Countdown
                float pos = DangerTime - it;
                if (it > deathCountDown)
                {
                    // Restart Death Count Down BGM.
                    AudioHelper::StopSample(deathBGMInstance);
                    if (SpeedMult != 0)
                        deathBGMInstance = AudioHelper::PlaySample("astronomia.ogg", false, AudioHelper::BGMVolume, pos);
                }
                float alpha = pos / DangerTime;
                alpha = std::max(0, std::min(255, static_cast<int>(alpha * alpha * 255)));
                dangerIndicator->Tint = al_map_rgba(255, 255, 255, alpha);
                newDeathCountDown = it;
                break;
            }
        }
    }
    deathCountDown = newDeathCountDown;
    if (SpeedMult == 0)
        AudioHelper::StopSample(deathBGMInstance);
    if (deathCountDown == -1 && lives > 0)
    {
        AudioHelper::StopSample(deathBGMInstance);
        dangerIndicator->Tint.a = 0;
    }
    if (SpeedMult == 0)
        deathCountDown = -1;
    for (int i = 0; i < SpeedMult; i++)
    {
        IScene::Update(deltaTime);
        // Check if we should create new enemy.
        ticks += deltaTime;
        if (enemyWaveData.empty())
        {
            if (EnemyGroup->GetObjects().empty())
            {
                // Free resources.
                /*delete TileMapGroup;
                delete GroundEffectGroup;
                delete DebugIndicatorGroup;
                delete TowerGroup;
                delete EnemyGroup;
                delete BulletGroup;
                delete EffectGroup;
                delete UIGroup;
                delete imgTarget;*/
                // Win.
                Engine::GameEngine::GetInstance().ChangeScene("win-scene");
                return;
            }
            continue;
        }
        auto current = enemyWaveData.front();
        if (ticks < current.second)
            continue;
        ticks -= current.second;
        enemyWaveData.pop_front();
        const Engine::Point SpawnCoordinate = Engine::Point(SpawnGridPoint.x * BlockSize + BlockSize / 2, SpawnGridPoint.y * BlockSize + BlockSize / 2);
        Enemy *enemy;
        switch (current.first)
        {
        case 1:
            EnemyGroup->AddNewObject(enemy = new SoldierEnemy(SpawnCoordinate.x, SpawnCoordinate.y));
            break;
        case 2:
            EnemyGroup->AddNewObject(enemy = new PlaneEnemy(SpawnCoordinate.x, SpawnCoordinate.y));
            break;
        case 3:
            EnemyGroup->AddNewObject(enemy = new TankEnemy(SpawnCoordinate.x, SpawnCoordinate.y));
            break;
        case 4:
            EnemyGroup->AddNewObject(enemy = new SuperTankEnemy(SpawnCoordinate.x, SpawnCoordinate.y));
            break;
        case 5:
            EnemyGroup->AddNewObject(enemy = new ExtremeTankEnemy(SpawnCoordinate.x, SpawnCoordinate.y));
            break;
        default:
            continue;
        }
        enemy->UpdatePath(mapDistance);
        // Compensate the time lost.
        enemy->Update(ticks);
    }
    if (preview)
    {
        preview->Position = Engine::GameEngine::GetInstance().GetMousePosition();
        // To keep responding when paused.
        preview->Update(deltaTime);
    }
}
void PlayScene::Draw() const
{
    IScene::Draw();
    if (DebugMode)
    {
        // Draw reverse BFS distance on all reachable blocks.
        for (int i = 0; i < MapHeight; i++)
        {
            for (int j = 0; j < MapWidth; j++)
            {
                if (mapDistance[i][j] != -1)
                {
                    Engine::Label label(std::to_string(mapDistance[i][j]), "pirulen.ttf", 32, (j + 0.5) * BlockSize, (i + 0.5) * BlockSize);
                    label.Anchor = Engine::Point(0.5, 0.5);
                    label.Draw();
                }
            }
        }
    }
}
void PlayScene::OnMouseDown(int button, int mx, int my)
{
    if ((button & 1) && !imgTarget->Visible && preview)
    {
        UIGroup->RemoveObject(preview->GetObjectIterator());
        preview = nullptr;
    }
    IScene::OnMouseDown(button, mx, my);
}
void PlayScene::OnMouseMove(int mx, int my)
{
    IScene::OnMouseMove(mx, my);
    const int x = mx / BlockSize;
    const int y = my / BlockSize;
    if (!preview || x < 0 || x >= MapWidth || y < 0 || y >= MapHeight)
    {
        imgTarget->Visible = false;
        return;
    }
    imgTarget->Visible = true;
    imgTarget->Position.x = x * BlockSize;
    imgTarget->Position.y = y * BlockSize;
}

bool PlayScene::CheckBomSpaceValid(int x, int y)
{
    return x >= 0 && x < MapWidth && y >= 0 && y < MapHeight && mapState[y][x] == TILE_DIRT;
}

void PlayScene::OnMouseUp(int button, int mx, int my)
{
    IScene::OnMouseUp(button, mx, my);
    if (!imgTarget->Visible)
        return;

    const int x = mx / BlockSize;
    const int y = my / BlockSize;

    if (dynamic_cast<Shovel *>(preview))
    {
        int x = mx / BlockSize;
        int y = my / BlockSize;
        dynamic_cast<Shovel *>(preview)->OnClick(x, y);

        // Exit shovel mode
        UIGroup->RemoveObject(preview->GetObjectIterator());
        preview = nullptr;
        return;
    }

    if (button & 1)
    {
        if (!preview)
            return;

        // Reject placement if already occupied
        if ((dynamic_cast<Turret *>(preview) && mapState[y][x] == TILE_OCCUPIED) ||
            (dynamic_cast<Bom *>(preview) && !CheckBomSpaceValid(x, y)))
        {
            return;
        }

        // Check placement validity
        if ((dynamic_cast<Turret *>(preview) && !CheckSpaceValid(x, y)) ||
            (dynamic_cast<Bom *>(preview) && !CheckBomSpaceValid(x, y)))
        {
            Engine::Sprite *sprite;
            GroundEffectGroup->AddNewObject(sprite = new DirtyEffect("play/target-invalid.png", 1,
                                                                     x * BlockSize + BlockSize / 2, y * BlockSize + BlockSize / 2));
            sprite->Rotation = 0;
            return;
        }

        // Purchase
        if (dynamic_cast<Bom *>(preview))
            EarnMoney(-Bom::Cost);

        // Remove preview from UI
        if (preview->GetObjectIterator()->second == UIGroup)
        {
            preview->GetObjectIterator()->first = false;
            UIGroup->RemoveObject(preview->GetObjectIterator());
        }

        // Set final position
        preview->Position.x = x * BlockSize + BlockSize / 2;
        preview->Position.y = y * BlockSize + BlockSize / 2;
        preview->Tint = al_map_rgba(255, 255, 255, 255);
        preview->Update(0);

        if (auto *turret = dynamic_cast<Turret *>(preview))
        {
            int price = turret->GetPrice();
            EarnMoney(-price);
            UIGroup->RemoveObject(preview->GetObjectIterator());

            Turret *finalTurret = nullptr;
            switch (selectedTurretId)
            {
            case 0:
                finalTurret = new MachineGunTurret(x * BlockSize + BlockSize / 2, y * BlockSize + BlockSize / 2);
                break;
            case 1:
                finalTurret = new LaserTurret(x * BlockSize + BlockSize / 2, y * BlockSize + BlockSize / 2);
                break;
            }

            if (finalTurret)
            {
                TowerGroup->AddNewObject(finalTurret);
                mapState[y][x] = TILE_OCCUPIED;
            }

            preview = nullptr;
            return;
        }
        else if (auto *mine = dynamic_cast<Bom *>(preview))
        {
            mine->placed = true;

            auto *scene = this;
            scene->EffectGroup->AddNewObject(new ExplosionEffect(mine->Position.x, mine->Position.y));
            AudioHelper::PlayAudio("explosion.wav");

            for (auto &obj : EnemyGroup->GetObjects())
            {
                Enemy *enemy = dynamic_cast<Enemy *>(obj);
                if (enemy && Engine::Collider::IsCircleOverlap(mine->Position, 10, enemy->Position, enemy->CollisionRadius))
                {
                    enemy->Hit(40);
                }
            }

            UIGroup->RemoveObject(preview->GetObjectIterator());
            preview = nullptr;
            return;
        }

        preview = nullptr;

        OnMouseMove(mx, my);
    }
}

void PlayScene::RemovePreviewIfMatched(Engine::IObject *obj)
{
    if (preview == obj)
    {
        preview = nullptr;
    }
}
void PlayScene::OnKeyDown(int keyCode)
{
    IScene::OnKeyDown(keyCode);

    if (keyCode == ALLEGRO_KEY_TAB)
    {
        DebugMode = !DebugMode;
    }
    else
    {
        keyStrokes.push_back(keyCode);
        if (keyStrokes.size() > code.size())
            keyStrokes.pop_front();
        if (keyStrokes.size() == code.size() && std::equal(code.begin(), code.end(), keyStrokes.begin()))
        {
            // Cheat code
            EffectGroup->AddNewObject(new Plane());
            AudioHelper::PlaySample("gun.wav");

            EarnMoney(10000);
        }
    }
    if (keyCode == ALLEGRO_KEY_Q)
    {
        // Hotkey for MachineGunTurret.
        UIBtnClicked(0);
    }
    else if (keyCode == ALLEGRO_KEY_W)
    {
        // Hotkey for LaserTurret.
        UIBtnClicked(1);
    }
    else if (keyCode >= ALLEGRO_KEY_0 && keyCode <= ALLEGRO_KEY_9)
    {
        // Hotkey for Speed up.
        SpeedMult = keyCode - ALLEGRO_KEY_0;
    }
}
void PlayScene::Hit()
{
    lives--;
    UILives->Text = std::string("Life ") + std::to_string(lives);
    if (lives <= 0)
    {
        Engine::GameEngine::GetInstance().ChangeScene("lose");
    }
}
int PlayScene::GetMoney() const
{
    return money;
}
int PlayScene::GetLives() const
{
    return lives;
}
void PlayScene::EarnMoney(int money)
{
    this->money += money;
    UIMoney->Text = std::string("$") + std::to_string(this->money);
}
void PlayScene::ReadMap()
{
    std::string filename = std::string("Resource/map") + std::to_string(MapId) + ".txt";
    // Read map file.
    char c;
    std::vector<bool> mapData;
    std::ifstream fin(filename);
    while (fin >> c)
    {
        switch (c)
        {
        case '0':
            mapData.push_back(false);
            break;
        case '1':
            mapData.push_back(true);
            break;
        case '\n':
        case '\r':
            if (static_cast<int>(mapData.size()) / MapWidth != 0)
                throw std::ios_base::failure("Map data is corrupted.");
            break;
        default:
            throw std::ios_base::failure("Map data is corrupted.");
        }
    }
    fin.close();
    // Validate map data.
    if (static_cast<int>(mapData.size()) != MapWidth * MapHeight)
        throw std::ios_base::failure("Map data is corrupted.");
    // Store map in 2d array.
    mapState = std::vector<std::vector<TileType>>(MapHeight, std::vector<TileType>(MapWidth));
    for (int i = 0; i < MapHeight; i++)
    {
        for (int j = 0; j < MapWidth; j++)
        {
            const int num = mapData[i * MapWidth + j];
            mapState[i][j] = num ? TILE_FLOOR : TILE_DIRT;
            if (num)
                TileMapGroup->AddNewObject(new Engine::Image("play/floor.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
            else
                TileMapGroup->AddNewObject(new Engine::Image("play/dirt.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
        }
    }
}
void PlayScene::ReadEnemyWave()
{
    std::string filename = std::string("Resource/enemy") + std::to_string(MapId) + ".txt";
    // Read enemy file.
    float type, wait, repeat;
    enemyWaveData.clear();
    std::ifstream fin(filename);
    while (fin >> type && fin >> wait && fin >> repeat)
    {
        for (int i = 0; i < repeat; i++)
            enemyWaveData.emplace_back(type, wait);
    }
    fin.close();
}
void PlayScene::ConstructUI()
{
    // Background
    UIGroup->AddNewObject(new Engine::Image("play/sand.png", 1280, 0, 320, 832));
    // Text
    UIGroup->AddNewObject(new Engine::Label(std::string("Stage ") + std::to_string(MapId), "pirulen.ttf", 32, 1294, 0));
    UIGroup->AddNewObject(UIMoney = new Engine::Label(std::string("$") + std::to_string(money), "pirulen.ttf", 24, 1294, 48));
    UIGroup->AddNewObject(UILives = new Engine::Label(std::string("Life ") + std::to_string(lives), "pirulen.ttf", 24, 1294, 88));
    TurretButton *btn;
    // Button 1
    btn = new TurretButton("play/floor.png", "play/dirt.png",
                           Engine::Sprite("play/tower-base.png", 1294, 136, 0, 0, 0, 0),
                           Engine::Sprite("play/turret-1.png", 1294, 136 - 8, 0, 0, 0, 0), 1294, 136, MachineGunTurret::Price);
    // Reference: Class Member Function Pointer and std::bind.
    btn->SetOnClickCallback(std::bind(&PlayScene::UIBtnClicked, this, 0));
    UIGroup->AddNewControlObject(btn);
    // Button 2
    btn = new TurretButton("play/floor.png", "play/dirt.png",
                           Engine::Sprite("play/tower-base.png", 1370, 136, 0, 0, 0, 0),
                           Engine::Sprite("play/turret-2.png", 1370, 136 - 8, 0, 0, 0, 0), 1370, 136, LaserTurret::Price);
    btn->SetOnClickCallback(std::bind(&PlayScene::UIBtnClicked, this, 1));
    UIGroup->AddNewControlObject(btn);

    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int shift = 135 + 25;
    dangerIndicator = new Engine::Sprite("play/benjamin.png", w - shift, h - shift);
    dangerIndicator->Tint.a = 0;
    UIGroup->AddNewObject(dangerIndicator);

    // Bom
    btn = new TurretButton("play/floor.png", "play/dirt.png",
                           Engine::Sprite("play/tower-base.png", 1445, 136, 0, 0, 0, 0),
                           Engine::Sprite("play/landmine.png", 1445 + 4, 136 + 5, 0, 0, 0, 0),
                           1445, 136, Bom::Cost);
    btn->SetOnClickCallback(std::bind(&PlayScene::UIBtnClicked, this, 3));
    UIGroup->AddNewControlObject(btn);

    // Shovel
    btn = new TurretButton("play/floor.png", "play/dirt.png",
                           Engine::Sprite("play/tower-base.png", 1510, 136, 0, 0, 0, 0),
                           Engine::Sprite("play/shovel.png", 1510 + 4, 136 + 5, 0, 0, 0, 0),
                           1510, 136, 0);
    btn->SetOnClickCallback(std::bind(&PlayScene::UIBtnClicked, this, 4));
    UIGroup->AddNewControlObject(btn);
}

void PlayScene::UIBtnClicked(int id)
{
    if (preview && preview->GetObjectIterator()->second == UIGroup)
        UIGroup->RemoveObject(preview->GetObjectIterator());

    // Handle turret selections
    if (id == 0 && money >= MachineGunTurret::Price)
    {
        preview = new MachineGunTurret(0, 0);
        selectedTurretId = id;
    }
    else if (id == 1 && money >= LaserTurret::Price)
    {
        preview = new LaserTurret(0, 0);
        selectedTurretId = id;
    }
    // Handle Bom (ID 3)
    else if (id == 3 && money >= Bom::Cost)
    {
        preview = new Bom(0, 0);
    }
    // Handle shovel (ID 4)
    else if (id == 4)
    {
        preview = new Shovel(0, 0);
    }

    if (!preview)
        return;

    preview->Position = Engine::GameEngine::GetInstance().GetMousePosition();
    preview->Tint = al_map_rgba(255, 255, 255, 200);

    if (auto *turret = dynamic_cast<Turret *>(preview))
    {
        turret->Enabled = false;
        turret->Preview = true;
    }

    UIGroup->AddNewObject(preview);
    OnMouseMove(preview->Position.x, preview->Position.y);
}

bool PlayScene::CheckSpaceValid(int x, int y)
{
    if (x < 0 || x >= MapWidth || y < 0 || y >= MapHeight)
        return false;
    auto map00 = mapState[y][x];
    mapState[y][x] = TILE_OCCUPIED;
    std::vector<std::vector<int>> map = CalculateBFSDistance();
    mapState[y][x] = map00;
    if (map[0][0] == -1)
        return false;
    for (auto &it : EnemyGroup->GetObjects())
    {
        Engine::Point pnt;
        pnt.x = floor(it->Position.x / BlockSize);
        pnt.y = floor(it->Position.y / BlockSize);
        if (pnt.x < 0)
            pnt.x = 0;
        if (pnt.x >= MapWidth)
            pnt.x = MapWidth - 1;
        if (pnt.y < 0)
            pnt.y = 0;
        if (pnt.y >= MapHeight)
            pnt.y = MapHeight - 1;
        if (map[pnt.y][pnt.x] == -1)
            return false;
    }
    mapState[y][x] = TILE_OCCUPIED;
    mapDistance = map;
    for (auto &it : EnemyGroup->GetObjects())
        dynamic_cast<Enemy *>(it)->UpdatePath(mapDistance);
    return true;
}
std::vector<std::vector<int>> PlayScene::CalculateBFSDistance()
{
    std::vector<std::vector<int>> map(MapHeight, std::vector<int>(std::vector<int>(MapWidth, -1)));
    std::queue<Engine::Point> que;
    if (mapState[MapHeight - 1][MapWidth - 1] != TILE_DIRT)
        return map;
    que.push(Engine::Point(MapWidth - 1, MapHeight - 1));
    map[MapHeight - 1][MapWidth - 1] = 0;
    while (!que.empty())
    {
        Engine::Point p = que.front();
        que.pop();
        for (const auto &dir : directions)
        {
            int nx = p.x + dir.x;
            int ny = p.y + dir.y;
            if (nx >= 0 && nx < MapWidth && ny >= 0 && ny < MapHeight)
            {
                if (map[ny][nx] == -1 && mapState[ny][nx] == TILE_DIRT)
                {
                    map[ny][nx] = map[p.y][p.x] + 1;
                    que.push(Engine::Point(nx, ny));
                }
            }
        }
    }
    return map;
}
