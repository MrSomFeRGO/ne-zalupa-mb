//
// Created by mrsomfergo on 13.07.2025.
//

#pragma once

#include <glad/glad.h>
#include <string>
#include <glm/glm.hpp>

class Shader {
public:
    Shader();
    ~Shader();

    // Load shaders from source code
    bool LoadFromSource(const std::string& vertexSource, const std::string& fragmentSource);

    // Load shaders from files
    bool LoadFromFiles(const std::string& vertexPath, const std::string& fragmentPath);

    // Use the shader program
    void Use() const;

    // Get program ID
    GLuint GetProgram() const { return m_program; }

    // Utility functions for setting uniforms
    void SetBool(const std::string& name, bool value) const;
    void SetInt(const std::string& name, int value) const;
    void SetFloat(const std::string& name, float value) const;
    void SetVec2(const std::string& name, const glm::vec2& value) const;
    void SetVec3(const std::string& name, const glm::vec3& value) const;
    void SetVec4(const std::string& name, const glm::vec4& value) const;
    void SetMat4(const std::string& name, const glm::mat4& value) const;

private:
    GLuint m_program = 0;

    // Compile individual shader
    GLuint CompileShader(const std::string& source, GLenum type);

    // Link shaders into program
    bool LinkProgram(GLuint vertexShader, GLuint fragmentShader);

    // Check for compilation/linking errors
    void CheckCompileErrors(GLuint shader, const std::string& type);

    // Load file content
    std::string LoadFile(const std::string& filepath);
};