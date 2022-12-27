#pragma once

namespace mc
{
    class Camera
    {
    public:
        Camera(float fov, u32 width, u32 height);

        void SetPosition(float3 pos) { m_pos = pos; }
        void SetRotation(float2 rot) { m_rot = rot; }
        
        float3 GetPosition() const { return m_pos; }
        float2 GetRotation() const { return m_rot; }

        Mat4 GetView() const;
        Mat4 GetProjection() const;

        void ResizeView(u32 width, u32 height);

    protected:
        float3 m_pos{};
        float2 m_rot{};

        float m_fov;
        Mat4 m_proj{1};
    };
}
