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
#include "csm.h"

//#define USE_SM
#define USE_CSM



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
float startFrame = 0.0f;
int frame = 0;

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void DrawShadowMap(Shader& shader, ShadowMap& sm, Model& model, glm::mat4& matModel, glm::vec3 lightDir);

unsigned int quadVAO = 0, quadVBO = 0;

void RenderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions   // texCoords
            -1.0f, -1.0f,  0.0f, 0.0f,
             3.0f, -1.0f,  2.0f, 0.0f,   // Trick: 只需 3 个顶点渲染全屏
            -1.0f,  3.0f,  0.0f, 2.0f
        };
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);

        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    }

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}


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

    // 初始化framebuffer

    // ----------------初始化g-buffer----------------------------
    GLuint gBuffer;
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    // 2. gPosition
    GLuint gPos; glGenTextures(1,&gPos);
    glBindTexture(GL_TEXTURE_2D,gPos);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB16F,SCR_WIDTH,SCR_HEIGHT,0,GL_RGB,GL_FLOAT,nullptr);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,gPos,0);
    GLuint gNormal;
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
    GLuint depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8, SCR_WIDTH, SCR_HEIGHT, 0, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    GLuint attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, attachments);

    // ---------- 初始化 sceneFBO，用于存储前向渲染结果 ----------
    GLuint sceneFBO, texSceneColor;
    glGenFramebuffers(1, &sceneFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
    // 创建颜色纹理（RGB16F 更适合 HDR 场景）
    glGenTextures(1, &texSceneColor);
    glBindTexture(GL_TEXTURE_2D, texSceneColor);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0,
                GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                        GL_TEXTURE_2D, texSceneColor, 0);
    // 重用 GBuffer 的深度纹理（假设已生成 texDepth）
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                        GL_TEXTURE_2D, depthMap, 0);
    // 检查 FBO 完整性
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "SceneFBO is not complete!" << std::endl;

    
    // ---------- SSR 输出帧缓 ----------
    GLuint ssrFBO, texSSR;
    glGenFramebuffers(1, &ssrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ssrFBO);
    // 创建 SSR 输出纹理（颜色）
    glGenTextures(1, &texSSR);
    glBindTexture(GL_TEXTURE_2D, texSSR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0,
                GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                        GL_TEXTURE_2D, texSSR, 0);
    // 无需深度附件，复用前面已有的 texDepth
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "SSR FBO incomplete!\n";


    

    // 加载着色器
    Shader shader("shaders/default.vert", "shaders/default.frag");
    Shader shaderDepth("shaders/depth.vert", "shaders/depth.frag");
    Shader shaderCSM("shaders/csm_shading.vert", "shaders/csm_shading.frag");

    Shader shaderSimple("shaders/simple.vert", "shaders/simple.frag");

    Shader shadergbuffer("shaders/gbuffer.vert", "shaders/gbuffer.frag");
    Shader shaderCombine("shaders/quad.vert", "shaders/quad_combine.frag");
    Shader shaderSSR("shaders/ssr.vert", "shaders/ssr.frag");
    Shader shaderNormal("shaders/normal.vert", "shaders/normal.frag");

    Shader activeShader = shader;

    // 加载模型
    Model model("assets/sponza/sponza.obj");
    // Model model("assets/model.obj");
    std::pair<glm::vec3, glm::vec3> temp = model.CalculateWorldAABB(glm::mat4(1.0f));
    std::cout << temp.first.x << " " << temp.first.y << " " << temp.first.z << std::endl;
    std::cout << temp.second.x << " " << temp.second.y << " " << temp.second.z << std::endl;
    
    int shadowMapResolution = 1024;
    ShadowMap sm(shadowMapResolution, shadowMapResolution);

    int cascadeCount = 4;
    float camNearPlane = 0.1f;
    float camFarPlane = 100.0f;
    CSM csm(cascadeCount, shadowMapResolution, camNearPlane, camFarPlane);

    startFrame = static_cast<float>(glfwGetTime());
    float lastPrint = 0;
    // 渲染循环
    while (!glfwWindowShouldClose(window))
    {
        // 时间逻辑
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        frame++;
        if (currentFrame - lastPrint >= 1.0f) {
            std::cout << "FPS: " << frame / (currentFrame - startFrame) << std::endl;
            lastPrint = currentFrame;
        }


        processInput(window);

        // 设置平行光
        glm::vec3 lightDir = glm::normalize(glm::vec3(-1, 3, -1)); // 从着色点指向光源
        glm::vec3 lightColor(1.0f);

        glm::mat4 matModel = glm::scale(glm::mat4(1.0f), glm::vec3(0.01f));

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
                                                (float)SCR_WIDTH / (float)SCR_HEIGHT,
                                                camNearPlane, camFarPlane);
        
                            
        //-------------------------- Simple Test ----------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClearColor(0.f, 0.f, 0.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shaderSimple.use();
        shaderSimple.setMat4("model", matModel);
        shaderSimple.setMat4("view", view);
        shaderSimple.setMat4("projection", projection);
        shaderSimple.setVec3("dirLight.dir", lightDir);
        shaderSimple.setVec3("dirLight.color", lightColor);
        shaderSimple.setVec3("viewPos", camera.Position);
        model.Draw(shaderSimple);
        glfwSwapBuffers(window);
        glfwPollEvents();
        continue;

        #ifdef USE_SM
            DrawShadowMap(shaderDepth, sm, model, matModel, lightDir);
        #endif
        #ifdef USE_CSM
            csm.ComputeLightSpaceMatrix(view, projection, matModel, camera.Position, camera.Front, model, lightDir);
            csm.DrawShadowMaps(shaderDepth, model, matModel);
        #endif


        // ---------- gBuffer pass -----------------
        // glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        // glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        // glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // shadergbuffer.use();
        // shadergbuffer.setMat4("view", view);
        // shadergbuffer.setMat4("projection", projection);
        // shadergbuffer.setMat4("model", matModel);
        // model.Draw(shadergbuffer);


        // ---------- scene pass ----------
        // glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClearColor(0.1f, 0.15f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        #ifdef USE_SM
            // 设置 Shader 和变换矩阵
            activeShader = shader;
            //shader.use();
            // 绑定texture
            glActiveTexture(GL_TEXTURE10);
            glBindTexture(GL_TEXTURE_2D, sm.GetDepthMapTexture());
        #endif
        #ifdef USE_CSM
            //shaderCSM.use();
            activeShader = shaderCSM;
            glActiveTexture(GL_TEXTURE10);
            glBindTexture(GL_TEXTURE_2D_ARRAY, csm.GetShadowMapArrayTexture());
        #endif
        activeShader.use();
        activeShader.setMat4("view", view);
        activeShader.setMat4("projection", projection);
        activeShader.setMat4("model", matModel);
        activeShader.setVec3("dirLight.dir", lightDir);
        activeShader.setVec3("dirLight.color", lightColor);
        // 设置摄像机位置用于计算镜面反射
        activeShader.setVec3("viewPos", camera.Position);

        #ifdef USE_SM
            glm::mat4 lightMatrix = glm::translate(glm::vec3(0.5f)) * glm::scale(glm::vec3(0.5f)) * sm.matProj * sm.matView;
            activeShader.setMat4("lightMatrix", lightMatrix);
        #endif
        #ifdef USE_CSM
            std::vector<float> cascadeSplit = csm.GetCascadeSplits();
            std::vector<glm::mat4> lightMatrices = csm.GetLightSpaceMatrices();
            for (int i = 0; i < cascadeCount; i++){
                std::string float_name = "cascadeSplit[" + std::to_string(i) + "]"; 
                activeShader.setFloat(float_name, cascadeSplit[i+1]);
                std::string mat4_name = "lightMatrices[" + std::to_string(i) + "]";
                activeShader.setMat4(mat4_name, lightMatrices[i]);
            }
        #endif

        // 渲染模型
        model.Draw(activeShader); // draw scene fbo
        
        glfwSwapBuffers(window);
        glfwPollEvents();
        continue;
        
        // ---------- SSR Pass ----------
        glBindFramebuffer(GL_FRAMEBUFFER, ssrFBO);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT);

        shaderSSR.use();
        shaderSSR.setMat4("invProj", glm::inverse(projection));
        shaderSSR.setMat4("proj", projection);
        shaderSSR.setMat4("view", view);
        shaderSSR.setFloat("thickness", 0.001f);
        shaderSSR.setFloat("stepSize",  0.05f);

        // 输入纹理绑定
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texSceneColor);
        shaderSSR.setInt("sceneColor", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        shaderSSR.setInt("normalTex", 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        shaderSSR.setInt("depthTex", 2);

        // 渲染到 texSSR
        RenderQuad();


        // combine pass
        // ---------- (5) Combine Pass ----------
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // 回到默认帧缓冲
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 使用合成 shader
        shaderCombine.use();
        shaderCombine.setFloat("roughnessScale", 1.0f);  // 控制 SSR 强度

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texSceneColor); // 前向颜色输出
        shaderCombine.setInt("sceneColor", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texSSR);        // SSR 输出
        shaderCombine.setInt("ssrTex", 1);
        
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        shaderCombine.setInt("normalTex", 2);

        // 画全屏三角形或四边形
        RenderQuad();
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
