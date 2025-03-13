#pragma once

#include "opengl.hpp"
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
	void addSource(std::string label, GLuint shaderType, unsigned int count, const char** lines, const int* lineLengths);
	void link();
	int getUniform(std::string name) const;
	void use();

private:
	GLuint mProgram;
};
