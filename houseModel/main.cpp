#include <iostream>

#include <glad/glad.h>

//// GLEW
//#define GLEW_STATIC
//#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// Other Libs
#include <SOIL2/SOIL2.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map>

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
unsigned int loadCubemap(std::vector<const GLchar*> faces);
// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;

// Positions of point lights
glm::vec3 pointLightPositions[] = {
    glm::vec3(1.0f, 0.4f, -1.2f), // lamp in the living room
    glm::vec3(-4.0f, 0.3f, 1.3f), // lamp in the bed room
};

#pragma region Camera Declare
//Camera camera(glm::vec3(0.0f, 0.0f, 6.0f));
Camera camera(glm::vec3(0.5f, 0.0f, 1.5f));
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

    // GLFW Options
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    //// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
    //glewExperimental = GL_TRUE;
    //// Initialize GLEW to setup the OpenGL Function pointers
    //glewInit();
    
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
#pragma region Init Shader Program
    // Build and compile our shader program
    Shader ourShader(".\\src\\shaders\\VertexShader.vs", ".\\src\\shaders\\FragmentShader.frag");
    Shader windowShader(".\\src\\shaders\\windowVertexShader.vs", ".\\src\\shaders\\windowFragmentShader.frag");
    Shader skyboxShader(".\\src\\shaders\\skybox.vert", ".\\src\\shaders\\skybox.frag");
    Shader simpleDepthShader(".\\src\\shaders\\shadow_mapping_depth.vert", ".\\src\\shaders\\shadow_mapping_depth.frag");
    Shader debugDepthQuad(".\\src\\shaders\\debug_quad.vert", ".\\src\\shaders\\debug_quad_depth.frag");
    /*Shader lightShader(".\\src\\shaders\\lightVertexShader.vs", ".\\src\\shaders\\lightFragmentShader.frag");*/
#pragma endregion

#pragma region Init Material for house structure
    Material* myMaterial = new Material(&ourShader,
        loadImageToGPU("..\\res\\textures\\roughWall2.jpg", GL_RGB, GL_RGB, ourShader.DIFFUSE),
        loadImageToGPU("..\\res\\textures\\roughWall_gray2.jpg", GL_RGB, GL_RGB, ourShader.SPECULAR),
        32.0f
    );
    Material* woodFloorMaterial = new Material(&ourShader,
        loadImageToGPU("..\\res\\textures\\wood_floor_big.jpg", GL_RGB, GL_RGB, ourShader.DIFFUSE),
        loadImageToGPU("..\\res\\textures\\wood_floor_spec_big.jpg", GL_RGB, GL_RGB, ourShader.SPECULAR),
        32.0f
    );
    Material* tileFloorMaterial = new Material(&ourShader,
        loadImageToGPU("..\\res\\textures\\011923501147_0istockphoto.jpg", GL_RGB, GL_RGB, ourShader.DIFFUSE),
        loadImageToGPU("..\\res\\textures\\011923501147_0istockphoto.jpg", GL_RGB, GL_RGB, ourShader.SPECULAR),
        32.0f
    );
    Material* roofMaterial = new Material(&ourShader,
        loadImageToGPU("..\\res\\textures\\roofSquare1.jpg", GL_RGB, GL_RGB, ourShader.DIFFUSE),
        loadImageToGPU("..\\res\\textures\\roofSquare1.jpg", GL_RGB, GL_RGB, ourShader.SPECULAR),
        32.0f
    );

    std::string windowPath = "..\\res\\textures\\thickerthanwateranovel.png";
    unsigned int windowTexture = loadTexture(windowPath.c_str());
#pragma endregion

#pragma region Model Data
    float x = 2.2; // length of the house
    float z = 0.8; // width of the house
    float y = 0.25; // height of the house
    float delta = 0.2;
    float bedroomDoorPos = -0.3;
    float frontDoorPos = 0.15;
    float diningDoorPos = 0.06;
    float leftWallPos = 0.3; 
    float rightWallPos = 0.5;
    float widthOfDoor = 0.3;
    GLfloat vertices[] = {
        // right part of back face
        -x * leftWallPos, -y, -z,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         x, -y, -z,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
         x,  y + delta, -z,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
         x,  y + delta, -z,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -x * leftWallPos,  y + delta, -z,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
        -x * leftWallPos, -y, -z,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
        // front face
        -x, -y,  z,  0.0f,  0.0f, 1.0f,  0.0f, 0.0f,
         x * frontDoorPos, -y,  z,  0.0f,  0.0f, 1.0f,  1.0f, 0.0f,
         x * frontDoorPos,  y + delta,  z,  0.0f,  0.0f, 1.0f,  1.0f, 1.0f,
         x * frontDoorPos,  y + delta,  z,  0.0f,  0.0f, 1.0f,  1.0f, 1.0f,
        -x,  y + delta,  z,  0.0f,  0.0f, 1.0f,  0.0f, 1.0f,
        -x, -y,  z,  0.0f,  0.0f, 1.0f,  0.0f, 0.0f,

         x * frontDoorPos + widthOfDoor, -y,  z,  0.0f,  0.0f, 1.0f,  0.0f, 0.0f,
         x, -y,  z,  0.0f,  0.0f, 1.0f,  1.0f, 0.0f,
         x,  y + delta,  z,  0.0f,  0.0f, 1.0f,  1.0f, 1.0f,
         x,  y + delta,  z,  0.0f,  0.0f, 1.0f,  1.0f, 1.0f,
         x * frontDoorPos + widthOfDoor,  y + delta,  z,  0.0f,  0.0f, 1.0f,  0.0f, 1.0f,
         x * frontDoorPos + widthOfDoor, -y,  z,  0.0f,  0.0f, 1.0f,  0.0f, 0.0f,
        // left face
        -x,  y + delta,  z,  -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -x,  y + delta, -z,  -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -x, -y, -z,  -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -x, -y, -z,  -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -x, -y,  z,  -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -x,  y + delta,  z,  -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        // left partition wall |
        -x * leftWallPos,  y + delta,  z * bedroomDoorPos,  -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -x * leftWallPos,  y + delta, -z,  -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -x * leftWallPos, -y, -z,  -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -x * leftWallPos, -y, -z,  -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -x * leftWallPos, -y,  z * bedroomDoorPos,  -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -x * leftWallPos,  y + delta,  z * bedroomDoorPos,  -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

        -x * leftWallPos,  y + delta,  z,  -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -x * leftWallPos,  y + delta,  z * bedroomDoorPos + widthOfDoor,  -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -x * leftWallPos, -y,  z * bedroomDoorPos + widthOfDoor,  -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -x * leftWallPos, -y,  z * bedroomDoorPos + widthOfDoor,  -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -x * leftWallPos, -y,  z,  -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -x * leftWallPos,  y + delta,  z,  -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        // left partition wall -
         -x, -y,  z* bedroomDoorPos,  0.0f,  0.0f, 1.0f,  0.0f, 0.0f,
         -x * leftWallPos - widthOfDoor, -y,  z* bedroomDoorPos,  0.0f,  0.0f, 1.0f,  1.0f, 0.0f,
         -x * leftWallPos - widthOfDoor,  y + delta,  z* bedroomDoorPos,  0.0f,  0.0f, 1.0f,  1.0f, 1.0f,
         -x * leftWallPos - widthOfDoor,  y + delta,  z* bedroomDoorPos,  0.0f,  0.0f, 1.0f,  1.0f, 1.0f,
         -x,  y + delta,  z* bedroomDoorPos,  0.0f,  0.0f, 1.0f,  0.0f, 1.0f,
         -x, -y,  z* bedroomDoorPos,  0.0f,  0.0f, 1.0f,  0.0f, 0.0f,
        // right partition wall |
        x * rightWallPos,  y + delta,  z* diningDoorPos,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        x * rightWallPos,  y + delta, -z,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        x * rightWallPos, -y, -z,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        x * rightWallPos, -y, -z,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        x * rightWallPos, -y,  z* diningDoorPos,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        x * rightWallPos,  y + delta,  z* diningDoorPos,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

        x * rightWallPos,  y + delta,  z,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        x * rightWallPos,  y + delta,  z* diningDoorPos + widthOfDoor,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        x * rightWallPos, -y,  z* diningDoorPos + widthOfDoor,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        x * rightWallPos, -y,  z* diningDoorPos + widthOfDoor,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        x * rightWallPos, -y,  z,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        x * rightWallPos,  y + delta,  z,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        // right partition wall -
         x, -y,  z* diningDoorPos + 0.5 * widthOfDoor,  0.0f,  0.0f, 1.0f,  0.0f, 0.0f,
         x* rightWallPos + widthOfDoor, -y,  z* diningDoorPos + 0.5 * widthOfDoor,  0.0f,  0.0f, 1.0f,  1.0f, 0.0f,
         x* rightWallPos + widthOfDoor,  y + delta,  z* diningDoorPos + 0.5 * widthOfDoor,  0.0f,  0.0f, 1.0f,  1.0f, 1.0f,
         x* rightWallPos + widthOfDoor,  y + delta,  z* diningDoorPos + 0.5 * widthOfDoor,  0.0f,  0.0f, 1.0f,  1.0f, 1.0f,
         x,  y + delta,  z* diningDoorPos + 0.5 * widthOfDoor,  0.0f,  0.0f, 1.0f,  0.0f, 1.0f,
         x, -y,  z* diningDoorPos + 0.5 * widthOfDoor,  0.0f,  0.0f, 1.0f,  0.0f, 0.0f,
         // right face
         x,  y + delta,  z,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         x,  y + delta, -z,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
         x, -y, -z,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         x, -y, -z,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         x, -y,  z,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
         x,  y + delta,  z,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
    };
    GLfloat woodFloorVertice[] = {
        // bottom face
        -x, -y, -z,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
         x* rightWallPos, -y, -z,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
         x* rightWallPos, -y,  z,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
         x* rightWallPos, -y,  z,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        -x, -y,  z,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
        -x, -y, -z,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
    };
    GLfloat tileFloorVertice[] = {
        // bottom face
        x* rightWallPos, -y, -z,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
        x, -y, -z,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
        x, -y,  z,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        x, -y,  z,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        x* rightWallPos, -y,  z,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
        x* rightWallPos, -y, -z,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
    };
    GLfloat windowVertice[] = {
        // left part of back face
        -x, -y, -z,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,
        -x * leftWallPos, -y, -z,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f,
        -x * leftWallPos,  y - 0.1, -z,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
        -x * leftWallPos,  y - 0.1, -z,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
        -x,  y - 0.1, -z,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f,
        -x, -y, -z,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,
    };

    float roofHeight = 0.7f;
    y = y + delta;
    x = x + 0.1f;
    z = z + 0.1f;
    GLfloat roofVertice[] = {
        // front
        -x,  y,  z,  0.0,  z / (z + roofHeight),  roofHeight / (z + roofHeight),  0.0f,  0.0f,
         x,  y,  z,  0.0,  z / (z + roofHeight),  roofHeight / (z + roofHeight),  1.0f,  0.0f,
         0.0, y + roofHeight, 0.0,  0.0,  z / (z + roofHeight),  roofHeight / (z + roofHeight),  0.5f,  1.0f,

        // right
         x,  y,  z,  roofHeight / (x + roofHeight),  x / (x + roofHeight),  0.0f,  0.0f,  0.0f,
         x,  y, -z,  roofHeight / (x + roofHeight),  x / (x + roofHeight),  0.0f,  1.0f,  0.0f,
         0.0, y + roofHeight, 0.0,  roofHeight / (x + roofHeight),  x / (x + roofHeight),  0.0f,  0.5f,  1.0f,

        // back
         x,  y, -z,  0.0,  z / (z + roofHeight),  -roofHeight / (z + roofHeight),  0.0f,  0.0f,
        -x,  y, -z,  0.0,  z / (z + roofHeight),  -roofHeight / (z + roofHeight),  1.0f,  0.0f,
         0.0, y + roofHeight, 0.0,  0.0,  z / (z + roofHeight),  -roofHeight / (z + roofHeight),  0.5f,  1.0f,

        //// left
        -x,  y, -z,  -roofHeight / (x + roofHeight),  x / (x + roofHeight),  0.0f,  0.0f,  0.0f,
        -x,  y,  z,  -roofHeight / (x + roofHeight),  x / (x + roofHeight),  0.0f,  1.0f,  0.0f,
         0.0, y + roofHeight, 0.0,  -roofHeight / (x + roofHeight),  x / (x + roofHeight),  0.0f,  0.5f,  1.0f,
    };

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

#pragma region funiture
    Model woodChair(".\\Debug\\tableAndChair\\seat.obj");
    Model woodTable(".\\Debug\\tableAndChair\\table.obj"); 
    Model sideTable(".\\Debug\\sideTable\\Liam_Side_Table_by_Minotti.obj");
    Model bed(".\\Debug\\simpleBed\\file.obj");
    Model kitchenSet(".\\Debug\\kitchenSet8\\file.obj");
    Model washBasin(".\\Debug\\washBasin\\file.obj");
    Model toilet(".\\Debug\\toilet\\obj.obj");
    Model bathTube(".\\Debug\\bathTube\\obj.obj");
    Model sofaSet(".\\Debug\\sofaSet\\file.obj");
    Model shoeCabinet(".\\Debug\\shoeCabinet2\\file.obj");
    Model clothShelf(".\\Debug\\clothShelf\\file.obj");
    Model bookShelf(".\\Debug\\cab\\file.obj");
    Model wardrobe(".\\Debug\\wardrobe2\\file.obj");
    Model tv(".\\Debug\\tv\\obj.obj");
    Model tvBox(".\\Debug\\ykq\\obj.obj");
    Model freezer(".\\Debug\\rifrig\\file.obj");
    Model woodCabin(".\\Debug\\bedTable\\file.obj");
    Model desk(".\\Debug\\desk\\file.obj");
    Model deskChair(".\\Debug\\deskChair\\file.obj");
    Model computer(".\\Debug\\computer\\file.obj");
    Model longue(".\\Debug\\sunChair\\file.obj");
    Model teddyBear(".\\Debug\\teddyBear\\file.obj");
    Model flowerBottle(".\\Debug\\flowerBottle\\file.obj");
    Model drawing(".\\Debug\\draw\\file.obj");
    Model bottleSet(".\\Debug\\bottleSet\\file.obj");
    Model cupAndPlates(".\\Debug\\cupAndPlates\\file.obj");
    Model towel(".\\Debug\\towel\\file.obj");
    Model shampoo(".\\Debug\\shampoo\\file.obj");
    Model floorLamp(".\\Debug\\floorLamp\\file.obj");
#pragma endregion

#pragma region Init and Load Models to VAO, VBO
    unsigned int VBO, VAO;
    unsigned int woodFloorVBO, woodFloorVAO;
    unsigned int tileFloorVBO, tileFloorVAO;
    unsigned int windowVAO, windowVBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &woodFloorVBO);
    //glGenVertexArrays(1, &lightVAO);
    glGenVertexArrays(1, &woodFloorVAO);
    glGenVertexArrays(1, &tileFloorVAO);
    glGenBuffers(1, &tileFloorVBO);
    glGenVertexArrays(1, &windowVAO);
    glGenBuffers(1, &windowVBO);

    unsigned int roofVAO, roofVBO;
    glGenVertexArrays(1, &roofVAO);
    glGenBuffers(1, &roofVBO);
    glBindVertexArray(roofVAO);
    glBindBuffer(GL_ARRAY_BUFFER, roofVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(roofVertice), roofVertice, GL_STATIC_DRAW);
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

    //glBindVertexArray(lightVAO);
    //// we only need to bind to the VBO
    //// Since the object's VBO's data already contains the data, no need to buffer data
    //glBindBuffer(GL_ARRAY_BUFFER, VBO);
    //// set the vertex attribute 
    //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    //glEnableVertexAttribArray(0);
    //glBindVertexArray(0); // Unbind VAO

    glBindVertexArray(woodFloorVAO);
    glBindBuffer(GL_ARRAY_BUFFER, woodFloorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(woodFloorVertice), woodFloorVertice, GL_STATIC_DRAW);
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

    glBindVertexArray(tileFloorVAO);
    glBindBuffer(GL_ARRAY_BUFFER, tileFloorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tileFloorVertice), tileFloorVertice, GL_STATIC_DRAW);
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

    glBindVertexArray(windowVAO);
    glBindBuffer(GL_ARRAY_BUFFER, windowVAO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(windowVertice), windowVertice, GL_STATIC_DRAW);
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

    // Configure depth map FBO
    const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    GLuint depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    // - Create depth texture
    GLuint depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

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

#pragma region set background images
    // Cubemap (Skybox)
    std::vector<const GLchar*> faces;
    faces.push_back("..\\res\\textures\\winter\\Backyard\\posxFlip.jpg");
    faces.push_back("..\\res\\textures\\winter\\Backyard\\negxFlip.jpg");
    faces.push_back("..\\res\\textures\\winter\\Backyard\\posy.jpg");
    faces.push_back("..\\res\\textures\\winter\\Backyard\\negy.jpg");
    faces.push_back("..\\res\\textures\\winter\\Backyard\\negzFlip.jpg");
    faces.push_back("..\\res\\textures\\winter\\Backyard\\poszFlip.jpg"); 
    /*faces.push_back("..\\res\\textures\\winter\\Tantolunden5\\posxFlip.jpg");
    faces.push_back("..\\res\\textures\\winter\\Tantolunden5\\negxFlip.jpg");
    faces.push_back("..\\res\\textures\\winter\\Tantolunden5\\posy.jpg");
    faces.push_back("..\\res\\textures\\winter\\Tantolunden5\\negy.jpg");
    faces.push_back("..\\res\\textures\\winter\\Tantolunden5\\negzFlip.jpg");
    faces.push_back("..\\res\\textures\\winter\\Tantolunden5\\poszFlip.jpg");*/
    unsigned int cubemapTexture = loadCubemap(faces);
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
        //glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 1. Render depth of scene to texture (from light's perspective)
        // - Get light projection/view matrix.
        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        GLfloat near_plane = 1.0f, far_plane = 7.5f;
        lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        //lightProjection = glm::perspective(45.0f, (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT, near_plane, far_plane); // Note that if you use a perspective projection matrix you'll have to change the light position as the current light position isn't enough to reflect the whole scene.
        lightView = glm::lookAt(pointLight1.position, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        lightSpaceMatrix = lightProjection * lightView;
        // - now render scene from light's point of view
        simpleDepthShader.Use();
        glUniformMatrix4fv(glGetUniformLocation(simpleDepthShader.Program, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        /*RenderScene(simpleDepthShader);*/
        

        // Activate shader
        glViewport(0, 0, WIDTH, HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //ourShader.Use();
        simpleDepthShader.Use();
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

        // Create transformations
        // initialize transform matrix
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);
#pragma region Draw Skybox
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

#pragma region Prepare Model, View, Proj Matrix of house structure
        // construct transform matrix
        ourShader.Use();
        model = glm::mat4(1.0f);
        view = glm::mat4(1.0f);
        projection = glm::mat4(1.0f);
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
        glUniform1f(matShineLoc, woodFloorMaterial->shininess);
        // Pass diffuse map information to fragment shader
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodFloorMaterial->diffuse);
        glUniform1i(glGetUniformLocation(ourShader.Program, "material.diffuse"), ourShader.DIFFUSE);
        // Pass specular map information to fragment shader
        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_2D, woodFloorMaterial->specular);
        glUniform1i(glGetUniformLocation(ourShader.Program, "material.specular"), ourShader.SPECULAR);
#pragma endregion
        // Draw floor
        glBindVertexArray(woodFloorVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

#pragma region Load Textures for house floor
        // Pass material information to shader
        matShineLoc = glGetUniformLocation(ourShader.Program, "material.shininess");
        glUniform1f(matShineLoc, tileFloorMaterial->shininess);
        // Pass diffuse map information to fragment shader
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tileFloorMaterial->diffuse);
        glUniform1i(glGetUniformLocation(ourShader.Program, "material.diffuse"), ourShader.DIFFUSE);
        // Pass specular map information to fragment shader
        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_2D, tileFloorMaterial->specular);
        glUniform1i(glGetUniformLocation(ourShader.Program, "material.specular"), ourShader.SPECULAR);
#pragma endregion
        // Draw floor
        glBindVertexArray(tileFloorVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

#pragma region Load Textures for roof
        // Pass material information to shader
        matShineLoc = glGetUniformLocation(ourShader.Program, "material.shininess");
        glUniform1f(matShineLoc, roofMaterial->shininess);
        // Pass diffuse map information to fragment shader
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, roofMaterial->diffuse);
        glUniform1i(glGetUniformLocation(ourShader.Program, "material.diffuse"), ourShader.DIFFUSE);
        // Pass specular map information to fragment shader
        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_2D, roofMaterial->specular);
        glUniform1i(glGetUniformLocation(ourShader.Program, "material.specular"), ourShader.SPECULAR);
#pragma endregion
        // Draw roof
        glBindVertexArray(roofVAO);
        glDrawArrays(GL_TRIANGLES, 0, 24);
        glBindVertexArray(0);

#pragma region draw furniture 
#pragma region Prepare Model, View, Proj Matrix for wood table
        // Create transformations
        // initialize transform matrix
        model = glm::mat4(1.0f);
        // construct transform matrix
        model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));
        model = glm::translate(model, glm::vec3(6.0, -1.0, -0.5));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
#pragma endregion
        // Draw object
        woodTable.Draw(&ourShader);

#pragma region Prepare Model, View, Proj Matrix for wood chair
        // Create transformations
        // initialize transform matrix
        model = glm::mat4(1.0f);
        // construct transform matrix
        model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));
        model = glm::translate(model, glm::vec3(5.3, -1.0, -0.5));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
#pragma endregion
        // Draw object
        woodChair.Draw(&ourShader);

#pragma region Prepare Model, View, Proj Matrix for side table
        // Create transformations
        // initialize transform matrix
        model = glm::mat4(1.0f);
        // construct transform matrix
        model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));
        model = glm::translate(model, glm::vec3(-6.0, -1.0, -1.5));
        model = glm::rotate(model, glm::radians(30.0f), glm::vec3(0.0, 1.0, 0.0));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
#pragma endregion
        // Draw object
        sideTable.Draw(&ourShader);

#pragma region Prepare Model, View, Proj Matrix for bed
        // Create transformations
        // initialize transform matrix
        model = glm::mat4(1.0f);
        // construct transform matrix
        model = glm::scale(model, glm::vec3(0.0007, 0.0007, 0.0007));
        model = glm::translate(model, glm::vec3(-5200.0, -750.0, 50.0));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
#pragma endregion
        // Draw object
        bed.Draw(&ourShader);

#pragma region Prepare Model, View, Proj Matrix for kitchen set
        // Create transformations
        // initialize transform matrix
        model = glm::mat4(1.0f);
        // construct transform matrix
        model = glm::scale(model, glm::vec3(0.0005, 0.0005, 0.0005));
        model = glm::translate(model, glm::vec3(7000.0, -1000.0, -2800.0));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
#pragma endregion
        // Draw object
        kitchenSet.Draw(&ourShader);

#pragma region Prepare Model, View, Proj Matrix for wash basin
        // Create transformations
        // initialize transform matrix
        model = glm::mat4(1.0f);
        // construct transform matrix
        model = glm::scale(model, glm::vec3(0.0007, 0.0007, 0.0007));
        model = glm::translate(model, glm::vec3(5700.0, -700.0, 850.0));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
#pragma endregion
        // Draw object
        washBasin.Draw(&ourShader);

#pragma region Prepare Model, View, Proj Matrix for toilet
        // Create transformations
        // initialize transform matrix
        model = glm::mat4(1.0f);
        // construct transform matrix
        model = glm::scale(model, glm::vec3(0.02, 0.02, 0.02));
        model = glm::translate(model, glm::vec3(200.0, -24.0, 67.0));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
#pragma endregion
        // Draw object
        toilet.Draw(&ourShader);

#pragma region Prepare Model, View, Proj Matrix for bath tube
        // Create transformations
        // initialize transform matrix
        model = glm::mat4(1.0f);
        // construct transform matrix
        model = glm::scale(model, glm::vec3(0.0006, 0.0006, 0.0006));
        model = glm::translate(model, glm::vec3(5000.0, -800.0, 2100.0));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0, 1.0, 0.0));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
#pragma endregion
        // Draw object
        bathTube.Draw(&ourShader);

#pragma region Prepare Model, View, Proj Matrix for sofa in livingroom
        // Create transformations
        // initialize transform matrix
        model = glm::mat4(1.0f);
        // construct transform matrix
        model = glm::scale(model, glm::vec3(0.02, 0.02, 0.02));
        model = glm::translate(model, glm::vec3(7.0, -26.0, -30.0));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
#pragma endregion
        // Draw object
        sofaSet.Draw(&ourShader);

#pragma region Prepare Model, View, Proj Matrix for shoe cabinet
// Create transformations
// initialize transform matrix
        model = glm::mat4(1.0f);
        // construct transform matrix
        model = glm::scale(model, glm::vec3(0.0001, 0.0001, 0.0001));
        model = glm::translate(model, glm::vec3(-4500.0, -5000.0, 14000.0));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
#pragma endregion
        // Draw object 
        shoeCabinet.Draw(&ourShader);

#pragma region Prepare Model, View, Proj Matrix for coat hanger
        // Create transformations
        // initialize transform matrix
        model = glm::mat4(1.0f);
        // construct transform matrix
        model = glm::scale(model, glm::vec3(0.0007, 0.0007, 0.0007));
        model = glm::translate(model, glm::vec3(2500.0, -700.0, 2000.0));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
#pragma endregion
        // Draw object
        clothShelf.Draw(&ourShader);

#pragma region Prepare Model, View, Proj Matrix for hang shelf
        // Create transformations
        // initialize transform matrix
        model = glm::mat4(1.0f);
        // construct transform matrix
        model = glm::scale(model, glm::vec3(0.001, 0.001, 0.001));
        model = glm::translate(model, glm::vec3(-1100.0, -75.0, 1200.0));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
#pragma endregion
        // Draw object
        bookShelf.Draw(&ourShader);

#pragma region Prepare Model, View, Proj Matrix for television
        // Create transformations
        // initialize transform matrix
        model = glm::mat4(1.0f);
        // construct transform matrix
        model = glm::scale(model, glm::vec3(0.0005, 0.0005, 0.0005));
        model = glm::translate(model, glm::vec3(-1000.0, -10.0, 2900.0));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0, 1.0, 0.0));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
#pragma endregion
        // Draw object
        tv.Draw(&ourShader);

#pragma region Prepare Model, View, Proj Matrix for television controller
        // Create transformations
        // initialize transform matrix
        model = glm::mat4(1.0f);
        // construct transform matrix
        model = glm::scale(model, glm::vec3(0.000005, 0.000005, 0.000005));
        model = glm::translate(model, glm::vec3(0.0, -55000.0, 10000.0));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
#pragma endregion
        // Draw object
        tvBox.Draw(&ourShader);

#pragma region Prepare Model, View, Proj Matrix for refrigirator
        // Create transformations
        // initialize transform matrix
        model = glm::mat4(1.0f);
        // construct transform matrix
        model = glm::scale(model, glm::vec3(0.007, 0.007, 0.007));
        model = glm::translate(model, glm::vec3(580.0, -70.0, 10.0));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0, 1.0, 0.0));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
#pragma endregion
        // Draw object
        freezer.Draw(&ourShader);

#pragma region Prepare Model, View, Proj Matrix for bedside table
        // Create transformations
        // initialize transform matrix
        model = glm::mat4(1.0f);
        // construct transform matrix
        model = glm::scale(model, glm::vec3(0.002, 0.002, 0.002));
        model = glm::translate(model, glm::vec3(-2050.0, -250.0, 430.0));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0, 1.0, 0.0));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
#pragma endregion
        // Draw object
        woodCabin.Draw(&ourShader);

#pragma region Prepare Model, View, Proj Matrix for wardrobe
        // Create transformations
        // initialize transform matrix
        model = glm::mat4(1.0f);
        // construct transform matrix
        model = glm::scale(model, glm::vec3(0.001, 0.001, 0.001));
        model = glm::translate(model, glm::vec3(-3100.0, -500.0, 1400.0));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0, 1.0, 0.0));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
#pragma endregion
        // Draw object
        wardrobe.Draw(&ourShader);

#pragma region Prepare Model, View, Proj Matrix for desk
        // Create transformations
        // initialize transform matrix
        model = glm::mat4(1.0f);
        // construct transform matrix
        model = glm::scale(model, glm::vec3(0.0007, 0.0007, 0.0007));
        model = glm::translate(model, glm::vec3(-2800.0, -700.0, 1800.0));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
#pragma endregion
        // Draw object
        desk.Draw(&ourShader);

#pragma region Prepare Model, View, Proj Matrix for desk chair
        // Create transformations
        // initialize transform matrix
        model = glm::mat4(1.0f);
        // construct transform matrix
        model = glm::scale(model, glm::vec3(0.00007, 0.00007, 0.00007));
        model = glm::translate(model, glm::vec3(-28000.0, -7000.0, 10000.0));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
#pragma endregion
        // Draw object
        deskChair.Draw(&ourShader);

#pragma region Prepare Model, View, Proj Matrix for computer
        // Create transformations
        // initialize transform matrix
        model = glm::mat4(1.0f);
        // construct transform matrix
        model = glm::scale(model, glm::vec3(0.001, 0.001, 0.001));
        model = glm::translate(model, glm::vec3(-2000.0, 55.0, 1200.0));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0, 1.0, 0.0));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
#pragma endregion
        // Draw object
        computer.Draw(&ourShader);

#pragma region Prepare Model, View, Proj Matrix for longue
        // Create transformations
        // initialize transform matrix
        model = glm::mat4(1.0f);
        // construct transform matrix
        model = glm::scale(model, glm::vec3(0.0007, 0.0007, 0.0007));
        model = glm::translate(model, glm::vec3(-5000.0, -750.0, -1400.0));
        model = glm::rotate(model, glm::radians(30.0f), glm::vec3(0.0, 1.0, 0.0));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
#pragma endregion
        // Draw object
        longue.Draw(&ourShader);

#pragma region Prepare Model, View, Proj Matrix for teddy bear
        // Create transformations
        // initialize transform matrix
        model = glm::mat4(1.0f);
        // construct transform matrix
        model = glm::scale(model, glm::vec3(0.0005, 0.0005, 0.0005));
        model = glm::translate(model, glm::vec3(-6800.0, -340.0, 0.0));
        model = glm::rotate(model, glm::radians(60.0f), glm::vec3(0.0, 1.0, 0.0));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
#pragma endregion
        // Draw object
        teddyBear.Draw(&ourShader);

#pragma region Prepare Model, View, Proj Matrix for flower bottle
        // Create transformations
        // initialize transform matrix
        model = glm::mat4(1.0f);
        // construct transform matrix
        model = glm::scale(model, glm::vec3(0.001, 0.001, 0.001));
        model = glm::translate(model, glm::vec3(-4250.0, -100.0, 700.0));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
#pragma endregion
        // Draw object
        flowerBottle.Draw(&ourShader);

#pragma region Prepare Model, View, Proj Matrix for drawing
        // Create transformations
        // initialize transform matrix
        model = glm::mat4(1.0f);
        // construct transform matrix
        model = glm::scale(model, glm::vec3(0.001, 0.001, 0.001));
        model = glm::translate(model, glm::vec3(2190.0, 600.0, -600.0));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(1.0, 0.0, 0.0));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
#pragma endregion
        // Draw object
        drawing.Draw(&ourShader);

#pragma region Prepare Model, View, Proj Matrix for bottle set
        // Create transformations
        // initialize transform matrix
        model = glm::mat4(1.0f);
        // construct transform matrix
        model = glm::scale(model, glm::vec3(0.0007, 0.0007, 0.0007));
        model = glm::translate(model, glm::vec3(-550.0, -400.0, 0.0));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
#pragma endregion
        // Draw object
        bottleSet.Draw(&ourShader);

#pragma region Prepare Model, View, Proj Matrix for cup and plates
        // Create transformations
        // initialize transform matrix
        model = glm::mat4(1.0f);
        // construct transform matrix
        model = glm::scale(model, glm::vec3(0.03, 0.03, 0.03));
        model = glm::translate(model, glm::vec3(100.0, -2.0, -8.0));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
#pragma endregion
        // Draw object
        cupAndPlates.Draw(&ourShader);

#pragma region Prepare Model, View, Proj Matrix for towel
        // Create transformations
        // initialize transform matrix
        model = glm::mat4(1.0f);
        // construct transform matrix
        model = glm::scale(model, glm::vec3(0.0005, 0.0005, 0.0005));
        model = glm::translate(model, glm::vec3(8700.0, -150.0, 1500.0));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0, 1.0, 0.0));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
#pragma endregion
        // Draw object
        towel.Draw(&ourShader);

#pragma region Prepare Model, View, Proj Matrix for shampoo
        // Create transformations
        // initialize transform matrix
        model = glm::mat4(1.0f);
        // construct transform matrix
        model = glm::scale(model, glm::vec3(0.015, 0.015, 0.015));
        model = glm::translate(model, glm::vec3(180.0, -9.5, 100.0));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
#pragma endregion
        // Draw object
        shampoo.Draw(&ourShader);

#pragma region Prepare Model, View, Proj Matrix for floor lamp
        // Create transformations
        // initialize transform matrix
        model = glm::mat4(1.0f);
        // construct transform matrix
        model = glm::scale(model, glm::vec3(0.015, 0.015, 0.015));
        model = glm::translate(model, glm::vec3(-270.0, -32.0, 90.0));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
#pragma endregion
        // Draw object
        floorLamp.Draw(&ourShader);
#pragma endregion

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

//#pragma region Draw Skybox
//        // Draw skybox last
//        //glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
//        skyboxShader.Use();
//        model = glm::mat4(1.0f);
//        model = glm::scale(model, glm::vec3(1000.0, 1000.0, 1000.0));
//        model = glm::translate(model, glm::vec3(0.0, 20.0, 0.0));
//        view = camera.GetViewMatrix();
//        //view = glm::mat4(glm::mat3(camera.GetViewMatrix()));	// Remove any translation component of the view matrix
//        glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
//        glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
//        glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
//        // skybox cube
//        glBindVertexArray(skyboxVAO);
//        glActiveTexture(GL_TEXTURE0);
//        glUniform1i(glGetUniformLocation(ourShader.Program, "skybox"), 0);
//        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
//        glDrawArrays(GL_TRIANGLES, 0, 36);
//        glBindVertexArray(0);
//        glDepthFunc(GL_LESS); // set depth function back to default
//#pragma endregion

        // 3. DEBUG: visualize depth map by rendering it to plane
        debugDepthQuad.Use();
        glUniform1f(glGetUniformLocation(debugDepthQuad.Program, "near_plane"), near_plane);
        glUniform1f(glGetUniformLocation(debugDepthQuad.Program, "far_plane"), far_plane);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        //RenderQuad();

#pragma region Draw house window
        windowShader.Use();
        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(2, 2, 2));
        view = camera.GetViewMatrix();
        GLint windowModelLoc = glGetUniformLocation(windowShader.Program, "model");
        GLint windowViewLoc = glGetUniformLocation(windowShader.Program, "view");
        GLint windowProjLoc = glGetUniformLocation(windowShader.Program, "projection");
        glUniformMatrix4fv(windowModelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(windowViewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(windowProjLoc, 1, GL_FALSE, glm::value_ptr(projection));
        // Draw window
        glUniform1i(glGetUniformLocation(windowShader.Program, "texture1"), 0);
        glBindVertexArray(windowVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, windowTexture);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
#pragma endregion

        // Activate light shader
        //lightShader.Use();
#pragma region Prepare Model, View, Proj Matrix for point light
        //// Create transformations for light
        //GLuint lightModelLoc = glGetUniformLocation(lightShader.Program, "model");
        //GLuint lightViewLoc = glGetUniformLocation(lightShader.Program, "view");
        //GLuint lightProjLoc = glGetUniformLocation(lightShader.Program, "projection");
        //glm::mat4 lightView = glm::mat4(1.0f);
        //glm::mat4 lightProjection = glm::mat4(1.0f);
        //lightView = camera.GetViewMatrix();
        //lightProjection = glm::perspective(camera.Zoom, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);
        //// Pass them to the shaders
        //glUniformMatrix4fv(lightViewLoc, 1, GL_FALSE, glm::value_ptr(lightView));
        //glUniformMatrix4fv(lightProjLoc, 1, GL_FALSE, glm::value_ptr(lightProjection));
        //// Draw object
        //glBindVertexArray(lightVAO);
        //for (GLuint i = 0; i < sizeof(pointLightPositions) / sizeof(pointLightPositions[0]); i++)
        //{
        //    glm::mat4 model = glm::mat4(1.0f);
        //    model = glm::translate(model, pointLightPositions[i]);
        //    model = glm::scale(model, glm::vec3(0.15f)); // Make it a smaller cube
        //    glUniformMatrix4fv(lightModelLoc, 1, GL_FALSE, glm::value_ptr(model));
        //    glDrawArrays(GL_TRIANGLES, 0, 36);
        //}
        //glBindVertexArray(0);
#pragma endregion
        // Swap the screen buffers
        glfwSwapBuffers(window);
    }
    // Properly de-allocate all resources once they've outlived their purpose
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &woodFloorVAO);
    glDeleteBuffers(1, &woodFloorVBO);
    glDeleteVertexArrays(1, &tileFloorVAO);
    glDeleteBuffers(1, &tileFloorVBO);
    glDeleteVertexArrays(1, &windowVAO);
    glDeleteBuffers(1, &windowVBO);
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