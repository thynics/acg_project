#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "ShadowMap.h"



// 屏幕尺寸
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// 摄像机
Camera camera(glm::vec3(0.0f, 1.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// 时间差
float deltaTime = 0.0f;
float lastFrame = 0.0f;

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void DrawShadowMap(Shader& shader, ShadowMap& sm, Model& model, glm::mat4& matModel, glm::vec3 lightDir);

int main()
{
    // 初始化 GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OBJ Model Viewer with Point Light", NULL, NULL);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // 捕获鼠标

    // 初始化 GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    // 加载着色器
    Shader shader("shaders/default.vert", "shaders/default.frag");
    Shader shaderDepth("shaders/depth.vert", "shaders/depth.frag");

    // 加载模型
    Model model("assets/model.obj");
    std::pair<glm::vec3, glm::vec3> temp = model.CalculateWorldAABB(glm::mat4(1.0f));
    std::cout << temp.first.x << " " << temp.first.y << " " << temp.first.z << std::endl;
    std::cout << temp.second.x << " " << temp.second.y << " " << temp.second.z << std::endl;


    ShadowMap sm(1024, 1024);

    // 渲染循环
    while (!glfwWindowShouldClose(window))
    {
        // 时间逻辑
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        // 设置平行光
        glm::vec3 lightDir(1, 1, 1); // 从着色点指向光源
        glm::vec3 lightColor(1.0f);

        glm::mat4 matModel = glm::mat4(1.0f);


        DrawShadowMap(shaderDepth, sm, model, matModel, lightDir);


        // 清屏
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClearColor(0.1f, 0.15f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 设置 Shader 和变换矩阵
        shader.use();

        // 绑定texture
        glActiveTexture(GL_TEXTURE10);
        glBindTexture(GL_TEXTURE_2D, sm.GetDepthMapTexture());

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
                                                (float)SCR_WIDTH / (float)SCR_HEIGHT,
                                                0.1f, 100.0f);
        glm::mat4 lightMatrix = glm::translate(glm::vec3(0.5f)) * glm::scale(glm::vec3(0.5f)) * sm.matProj * sm.matView;

        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        shader.setMat4("model", matModel);
        shader.setMat4("lightMatrix", lightMatrix);

        // 设置点光源
        // glm::vec3 lightPos(1.5f, 2.0f, 2.0f);
        // glm::vec3 lightColor(1.0f); // 白光

        // shader.setVec3("light.position", lightPos);
        // shader.setVec3("light.color", lightColor);
        

        shader.setVec3("dirLight.dir", lightDir);
        shader.setVec3("dirLight.color", lightColor);

        // 设置摄像机位置用于计算镜面反射
        shader.setVec3("viewPos", camera.Position);

        // 渲染模型
        model.Draw(shader);

        // 交换缓冲和事件
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}


void DrawShadowMap(Shader &shader, ShadowMap &sm, Model &model, glm::mat4& matModel, glm::vec3 lightDir)
{
    sm.BindForWriting();
    shader.use();

    std::pair<glm::vec3, glm::vec3> aabb = model.CalculateWorldAABB(matModel);
    glm::vec3 aabbCenter((aabb.first + aabb.second)*0.5f);
    float radius = glm::distance(aabbCenter, aabb.first);
    glm::vec3 shadowMapEye(aabbCenter + glm::normalize(lightDir)*radius);
    glm::vec3 shadowMapAt = aabbCenter;
    glm::vec3 shadowMapUp(0.0f, 1.0f, 0.0f);
    glm::mat4 matShadowView = glm::lookAt(shadowMapEye, shadowMapAt, shadowMapUp);
    glm::mat4 matShadowProj = glm::ortho(-radius, radius, -radius, radius, 0.1f, radius * 2.0f);

    // glm::vec3 shadowMapEye(5.0f, 5.0f, 5.0f);
    // glm::vec3 shadowMapAt(0.0f);
    // glm::vec3 shadowMapUp(0.0f, 1.0f, 0.0f);
    // glm::mat4 matShadowView = glm::lookAt(shadowMapEye, shadowMapAt, shadowMapUp);
    // float radius = 5;
    // glm::mat4 matShadowProj = glm::ortho(-radius, radius, -radius, radius, 0.1f, radius * 2.0f);

    sm.matView = matShadowView;
    sm.matProj = matShadowProj;

    shader.setMat4("modelViewProjectionMatrix", matShadowProj*matShadowView*matModel);
    
    model.Draw(shader);

    sm.Unbind();
}


void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera_Movement::FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera_Movement::BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera_Movement::LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera_Movement::RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera_Movement::DOWN, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera_Movement::UP, deltaTime);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);
        firstMouse = false;
    }

    float xoffset = static_cast<float>(xpos - lastX);
    float yoffset = static_cast<float>(lastY - ypos); // 反转 Y 方向

    lastX = static_cast<float>(xpos);
    lastY = static_cast<float>(ypos);

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
