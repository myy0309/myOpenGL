#include <iostream>

#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// glut and glui
#include <GL/glut.h>
#include<gl/gl.h>
#include<gl/GLU.h>
#include <GL/glui.h>

// Other Libs
#include <SOIL2/SOIL2.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map>
#include <vector>

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
unsigned int loadTexture(char const* path);
void feedLightPoint(Shader* shader, LightPoint pointLight, std::string lightNum);
void feedLightDir(Shader* shader, LightDirectional directionalLight);
void loadFloorMaterial(Shader* shader, Material* material);
unsigned int loadCubemap(std::vector<const GLchar*> faces);
Material* initMaterialPath(Shader* shader, char const* difTextureName, char const* speTextureName);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;
// Factors allow user to scale the house model
float scaleWidth = 1.0f;
float scaleLength = 1.0f;
// Factors allow user to move the house model
float deltaX = 1.0f;
float deltaZ = 1.0f;
// Factors allow user to choose room types
enum roomType{wood, carpet, marble} myRoom;
Material *currentMaterial;
glm::mat4 model = glm::mat4(1.0f);
// Information comfirmed rooms
std::vector<glm::mat4> modelArray;
std::vector< Material*> materialArray;
// Positions of point lights
glm::vec3 pointLightPositions[] = {
    glm::vec3(1.0f, 0.4f, -1.2f), // lamp in the living room
    glm::vec3(-4.0f, 0.3f, 1.3f), // lamp in the bed room
};

#pragma region Camera Declare
Camera camera(glm::vec3(3.0f, 4.0f, 8.0f));
bool firstMouse = true;
GLfloat lastX = WIDTH / 2.0, lastY = HEIGHT / 2.0;
// Delta time
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame
// Record which key is pressed
bool keys[1024];
#pragma endregion
#pragma region Light Declare
LightDirectional directionalLight = LightDirectional(glm::vec3(0.2f, 1.0f, -0.3f), 0.5f, 0.4f, 0.5f);
LightPoint pointLight1 = LightPoint(pointLightPositions[0], 0.05f, 0.8f, 1.0f);
LightPoint pointLight2 = LightPoint(pointLightPositions[1], 0.05f, 0.8f, 1.0f);
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
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // GLFW Options
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // disable mouse cursor

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Define the viewport dimensions
    glViewport(0, 0, WIDTH, HEIGHT);

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    // Setup OpenGL options
    glEnable(GL_DEPTH_TEST); // enable depth buffer
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#pragma endregion

#pragma region operation instructions
    std::cout << "Use 'I''K''J''L' to change position of room " << std::endl;
    std::cout << "Use direction keys to change size of room " << std::endl;
    std::cout << "Use number keys to choose a texture for room " << std::endl;
    std::cout << "Click left key of mouse to confirm" << std::endl;
    std::cout << "Click right key of mouse to delete the last room" << std::endl;
    std::cout << " " << std::endl;
#pragma endregion

#pragma region Init Shader Program
    // Build and compile our shader program
    Shader ourShader(".\\src\\shaders\\VertexShader.vs", ".\\src\\shaders\\FragmentShader.frag");
    Shader windowShader(".\\src\\shaders\\windowVertexShader.vs", ".\\src\\shaders\\windowFragmentShader.frag");
    Shader skyboxShader(".\\src\\shaders\\skybox.vert", ".\\src\\shaders\\skybox.frag");
#pragma endregion

#pragma region Init Material for house structure
    Material* myMaterial = initMaterialPath(&ourShader, "roughWall2.jpg", "roughWall_gray2.jpg");
    Material* woodFloorMaterial = initMaterialPath(&ourShader, "wood_floor_big.jpg", "wood_floor_spec_big.jpg");
    Material* tileFloorMaterial = initMaterialPath(&ourShader, "011923501147_0istockphoto.jpg", "011923501147_0istockphoto.jpg");
    Material* livingroomFloorMaterial = initMaterialPath(&ourShader, "carpetagpxmrn.jpg", "roughWall_gray2.jpg");
#pragma endregion

#pragma region Model Data
    float x = 1.5; // length of the house
    float z = 1.5; // width of the house
    float y = 0.5; // height of the house
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
         x, -y,  z,  0.0f,  0.0f, 1.0f,  1.0f, 0.0f,
         x,  y,  z,  0.0f,  0.0f, 1.0f,  1.0f, 1.0f,
         x,  y,  z,  0.0f,  0.0f, 1.0f,  1.0f, 1.0f,
        -x,  y,  z,  0.0f,  0.0f, 1.0f,  0.0f, 1.0f,
        -x, -y,  z,  0.0f,  0.0f, 1.0f,  0.0f, 0.0f,
        // left face
        -x,  y,  z,  -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -x,  y, -z,  -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -x, -y, -z,  -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -x, -y, -z,  -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -x, -y,  z,  -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -x,  y,  z,  -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
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
#pragma region skybox vertices
    float skyX = 50.0f;
    float skyY = 50.0f;
    float skyZ = 50.0f;
    GLfloat skyboxVertices[] = {
        // Positions          
        -skyX,  skyY, -skyZ,
        -skyX, -skyY, -skyZ,
         skyX, -skyY, -skyZ,
         skyX, -skyY, -skyZ,
         skyX,  skyY, -skyZ,
        -skyX,  skyY, -skyZ,

        -skyX, -skyY,  skyZ,
        -skyX, -skyY, -skyZ,
        -skyX,  skyY, -skyZ,
        -skyX,  skyY, -skyZ,
        -skyX,  skyY,  skyZ,
        -skyX, -skyY,  skyZ,

         skyX, -skyY, -skyZ,
         skyX, -skyY,  skyZ,
         skyX,  skyY,  skyZ,
         skyX,  skyY,  skyZ,
         skyX,  skyY, -skyZ,
         skyX, -skyY, -skyZ,

        -skyX, -skyY,  skyZ,
        -skyX,  skyY,  skyZ,
         skyX,  skyY,  skyZ,
         skyX,  skyY,  skyZ,
         skyX, -skyY,  skyZ,
        -skyX, -skyY,  skyZ,

        -skyX,  skyY, -skyZ,
         skyX,  skyY, -skyZ,
         skyX,  skyY,  skyZ,
         skyX,  skyY,  skyZ,
        -skyX,  skyY,  skyZ,
        -skyX,  skyY, -skyZ,

        -skyX, -skyY, -skyZ,
        -skyX, -skyY,  skyZ,
         skyX, -skyY, -skyZ,
         skyX, -skyY, -skyZ,
        -skyX, -skyY,  skyZ,
         skyX, -skyY,  skyZ
    };
#pragma endregion
#pragma endregion

#pragma region Init and Load Models to VAO, VBO
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
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

    unsigned int floorVBO, floorVAO;
    glGenBuffers(1, &floorVBO);
    glGenVertexArrays(1, &floorVAO);
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

    // Setup skybox VAO
    GLuint skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glBindVertexArray(0);
#pragma endregion

#pragma region set background images skybox
    // Cubemap (Skybox)
    std::vector<const GLchar*> faces;
    faces.push_back("..\\res\\textures\\winter\\Backyard\\posxFlip.jpg");
    faces.push_back("..\\res\\textures\\winter\\Backyard\\negxFlip.jpg");
    faces.push_back("..\\res\\textures\\winter\\Backyard\\posy.jpg");
    faces.push_back("..\\res\\textures\\winter\\Backyard\\negy.jpg");
    faces.push_back("..\\res\\textures\\winter\\Backyard\\negzFlip.jpg");
    faces.push_back("..\\res\\textures\\winter\\Backyard\\poszFlip.jpg");
    unsigned int cubemapTexture = loadCubemap(faces);
#pragma endregion

    // Game loop
    while (!glfwWindowShouldClose(window))
    {
#pragma region initialization settings
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
#pragma endregion

        // Activate shader
        ourShader.Use();
#pragma region Lighting Setting
        // Pass light information to vertex shader so that we can calculate the lighting conditions
        GLint viewPosLoc = glGetUniformLocation(ourShader.Program, "viewPos");
        glUniform3f(viewPosLoc, camera.Position.x, camera.Position.y, camera.Position.z);
        // Directional light
        feedLightDir(&ourShader, directionalLight);
        // Point light 1, 2
        feedLightPoint(&ourShader, pointLight1, "0");
        feedLightPoint(&ourShader, pointLight2, "1");
#pragma endregion

#pragma region Draw Skybox
        // Create transformations
        // initialize transform matrix
        model = glm::mat4(1.0f);
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);
        // Draw skybox last
        //glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        glDepthMask(GL_FALSE);
        skyboxShader.Use();
        //model = glm::scale(model, glm::vec3(0.0, 0.5, 0.0));
        //model = glm::translate(model, glm::vec3(0.0, 8, 0.0));
        projection = glm::perspective(camera.Zoom, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);
        view = camera.GetViewMatrix();
        //view = glm::mat4(glm::mat3(camera.GetViewMatrix()));	// Remove any translation component of the view matrix
        glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(glGetUniformLocation(ourShader.Program, "skybox"), 0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthMask(GL_TRUE);
        // glDepthFunc(GL_LESS); // set depth function back to default
#pragma endregion

#pragma region draw the comfirmed part
        ourShader.Use();
        model = glm::mat4(1.0f);
        view = glm::mat4(1.0f);
        projection = glm::mat4(1.0f);
        // construct transform matrix
        view = camera.GetViewMatrix();
        projection = glm::perspective(camera.Zoom, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);
        // Get their uniform location
        GLint modelLoc = glGetUniformLocation(ourShader.Program, "model");
        GLint viewLoc = glGetUniformLocation(ourShader.Program, "view");
        GLint projLoc = glGetUniformLocation(ourShader.Program, "projection");
        // Pass them to the shaders
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        for (int i = 0; i < modelArray.size(); i++)
            /*for (std::vector<glm::mat4>::iterator iter = modelArray.begin(); iter != modelArray.end(); iter++)*/
        {
            model = modelArray[i];
            loadFloorMaterial(&ourShader, materialArray[i]);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            // Draw floor
            glBindVertexArray(floorVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
            // Draw walls
            loadFloorMaterial(&ourShader, myMaterial);
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 24);
            glBindVertexArray(0);
        }
#pragma endregion

#pragma region Prepare Model, View, Proj Matrix of house structure
        // construct transform matrix
        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(scaleLength, 1.0, scaleWidth));
        model = glm::translate(model, glm::vec3(deltaX, 1.0, deltaZ));
        // Pass them to the shaders
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
#pragma endregion

#pragma region Load Textures for house walls and draw them
        loadFloorMaterial(&ourShader, myMaterial);
        // Draw walls
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 24);
        glBindVertexArray(0);
#pragma endregion

#pragma region Load Textures for house wood floor and draw it
        // Load textures for floor sccording to user's need
        switch (myRoom)
        {
        case wood:
            currentMaterial = woodFloorMaterial; 
            break;
        case carpet:
            currentMaterial = livingroomFloorMaterial;
            break;
        case marble:
            currentMaterial = tileFloorMaterial;
            break;
        default:
            break;
        }
        // Draw floor
        loadFloorMaterial(&ourShader, currentMaterial);
        glBindVertexArray(floorVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
#pragma endregion

//#pragma region draw the comfirmed part
//        for (int i = 0 ; i < modelArray.size(); i++)
//        /*for (std::vector<glm::mat4>::iterator iter = modelArray.begin(); iter != modelArray.end(); iter++)*/
//        {
//            //std::cout << modelArray.size() << std::endl;
//            model = modelArray[i];
//            loadFloorMaterial(&ourShader, materialArray[i]);
//            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
//            // Draw floor
//            glBindVertexArray(floorVAO);
//            glDrawArrays(GL_TRIANGLES, 0, 6);
//            glBindVertexArray(0);
//            // Draw walls
//            loadFloorMaterial(&ourShader, myMaterial);
//            glBindVertexArray(VAO);
//            glDrawArrays(GL_TRIANGLES, 0, 24);
//            glBindVertexArray(0);
//        }
//#pragma endregion

        // Swap the screen buffers
        glfwSwapBuffers(window);
    }

#pragma region delete VAO & VBO
    // Properly de-allocate all resources once they've outlived their purpose
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &floorVAO);
    glDeleteBuffers(1, &floorVBO);;
#pragma endregion

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
    // scale the house model
    if (keys[GLFW_KEY_UP])
        scaleWidth = scaleWidth + 0.15 * deltaTime;
    if (keys[GLFW_KEY_DOWN])
        scaleWidth = scaleWidth - 0.15 * deltaTime;
    if (keys[GLFW_KEY_LEFT])
        scaleLength = scaleLength - 0.15 * deltaTime;
    if (keys[GLFW_KEY_RIGHT])
        scaleLength = scaleLength + 0.15 * deltaTime;
    // move the house model
    if (keys[GLFW_KEY_I])
        deltaZ = deltaZ - 0.4 * deltaTime;
    if (keys[GLFW_KEY_K])
        deltaZ = deltaZ + 0.4 * deltaTime;
    if (keys[GLFW_KEY_J])
        deltaX = deltaX - 0.4 * deltaTime;
    if (keys[GLFW_KEY_L])
        deltaX = deltaX + 0.4 * deltaTime;
    // choose floor texture
    if (keys[GLFW_KEY_1])
        myRoom = wood;
    if (keys[GLFW_KEY_2])
        myRoom = carpet;
    if (keys[GLFW_KEY_3])
        myRoom = marble;
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

// Use mouse click to add or delete new rooms
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (action == GLFW_PRESS) switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
    {
        modelArray.push_back(model);
        materialArray.push_back(currentMaterial);
        std::cout << "finish a room" << std::endl;
        break;;
    }
    case GLFW_MOUSE_BUTTON_RIGHT:
    {
        modelArray.pop_back();
        materialArray.pop_back();
        std::cout << "delete a room" << std::endl;
        break;
    }
    default:
        return;
    }
    return;
}
#pragma endregion

#pragma region load texture to GPU
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
unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
// construct the texture material texture
Material* initMaterialPath(Shader* shader, char const* difTextureName, char const* speTextureName)
{
    std::string difTexturePath = std::string("..\\res\\textures\\") + difTextureName;
    std::string speTexturePath = std::string("..\\res\\textures\\") + speTextureName;
    Material* myMaterial = new Material(shader,
        loadImageToGPU(difTexturePath.c_str(), GL_RGB, GL_RGB, (*shader).DIFFUSE),
        loadImageToGPU(speTexturePath.c_str(), GL_RGB, GL_RGB, (*shader).SPECULAR),
        32.0f
    );
    return myMaterial;
}
#pragma endregion

#pragma region feed light parameters
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
    glUniform3f(glGetUniformLocation(shader->Program, namePos.c_str()), pointLight.position.x, pointLight.position.y, pointLight.position.z);
    glUniform3f(glGetUniformLocation(shader->Program, nameAmbient.c_str()), pointLight.ambient.x, pointLight.ambient.y, pointLight.ambient.z);
    glUniform3f(glGetUniformLocation(shader->Program, nameDiffuse.c_str()), pointLight.diffuse.x, pointLight.diffuse.y, pointLight.diffuse.z);
    glUniform3f(glGetUniformLocation(shader->Program, nameSpecular.c_str()), pointLight.specular.x, pointLight.specular.y, pointLight.specular.z);
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
#pragma endregion

#pragma region load floor material
void loadFloorMaterial(Shader* shader, Material* material) {
    // Pass material information to shader
    GLint matShineLoc = glGetUniformLocation(shader->Program, "material.shininess");
    glUniform1f(matShineLoc, material->shininess);
    // Pass diffuse map information to fragment shader
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, material->diffuse);
    glUniform1i(glGetUniformLocation(shader->Program, "material.diffuse"), shader->DIFFUSE);
    // Pass specular map information to fragment shader
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, material->specular);
    glUniform1i(glGetUniformLocation(shader->Program, "material.specular"), shader->SPECULAR);
}
#pragma endregion

#pragma region skybox configuration
// Loads a cubemap texture from 6 individual texture faces
// Order should be:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front) 
// -Z (back)
unsigned int loadCubemap(std::vector<const GLchar*> faces)
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    glActiveTexture(GL_TEXTURE0);

    int width, height;
    unsigned char* image;

    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    for (GLuint i = 0; i < faces.size(); i++)
    {
        image = SOIL_load_image(faces[i], &width, &height, 0, SOIL_LOAD_RGB);
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
            GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image
        );
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return textureID;
}
#pragma endregion