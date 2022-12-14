#pragma once
#include "Mesh.h"
#include "RendererAPI.h"

namespace mc
{
    template <typename T>
    void Mesh::SetVertices(std::span<T> vertices) {
        u64 newSize = sizeof(T) * vertices.size();

        if(m_vertexBufferSize == 0)
            m_vertexBuffer = RendererAPI::CreateVertexBuffer<T>(vertices);
        else
            throw std::exception("TODO: add buffer updating");

        m_vertexBufferSize = newSize;
    }
}
