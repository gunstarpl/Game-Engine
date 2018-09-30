/*
    Copyright (c) 2018 Piotr Doan. All rights reserved.
*/

#pragma once

/*
    Texture View

    Utility object used to represent a rectangular area on a texture.
*/

namespace Graphics
{
    // Forward declaration.
    class Texture;

    // Texture view class.
    class TextureView
    {
    public:
        // Type declarations.
        using TexturePtr = std::shared_ptr<const Texture>;

    public:
        TextureView();
        ~TextureView();

        TextureView(TexturePtr texture);
        TextureView(TexturePtr texture, glm::ivec4 rectangle);
        TextureView(TexturePtr texture, glm::vec4 coordinates);

        TextureView(const TextureView& other);
        TextureView& operator=(const TextureView& other);

        TextureView(TextureView&& other);
        TextureView& operator=(TextureView&& other);

        // Sets the texture reference.
        void SetTexture(TexturePtr texture);

        // Sets the texture rectangle in pixel space.
        void SetRectangle(const glm::ivec4 rect);

        // Sets the texture coordinates in texture space.
        void SetCoordinates(const glm::vec4 coords);

        // Gets the texture reference.
        TexturePtr GetTexture() const;

        // Gets the texture rectangle in pixel space.
        glm::ivec4 GetRectangle() const;

        // Gets the texture coordinates in texture space.
        glm::vec4 GetCoordinates() const;

    private:
        // Texture reference.
        TexturePtr m_texture;

        // Texture coordinates.
        glm::vec4 m_coordinates;
    };
}