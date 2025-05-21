#include <cmath>
#include <string>

#include "Engine/GameEngine.hpp"
#include "Engine/Group.hpp"
#include "Engine/IScene.hpp"
#include "Engine/Resources.hpp"
#include "ExplosionEffect2.hpp"
#include "Scene/PlayScene.hpp"

PlayScene *ExplosionEffect2::getPlayScene()
{
    return dynamic_cast<PlayScene *>(Engine::GameEngine::GetInstance().GetActiveScene());
}
ExplosionEffect2::ExplosionEffect2(float x, float y)
    : Sprite("", x, y), timeTicks(0)
{
    for (int i = 1; i <= 5; i++)
    {
        bmps.push_back(Engine::Resources::GetInstance().GetBitmap("play/explosion-" + std::to_string(i) + ".png"));
    }
    bmp = bmps[0];
}

void ExplosionEffect2::Update(float deltaTime)
{
    timeTicks += deltaTime;
    if (timeTicks >= timeSpan)
    {
        getPlayScene()->EffectGroup->RemoveObject(objectIterator);
        return;
    }
    int phase = floor(timeTicks / timeSpan * bmps.size());
    bmp = bmps[phase];
    Sprite::Update(deltaTime);
}
