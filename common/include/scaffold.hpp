#include <glfw/glfw3.h>
#include <string>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_glfw.h>
#include <glad/glad.h>

// Class with a lot of methods that you can override in order to hook code to different points in the window lifecycle.
class Scaffold {
public:
    // Return the app window's title
    virtual std::string initTitle() { return "Application"; }
    // Run once when the graphics context and imgui are ready
    virtual void setup() {}
    // Run once before the imgui and window context get destroyed
    virtual void cleanup() {}
    // Run once every frame after the frame buffer is cleared
    virtual void draw() {}
    // Run once every frame before imgui gets rendered
    virtual int imgui() { return 0; }
    // Run once whenever the window gets resized
    virtual void onResize() {}

    // Width of the frame buffer. You can use this to compute the rendering aspect ratio
    int width;
    // Height of the frame buffer. You can use this to compute the rendering aspect ratio
    int height;
};

class BaseScaffold : public Scaffold {
};

bool windowSizeNeedsUpdate = true;
bool mouseNeedsUpdate = true;

void resizeCallback(GLFWwindow* window, int width, int height)
{
    windowSizeNeedsUpdate = true;
}

// Run the given application in a new window, calling the scaffold methods whenever appropriate
int runApplication(Scaffold& app) {
    
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, app.initTitle().c_str(), NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* Load GLAD bindings */
    gladLoadGL();

    /* Create Context of ImGui */
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    app.setup();

    glfwSetWindowSizeCallback(window, &resizeCallback);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        if (windowSizeNeedsUpdate) {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            glViewport(0, 0, width, height);
            app.width = width;
            app.height = height;
            app.onResize();
            windowSizeNeedsUpdate = false;
        }

        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        app.imgui();
        ImGui::Render();

        /* Render here */
        glClearColor(0.1, 0.1, 0.1, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        app.draw();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        /* Swap front and back buffers */
        glfwSwapBuffers(window);
    }

    app.cleanup();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
