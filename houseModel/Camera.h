#pragma once

// Std. Includes
#include <vector>
#include <algorithm>

// GL Includes
//#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>



// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

// Default camera values
const GLfloat YAW = -90.0f;
const GLfloat PITCH = 0.0f;
const GLfloat SPEED = 2.0f;
const GLfloat SENSITIVTY = 0.25f;
const GLfloat ZOOM = 45.0f;
const GLfloat zoomScaler = 0.1f;

// An abstract camera class that processes input and calculates the corresponding Eular Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
    // Camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // Eular Angles
    GLfloat Yaw;
    GLfloat Pitch;
    // Camera options
    GLfloat MovementSpeed;
    GLfloat MouseSensitivity;
    GLfloat Zoom;

    // Constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), GLfloat yaw = YAW, GLfloat pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVTY), Zoom(ZOOM)
    {
        this->Position = position;
        this->WorldUp = up;
        this->Yaw = yaw;
        this->Pitch = pitch;
        this->updateCameraVectors();
    }
    // Constructor with scalar values
    Camera(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat upX, GLfloat upY, GLfloat upZ, GLfloat yaw, GLfloat pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVTY), Zoom(ZOOM)
    {
        this->Position = glm::vec3(posX, posY, posZ);
        this->WorldUp = glm::vec3(upX, upY, upZ);
        this->Yaw = yaw;
        this->Pitch = pitch;
        this->updateCameraVectors();
    }

    // Returns the view matrix calculated using Eular Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(this->Position, this->Position + this->Front, this->Up);
    }

    // Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime)
    {
        GLfloat velocity = this->MovementSpeed * deltaTime;
        verifyRight();
        if (direction == FORWARD)
        {
            this->Position += this->Front * velocity;
            /*std::cout << "z+=" << this->Position.z << std::endl;*/
        }
        if (direction == BACKWARD)
        {
            this->Position -= this->Front * velocity;
            /*std::cout << "z-=" << this->Position.z << std::endl;*/
        }
        if (direction == LEFT)
        {
            this->Position -= this->Right * velocity;
            /*std::cout << "x left=" << this->Position.x << std::endl;*/
        }
        if (direction == RIGHT)
        {
            this->Position += this->Right * velocity;
            /*std::cout << "x right=" << this->Position.x << std::endl;*/
        }
        if (direction == UP)
            this->Position += (glm::cross(this->Front, this->Right)) * velocity;
        if (direction == DOWN)
            this->Position -= (glm::cross(this->Front, this->Right)) * velocity;
    }

    // Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= this->MouseSensitivity;
        yoffset *= this->MouseSensitivity;

        this->Yaw += xoffset;
        this->Pitch += yoffset;

        // Make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
            if (this->Pitch > 89.0f)
                this->Pitch = 89.0f;
            if (this->Pitch < -89.0f)
                this->Pitch = -89.0f;
        }

        // Update Front, Right and Up Vectors using the updated Eular angles
        this->updateCameraVectors();
    }

    // Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(GLfloat yoffset)
    {
        if (this->Zoom >= 1.0f && this->Zoom <= 45.0f)
            this->Zoom -= (float)yoffset * zoomScaler;
        if (this->Zoom < 1.0f)
            this->Zoom = 1.0f;
        if (this->Zoom > 45.0f)
            this->Zoom = 45.0f;
    }

private:
    // Calculates the front vector from the Camera's (updated) Eular Angles
    void updateCameraVectors()
    {
        // Calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
        front.y = sin(glm::radians(this->Pitch));
        front.z = sin(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
        this->Front = glm::normalize(front);
        // Also re-calculate the Right and Up vector
        this->Right = glm::normalize(glm::cross(this->Front, this->WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        this->Up = glm::normalize(glm::cross(this->Right, this->Front));
    }

    void verifyRight()
    {
        this->Position.y = clamp(this->Position.y, 0.5f, 1.0f);
        // living room
        if (this->Position.x > -1.2f && this->Position.x < 2.2f && this->Position.z < 1.4f && this->Position.z > -0.9f)
        {
            // left door
            if ( (this->Position.x < 0) && (this->Position.z > 0.15f || this->Position.z < -0.7f))
            {
                this->Position.x = clamp(this->Position.x, -1.1f, 4.2f);
            }
            // right door
            if ( (this->Position.x > 0) && this->Position.z > 0.75f || this->Position.z < 0.0f)
            {
                this->Position.x = clamp(this->Position.x, -4.2f, 2.0f);
            }
            // gate
            if (this->Position.x < 0.5f || this->Position.x > 1.25f)
            {
                this->Position.z = clamp(this->Position.z, -1.0f, 1.0f);
            }
        }
        // left part
        if (this->Position.x > -4.3f && this->Position.x < -1.2f && this->Position.z < 1.4f && this->Position.z > -0.9f)
        {
            // door
            if (this->Position.z > 0.15f || this->Position.z < -0.7f)
            {
                this->Position.x = clamp(this->Position.x, -4.2f, -1.5f);
            }
            // bedroom
            if (this->Position.z > -0.6f && this->Position.z < 1.4f && this->Position.x < -1.94f)
            {
                this->Position.z = clamp(this->Position.z, -0.35f, 1.1f);
                this->Position.x = clamp(this->Position.x, -3.7f, -1.5f);
            }
            // balcony
            if (this->Position.z < -0.6f && this->Position.z > -1.3f && this->Position.x < -1.94f)
            {
                this->Position.z = clamp(this->Position.z, -0.9f, -0.75f);
                this->Position.x = clamp(this->Position.x, -3.7f, -1.5f);
            }
        }
        // right part
        if (this->Position.x > 2.2f && this->Position.x < 4.3f && this->Position.z < 1.4f && this->Position.z > -1.3f)
        {
            //door
            if (this->Position.z > 0.75f || this->Position.z < 0.0f)
            {
                this->Position.x = clamp(this->Position.x, 2.4f, 4.2f);
            }
            // bathroom
            if (this->Position.z > 0.43f && this->Position.z < 1.4f && this->Position.x > 2.7f)
            {
                this->Position.z = clamp(this->Position.z, 0.55f, 1.0f);
                this->Position.x = clamp(this->Position.x, 2.3f, 4.1f);
            }
            // kitchen
            if (this->Position.z < 0.43f && this->Position.z > -1.3f && this->Position.x > 2.7f)
            {
                this->Position.z = clamp(this->Position.z, -0.9f, 0.25f);
                this->Position.x = clamp(this->Position.x, 2.3f, 4.1f);
                if (this->Position.z > -0.23)
                {
                    this->Position.x = clamp(this->Position.x, 2.2f, 3.63f);
                }
            }
        }
        // outside
        // left
        if (this->Position.x < - 4.2f && this->Position.z < 1.4f && this->Position.z > -1.3f)
        {
            this->Position.x = clamp(this->Position.x, -100.0f, -4.65f);
        }
        // right
        if (this->Position.x > 4.2f && this->Position.z < 1.4f && this->Position.z > -1.3f)
        {
            this->Position.x = clamp(this->Position.x, 4.65f, 100.0f);
        }
        // front
        if (this->Position.z > 1.4f && this->Position.x < 4.2f && this->Position.x > -4.2f)
        {
            // gate
            if (this->Position.x < 0.5f || this->Position.x > 1.25f)
            {
                this->Position.z = clamp(this->Position.z, 1.75f, 100.0f);
            }
        }
        // back
        if (this->Position.z < -1.3f && this->Position.x < 4.2f && this->Position.x > -4.2f)
        {
            this->Position.z = clamp(this->Position.z, -100.0f, -1.8f);
        }
        
    }

    float clamp(float value, float min, float max)
    {
        if (value < min)
        {
            value = min;
        }
        if (value > max)
        {
            value = max;
        }
        return value;
    }
};