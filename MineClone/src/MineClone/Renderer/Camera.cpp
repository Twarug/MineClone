#include "mcpch.h"
#include "Camera.h"

#include "glm/gtc/matrix_transform.hpp"

namespace mc
{
    Camera::Camera(float fov, u32 width, u32 height)
        : m_fov(fov), m_proj(glm::perspective(glm::radians(fov), (float)width/(float)height, 0.1f, 1000.f))
    {}
    
    Mat4 Camera::GetView() const
    {
        return glm::inverse(
            glm::translate(Mat4(1), m_pos) *
            glm::rotate(Mat4(1), glm::radians(m_rot.y), {0, 1, 0}) *
            glm::rotate(Mat4(1), glm::radians(m_rot.x), {1, 0, 0})
        );
    }
    
    Mat4 Camera::GetProjection() const
    {
        return m_proj;   
    }
    
    void Camera::OnResize(u32 width, u32 height)
    {
        m_proj = glm::perspective(glm::radians(m_fov), (float)width/(float)height, 0.1f, 1000.f);
    }
}
