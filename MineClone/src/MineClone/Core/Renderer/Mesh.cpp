#include "mcpch.h"
#include "Mesh.h"

#include "RendererAPI.h"

namespace mc
{
    Mesh::Mesh() = default;

    void Mesh::Render(const Mat4& transform) const {
        RendererAPI::Draw(transform, m_vertexBuffer, m_indexBuffer, m_indicesCount);
    }

    void Mesh::SetIndices(std::span<u32> indices) {
        u32 newCount = static_cast<u32>(indices.size());

        if(m_indicesCount == 0)
            m_indexBuffer = RendererAPI::CreateIndexBuffer(indices);
        else
            throw std::exception("TODO: add buffer updating");

        m_indicesCount = newCount;
    }
}
