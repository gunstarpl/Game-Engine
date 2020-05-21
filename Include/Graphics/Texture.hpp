/*
    Copyright (c) 2018-2020 Piotr Doan. All rights reserved.
*/

#pragma once

#include <Core/ServiceStorage.hpp>
#include "Graphics/RenderState.hpp"

namespace System
{
    class FileSystem;
}

/*
    Texture
    
    Encapsulates an OpenGL texture object which can be loaded from PNG file.
*/

namespace Graphics
{
    class RenderContext;

    class Texture final : private Common::NonCopyable
    {
    public:
        struct CreateFromParams
        {
            RenderContext* renderContext = nullptr;
            GLenum format = OpenGL::InvalidEnum;
            int width = 0;
            int height = 0;
            bool mipmaps = true;
            const void* data = nullptr;
        };

        struct LoadFromFile
        {
            const Core::ServiceStorage* services = nullptr;
            std::string filePath;
            std::string relativePath;
            bool mipmaps = true;
        };

        enum class CreateErrors
        {
            InvalidArgument,
            FailedTextureCreation,
            FailedFilePathResolve,
            FailedFileOpening,
            FailedPngLoading,
        };

        using CreateResult = Common::Result<std::unique_ptr<Texture>, CreateErrors>;
        static CreateResult Create(const CreateFromParams& params);
        static CreateResult Create(const LoadFromFile& params);

    public:
        ~Texture();
        void Update(const void* data);

        GLuint GetHandle() const;
        int GetWidth() const;
        int GetHeight() const;

    private:
        Texture();

    private:
        RenderContext* m_renderContext = nullptr;
        GLuint m_handle = OpenGL::InvalidHandle;
        GLenum m_format = OpenGL::InvalidEnum;
        int m_width = 0;
        int m_height = 0;
    };
    
    using TexturePtr = std::shared_ptr<Texture>;
    using ConstTexturePtr = std::shared_ptr<const Texture>;
}
