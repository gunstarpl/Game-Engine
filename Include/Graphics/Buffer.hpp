/*
    Copyright (c) 2018-2020 Piotr Doan. All rights reserved.
*/

#pragma once

#include "Graphics/RenderState.hpp"

/*
    Buffer
    
    Generic buffer base class that can handle different types of OpenGL buffers.
    Supported buffer types include vertex buffer, index buffer and instance buffer.
*/

namespace Graphics
{
    class RenderContext;

    class Buffer : private NonCopyable
    {
    public:
        struct CreateFromParams
        {
            GLenum usage = GL_STATIC_DRAW;
            std::size_t elementSize = 0;
            std::size_t elementCount = 0;
            const void* data = nullptr;
        };

        enum class CreateErrors
        {
            InvalidArgument,
            FailedResourceCreation,
        };

        using CreateResult = Result<void, CreateErrors>;

    public:
        void Update(const void* data, std::size_t elementCount);

        GLenum GetType() const;
        GLuint GetHandle() const;
        std::size_t GetElementSize() const;
        std::size_t GetElementCount() const;
        virtual GLenum GetElementType() const;
        virtual bool IsInstanced() const;

    protected:
        Buffer();
        virtual ~Buffer();

        CreateResult CreateBase(RenderContext* renderContext, GLenum type, const CreateFromParams& params);

    protected:
        RenderContext* m_renderContext = nullptr;

        GLuint m_handle = OpenGL::InvalidHandle;
        GLenum m_type = OpenGL::InvalidEnum;
        GLenum m_usage = OpenGL::InvalidEnum;

        std::size_t m_elementSize = 0;
        std::size_t m_elementCount = 0;
    };
}

/*
    Vertex Buffer
*/

namespace Graphics
{
    class VertexBuffer final : public Buffer
    {
    public:
        using CreateResult = Result<std::unique_ptr<VertexBuffer>, Buffer::CreateErrors>;
        static CreateResult Create(RenderContext* renderContext, const Buffer::CreateFromParams& params);

    public:
        ~VertexBuffer();

    private:
        VertexBuffer();
    };
}

/*
    Index Buffer
*/

namespace Graphics
{
    class IndexBuffer final : public Buffer
    {
    public:
        using CreateResult = Result<std::unique_ptr<IndexBuffer>, Buffer::CreateErrors>;
        static CreateResult Create(RenderContext* renderContext, const Buffer::CreateFromParams& params);

    public:
        ~IndexBuffer();

        GLenum GetElementType() const override;

    private:
        IndexBuffer();
    };
}

/*
    Instance Buffer
*/

namespace Graphics
{
    class InstanceBuffer final : public Buffer
    {
    public:
        using CreateResult = Result<std::unique_ptr<InstanceBuffer>, Buffer::CreateErrors>;
        static CreateResult Create(RenderContext* renderContext, const Buffer::CreateFromParams& params);

    public:
        ~InstanceBuffer();

        bool IsInstanced() const override;

    private:
        InstanceBuffer();
    };
}
