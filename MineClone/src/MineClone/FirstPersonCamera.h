#pragma once
#include "MineClone/Renderer/Camera.h"

namespace mc
{
    class FirstPersonCamera : public Camera
    {
    public:
        FirstPersonCamera(float fov, u32 width, u32 height);

        void Update(float deltaTime);

    private:
        int2 m_prevMousePos{};
    };
}
