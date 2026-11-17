
#include "Physics/PhysicsSystem.h"
#include <iostream>
#include <stdexcept>

void check(bool c)
{
    if (!c)
        throw std::runtime_error("Regression failed");
}

int main()
{
    Registry r;

    auto a = r.CreateEntity();
    r.AddComponent<TransformComponent>(a);
    r.DestroyEntity(a);
    r.DestroyEntity(a);
    r.FlushDestroyedEntities();

    auto b = r.CreateEntity(), c = r.CreateEntity();

    check(b != c);
    check(r.EntityCount() == 2);

    check(!AABBvsAABB({ 0,0 }, { 1,1 }, { 2,2 }, { 3,3 }).collided);
    check(AABBvsAABB({ 0,0 }, { 2,2 }, { 1,1 }, { 3,3 }).collided);

    SpatialHashGrid grid(1);

    grid.Insert(b, { -2,-2 }, { 2,2 });
    grid.Insert(c, { -1,-1 }, { 1,1 });

    check(grid.GetCandidatePairs().size() == 1);

    grid.Clear();

    check(grid.GetCandidatePairs().empty());

    for (auto e : { b,c })
    {
        r.AddComponent<TransformComponent>(e);
        r.AddComponent<ColliderComponent>(e);
        r.AddComponent<RigidbodyComponent>(e);
    }

    r.GetComponent<RigidbodyComponent>(b).isStatic = true;
    r.GetComponent<TransformComponent>(c).position = { 0,.8f };

    PhysicsSystem physics;

    physics.Update(r, 1.f / 60);

    check(!physics.GetCollisionsThisFrame().empty());
    check(r.GetComponent<TransformComponent>(b).position == glm::vec2(0));

    r.GetComponent<ColliderComponent>(c).layerMask = 0;

    physics.Update(r, 1.f / 60);

    check(physics.GetCollisionsThisFrame().empty());

    std::cout << "ECS lifecycle, AABB, grid, static response and layer filtering passed\n";
}
