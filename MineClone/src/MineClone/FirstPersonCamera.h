#pragma once
#include "MineClone/Core/Renderer/Camera.h"

#include "Core/Event/EventHandler.h"
#include "Core/Event/WindowEvents.h"

namespace mc
{
    class FirstPersonCamera : public Camera, EventHandler<WindowResizeEvent>
    {
    public:
        FirstPersonCamera(float fov, u32 width, u32 height);

        void Update(float deltaTime);

    protected:
        void OnEvent(WindowResizeEvent& ev) override;
        
    private:
        int2 m_prevMousePos{};
    };
}
