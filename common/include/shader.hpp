#pragma once

#include <glad/glad.h>
#include <string>

class Shader {
public:
	Shader() {
		mProgram = glCreateProgram();
	}

	~Shader() {
		glDeleteProgram(mProgram);
	}

	void addSource(std::string filename);
	void link();
	int getUniform(std::string name) const;
	void use();

private:
	GLuint mProgram;
};
