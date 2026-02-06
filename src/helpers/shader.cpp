#include "shader.h"
#include "config.h"

Shader::Shader(
    const std::string& name,
    const std::string& vertexPath, 
    const std::string& fragmentPath):
    m_name(name),
    m_vertexPath(vertexPath),
    m_fragmentPath(fragmentPath) { 

    m_vertTime = std::filesystem::last_write_time(vertexPath);
    m_fragTime = std::filesystem::last_write_time(fragmentPath);

    m_program = make_shader();

    std::cout << "[Shader: " << m_name << "] created\n";

}

GLuint Shader::make_module(const std::string& file_path, GLenum module_type) {
    std::ifstream file(file_path);
    std::stringstream buffer;
    buffer << file.rdbuf();

    std::string src = buffer.str();
    const char* shaderSrc = src.c_str();

    GLuint module = glCreateShader(module_type);
    glShaderSource(module, 1, &shaderSrc, nullptr);
    glCompileShader(module);

    int success;
    glGetShaderiv(module, GL_COMPILE_STATUS, &success);
    if (!success) {
        char errorLog[1024];
        glGetShaderInfoLog(module, 1024, NULL, errorLog);
        std::cout << "Shader module compilation error:\n " << errorLog << std::endl;
    }

    return module;
}

GLuint Shader::make_shader() {
    GLuint vert = make_module(m_vertexPath, GL_VERTEX_SHADER);
    GLuint frag = make_module(m_fragmentPath, GL_FRAGMENT_SHADER);
    
    if (!vert || !frag) return 0;

    GLuint program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char errorLog[1024];
        glGetShaderInfoLog(program, 1024, NULL, errorLog);
        std::cout << "Shader program linking error:\n " << errorLog << std::endl;
    }

    glDeleteShader(vert);
    glDeleteShader(frag);

    return program;

}

void Shader::hotReloadIfChanged() {
    auto vt = std::filesystem::last_write_time(m_vertexPath);
    auto ft = std::filesystem::last_write_time(m_fragmentPath);

    if (vt != m_vertTime || ft != m_fragTime) {
        std::cout << "[Shader: " << m_name <<"] File changed, recompiling" << std::endl;
        reload();
        m_vertTime = vt;
        m_fragTime = ft;
    }
}

void Shader::reload() {
    GLuint new_program = make_shader();

    if (!new_program) {
        std::cout << "[Shader: " << m_name << "] reload failed" << std::endl;
        return;
    }

    glUseProgram(0);
    if (m_program) {
        glDeleteProgram(m_program);
    }

    m_program = new_program;
    std::cout << "[Shader: " << m_name << "] reloaded successfully" << std::endl;
}

void Shader::use() {
    glUseProgram(m_program);
}


// unsigned int make_module(const std::string& file_path, unsigned int module_type) {
//     std::ifstream file;
//     std::stringstream bufferedLines;
//     std::string line;

//     file.open(file_path);
//     while(std::getline(file, line)) {
//         bufferedLines << line << "\n";
//     }
//     std::string shaderSource = bufferedLines.str();
//     const char* shaderSrc = shaderSource.c_str();
//     bufferedLines.str("");
//     file.close();

//     unsigned int shaderModule = glCreateShader(module_type);
//     glShaderSource(shaderModule, 1, &shaderSrc, NULL);
//     glCompileShader(shaderModule);

//     int success;
//     glGetShaderiv(shaderModule, GL_COMPILE_STATUS, &success);
//     if (!success) {
//         char errorLog[1024];
//         glGetShaderInfoLog(shaderModule, 1024, NULL, errorLog);
//         std::cout << "Shader module compilation error:\n" << errorLog << std::endl;
//     }

//     return shaderModule;
// }

// unsigned int make_shader(const std::string& vertex_file_path, const std::string& fragment_file_path) { 
//     std::vector<unsigned int> modules;
//     modules.push_back(make_module(vertex_file_path, GL_VERTEX_SHADER));
//     modules.push_back(make_module(fragment_file_path, GL_FRAGMENT_SHADER));

//     unsigned int shader = glCreateProgram();
//     for (unsigned int shaderModule : modules) {
//         glAttachShader(shader, shaderModule);
//     }
//     glLinkProgram(shader);

//     int success;
//     glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
//     if (!success) {
//         char errorLog[1024];
//         glGetProgramInfoLog(shader, 1024, NULL, errorLog);
//         std::cout << "Shader linking error:\n" << errorLog << std::endl;
//     }

//     for (unsigned int shaderModule : modules) {
//         glDeleteShader(shaderModule);
//     }

//     return shader;

// }