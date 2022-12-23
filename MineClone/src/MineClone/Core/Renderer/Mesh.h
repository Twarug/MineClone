#pragma once
#include "Buffer.h"

namespace mc
{
    class Mesh
    {
    public:
        Mesh() = default;
        ~Mesh();

        void Render(const Mat4& transform) const;

        void SetIndices(std::span<const u32> indices);

        template <typename T>
        void SetVertices(std::span<T> vertices);

        void Dispose();
        
    private:
        Ref<Buffer> m_vertexBuffer;
        Ref<Buffer> m_indexBuffer;
        u64 m_vertexBufferCap = 0;
        u64 m_indexBufferCap = 0;

        u32 m_vertexCount = 0;
        u32 m_indicesCount = 0;
    };
}

#include "Mesh.tpp"