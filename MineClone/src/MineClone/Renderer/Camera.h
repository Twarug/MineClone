#pragma once

namespace mc
{
    class Camera
    {
    public:
        Camera(float fov, u32 width, u32 height);
        
        void Update(float deltaTime);

        void SetPos(float3 pos) { m_pos = pos; }
        void SetRot(float2 rot) { m_rot = rot; }
        
        Mat4 GetView() const;
        Mat4 GetProjection() const;

        void OnResize(u32 width, u32 height);
    private:
        float3 m_pos{};
        float2 m_rot{};

        float m_fov;
        Mat4 m_proj{1};
    };
}
