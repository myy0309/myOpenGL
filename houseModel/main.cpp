#include <iostream>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// Other Libs
#include <SOIL2/SOIL2.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Other includes
#include "Shader.h"
#include "Camera.h"
#include "Mesh.h"
#include "Model.h"
#include "Material.h"
#include "LightDirectional.h"
#include "LightPoint.h"
#include "stb_image.h"


// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void do_movement();
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
unsigned int loadImageToGPU(const char* filename, GLuint internalFormat, GLenum format, int textureslot);
void feedLightPoint(Shader* shader, LightPoint pointLight, std::string lightNum);
void feedLightDir(Shader* shader, LightDirectional directionalLight);
// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;

// Positions of point lights
glm::vec3 pointLightPositions[] = {
    glm::vec3(0.7f,  0.2f,  2.0f),
    glm::vec3(1.0f, 1.0f, 0.0f),
    glm::vec3(-0.5f,  1.0f, 0.0f),
    glm::vec3(0.0f,  0.0f, -3.0f)
};

#pragma region Camera Declare
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
bool firstMouse = true;
GLfloat lastX = WIDTH / 2.0, lastY = HEIGHT / 2.0;
// Delta time
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame
// Record which key is pressed
bool keys[1024];
#pragma endregion
#pragma region Light Declare
//LightDirectional directionalLight = LightDirectional(glm::vec3(-0.2f, -1.0f, -0.3f), glm::vec3(0.05f, 0.05f, 0.05f), glm::vec3(0.4f, 0.4f, 0.4f), glm::vec3(0.5f, 0.5f, 0.5f));
//LightDirectional directionalLight = LightDirectional(glm::vec3(0.2f, 1.0f, -0.3f), 0.05f, 0.4f, 0.5f);
LightDirectional directionalLight = LightDirectional(glm::vec3(0.2f, 1.0f, -0.3f), 0.5f, 0.4f, 0.5f);
LightPoint pointLight1 = LightPoint(glm::vec3(0.7f, 0.2f, 2.0f), 0.05f, 0.8f, 1.0f);
LightPoint pointLight2 = LightPoint(glm::vec3(2.3f, -3.3f, -4.0f), 0.05f, 0.8f, 1.0f);
LightPoint pointLight3 = LightPoint(glm::vec3(-4.0f, 2.0f, -12.0f), 0.05f, 0.8f, 1.0f);
LightPoint pointLight4 = LightPoint(glm::vec3(0.0f, 0.0f, -3.0f), 0.05f, 0.8f, 1.0f);
#pragma endregion

// The MAIN function, from here we start the application and run the game loop
int main()
{
#pragma region Open a Window
    // Init GLFW
    glfwInit();
    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // Create a GLFWwindow object that we can use for GLFW's functions
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "house model", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // Set the required callback functions
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // GLFW Options
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
    glewExperimental = GL_TRUE;
    // Initialize GLEW to setup the OpenGL Function pointers
    glewInit();

    // Define the viewport dimensions
    glViewport(0, 0, WIDTH, HEIGHT);

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    // Setup OpenGL options
    glEnable(GL_DEPTH_TEST); // enable depth buffer
#pragma endregion

#pragma region Init Shader Program
    // Build and compile our shader program
    Shader ourShader(".\\src\\shaders\\VertexShader.vs", ".\\src\\shaders\\FragmentShader.frag");
    Shader lightShader(".\\src\\shaders\\lightVertexShader.vs", ".\\src\\shaders\\lightFragmentShader.frag");
#pragma endregion

#pragma region Init Material for house structure
    Material* myMaterial = new Material(&ourShader,
        loadImageToGPU("..\\res\\textures\\roughWall3.jpg", GL_RGB, GL_RGB, ourShader.DIFFUSE),
        loadImageToGPU("..\\res\\textures\\roughWall_gray.jpg", GL_RGB, GL_RGB, ourShader.SPECULAR),
        32.0f
    );
    Material* floorMaterial = new Material(&ourShader,
        loadImageToGPU("..\\res\\textures\\wood_floor_big.jpg", GL_RGB, GL_RGB, ourShader.DIFFUSE),
        loadImageToGPU("..\\res\\textures\\wood_floor_spec_big.jpg", GL_RGB, GL_RGB, ourShader.SPECULAR),
        32.0f
    );
#pragma endregion

#pragma region Model Data
    float x = 2.2; // length of the house
    float z = 0.8; // width of the house
    float y = 0.25; // height of the house
    float bedroomDoorPos = -0.3;
    float frontDoorPos = 0.15;
    float diningDoorPos = 0.06;
    float leftWallPos = 0.3; 
    float rightWallPos = 0.5;
    float widthOfDoor = 0.3;
    GLfloat vertices[] = {
        // back face
        -x, -y, -z,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         x, -y, -z,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
         x,  y, -z,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
         x,  y, -z,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -x,  y, -z,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
        -x, -y, -z,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
        // front face
        -x, -y,  z,  0.0f,  0.0f, 1.0f,  0.0f, 0.0f,
         x * frontDoorPos, -y,  z,  0.0f,  0.0f, 1.0f,  1.0f, 0.0f,
         x * frontDoorPos,  y,  z,  0.0f,  0.0f, 1.0f,  1.0f, 1.0f,
         x * frontDoorPos,  y,  z,  0.0f,  0.0f, 1.0f,  1.0f, 1.0f,
        -x,  y,  z,  0.0f,  0.0f, 1.0f,  0.0f, 1.0f,
        -x, -y,  z,  0.0f,  0.0f, 1.0f,  0.0f, 0.0f,

         x * frontDoorPos + widthOfDoor, -y,  z,  0.0f,  0.0f, 1.0f,  0.0f, 0.0f,
         x, -y,  z,  0.0f,  0.0f, 1.0f,  1.0f, 0.0f,
         x,  y,  z,  0.0f,  0.0f, 1.0f,  1.0f, 1.0f,
         x,  y,  z,  0.0f,  0.0f, 1.0f,  1.0f, 1.0f,
         x * frontDoorPos + widthOfDoor,  y,  z,  0.0f,  0.0f, 1.0f,  0.0f, 1.0f,
         x * frontDoorPos + widthOfDoor, -y,  z,  0.0f,  0.0f, 1.0f,  0.0f, 0.0f,
        // left face
        -x,  y,  z,  -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -x,  y, -z,  -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -x, -y, -z,  -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -x, -y, -z,  -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -x, -y,  z,  -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -x,  y,  z,  -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        // left partition wall |
        -x * leftWallPos,  y,  z * bedroomDoorPos,  -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -x * leftWallPos,  y, -z,  -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -x * leftWallPos, -y, -z,  -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -x * leftWallPos, -y, -z,  -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -x * leftWallPos, -y,  z * bedroomDoorPos,  -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -x * leftWallPos,  y,  z * bedroomDoorPos,  -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

        -x * leftWallPos,  y,  z,  -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -x * leftWallPos,  y,  z * bedroomDoorPos + widthOfDoor,  -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -x * leftWallPos, -y,  z * bedroomDoorPos + widthOfDoor,  -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -x * leftWallPos, -y,  z * bedroomDoorPos + widthOfDoor,  -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -x * leftWallPos, -y,  z,  -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -x * leftWallPos,  y,  z,  -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        // right partition wall -
         -x, -y,  z* bedroomDoorPos,  0.0f,  0.0f, 1.0f,  0.0f, 0.0f,
         -x * leftWallPos - widthOfDoor, -y,  z* bedroomDoorPos,  0.0f,  0.0f, 1.0f,  1.0f, 0.0f,
         -x * leftWallPos - widthOfDoor,  y,  z* bedroomDoorPos,  0.0f,  0.0f, 1.0f,  1.0f, 1.0f,
         -x * leftWallPos - widthOfDoor,  y,  z* bedroomDoorPos,  0.0f,  0.0f, 1.0f,  1.0f, 1.0f,
         -x,  y,  z* bedroomDoorPos,  0.0f,  0.0f, 1.0f,  0.0f, 1.0f,
         -x, -y,  z* bedroomDoorPos,  0.0f,  0.0f, 1.0f,  0.0f, 0.0f,
        // right partition wall |
        x * rightWallPos,  y,  z* diningDoorPos,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        x * rightWallPos,  y, -z,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        x * rightWallPos, -y, -z,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        x * rightWallPos, -y, -z,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        x * rightWallPos, -y,  z* diningDoorPos,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        x * rightWallPos,  y,  z* diningDoorPos,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

        x * rightWallPos,  y,  z,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        x * rightWallPos,  y,  z* diningDoorPos + widthOfDoor,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        x * rightWallPos, -y,  z* diningDoorPos + widthOfDoor,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        x * rightWallPos, -y,  z* diningDoorPos + widthOfDoor,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        x * rightWallPos, -y,  z,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        x * rightWallPos,  y,  z,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        // right partition wall -
         x, -y,  z* diningDoorPos + 0.5 * widthOfDoor,  0.0f,  0.0f, 1.0f,  0.0f, 0.0f,
         x* rightWallPos, -y,  z* diningDoorPos + 0.5 * widthOfDoor,  0.0f,  0.0f, 1.0f,  1.0f, 0.0f,
         x* rightWallPos,  y,  z* diningDoorPos + 0.5 * widthOfDoor,  0.0f,  0.0f, 1.0f,  1.0f, 1.0f,
         x* rightWallPos,  y,  z* diningDoorPos + 0.5 * widthOfDoor,  0.0f,  0.0f, 1.0f,  1.0f, 1.0f,
         x,  y,  z* diningDoorPos + 0.5 * widthOfDoor,  0.0f,  0.0f, 1.0f,  0.0f, 1.0f,
         x, -y,  z* diningDoorPos + 0.5 * widthOfDoor,  0.0f,  0.0f, 1.0f,  0.0f, 0.0f,
         // right face
         x,  y,  z,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         x,  y, -z,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
         x, -y, -z,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         x, -y, -z,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         x, -y,  z,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
         x,  y,  z,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
    };
    GLfloat floorVertice[] = {
        // bottom face
        -x, -y, -z,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
         x, -y, -z,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
         x, -y,  z,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
         x, -y,  z,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        -x, -y,  z,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
        -x, -y, -z,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
    };
#pragma endregion
    Model woodChair(".\\Debug\\tableAndChair\\seat.obj");
    Model woodTable(".\\Debug\\tableAndChair\\table.obj"); 
    Model sideTable(".\\Debug\\sideTable\\Liam_Side_Table_by_Minotti.obj");
    Model leatherSofa(".\\Debug\\leatherSofa\\export\\Armchair_Monti_156__corona.obj"); 
    Model bed(".\\Debug\\simpleBed\\file.obj");
    Model bigWindow(".\\Debug\\Window\\window_big.obj");
    Model kitchenSet(".\\Debug\\kitchenSet8\\file.obj");
    Model washBasin(".\\Debug\\washBasin\\file.obj");
    Model toilet(".\\Debug\\toilet\\obj.obj");
    Model bathTube(".\\Debug\\bathTube\\obj.obj");
    Model sofaSet(".\\Debug\\sofaSet\\file.obj");

#pragma region Init and Load Models to VAO, VBO
    GLuint VBO, floorVBO, VAO, floorVAO, lightVAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &floorVBO);
    glGenVertexArrays(1, &lightVAO);
    glGenVertexArrays(1, &floorVAO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    // diffuse texture attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0); // Unbind VAO

    glBindVertexArray(lightVAO);
    // we only need to bind to the VBO
    // Since the object's VBO's data already contains the data, no need to buffer data
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // set the vertex attribute 
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0); // Unbind VAO

    glBindVertexArray(floorVAO);
    glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertice), floorVertice, GL_STATIC_DRAW);
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    // diffuse texture attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0); // Unbind VAO
#pragma endregion


    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        // Calculate deltatime of current frame
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
        glfwPollEvents();
        do_movement();

        // Render
        // Clear the colorbuffer
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Activate shader
        ourShader.Use();
#pragma region Lighting Setting
        // Pass light information to vertex shader so that we can calculate the lighting conditions
        GLint viewPosLoc = glGetUniformLocation(ourShader.Program, "viewPos");
        glUniform3f(viewPosLoc, camera.Position.x, camera.Position.y, camera.Position.z);
        // Directional light
        feedLightDir(&ourShader, directionalLight);
        // Point light 1, 2, 3, 4
        feedLightPoint(&ourShader, pointLight1, "0");
        feedLightPoint(&ourShader, pointLight2, "1");
        feedLightPoint(&ourShader, pointLight3, "2");
        feedLightPoint(&ourShader, pointLight4, "3");
#pragma endregion

#pragma region Prepare Model, View, Proj Matrix of house structure
        // Create transformations
        // initialize transform matrix
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);
        // construct transform matrix
        model = glm::scale(model, glm::vec3(2, 2, 2));
        // construct transform matrix
        view = camera.GetViewMatrix();
        projection = glm::perspective(camera.Zoom, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);
        // Get their uniform location
        GLint modelLoc = glGetUniformLocation(ourShader.Program, "model");
        GLint viewLoc = glGetUniformLocation(ourShader.Program, "view");
        GLint projLoc = glGetUniformLocation(ourShader.Program, "projection");
        // Pass them to the shaders
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));  
#pragma endregion
#pragma region Load Textures for house structure
        // Pass material information to shader
        GLint matShineLoc = glGetUniformLocation(ourShader.Program, "material.shininess");
        glUniform1f(matShineLoc, myMaterial->shininess);
        // Pass diffuse map information to fragment shader
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, myMaterial->diffuse);
        glUniform1i(glGetUniformLocation(ourShader.Program, "material.diffuse"), ourShader.DIFFUSE);
        // Pass specular map information to fragment shader
        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_2D, myMaterial->specular);
        glUniform1i(glGetUniformLocation(ourShader.Program, "material.specular"), ourShader.SPECULAR);
#pragma endregion
        // Draw walls
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 72);
        glBindVertexArray(0);
#pragma region Load Textures for house floor
        // Pass material information to shader
        matShineLoc = glGetUniformLocation(ourShader.Program, "material.shininess");
        glUniform1f(matShineLoc, floorMaterial->shininess);
        // Pass diffuse map information to fragment shader
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorMaterial->diffuse);
        glUniform1i(glGetUniformLocation(ourShader.Program, "material.diffuse"), ourShader.DIFFUSE);
        // Pass specular map information to fragment shader
        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_2D, floorMaterial->specular);
        glUniform1i(glGetUniformLocation(ourShader.Program, "material.specular"), ourShader.SPECULAR);
#pragma endregion
        // Draw walls
        glBindVertexArray(floorVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

#pragma region Prepare Model, View, Proj Matrix for wood table
        // Create transformations
        // initialize transform matrix
        glm::mat4 model1 = glm::mat4(1.0f);
        // construct transform matrix
        model1 = glm::scale(model1, glm::vec3(0.5, 0.5, 0.5));
        model1 = glm::translate(model1, glm::vec3(7.0, -1.0, -0.5));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1));
#pragma endregion
        // Draw object
        woodTable.Draw(&ourShader);

#pragma region Prepare Model, View, Proj Matrix for side table
        // Create transformations
        // initialize transform matrix
        glm::mat4 model2 = glm::mat4(1.0f);
        // construct transform matrix
        model2 = glm::scale(model2, glm::vec3(0.5, 0.5, 0.5));
        model2 = glm::translate(model2, glm::vec3(5.8, -1.0, -0.5));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model2));
#pragma endregion
        // Draw object
        woodChair.Draw(&ourShader);

#pragma region Prepare Model, View, Proj Matrix for sofa
        // Create transformations
        // initialize transform matrix
        glm::mat4 model3 = glm::mat4(1.0f);
        // construct transform matrix
        model3 = glm::scale(model3, glm::vec3(0.5, 0.5, 0.5));
        model3 = glm::translate(model3, glm::vec3(-3.3, -1.0, 1.5));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model3));
#pragma endregion
        // Draw object
        sideTable.Draw(&ourShader);

#pragma region Prepare Model, View, Proj Matrix for single sofa
        // Create transformations
        // initialize transform matrix
        glm::mat4 model4 = glm::mat4(1.0f);
        // construct transform matrix
        model4 = glm::scale(model4, glm::vec3(0.005, 0.005, 0.005));
        model4 = glm::translate(model4, glm::vec3(-320.0, -100.5, 250.0));
        model4 = glm::rotate(model4, glm::radians(-90.0f), glm::vec3(0.0, 1.0, 0.0));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model4));
#pragma endregion
        // Draw object
        leatherSofa.Draw(&ourShader);

#pragma region Prepare Model, View, Proj Matrix for bed
        // Create transformations
        // initialize transform matrix
        glm::mat4 model5 = glm::mat4(1.0f);
        // construct transform matrix
        model5 = glm::scale(model5, glm::vec3(0.0007, 0.0007, 0.0007));
        model5 = glm::translate(model5, glm::vec3(-5500.0, -800.5, 900.0));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model5));
#pragma endregion
        // Draw object
        bed.Draw(&ourShader);

#pragma region Prepare Model, View, Proj Matrix for window
        // Create transformations
        // initialize transform matrix
        glm::mat4 model6 = glm::mat4(1.0f);
        // construct transform matrix
        model6 = glm::scale(model6, glm::vec3(0.61, 0.5, 0.5));
        model6 = glm::translate(model6, glm::vec3(-4.7, -1.0, -3.2));
        model6 = glm::rotate(model6, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model6));
#pragma endregion
        // Draw object
        bigWindow.Draw(&ourShader);

#pragma region Prepare Model, View, Proj Matrix for kitchen set
        // Create transformations
        // initialize transform matrix
        glm::mat4 model7 = glm::mat4(1.0f);
        // construct transform matrix
        model7 = glm::scale(model7, glm::vec3(0.0005, 0.0005, 0.0005));
        model7 = glm::translate(model7, glm::vec3(7000.0, -900.0, -2800.0));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model7));
#pragma endregion
        // Draw object
        kitchenSet.Draw(&ourShader);

#pragma region Prepare Model, View, Proj Matrix for wash basin
        // Create transformations
        // initialize transform matrix
        glm::mat4 model8 = glm::mat4(1.0f);
        // construct transform matrix
        model8 = glm::scale(model8, glm::vec3(0.0007, 0.0007, 0.0007));
        model8 = glm::translate(model8, glm::vec3(5700.0, -700.0, 850.0));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model8));
#pragma endregion
        // Draw object
        washBasin.Draw(&ourShader);

#pragma region Prepare Model, View, Proj Matrix for toilet
        // Create transformations
        // initialize transform matrix
        glm::mat4 model9 = glm::mat4(1.0f);
        // construct transform matrix
        model9 = glm::scale(model9, glm::vec3(0.02, 0.02, 0.02));
        model9 = glm::translate(model9, glm::vec3(200.0, -24.0, 67.0));
        model9 = glm::rotate(model9, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model9));
#pragma endregion
        // Draw object
        toilet.Draw(&ourShader);

#pragma region Prepare Model, View, Proj Matrix for bath tube
        // Create transformations
        // initialize transform matrix
        glm::mat4 model10 = glm::mat4(1.0f);
        // construct transform matrix
        model10 = glm::scale(model10, glm::vec3(0.0006, 0.0006, 0.0006));
        model10 = glm::translate(model10, glm::vec3(5000.0, -800.0, 2100.0));
        model10 = glm::rotate(model10, glm::radians(180.0f), glm::vec3(0.0, 1.0, 0.0));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model10));
#pragma endregion
        // Draw object
        bathTube.Draw(&ourShader);

#pragma region Prepare Model, View, Proj Matrix for sofa in livingroom
        // Create transformations
        // initialize transform matrix
        glm::mat4 model11 = glm::mat4(1.0f);
        // construct transform matrix
        model11 = glm::scale(model11, glm::vec3(0.02, 0.02, 0.02));
        model11 = glm::translate(model11, glm::vec3(7.0, -26.0, -40.0));
        //model11 = glm::rotate(model11, glm::radians(180.0f), glm::vec3(0.0, 1.0, 0.0));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model11));
#pragma endregion
        // Draw object
        sofaSet.Draw(&ourShader);

        // Activate light shader
        lightShader.Use();
#pragma region Prepare Model, View, Proj Matrix for point light
        // Create transformations for light
        GLuint lightModelLoc = glGetUniformLocation(lightShader.Program, "model");
        GLuint lightViewLoc = glGetUniformLocation(lightShader.Program, "view");
        GLuint lightProjLoc = glGetUniformLocation(lightShader.Program, "projection");
        glm::mat4 lightView = glm::mat4(1.0f);
        glm::mat4 lightProjection = glm::mat4(1.0f);
        lightView = camera.GetViewMatrix();
        lightProjection = glm::perspective(camera.Zoom, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);
        // Pass them to the shaders
        glUniformMatrix4fv(lightViewLoc, 1, GL_FALSE, glm::value_ptr(lightView));
        glUniformMatrix4fv(lightProjLoc, 1, GL_FALSE, glm::value_ptr(lightProjection));
        // Draw object
        glBindVertexArray(lightVAO);
        for (GLuint i = 0; i < sizeof(pointLightPositions) / sizeof(pointLightPositions[0]); i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, pointLightPositions[i]);
            model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
            glUniformMatrix4fv(lightModelLoc, 1, GL_FALSE, glm::value_ptr(model));
            //glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glBindVertexArray(0);
#pragma endregion
        // Swap the screen buffers
        glfwSwapBuffers(window);
    }
    // Properly de-allocate all resources once they've outlived their purpose
    glDeleteVertexArrays(1, &VAO);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteBuffers(1, &VBO);
    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();
    return 0;
}

#pragma region Camera Control
// Use mouse to control the camera's pitch and yaw
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos;  // Reversed since y-coordinates go from bottom to left
    lastX = xpos;
    lastY = ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);
}

// Use mouse scroll to zoom in and zoom out
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

// Decide what to do after specific keys are pressed 
void do_movement()
{
    // Camera controls
    if (keys[GLFW_KEY_W])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A])
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D])
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (keys[GLFW_KEY_F])
        camera.ProcessKeyboard(UP, deltaTime);
    if (keys[GLFW_KEY_R])
        camera.ProcessKeyboard(DOWN, deltaTime);
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    // record which keys are pressed
    if (action == GLFW_PRESS)
        keys[key] = true;
    else if (action == GLFW_RELEASE)
        keys[key] = false;
}
#pragma endregion

// generate and bind image texture
unsigned int loadImageToGPU(const char* filename, GLuint internalFormat, GLenum format, int textureslot)
{
    unsigned int TexBuffer = 0;
    glGenTextures(1, &TexBuffer);
    glActiveTexture(GL_TEXTURE0 + textureslot);
    glBindTexture(GL_TEXTURE_2D, TexBuffer); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object
    // Set our texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	// Set texture wrapping to GL_REPEAT
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // Set texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Load, create texture and generate mipmaps
    int width, height;
    unsigned char* image = SOIL_load_image(filename, &width, &height, 0, SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    SOIL_free_image_data(image);
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.
    return TexBuffer;
}

// pass point light parameters to fragment shader
void feedLightPoint(Shader* shader, LightPoint pointLight, std::string lightNum)
{
    std::string name = std::string("pointLights[") + lightNum;
    std::string namePos = name + std::string("].position");
    std::string nameAmbient = name + std::string("].ambient");
    std::string nameDiffuse = name + std::string("].diffuse");
    std::string nameSpecular = name + std::string("].specular");
    std::string nameConstant = name + std::string("].constant");
    std::string nameLinear = name + std::string("].linear");
    std::string nameQuadratic = name + std::string("].quadratic");
    glUniform3f(glGetUniformLocation(shader->Program, namePos.c_str()), pointLight.position.x, pointLight1.position.y, pointLight1.position.z);
    glUniform3f(glGetUniformLocation(shader->Program, nameAmbient.c_str()), pointLight.ambient.x, pointLight1.ambient.y, pointLight1.ambient.z);
    glUniform3f(glGetUniformLocation(shader->Program, nameDiffuse.c_str()), pointLight.diffuse.x, pointLight1.diffuse.y, pointLight1.diffuse.z);
    glUniform3f(glGetUniformLocation(shader->Program, nameSpecular.c_str()), pointLight.specular.x, pointLight1.specular.y, pointLight1.specular.z);
    glUniform1f(glGetUniformLocation(shader->Program, nameConstant.c_str()), pointLight.constant);
    glUniform1f(glGetUniformLocation(shader->Program, nameLinear.c_str()), pointLight.linear);
    glUniform1f(glGetUniformLocation(shader->Program, nameQuadratic.c_str()), pointLight.quadratic);
}

// pass directional light parameters to fragment shader
void feedLightDir(Shader* shader, LightDirectional directionalLight)
{
    glUniform3f(glGetUniformLocation(shader->Program, "dirLight.direction"), directionalLight.direction.x, directionalLight.direction.y, directionalLight.direction.z);
    glUniform3f(glGetUniformLocation(shader->Program, "dirLight.ambient"), directionalLight.ambient.x, directionalLight.ambient.y, directionalLight.ambient.z);
    glUniform3f(glGetUniformLocation(shader->Program, "dirLight.diffuse"), directionalLight.diffuse.x, directionalLight.diffuse.y, directionalLight.diffuse.z);
    glUniform3f(glGetUniformLocation(shader->Program, "dirLight.specular"), directionalLight.specular.x, directionalLight.specular.y, directionalLight.specular.z);
}