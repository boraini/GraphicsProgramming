#include "scaffold.hpp"
#include <memory>
#include "SkinnedMesh.hpp"
#include "shader.hpp"
#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include "MaterialManager.hpp"
#include <spdlog/spdlog.h>

#ifndef __EMSCRIPTEN__
void debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
    spdlog::info("OpenGL debug message: {}", message);
}
#endif

class App : public BaseScaffold {
    void setup() {
        mGlobalMaterialManager = std::make_unique<MaterialManager>();
        globalMaterialManager = mGlobalMaterialManager.get();
        mMesh = std::make_unique<SkinnedMesh>("dancing_vampire/dancing_vampire.dae");

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

#ifndef __EMSCRIPTEN__
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(debugCallback, nullptr);
#endif
    }

    void cleanup() {
        globalMaterialManager->unloadTextures();
    }

    void draw() {
        float nowTime = glfwGetTime();

        float aspect = height == 0 || width == 0 ? 1.0 : (float)width / height;
        glm::mat4 projectionMatrix = glm::perspective(glm::radians(90.0f), aspect, 1.0f, 10.0f);

        glm::mat4 cameraMatrix = glm::identity<glm::mat4>();

        // Move camera up
        cameraMatrix = glm::translate(cameraMatrix, glm::vec3(0.0, 2.0, 0.0));
        // Rotate camera around the vertical axis (azimuth)
        cameraMatrix = glm::rotate(cameraMatrix, 0.5f * nowTime, glm::vec3(0.0f, 1.0f, 0.0f));
        // Rotate camera towards above (pitch)
        cameraMatrix = glm::rotate(cameraMatrix, glm::radians(-15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        // Move camera away from the subject
        cameraMatrix = glm::translate(cameraMatrix, glm::vec3(0.0f, 0.0f, 5.0f));

        glm::mat4 modelMatrix = glm::identity<glm::mat4>();
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.02f, 0.02f, 0.02f));

        mMesh->draw(projectionMatrix, glm::inverse(cameraMatrix), modelMatrix);
    }

private:
    std::unique_ptr<SkinnedMesh> mMesh;
    std::unique_ptr<MaterialManager> mGlobalMaterialManager;
};

int main() {
    std::unique_ptr<App> app = std::make_unique<App>();
    return runApplication(*app);
}
