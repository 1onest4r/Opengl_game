#pragma once

#include "config.h"

class Shader {
public:
    Shader(
        const std::string& name,
        const std::string& vertexPath,
        const std::string& fragmentPath);
    void use();
    void reload();
    void hotReloadIfChanged();

    GLuint id() const {return m_program;}

private:
    std::string m_name;
    
    GLuint m_program = 0;
    
    std::string m_vertexPath;
    std::string m_fragmentPath;

    std::filesystem::file_time_type m_vertTime;
    std::filesystem::file_time_type m_fragTime;

    GLuint make_module(const std::string& path, GLuint module_type);
    GLuint make_shader();
};

// unsigned int make_module(const std::string& file_path, unsigned int module_type);

// unsigned int make_shader(const std::string& vertex_file_path, const std::string& fragment_file_path);