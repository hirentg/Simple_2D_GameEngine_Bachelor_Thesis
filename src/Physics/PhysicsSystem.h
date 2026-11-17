#pragma once
#include "../ECS/Registry.h"
#include "../ECS/Components.h"
#include "SpatialHashGrid.h"
#include "CollisionDetection.h"
#include "CollisionResponse.h"

class PhysicsSystem {
public:

glm::vec2 Gravity {0,-9.81f};

void Update(Registry& r,float dt) 
{
    for(auto e:r.GetView<TransformComponent,RigidbodyComponent>()) 
    {
        auto& b = r.GetComponent<RigidbodyComponent>(e);

        if(!b.isStatic) 
        {
            b.mass = std::max(b.mass,0.01f);
            b.velocity += (Gravity+b.acceleration)*dt;
            r.GetComponent<TransformComponent>(e).position += b.velocity * dt;
        }
    }
 
grid.Clear(); collisions.clear();

for(auto e:r.GetView<TransformComponent,ColliderComponent>()) 
{
    auto& t = r.GetComponent<TransformComponent>(e);
    auto& c = r.GetComponent<ColliderComponent>(e);

    c.halfExtents = glm::max(c.halfExtents, glm::vec2(0.01f));
    grid.Insert(e,t.position + c.offset - c.halfExtents, t.position + c.offset + c.halfExtents);
}


for(auto [a,b] : grid.GetCandidatePairs()) 
{
    if(!r.HasComponent<RigidbodyComponent>(a) || !r.HasComponent<RigidbodyComponent>(b))
        continue;

    auto& ta = r.GetComponent<TransformComponent>(a);
    auto& tb = r.GetComponent<TransformComponent>(b);

    auto& ca = r.GetComponent<ColliderComponent>(a);
    auto& cb=r.GetComponent<ColliderComponent>(b);

    auto& ra = r.GetComponent<RigidbodyComponent>(a);
    auto& rb = r.GetComponent<RigidbodyComponent>(b);

    if (!(ca.layerMask&cb.layerMask) || (ra.isStatic&&rb.isStatic))
        continue;

    auto hit = AABBvsAABB(ta.position + ca.offse - ca.halfExtents, ta.position + ca.offset + ca.halfExtents, 
        tb.position + cb.offset - cb.halfExtents,tb.position + cb.offset + cb.halfExtents);

    if(hit.collided)
    {
        ResolveCollision(ta,ra,ca,tb,rb,cb,hit);collisions.emplace_back(a,b);
    }
}
}

const std::vector<EntityPair>& GetCollisionsThisFrame() const
{
    return collisions;
}

private:
    SpatialHashGrid grid{2.f};
    std::vector<EntityPair> collisions;
};
