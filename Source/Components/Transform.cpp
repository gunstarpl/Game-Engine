/*
    Copyright (c) 2018 Piotr Doan. All rights reserved.
*/

#include "Precompiled.hpp"
#include "Components/Transform.hpp"

Components::Transform::Transform() :
    position(0.0f, 0.0f, 0.0f),
    rotation(0.0f, 0.0f, 0.0f),
    scale(1.0f, 1.0f, 1.0f)
{
}

Components::Transform::~Transform()
{
}

glm::mat4 Components::Transform::CalculateMatrix(const glm::mat4& input) const
{
    glm::mat4 output(input);

    output = glm::translate(output, position);
    output = glm::rotate(output, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    output = glm::rotate(output, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    output = glm::rotate(output, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    output = glm::scale(output, scale);
    
    return output;
}