#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

#include "glad/glad.h"
#include "GLFW/glfw3.h"


    struct Plan
    {
        // unit vector
        glm::vec3 normal = { 0.f, 1.f, 0.f };

        // distance from origin to the nearest point in the plan
        float     distance = 0.f;

        Plan() = default;

        Plan(const glm::vec3& p1, const glm::vec3& norm)
            : normal(glm::normalize(norm)),
            distance(glm::dot(normal, p1))
        {}

        float getSignedDistanceToPlan(const glm::vec3& point) const
        {
            return glm::dot(normal, point) - distance;
        }
    };

    struct Frustum
    {
        Plan topFace;
        Plan bottomFace;

        Plan rightFace;
        Plan leftFace;

        Plan farFace;
        Plan nearFace;

        bool isOnOrForwardPlan(Plan plan, float extent, glm::vec3 centre)
        {
            // Compute the projection interval radius of b onto L(t) = b.c + t * p.n
            const float r = extent * (std::abs(plan.normal.x) + std::abs(plan.normal.y) + std::abs(plan.normal.z));
            return -r <= plan.getSignedDistanceToPlan(centre);
        }

        bool IsAABBVisable(glm::vec3 centre, float extent)
        {
            return (isOnOrForwardPlan(leftFace, extent, centre) &&
                isOnOrForwardPlan(rightFace, extent, centre) &&
                isOnOrForwardPlan(topFace, extent, centre) &&
                isOnOrForwardPlan(bottomFace, extent, centre) &&
                isOnOrForwardPlan(nearFace, extent, centre) &&
                isOnOrForwardPlan(farFace, extent, centre));
        };

        bool AABBisOnOrForwardPlan(Plan plan, glm::vec3 center, glm::vec3 extents)
        {
            // Compute the projection interval radius of b onto L(t) = b.c + t * p.n
            const float r = extents.x * std::abs(plan.normal.x) + extents.y * std::abs(plan.normal.y) +
                extents.z * std::abs(plan.normal.z);

            return -r <= plan.getSignedDistanceToPlan(center);
        }

        bool AABBisOnFrustum(glm::vec3 center, glm::vec3 extents)
        {
            return (AABBisOnOrForwardPlan(leftFace, center, extents) &&
                AABBisOnOrForwardPlan(rightFace, center, extents) &&
                AABBisOnOrForwardPlan(topFace, center, extents) &&
                AABBisOnOrForwardPlan(bottomFace, center, extents) &&
                AABBisOnOrForwardPlan(nearFace, center, extents) &&
                AABBisOnOrForwardPlan(farFace, center, extents));
        };
    };

    class Camera
    {
    public:
        glm::vec3 Forward();
        glm::vec3 Up();
        glm::vec3 Right();
        glm::mat4 GetViewMatrix();

        glm::vec3 GetPosition();
        void      SetPosition(glm::vec3 position);

        void Initialize();
        void SetFOV(float FOV);
        int  GetFOV();
        void SetAngles(float yaw, float pitch);
        void AddAngles(float yaw, float pitch);

        void BindViewBuffer(unsigned int shaderID);
        void OnResolutionChange(int width, int height);
        void Update();
        void Dispose();

        Frustum* GetViewFrustum();

        static bool FRUSTUM_UPDATE;
    private:
        Frustum frustum;

        glm::vec3 camera_position;
        glm::vec3 camera_target;
        glm::vec3 camera_direction;

        glm::vec3 camera_forward;
        glm::vec3 camera_right;
        glm::vec3 camera_up;

        glm::mat4 projection;

        float camera_yaw;
        float camera_pitch;
        float camera_fov;
        float camera_aspect;

        int screen_width;
        int screen_height;

        unsigned int uniformBufferID;

        void InitializeUniform();
        void UpdateProjection();
        void UpdateVectors();

        void UpdateFrustumFromCamera(float aspect, float fovY, float zNear, float zFar);
    };
