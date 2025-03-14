#include "shader.hpp"
#include <iostream>
#include <sstream>
#include <fstream>
#include <streambuf>
#include <spdlog/spdlog.h>

int getShaderType(std::string filename) {
	std::string ext = filename.substr(filename.length() - 4);
	if (ext == "vert") {
		return GL_VERTEX_SHADER;
	}
	if (ext == "frag") {
		return GL_FRAGMENT_SHADER;
	}

	return 0;
}

void Shader::addSource(std::string filename) {
	GLuint shader = 0;

	std::ifstream t(filename);

	if (t.fail()) {
		spdlog::critical("Cannot open shader file {}", filename.c_str());
		return;
	}

	std::stringstream buffer;
	buffer << t.rdbuf();
	std::string source = buffer.str();
	const char* sourceStr = source.c_str();
	const int sourceLength = source.length();

	addSource(filename, getShaderType(filename), 1, &sourceStr, &sourceLength);
}

void Shader::addSource(std::string label, GLuint shaderType, unsigned int count, const char** lines, const int* lineLengths) {
	GLuint shader = 0;

	shader = glCreateShader(shaderType);

	glShaderSource(shader, count, lines, lineLengths);

	glCompileShader(shader);

	int logLength = 0;
	char infoLogBuffer[1024];
	glGetShaderInfoLog(shader, sizeof(infoLogBuffer), &logLength, infoLogBuffer);

	if (logLength > 0) {
		spdlog::critical("There are shader compilation errors for {}!\n{}", label, infoLogBuffer);
		return;
	}

	glAttachShader(mProgram, shader);

	glDeleteShader(shader);
}

void Shader::link() {
	glLinkProgram(mProgram);

	int logLength = 0;
	char infoLogBuffer[1024];
	glGetProgramInfoLog(mProgram, sizeof(infoLogBuffer), &logLength, infoLogBuffer);

	if (logLength > 0) {
		spdlog::critical("There are shader linking errors!\n{}", infoLogBuffer);
		return;
	}
}

int Shader::getAttribute(std::string name) const {
	int location = glGetAttribLocation(mProgram, name.c_str());

	if (location == -1) {
		spdlog::warn("Attribute location \"{}\" not found!", name);
	}

	return location;
}

int Shader::getUniform(std::string name) const {
	int location = glGetUniformLocation(mProgram, name.c_str());

	if (location == -1) {
		spdlog::warn("Uniform location \"{}\" not found!", name);
	}

	return location;
}

void Shader::use() {
	glUseProgram(mProgram);
}

GLuint Shader::get() {
	return mProgram;
}
