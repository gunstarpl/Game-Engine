/*
    Copyright (c) 2018-2021 Piotr Doan. All rights reserved.
    Software distributed under the permissive MIT License.
*/

#pragma once

#include <Core/EngineSystem.hpp>

namespace System
{
    class FileHandle;
}

/*
    Texture Atlas

    Stores multiple images that can be referenced by name in a single texture.
*/

namespace Graphics
{
    class RenderContext;
    class Texture;
    class TextureView;

    class TextureAtlas final : private Common::NonCopyable
    {
    public:
        struct LoadFromFile
        {
            const Core::EngineSystemStorage* engineSystems = nullptr;
        };

        enum class CreateErrors
        {
            InvalidArgument,
            FailedResourceLoading,
            InvalidResourceContents,
        };

        using CreateResult = Common::Result<std::unique_ptr<TextureAtlas>, CreateErrors>;

        static CreateResult Create();
        static CreateResult Create(System::FileHandle& file, const LoadFromFile& params);

        using ConstTexturePtr = std::shared_ptr<const Texture>;
        using RegionMap = std::unordered_map<std::string, glm::ivec4>;

    public:
        ~TextureAtlas();

        bool AddRegion(std::string name, glm::ivec4 pixelCoords);
        TextureView GetRegion(std::string name);

    private:
        TextureAtlas();

    private:
        ConstTexturePtr m_texture;
        RegionMap m_regions;
    };
};
