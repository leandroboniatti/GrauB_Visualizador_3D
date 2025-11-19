#include "Shader.h"
#include <iostream>
#include <fstream>
#include <sstream>

Shader::Shader() : ID(0) {}

Shader::Shader(const string& vertexPath, const string& fragmentPath) : ID(0) {
    loadFromFiles(vertexPath, fragmentPath);
}

Shader::~Shader() {
    cleanup();
}

bool Shader::loadFromFiles(const string& vertexPath, const string& fragmentPath) {
    string vertexSource = readFile(vertexPath);
    string fragmentSource = readFile(fragmentPath);
    
    if (vertexSource.empty() || fragmentSource.empty()) {
        cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << endl;
        return false;
    }
    
    return loadFromStrings(vertexSource, fragmentSource);
}

bool Shader::loadFromStrings(const string& vertexSource, const string& fragmentSource) {
    cleanup();
    
    // Compile shaders
    unsigned int vertex = compileShader(vertexSource, GL_VERTEX_SHADER);
    unsigned int fragment = compileShader(fragmentSource, GL_FRAGMENT_SHADER);
    
    if (vertex == 0 || fragment == 0) {
        if (vertex != 0) glDeleteShader(vertex);
        if (fragment != 0) glDeleteShader(fragment);
        return false;
    }
    
    // Create shader program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    
    // Check for linking errors
    if (!checkCompileErrors(ID, "PROGRAM")) {
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        glDeleteProgram(ID);
        ID = 0;
        return false;
    }
    
    // Delete shaders as they're linked into program now
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    
    cout << "Shader program created successfully (ID: " << ID << ")" << endl;
    return true;
}

void Shader::use() const {
    if (ID != 0) {
        glUseProgram(ID);
    }
}

string Shader::readFile(const string& filePath) const {
    string content;
    ifstream file;
    
    file.exceptions(ifstream::failbit | ifstream::badbit);
    
    try {
        file.open(filePath);
        stringstream stream;
        stream << file.rdbuf();
        file.close();
        content = stream.str();
    } catch (ifstream::failure& e) {
        cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << filePath << endl;
        cout << "Error: " << e.what() << endl;
    }
    
    return content;
}

unsigned int Shader::compileShader(const string& source, GLenum shaderType) const {
    unsigned int shader = glCreateShader(shaderType);
    const char* sourceCStr = source.c_str();
    glShaderSource(shader, 1, &sourceCStr, NULL);
    glCompileShader(shader);
    
    string type = (shaderType == GL_VERTEX_SHADER) ? "VERTEX" : "FRAGMENT";
    if (!checkCompileErrors(shader, type)) {
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

bool Shader::checkCompileErrors(unsigned int shader, const string& type) const {
    int success;
    char infoLog[1024];
    
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << endl;
            return false;
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << endl;
            return false;
        }
    }
    
    return true;
}

void Shader::cleanup() {
    if (ID != 0) {
        glDeleteProgram(ID);
        ID = 0;
    }
}

// Utility uniform functions
void Shader::setBool(const string& name, bool value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::setInt(const string& name, int value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const string& name, float value) const {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setVec3(const string& name, const vec3& value) const {
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Shader::setVec3(const string& name, float x, float y, float z) const {
    glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
}

void Shader::setMat4(const string& name, const mat4& mat) const {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
