#include "scaffold.hpp"
#include "shader.hpp"
#include <glad/glad.h>
#include <memory>
#include <backends/imgui_impl_opengl3.h>

struct Vertex {
	float position[2];
	float color[3];
};
Vertex vertices_initial[] = {
	{ { 0.0, 0.6 }, { 1.0, 0.0, 0.0 } },
	{ {  -0.4, 0.0 }, { 0.0, 1.0, 0.0 } },
	{ { 0.4, 0.0 }, { 0.0, 0.0, 1.0 } }
};

class App : public BaseScaffold {
public:
	~App() {
		glDeleteBuffers(1, &mVertexBuffer);
	}
	void setup() {
		mShader = std::make_unique<Shader>();
		mShader->addSource(PROJECT_SOURCE_DIR "shaders/2d.vert");
		mShader->addSource(PROJECT_SOURCE_DIR "shaders/2d.frag");
		mShader->link();

		glGenVertexArrays(1, &mVertexArray);
		glBindVertexArray(mVertexArray);

		glGenBuffers(1, &mVertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_initial), vertices_initial, GL_DYNAMIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	void draw() {
		mShader->use();

		glBindVertexArray(mVertexArray);
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}

	int imgui() {
		bool colorsChanged = false;
		
		ImGui::Begin("Triangle");
		if (ImGui::ColorEdit3("Vertex 1", vertices_initial[0].color)) {
			colorsChanged = true;
		}
		if (ImGui::ColorEdit3("Vertex 2", vertices_initial[1].color)) {
			colorsChanged = true;
		}
		if (ImGui::ColorEdit3("Vertex 3", vertices_initial[2].color)) {
			colorsChanged = true;
		}
		ImGui::End();

		if (colorsChanged) {
			glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_initial), vertices_initial, GL_DYNAMIC_DRAW);
		}

		return 1;
	}
private:
	std::unique_ptr<Shader> mShader;
	GLuint mVertexBuffer;
	GLuint mVertexArray;
};

int main() {
	std::unique_ptr<App> app = std::make_unique<App>();

	return runApplication(*app);
}
