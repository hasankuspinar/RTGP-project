
#include <string>

#ifdef _WIN32
    #define APIENTRY __stdcall
#endif

#include <glad/glad.h>

#include <glfw/glfw3.h>

#ifdef _WINDOWS_
    #error windows.h was included!
#endif

#include <utils/shader.h>
#include <utils/model.h>
#include <utils/camera.h>
#include <utils/physics.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

GLuint screenWidth = 1200, screenHeight = 900;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void apply_camera_movements();
bool keys[1024];

GLfloat lastX, lastY;

double cursorX,cursorY;

bool firstMouse = true;


GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

GLboolean wireframe = GL_FALSE;

glm::mat4 view, projection;

Camera camera(glm::vec3(0.0f, 0.0f, 9.0f), GL_FALSE);

glm::vec3 lightPos0 = glm::vec3(0.0f, 5.0f, 0.0f);

GLfloat Kd = 3.0f;
GLfloat alpha = 0.2f;
GLfloat F0 = 0.9f;

glm::vec3 ambientColor = glm::vec3(0.1f, 0.1f, 0.1f); 
float Ka = 1.0f;

GLfloat diffuseColor[] = {1.0f,0.0f,0.0f};

GLfloat planeMaterial[] = {0.0f,0.5f,0.0f};

GLfloat shootColor[] = {1.0f,1.0f,0.0f};

glm::vec3 sphere_size = glm::vec3(0.2f, 0.2f, 0.2f);


Physics bulletSimulation;
struct Room {
    glm::vec3 position; 
    glm::vec3 size;     
    glm::vec3 gravity;     
    float friction; 
};


bool isInsideRoom(const glm::vec3& position, const Room& room);


std::vector<Room> rooms;
double lastTime = glfwGetTime();
int frameCount = 0;

Room* getCurrentRoom(glm::vec3 position);
void updateBulletPhysics();


int main()
{
    
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);


    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "RTGP_lecture06b", nullptr, nullptr);
    if (!window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);


    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    glEnable(GL_DEPTH_TEST);

    glClearColor(0.26f, 0.46f, 0.98f, 1.0f);

    Shader object_shader = Shader("09_illumination_models.vert", "10_illumination_models.frag");

    Model cubeModel("../../models/cube.obj");
    Model sphereModel("../../models/sphere.obj");

    glm::vec3 plane_pos = glm::vec3(0.0f, -1.0f, 0.0f);
    glm::vec3 plane_size = glm::vec3(200.0f, 0.1f, 200.0f);
    glm::vec3 plane_rot = glm::vec3(0.0f, 0.0f, 0.0f);

    btRigidBody* plane = bulletSimulation.createRigidBody(BOX,plane_pos,plane_size,plane_rot,0.0f,0.3f,0.3f);


    GLfloat maxSecPerFrame = 1.0f / 60.0f;

    projection = glm::perspective(45.0f, (float)screenWidth/(float)screenHeight, 0.1f, 10000.0f);

    glm::mat4 objModelMatrix = glm::mat4(1.0f);
    glm::mat3 objNormalMatrix = glm::mat3(1.0f);
    glm::mat4 planeModelMatrix = glm::mat4(1.0f);
    glm::mat3 planeNormalMatrix = glm::mat3(1.0f);
    rooms.push_back(Room{glm::vec3(-10.0f, 0.0f, 0.0f), glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f, -30.8f, 0.0f), 0.5f});
    rooms.push_back(Room{ glm::vec3(10.0f, 0.0f, 0.0f), glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f, -5.0f, 0.0f), 3.5f});
    rooms.push_back(Room{ glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f, -15.0f, 0.0f), 1.5f});
    rooms.push_back(Room{ glm::vec3(0.0f, 0.0f, -10.0f), glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f, -100.0f, 0.0f), 5.5f});

    while(!glfwWindowShouldClose(window))
    {
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
    
        double currentTime = glfwGetTime();
        frameCount++;
        if ( currentTime - lastTime >= 1.0 ){ 
            double fps = frameCount/(lastTime-currentTime);
            string newTitle = "RTGP Project - " + to_string(fps) + "FPS";
            glfwSetWindowTitle(window, newTitle.c_str());
            frameCount = 0;
            lastTime += 1.0;
        }

        glfwPollEvents();

        apply_camera_movements();
        updateBulletPhysics();

        view = camera.GetViewMatrix();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        
        if (wireframe)
        
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        
        bulletSimulation.dynamicsWorld->stepSimulation((deltaTime < maxSecPerFrame ? deltaTime : maxSecPerFrame),10);

        
        object_shader.Use();
        
        GLuint index = glGetSubroutineIndex(object_shader.Program, GL_FRAGMENT_SHADER, "GGX");
       
        glUniformSubroutinesuiv( GL_FRAGMENT_SHADER, 1, &index);

        
        glUniformMatrix4fv(glGetUniformLocation(object_shader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(object_shader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));


        GLint objDiffuseLocation = glGetUniformLocation(object_shader.Program, "diffuseColor");
        GLint pointLightLocation = glGetUniformLocation(object_shader.Program, "pointLightPosition");
        GLint kdLocation = glGetUniformLocation(object_shader.Program, "Kd");
        GLint alphaLocation = glGetUniformLocation(object_shader.Program, "alpha");
        GLint f0Location = glGetUniformLocation(object_shader.Program, "F0");


        glUniform3fv(pointLightLocation, 1, glm::value_ptr(lightPos0));
        glUniform1f(kdLocation, Kd);
        glUniform1f(alphaLocation, alpha);
        glUniform1f(f0Location, F0);
        glUniform3f(glGetUniformLocation(object_shader.Program, "ambientColor"), ambientColor.x, ambientColor.y, ambientColor.z);
        glUniform1f(glGetUniformLocation(object_shader.Program, "Ka"), Ka);

        glUniform3fv(objDiffuseLocation, 1, planeMaterial);


        planeModelMatrix = glm::mat4(1.0f);
        planeNormalMatrix = glm::mat3(1.0f);
        planeModelMatrix = glm::translate(planeModelMatrix, plane_pos);
        planeModelMatrix = glm::scale(planeModelMatrix, plane_size);
        planeNormalMatrix = glm::inverseTranspose(glm::mat3(view*planeModelMatrix));
        glUniformMatrix4fv(glGetUniformLocation(object_shader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(planeModelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(object_shader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(planeNormalMatrix));

        cubeModel.Draw();
        planeModelMatrix = glm::mat4(1.0f);
        
        glUniform3fv(objDiffuseLocation, 1, diffuseColor);
        for (Room& room : rooms) 
        {
            glm::mat4 modelMatrix = glm::mat4(1.0f);
            glm::mat3 normalMatrix = glm::mat3(1.0f);
            modelMatrix = glm::translate(modelMatrix, room.position);
            modelMatrix = glm::scale(modelMatrix, room.size);
            normalMatrix = glm::inverseTranspose(glm::mat3(view * modelMatrix));

            glUniformMatrix4fv(glGetUniformLocation(object_shader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
            glUniformMatrix3fv(glGetUniformLocation(object_shader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));

            cubeModel.Draw();
        }
        GLfloat matrix[16];
        btTransform transform;

        glm::vec3 obj_size;
        Model* objectModel;

        int num_cobjs = bulletSimulation.dynamicsWorld->getNumCollisionObjects();

        for (int i=0; i<num_cobjs; i++)
        {

            if (i > 0)  
            {
                objectModel = &sphereModel;
                obj_size = sphere_size;
                glUniform3fv(objDiffuseLocation, 1, shootColor);

                btCollisionObject* obj = bulletSimulation.dynamicsWorld->getCollisionObjectArray()[i];

                btRigidBody* body = btRigidBody::upcast(obj);

                body->getMotionState()->getWorldTransform(transform);
                transform.getOpenGLMatrix(matrix);

                objModelMatrix = glm::mat4(1.0f);
                objNormalMatrix = glm::mat3(1.0f);

                objModelMatrix = glm::make_mat4(matrix) * glm::scale(objModelMatrix, obj_size);

                objNormalMatrix = glm::inverseTranspose(glm::mat3(view*objModelMatrix));
                glUniformMatrix4fv(glGetUniformLocation(object_shader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(objModelMatrix));
                glUniformMatrix3fv(glGetUniformLocation(object_shader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(objNormalMatrix));

                objectModel->Draw();
            }
        }


        glfwSwapBuffers(window);
    }

    object_shader.Delete();

    bulletSimulation.Clear();

    glfwTerminate();
    return 0;
}
bool isInsideRoom(const glm::vec3& position, const Room& room) 
{
    glm::vec3 minBounds = room.position - room.size / 2.0f;
    glm::vec3 maxBounds = room.position + room.size / 2.0f;
    return (position.x >= minBounds.x && position.x <= maxBounds.x &&
    position.y >= minBounds.y && position.y <= maxBounds.y &&
    position.z >= minBounds.z && position.z <= maxBounds.z);
}
Room* getCurrentRoom(glm::vec3 position) 
{
    for (Room& room : rooms) 
    {
        if (isInsideRoom(position, room)) 
        {
            return &room;
        }
    }
    return nullptr;
}
void updateBulletPhysics() {
    int num_cobjs = bulletSimulation.dynamicsWorld->getNumCollisionObjects();
    for (int i = 0; i < num_cobjs; i++) {
        btCollisionObject* obj = bulletSimulation.dynamicsWorld->getCollisionObjectArray()[i];
        btRigidBody* body = btRigidBody::upcast(obj);
        if (body && body->getMass() == 1.0f) { 
            btTransform transform;
            body->getMotionState()->getWorldTransform(transform);
            glm::vec3 position = glm::vec3(transform.getOrigin().getX(), transform.getOrigin().getY(), transform.getOrigin().getZ());

            Room* currentRoom = getCurrentRoom(position);
            if (currentRoom) {
                btVector3 roomGravity = btVector3(currentRoom->gravity.x, currentRoom->gravity.y, currentRoom->gravity.z);
                body->setGravity(roomGravity);
            }
        }
    }
}

void apply_camera_movements()
{
    float originalY = camera.Position.y;
    GLboolean diagonal_movement = (keys[GLFW_KEY_W] ^ keys[GLFW_KEY_S]) && (keys[GLFW_KEY_A] ^ keys[GLFW_KEY_D]); 
    camera.SetMovementCompensation(diagonal_movement);
    
    if(keys[GLFW_KEY_W])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if(keys[GLFW_KEY_S])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if(keys[GLFW_KEY_A])
        camera.ProcessKeyboard(LEFT, deltaTime);
    if(keys[GLFW_KEY_D])
        camera.ProcessKeyboard(RIGHT, deltaTime);

    camera.Position.y = originalY;
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if(key == GLFW_KEY_L && action == GLFW_PRESS)
        wireframe=!wireframe;

    
    btVector3 impulse;
    glm::vec3 rot = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec4 shoot;
    
    GLfloat shootInitialSpeed = 15.0f;
  
    btRigidBody* sphere;
    glm::mat4 unproject;

    if(key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {

        Room* firingRoom = getCurrentRoom(glm::vec3(camera.Position.x, camera.Position.y, camera.Position.z));

        if (firingRoom) {
            sphere = bulletSimulation.createRigidBody(SPHERE, camera.Position, sphere_size, rot, 1.0f, firingRoom->friction, 0.3f);
            sphere->setGravity(btVector3(firingRoom->gravity.x, firingRoom->gravity.y, firingRoom->gravity.z));
        } else {
            sphere = bulletSimulation.createRigidBody(SPHERE, camera.Position, sphere_size, rot, 1.0f, 0.3f, 0.3f);
        }
        shoot.x = (cursorX/screenWidth) * 2.0f - 1.0f;
        shoot.y = -(cursorY/screenHeight) * 2.0f + 1.0f; 
        
        shoot.z = 1.0f;

        shoot.w = 1.0f;
        unproject = glm::inverse(projection * view);

        
        shoot = glm::normalize(unproject * shoot) * shootInitialSpeed;

        impulse = btVector3(shoot.x, shoot.y, shoot.z);
        sphere->applyCentralImpulse(impulse);
    }

    
    if(action == GLFW_PRESS)
        keys[key] = true;
    else if(action == GLFW_RELEASE)
        keys[key] = false;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
   
    if(firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    cursorX = xpos;
    cursorY = ypos;

    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
    

}
