#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library
#include <cmath>
#include <Camera.h>

//Texture Loading utility functions
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif
#include <vector>


namespace {
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;
    bool projectionOrtho = false;
    float cameraSpeed = 2.5f;

    const int steps = 10;
    float xPos = 0; float yPos = 0; float radius = 1.0f;


    struct each_pole {
        GLfloat x, z, y_start, y_end;
    }; // struct

    std::vector<each_pole> each_pole_vector; // vector of structs

    struct GLMesh
    {
        GLuint vao;         // Handle for the vertex array object
        GLuint vbos[4];     // Handles for the vertex buffer objects
        GLuint nVertices;    // Number of indices of the mesh
    };
    //Camera Position
    Camera gCamera(glm::vec3(0.0f, 0.0f, 3.0f));
    
    //Camera Movement
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    //timing
    float gDeltaTime = 0.0f;
    float gLastFrame = 0.0f;

    //Main GLFW window
    GLFWwindow* gWindow = nullptr;
    // Charger mesh data
    GLMesh chargerCube;
    GLMesh cubeProngOne;
    GLMesh cubeProngTwo;
    GLMesh eraserHead;
    GLMesh eraserBody;

    //Plane Mesh Data
    GLMesh plane;
    // 
    // Shader program
    GLuint gProgramId;
    GLuint gProgramId2;

    //Texture Ids
    GLuint gPlugBodyId;
    GLuint gPlugProngOneId;
    GLuint gPlugProngTwoId;
    GLuint gPlane;
    GLuint gEraserHead;
    GLuint gEraserBody;
}

//User-defined function prototypes
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);
void UCreateCube(GLMesh& mesh);
void UCreatePlane(GLMesh& mesh);
void UCreatePlugBody(GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
bool UCreateTexture(const char* filename, GLuint& textureId);



/* Vertex Shader Source Code*/
const GLchar* vertexShaderSource = GLSL(440,
    layout(location = 0) in vec3 position; // Vertex data from Vertex Attrib Pointer 0
layout(location = 1) in vec2 textureCoordinate;  // Texture data from Vertex Attrib Pointer 2

out vec2 vertexTextureCoordinate; // variable to transfer color data to the fragment shader

uniform mat4 shaderTransform; // 4x4 matrix variable for transforming vertex data
// Global Variables for transform matricies
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // transforms vertices to clip coordinates
    vertexTextureCoordinate = textureCoordinate; // references incoming color data
}
);



/* Fragment Shader Source Code*/
const GLchar* fragmentShaderSource = GLSL(440,
    in vec2 vertexTextureCoordinate; // Variable to hold incoming color data from vertex shader

out vec4 fragmentColor;

uniform sampler2D uTexture;

void main()
{
    fragmentColor = texture(uTexture, vertexTextureCoordinate);
}
);

int main(int argc, char* argv[]) {


    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;
    std::cout << "Initialized" << std::endl;


    UCreateCube(chargerCube);
    std::cout << "Plug BodyMesh Created" << std::endl;

    // Load texture(relative to project's directory)
    const char* plugBody = "./resources/textures/WhitePlastic.png";
    if (!UCreateTexture(plugBody, gPlugBodyId))
    {
        std::cout << "Failed to load texture " << plugBody << std::endl;
        return EXIT_FAILURE;
    }

    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gProgramId);

    // We set the texture as texture unit 
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);

    //-----------------------------------------------------------------------------

    //Create Prong One and Texture
    UCreateCube(cubeProngOne);
    std::cout << "Cube Mesh2 Created" << std::endl;

    // Load texture(relative to project's directory)
    const char* texFilename = "./resources/textures/metal.png";
    if (!UCreateTexture(texFilename, gPlugProngOneId))
    {
        std::cout << "Failed to load texture " << texFilename << std::endl;
        return EXIT_FAILURE;
    }

    // We set the texture as texture unit 
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);

    //-----------------------------------------------------------------------------

    UCreateCube(cubeProngTwo);
    std::cout << "Cube Mesh2 Created" << std::endl;

    // Load texture(relative to project's directory)
    if (!UCreateTexture(texFilename, gPlugProngTwoId))
    {
        std::cout << "Failed to load texture " << texFilename << std::endl;
        return EXIT_FAILURE;
    }

    // We set the texture as texture unit
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);

    
    //-----------------------------------------------------------------------------

    UCreateCube(eraserHead);
    std::cout << "Eraser head mesh Created" << std::endl;


    // Load texture(relative to project's directory)
    const char* eraserHead = "./resources/textures/Eraser.png";
    if (!UCreateTexture(eraserHead, gEraserHead))
    {
        std::cout << "Failed to load texture " << eraserHead << std::endl;
        return EXIT_FAILURE;
    }

    // We set the texture as texture unit 2
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);

    
    //-----------------------------------------------------------------------------
    
    UCreateCube(eraserBody);
    std::cout << "Eraser Body mesh Created" << std::endl;


    // Load texture(relative to project's directory)
    const char* eraserBody = "./resources/textures/EraserBody.png";
    if (!UCreateTexture(eraserBody, gEraserBody))
    {
        std::cout << "Failed to load texture " << eraserBody << std::endl;
        return EXIT_FAILURE;
    }

    // We set the texture as texture unit 2
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);


    //-----------------------------------------------------------------------------

    UCreateCube(plane);
    std::cout << "Plane mesh Created" << std::endl;


    // Load texture(relative to project's directory)
    const char* planeTex = "./resources/textures/CuttingMat.png";
    if (!UCreateTexture(planeTex, gPlane))
    {
        std::cout << "Failed to load texture " << planeTex << std::endl;
        return EXIT_FAILURE;
    }

    // We set the texture as texture unit 2
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);

    //-----------------------------------------------------------------------------

    // Create the shader program
    if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
        return EXIT_FAILURE;

    //Set background to black
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);


    while (!glfwWindowShouldClose(gWindow))
    {

        UProcessInput(gWindow);

        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        URender();


        glfwPollEvents();
    }

    UDestroyMesh(chargerCube);
    UDestroyMesh(cubeProngOne);
    UDestroyMesh(cubeProngTwo);
    UDestroyMesh(plane);
    UDestroyShaderProgram(gProgramId);
    exit(EXIT_SUCCESS);

}

bool UInitialize(int argc, char* argv[], GLFWwindow** window) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);





    *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "5-5 Andrei Kourouchin", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);
    glfwSetCursorPosCallback(*window, UMousePositionCallback);
    glfwSetScrollCallback(*window, UMouseScrollCallback);
    glfwSetMouseButtonCallback(*window, UMouseButtonCallback);

    // GLEW: initialize
    // ----------------
    // Note: if using GLEW version 1.13 or earlier
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }


    // Displays GPU OpenGL version
    std::cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << std::endl;




    return true;
}

// Images are loaded with Y axis going down, but OpenGL's Y axis goes up, so let's flip it
void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
    for (int j = 0; j < height / 2; ++j)
    {
        int index1 = j * width * channels;
        int index2 = (height - 1 - j) * width * channels;

        for (int i = width * channels; i > 0; --i)
        {
            unsigned char tmp = image[index1];
            image[index1] = image[index2];
            image[index2] = tmp;
            ++index1;
            ++index2;
        }
    }
}

bool UCreateTexture(const char* filename, GLuint& textureId)
{
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (image)
    {
        flipImageVertically(image, width, height, channels);

        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (channels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (channels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        else
        {
            std::cout << "Not implemented to handle image with " << channels << " channels" << std::endl;
            return false;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

        return true;
    }

    // Error loading the image
    return false;
}

// glfw: Whenever the mouse moves, this callback is called.
// -------------------------------------------------------
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (gFirstMouse)
    {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouse = false;
    }

    float xoffset = xpos - gLastX;
    float yoffset = gLastY - ypos; // reversed since y-coordinates go from bottom to top

    gLastX = xpos;
    gLastY = ypos;

    gCamera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: Whenever the mouse scroll wheel scrolls, this callback is called.
// ----------------------------------------------------------------------
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    gCamera.ProcessMouseScroll(yoffset);
}

// glfw: Handle mouse button events.
// --------------------------------
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
    {
        if (action == GLFW_PRESS)
            std::cout << "Left mouse button pressed" << std::endl;
        else
            std::cout << "Left mouse button released" << std::endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_MIDDLE:
    {
        if (action == GLFW_PRESS)
            std::cout << "Middle mouse button pressed" << std::endl;
        else
            std::cout << "Middle mouse button released" << std::endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_RIGHT:
    {
        if (action == GLFW_PRESS)
            std::cout << "Right mouse button pressed" << std::endl;
        else
            std::cout << "Right mouse button released" << std::endl;
    }
    break;

    default:
        std::cout << "Unhandled mouse button event" << std::endl;
        break;
    }
}

void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void UProcessInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        gCamera.ProcessKeyboard(FORWARD, gDeltaTime);

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        gCamera.ProcessKeyboard(LEFT, gDeltaTime);

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        gCamera.ProcessKeyboard(RIGHT, gDeltaTime);

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        gCamera.ProcessKeyboard(UP, gDeltaTime);

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        gCamera.ProcessKeyboard(DOWN, gDeltaTime);

    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
            projectionOrtho = true;
    
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
            projectionOrtho = false;
    
}

void URender() {

    // Enable Z-depth.
    glEnable(GL_DEPTH_TEST);


    // Clear the frame and Z buffers.
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 projection;

    //Draws the first cube
    // 
    // 
    // 
    // camera/view transformation
    glm::mat4 view = gCamera.GetViewMatrix();

    if (projectionOrtho == false) {
        projection = glm::perspective(45.0f, (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.2f, 100.0f);
    }
    else if (projectionOrtho == true) {
        projection = glm::ortho(-2.0f, 2.0f, -1.5f, 1.5f, 1.0f, 100.0f);

    }

    // 1. Scales the shape by 2
    glm::mat4 scale = glm::scale(glm::vec3(1.0f, 1.2f, 1.0f));

    // 2. Rotates shape by 45 degrees on the x, y and z axis
    glm::mat4 rotation = glm::rotate(0.0f, glm::vec3(0.0f, 1.0f, 0.2f));
   

    // 3. Places object at the origin
    glm::mat4 translation = glm::translate(glm::vec3(0.0f, -1.0f, 0.0f));

    // Transformations are applied right-to-left order
    glm::mat4 model = translation * rotation * scale;

    //Set the shader to be used
    glUseProgram(gProgramId);

    // Retrieves and passes transform matrices to the shader program
    GLint modelLoc = glGetUniformLocation(gProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gProgramId, "view");
    GLint projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    
    //activate the VBOs contained within the mesh's VAO
    glBindVertexArray(chargerCube.vao);

    //Bind textures to corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPlugBodyId);
    
    glDrawElements(GL_TRIANGLES, chargerCube.nVertices, GL_UNSIGNED_SHORT, NULL); // Draws the triangle

    // Draws the second cube
    //
    //
    //
    //
    // 1. Scales the shape by 2
    scale = glm::scale(glm::vec3(0.3f, 0.8f, 0.05f));

    // 2. Rotates shape by 45 degrees on the x, y and z axis
    rotation = glm::rotate(0.0f, glm::vec3(0.0f, 1.3f, 0.2f));

    // 3. Places plug spoke at right location
    translation = glm::translate(glm::vec3(0.35f, 0.1f, -0.75f));

    // Transformations are applied right-to-left order
    model = rotation * translation * scale;

    //Set the shader to be used
    glUseProgram(gProgramId);

    // Retrieves and passes transform matrices to the shader program
    
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //activate the VBOs contained within the mesh's VAO
    glBindVertexArray(cubeProngOne.vao);

    //Bind textures to corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPlugProngOneId);
    
    glDrawElements(GL_TRIANGLES, cubeProngOne.nVertices, GL_UNSIGNED_SHORT, NULL); // Draws the triangle


    // Draws the third cube
    //
    //
    //
    
    // 1. Shrinks the shape
    scale = glm::scale(glm::vec3(0.3f, 0.8f, 0.05f));

    // 2. Rotates shape 
    rotation = glm::rotate(0.0f, glm::vec3(0.0f, 1.0f, 0.2f));

    // 3. Places plug spoke at right location
    translation = glm::translate(glm::vec3(0.35f, 0.1f, -0.2f));

    // Transformations are applied right-to-left order
    model = rotation * translation * scale;

    //Set the shader to be used
    glUseProgram(gProgramId);

    // Retrieves and passes transform matrices to the shader program
    
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //activate the VBOs contained within the mesh's VAO
    glBindVertexArray(cubeProngTwo.vao);

    //Bind textures to corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPlugProngTwoId);

    // Draws the prong
    glDrawElements(GL_TRIANGLES, cubeProngOne.nVertices, GL_UNSIGNED_SHORT, NULL); // Draws the triangle


    //
    //
    //
    //
    //
    // Draws the Eraser head
    //
    //
    //

    // 1. Shrinks the shape
    scale = glm::scale(glm::vec3(1.0f, 0.9f, 0.55f));

    // 2. Rotates shape 
    rotation = glm::rotate(15.0f, glm::vec3(4.0f, 7.0f, -7.0f));

    // 3. Places plug spoke at right location
    translation = glm::translate(glm::vec3(-3.5f, -1.5f, 1.1f));

    // Transformations are applied right-to-left order
    model = rotation * translation * scale;

    //Set the shader to be used
    glUseProgram(gProgramId);

    // Retrieves and passes transform matrices to the shader program

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //activate the VBOs contained within the mesh's VAO
    glBindVertexArray(cubeProngTwo.vao);

    //Bind textures to corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gEraserHead);

    // Draws the triangle
    glDrawElements(GL_TRIANGLES, eraserHead.nVertices, GL_UNSIGNED_SHORT, NULL); // Draws the triangle


    // Draws the Eraser Body
    //
    //
    //

    // 1. Shrinks the shape
    scale = glm::scale(glm::vec3(1.0f, -2.5f, 0.55f));

    // 2. Rotates shape 
    rotation = glm::rotate(15.0f, glm::vec3(4.0f, 7.0f, -7.0f));

    // 3. Places plug spoke at right location
    translation = glm::translate(glm::vec3(-3.5f, -1.5f, 1.1f));

    // Transformations are applied right-to-left order
    model = rotation * translation * scale;

    //Set the shader to be used
    glUseProgram(gProgramId);

    // Retrieves and passes transform matrices to the shader program

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //activate the VBOs contained within the mesh's VAO
    glBindVertexArray(eraserBody.vao);

    //Bind textures to corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gEraserBody);

    // Draws the triangle
    glDrawElements(GL_TRIANGLES, eraserBody.nVertices, GL_UNSIGNED_SHORT, NULL); // Draws the triangle


    // Draws the Plane
    //
    //
    //

    // 1. Scales the shape
    scale = glm::scale(glm::vec3(20.0f, 20.0f, 0.1f));

    // 2. Rotates shape 
    rotation = glm::rotate(90.0f, glm::vec3(-0.5f, 0.5f, 0.5f));

    // 3. Places object in place
    translation = glm::translate(glm::vec3(-10.0f, -10.0f, -1.0f));

    // Transformations are applied right-to-left order
    model = rotation * translation * scale;

    //Set the shader to be used
    glUseProgram(gProgramId);

    // Retrieves and passes transform matrices to the shader program

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //activate the VBOs contained within the mesh's VAO
    glBindVertexArray(plane.vao);

    //Bind textures to corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPlane);

    // Draws the triangle
    glDrawElements(GL_TRIANGLES, plane.nVertices, GL_UNSIGNED_SHORT, NULL); // Draws the triangle

    glBindVertexArray(0);

    glfwSwapBuffers(gWindow);


}




void UCreateCube(GLMesh& mesh) {

    // Specifies normalized device coordinates (x,y,z) and color for Triangle vertices
    GLfloat cubeVerts[] =
    {

    //Vertex Positions      //Texture Positions
    
    //Front Facing Lower Left Face
     0.0f, 0.0f, -1.0f,	    0.0f,0.0f,
     1.0f, 0.0f, -1.0f, 	1.0f,0.0f,
     0.0f, 1.0f, -1.0f,  	0.0f,1.0f,
 
    //Front Facing Upper Right Face
     1.0f, 0.0f, -1.0f, 	1.0f,0.0f,
     0.0f, 1.0f, -1.0f, 	0.0f,1.0f,
     1.0f, 1.0f, -1.0f, 	1.0f,1.0f,

    //Right Facing Lower Left Face
     1.0f, 0.0f, -1.0f, 	0.0f,0.0f,
     1.0f, 0.0f, 0.0f, 	    1.0f,0.0f,
     1.0f, 1.0f, -1.0f, 	0.0f,1.0f,
    
    //Right Facing Upper Right Face
     1.0f, 0.0f, 0.0f, 	    1.0f,0.0f,
     1.0f, 1.0f, -1.0f,     0.0f,1.0f,
     1.0f, 1.0f, 0.0f, 	    1.0f,1.0f,

    //Left Facing Lower Left Face
     0.0f, 0.0f, 0.0f, 	    0.0f,0.0f,
     0.0f, 0.0f, -1.0f,	    1.0f,0.0f,
     0.0f, 1.0f, 0.0f, 	    0.0f,1.0f,


     //Left Facing Upper Right Face
     0.0f, 0.0f, -1.0f,	    1.0f,0.0f,
     0.0f, 1.0f, 0.0f, 	    0.0f,1.0f,
     0.0f, 1.0f, -1.0f, 	1.0f,1.0f,

    //Back Facing Lower Left Face

     0.0f, 0.0f, 0.0f, 	    0.0f,0.0f,
     1.0f, 0.0f, 0.0f, 	    1.0f,0.0f,
     0.0f, 1.0f, 0.0f, 	    0.0f,1.0f,

    //Back Facing Upper Right Face

     1.0f, 0.0f, 0.0f, 	    1.0f,0.0f,
     0.0f, 1.0f, 0.0f, 	    0.0f,1.0f,
     1.0f, 1.0f, 0.0f, 	    1.0f,1.0f,

    //Upwards Facing Lower Left Face

     0.0f, 1.0f, -1.0f, 	0.0f,0.0f,
     1.0f, 1.0f, -1.0f, 	1.0f,0.0f,
     0.0f, 1.0f, 0.0f, 	    0.0f,1.0f,

    //Upwards facing Upper Right Face

     1.0f, 1.0f, -1.0f, 	1.0f,0.0f,
     0.0f, 1.0f, 0.0f,  	0.0f,1.0f,
     1.0f, 1.0f, 0.0f, 	    1.0f,1.0f,

    //Downwards Facing Lower Left Face
     0.0f, 0.0f, -1.0f,	    0.0f,0.0f,
     1.0f, 0.0f, -1.0f, 	1.0f,0.0f,
     0.0f, 0.0f, 0.0f, 	    0.0f,1.0f,


    //Downwards Facing Upper Right Face

     1.0f, 0.0f, -1.0f,     1.0f,0.0f,
     0.0f, 0.0f, 0.0f,      0.0f,1.0f,
     1.0f, 0.0f, 0.0f, 	    1.0f,1.0f,


    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerUV = 2;

    GLushort cubeIndices[] = {
    0,1,2,

    3,4,5,

    6,7,8,

    9,10,11,

    12,13,14,

    15,16,17,

    18,19,20,

    21,22,23,

    24,25,26,

    27,28,29,

    30,31,32,

    33,34,35
        
    };



    //Cube 1

    glGenVertexArrays(4, &mesh.vao);
    glBindVertexArray(mesh.vao);


    glGenBuffers(4, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerts), cubeVerts, GL_STATIC_DRAW);

    //Data for the indices
    mesh.nVertices = sizeof(cubeIndices) / sizeof(cubeIndices[0]);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

    //Strides between vertex coordinates is 7
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerUV);


    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);


}



// Implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    // Compilation and linkage error reporting
    int success = 0;
    char infoLog[512];

    // Create a Shader program object.
    programId = glCreateProgram();

    // Create the vertex and fragment shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Retrive the shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Compile the vertex shader, and print compilation errors (if any)
    glCompileShader(vertexShaderId); // compile the vertex shader
    // check for shader compile errors
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glCompileShader(fragmentShaderId); // compile the fragment shader
    // check for shader compile errors
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Attached compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);   // links the shader program
    // check for linking errors
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glUseProgram(programId);    // Uses the shader program
    glDeleteShader(vertexShaderId);
    glDeleteShader(fragmentShaderId);
    return true;
}


void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}


void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(2, mesh.vbos);
}

