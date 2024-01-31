//
// Created by zphrfx on 21/01/2024.
//

#pragma once

#include "engine_model.hpp"

namespace vke {
    struct Transform2DComponent {
        glm::vec2 translation{};
        glm::vec2 scale{1.f, 1.f};
        f32 rotation = 0.f;

        [[nodiscard]] glm::mat2 mat2() const {
            const f32 s = glm::sin(rotation);
            const f32 c = glm::cos(rotation);

            const glm::mat2 rotationMatrix{{c, s}, {-s, c}};
            const glm::mat2 scaleMatrix{{scale.x, 0.f}, {0.f, scale.y}};

            return rotationMatrix * scaleMatrix;
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
        Transform2DComponent mTransform;

    private:
        explicit VkEngineGameObjects(const ObjectID id)
            : mId(id) {
        }

        ObjectID mId = 0;
    };
} // vke
