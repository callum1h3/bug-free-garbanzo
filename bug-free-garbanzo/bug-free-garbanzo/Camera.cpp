#include "camera.h"


    bool Camera::FRUSTUM_UPDATE = true;

    glm::mat4 Camera::GetViewMatrix() { return glm::lookAt(this->camera_position, this->camera_position + Forward(), Up()); }
    glm::vec3 Camera::Up() { return camera_up; }
    glm::vec3 Camera::Right() { return camera_right; }
    glm::vec3 Camera::Forward() { return camera_forward; }
    glm::vec3 Camera::GetPosition() { return camera_position; }
    int       Camera::GetFOV() { return camera_fov; }


    void Camera::SetPosition(glm::vec3 position)
    {
        camera_position = position;
        UpdateVectors();
    }

    void Camera::Initialize()
    {
        InitializeUniform();

        SetPosition(glm::vec3(0, 0, 5));
        SetAngles(-90.0f, 0);
        SetFOV(90.0f);

        UpdateVectors();
    }

    void Camera::Update()
    {
        glm::mat4 view_matrix = GetViewMatrix();
        if (FRUSTUM_UPDATE)
            UpdateFrustumFromCamera(camera_aspect, camera_fov, 0.1f, 1000.0f);

        glBindBuffer(GL_UNIFORM_BUFFER, uniformBufferID);
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view_matrix));
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, sizeof(glm::vec3), glm::value_ptr(camera_position));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

    }

    void Camera::UpdateVectors()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(camera_yaw)) * cos(glm::radians(camera_pitch));
        front.y = sin(glm::radians(camera_pitch));
        front.z = sin(glm::radians(camera_yaw)) * cos(glm::radians(camera_pitch));

        this->camera_forward = glm::normalize(front);
        this->camera_right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
        this->camera_up = glm::normalize(glm::cross(camera_right, front));
    }

    void Camera::SetFOV(float FOV)
    {
        camera_fov = FOV;

        UpdateProjection();
    }

    void Camera::SetAngles(float yaw, float pitch)
    {
        this->camera_yaw = yaw;
        this->camera_pitch = pitch;
    }

    void Camera::AddAngles(float yaw, float pitch)
    {
        this->camera_yaw = camera_yaw + yaw;
        this->camera_pitch = camera_pitch + pitch;
    }

    void Camera::OnResolutionChange(int width, int height)
    {
        screen_width = width;
        screen_height = height;

        UpdateProjection();
    }

    void Camera::UpdateProjection()
    {
        camera_aspect = (float)screen_width / (float)screen_height;
        projection = glm::perspective(glm::radians(camera_fov), camera_aspect, 0.1f, 1000.0f);

        glBindBuffer(GL_UNIFORM_BUFFER, uniformBufferID);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    void Camera::InitializeUniform()
    {
        unsigned int buffer_size = (sizeof(glm::mat4) * 2) + sizeof(glm::vec3);

        glGenBuffers(1, &uniformBufferID);

        glBindBuffer(GL_UNIFORM_BUFFER, uniformBufferID);
        glBufferData(GL_UNIFORM_BUFFER, buffer_size, NULL, GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glBindBufferRange(GL_UNIFORM_BUFFER, 0, uniformBufferID, 0, buffer_size);
    }

    void Camera::BindViewBuffer(unsigned int shaderID)
    {
        unsigned int index = glGetUniformBlockIndex(shaderID, "Matrices");
        glUniformBlockBinding(shaderID, index, 0);
    }

    void Camera::Dispose()
    {
        glDeleteBuffers(1, &uniformBufferID);
    }


    void Camera::UpdateFrustumFromCamera(float aspect, float fovY, float zNear, float zFar)
    {
        const float halfVSide = zFar * tanf(fovY * .5f);
        const float halfHSide = halfVSide * aspect;
        const glm::vec3 frontMultFar = zFar * camera_forward;

        frustum.nearFace = { camera_position + zNear * camera_forward, camera_forward };
        frustum.farFace = { camera_position + frontMultFar, -camera_forward };
        frustum.rightFace = { camera_position, glm::cross(camera_up, frontMultFar + camera_right * halfHSide) };
        frustum.leftFace = { camera_position, glm::cross(frontMultFar - camera_right * halfHSide, camera_up) };
        frustum.topFace = { camera_position, glm::cross(camera_right, frontMultFar - camera_up * halfVSide) };
        frustum.bottomFace = { camera_position, glm::cross(frontMultFar + camera_up * halfVSide, camera_right) };
    }

    Frustum* Camera::GetViewFrustum()
    {
        return &frustum;
    }
