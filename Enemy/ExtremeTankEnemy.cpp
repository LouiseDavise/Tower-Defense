#include <string>
#include "ExtremeTankEnemy.hpp"
#include "UI/Animation/ExplosionEffect.hpp"
#include "Engine/Collider.hpp"
#include "Engine/GameEngine.hpp"
#include "Scene/PlayScene.hpp"
#include "Engine/AudioHelper.hpp"

ExtremeTankEnemy::ExtremeTankEnemy(int x, int y)
    : Enemy("play/enemy-5.png", x, y, 18, 30, 40, 8),
      armorOverlay("play/armor-overlay.png", x, y)
{
    armorOverlay.Anchor = Anchor;
    shieldToggleTimer = shieldToggleInterval;
}

void ExtremeTankEnemy::Hit(float damage)
{
    float actualDamage = shieldActive ? damage * (1.0f - damageReductionRatio) : damage;
    Enemy::Hit(actualDamage);
}

void ExtremeTankEnemy::Update(float deltaTime)
{
    shieldToggleTimer -= deltaTime;
    if (shieldToggleTimer <= 0)
    {
        shieldToggleTimer = shieldToggleInterval;
        shieldActive = (rand() % 2 == 0);
    }

    armorOverlay.Position = Position;
    Enemy::Update(deltaTime);
}

void ExtremeTankEnemy::Draw() const
{
    Enemy::Draw();
    if (shieldActive)
    {
        armorOverlay.Draw();
    }
}
