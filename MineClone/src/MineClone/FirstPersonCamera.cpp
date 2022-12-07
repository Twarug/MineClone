#include "mcpch.h"
#include "FirstPersonCamera.h"

#include "MineClone/Input/Input.h"

namespace mc
{
    static constexpr float SPEED = 5.f; 
    static constexpr float SENSITIVITY = .1f; 
    
    FirstPersonCamera::FirstPersonCamera(float fov, u32 width, u32 height)
        : Camera(fov, width, height)
    {}

    void FirstPersonCamera::Update(float deltaTime)
    {
        auto mouse = Input::GetButton(MouseCode::Button0);
        int2 mousePos = Input::GetMousePos();
        if(mouse.down)
            m_prevMousePos = mousePos;
        
        if(mouse.pressed)
        {
            float2 mouseDelta = float2(mousePos - m_prevMousePos).yx * SENSITIVITY;
            m_prevMousePos = mousePos;

            // mouseDelta.y *= -1.f;
            m_rot -= mouseDelta;
        }   
        
        if(Input::GetKey(KeyCode::Q).pressed)
            m_rot.y += 10.f * SPEED * deltaTime;
        else if(Input::GetKey(KeyCode::E).pressed)
            m_rot.y -= 10.f * SPEED * deltaTime;
        
        if(Input::GetKey(KeyCode::Space).pressed)
            m_pos.y += 1.f * SPEED * deltaTime;
        else if(Input::GetKey(KeyCode::LeftShift).pressed)
            m_pos.y -= 1.f * SPEED * deltaTime;

        
        const float3 right = {cos(glm::radians(m_rot.y)), 0, sin(-glm::radians(m_rot.y))};
        const float3 forward = glm::cross(right, {0, -1, 0});
        
        float3 vel{0};
        if(Input::GetKey(KeyCode::W).pressed)
            vel += forward;
        else if(Input::GetKey(KeyCode::S).pressed)
            vel -= forward;

        if(Input::GetKey(KeyCode::D).pressed)
            vel += right;
        else if(Input::GetKey(KeyCode::A).pressed)
            vel -= right;
        
        if(glm::length(vel) == 0.f)
            return;
                
        m_pos += vel * SPEED * deltaTime;
    }
}
