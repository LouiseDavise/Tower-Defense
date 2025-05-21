#include "SuperTankEnemy.hpp"
#include "UI/Animation/ExplosionEffect.hpp"
#include "Engine/Collider.hpp"
#include "Engine/GameEngine.hpp"
#include "Scene/PlayScene.hpp"
#include "Engine/AudioHelper.hpp"

SuperTankEnemy::SuperTankEnemy(int x, int y)
    : Enemy("play/enemy-4.png", x, y, 16, 40, 30, 5),
      armorOverlay("play/armor-overlay.png", x, y)
{
    armorOverlay.Anchor = Anchor;
}

void SuperTankEnemy::Hit(float damage)
{
    float reducedDamage = damage * (1.0f - damageReductionRatio);
    Enemy::Hit(reducedDamage);
}

void SuperTankEnemy::Update(float deltaTime)
{
    armorOverlay.Position = Position; // Align armor overlay
    Enemy::Update(deltaTime);         // Keep original movement/logic
}

void SuperTankEnemy::Draw() const
{
    Enemy::Draw();       // Draw tank
    armorOverlay.Draw(); // Draw armor visual
}
