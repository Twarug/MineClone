#pragma once
#include "Buffer.h"

namespace mc
{
    class Mesh
    {
    public:
        Mesh();

        void Render(const Mat4& transform) const;

        void SetIndices(std::span<u32> indices);

        template <typename T>
        void SetVertices(std::span<T> vertices);

    private:
        Ref<AllocatedBuffer> m_vertexBuffer;
        Ref<AllocatedBuffer> m_indexBuffer;

        u32 m_indicesCount = 0;
        u64 m_vertexBufferSize = 0;
    };
}
