//
// Created by zphrfx on 21/01/2024.
//

#pragma once

#include "engine_model.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace vke {
    struct TransformComponent {
        glm::vec3 translation{};
        glm::vec3 rotation{};
        glm::vec3 scale{1.f, 1.f, 1.f};

        [[nodiscard]] glm::mat4 mat4() const {
            auto transform = glm::translate(glm::mat4{1.f}, translation);
            transform = glm::rotate(transform, glm::radians(rotation.y), {0.f, 1.f, 0.f});
            transform = glm::rotate(transform, glm::radians(rotation.x), {1.f, 0.f, 0.f});
            transform = glm::rotate(transform, glm::radians(rotation.z), {0.f, 0.f, 1.f});
            transform = glm::scale(transform, scale);
            return transform;
        }
    };


    // has a private constructor so we can't create an instance of this class
    // without using the static createGameObject() method which increments the id
    class VkEngineGameObjects {
    public:
        using ObjectID = u32;

        // delete copy constructor and assignment operator
        VkEngineGameObjects(const VkEngineGameObjects&) = delete;

        VkEngineGameObjects& operator=(const VkEngineGameObjects&) = delete;

        // default move constructor and move assignment operator
        VkEngineGameObjects(VkEngineGameObjects&&) = default;

        VkEngineGameObjects& operator=(VkEngineGameObjects&&) = default;


        static VkEngineGameObjects createGameObject() {
            static ObjectID id = 0;
            return VkEngineGameObjects{id++};
        }

        ObjectID getId() const { return mId; }

        std::shared_ptr<VkEngineModel> pModel = nullptr;
        glm::vec3 mColor = {1.f, 1.f, 1.f};
        TransformComponent mTransform;

    private:
        explicit VkEngineGameObjects(const ObjectID id)
            : mId(id) {
        }

        ObjectID mId = 0;
    };
} // vke
