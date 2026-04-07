#include "shader.h"
#include "config.h"

Shader::Shader(
    const std::string& name,
    const std::string& vertexPath, 
    const std::string& fragmentPath,
    const std::string& geometryPath,
    const std::string& tessControlPath,
    const std::string& tessEvalPath):
    m_name(name),
    m_vertexPath(vertexPath),
    m_fragmentPath(fragmentPath),
    m_geometryPath(geometryPath),
    m_tessControlPath(tessControlPath),
    m_tessEvalPath(tessEvalPath) { 

    m_vertTime = std::filesystem::last_write_time(vertexPath);
    m_fragTime = std::filesystem::last_write_time(fragmentPath);
    if (!m_geometryPath.empty()) {
        m_geomTime = std::filesystem::last_write_time(m_geometryPath);
    }
    if (!m_tessControlPath.empty()) {
        m_tessCtrlTime = std::filesystem::last_write_time(m_tessControlPath);
    }
    if (!m_tessEvalPath.empty()) {
        m_tessEvalTime = std::filesystem::last_write_time(m_tessEvalPath);
    }

    m_program = make_shader();

    std::cout << "[Shader: " << m_name << "] created\n";

}

GLuint Shader::make_module(const std::string& file_path, GLenum module_type) {
    if (file_path.empty()) {
        return 0;
    }

    std::ifstream file(file_path);
    if (!file.is_open()) {
        std::cout << "Failed to open shader file: " << file_path << std::endl;
        return 0;
    }
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
        std::cout << "Shader module compilation error in " << file_path << ":\n " << errorLog << std::endl;
        glDeleteShader(module);
        return 0;
    }

    return module;
}

GLuint Shader::make_shader() {
    GLuint vert = make_module(m_vertexPath, GL_VERTEX_SHADER);
    GLuint frag = make_module(m_fragmentPath, GL_FRAGMENT_SHADER);
    GLuint geom = make_module(m_geometryPath, GL_GEOMETRY_SHADER);
    GLuint tessCtrl = make_module(m_tessControlPath, GL_TESS_CONTROL_SHADER);
    GLuint tessEval = make_module(m_tessEvalPath, GL_TESS_EVALUATION_SHADER);
    
    if (!vert || !frag) return 0;

    GLuint program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    if (geom) glAttachShader(program, geom);
    if (tessCtrl) glAttachShader(program, tessCtrl);
    if (tessEval) glAttachShader(program, tessEval);
    
    glLinkProgram(program);

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char errorLog[1024];
        glGetProgramInfoLog(program, 1024, NULL, errorLog);
        std::cout << "Shader program linking error:\n " << errorLog << std::endl;
    }

    glDeleteShader(vert);
    glDeleteShader(frag);
    if (geom) glDeleteShader(geom);
    if (tessCtrl) glDeleteShader(tessCtrl);
    if (tessEval) glDeleteShader(tessEval);

    return program;

}

void Shader::hotReloadIfChanged() {
    bool changed = false;
    auto vt = std::filesystem::last_write_time(m_vertexPath);
    if (vt != m_vertTime) {
        m_vertTime = vt;
        changed = true;
    }

    auto ft = std::filesystem::last_write_time(m_fragmentPath);
    if (ft != m_fragTime) {
        m_fragTime = ft;
        changed = true;
    }

    if (!m_geometryPath.empty()) {
        auto gt = std::filesystem::last_write_time(m_geometryPath);
        if (gt != m_geomTime) {
            m_geomTime = gt;
            changed = true;
        }
    }

    if (!m_tessControlPath.empty()) {
        auto tct = std::filesystem::last_write_time(m_tessControlPath);
        if (tct != m_tessCtrlTime) {
            m_tessCtrlTime = tct;
            changed = true;
        }
    }

    if (!m_tessEvalPath.empty()) {
        auto tet = std::filesystem::last_write_time(m_tessEvalPath);
        if (tet != m_tessEvalTime) {
            m_tessEvalTime = tet;
            changed = true;
        }
    }

    if (changed) {
        std::cout << "[Shader: " << m_name <<"] File changed, recompiling" << std::endl;
        reload();
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