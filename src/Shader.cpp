#include "Shader.h"
#include <iostream>

Shader::Shader() : ID(0) {}

Shader::~Shader() { cleanup(); }

bool Shader::loadShaders(const string& vertexSource, const string& fragmentSource) {

    cleanup(); // Limpa qualquer shader previamente carregado
    
    // Compila os shaders
    unsigned int vertShader = compileShader(vertexSource,   GL_VERTEX_SHADER  ); // compila vertex shader
    unsigned int fragShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER); // compila fragment shader
    
    if (vertShader == 0 || fragShader == 0) {     // verifica se alguma compilação falhou e
        if (vertShader != 0) glDeleteShader(vertShader);    // deleta o shader que eventualmente foi compilado com sucesso
        if (fragShader != 0) glDeleteShader(fragShader);    // deleta o shader que eventualmente foi compilado com sucesso
        return false;
    }
    
    // Cria o programa de shader linkando os shaders compilados
    ID = glCreateProgram();
    if (ID == 0) {
        cout << "ERRO na reserva de recursos da OpenGL para o programa de shader" << endl;
        glDeleteShader(vertShader);
        glDeleteShader(fragShader);
        return false;
    }
    glAttachShader(ID, vertShader);
    glAttachShader(ID, fragShader);
    glLinkProgram(ID);
    
    // Verifica erros de linkagem
    if (!checkCompileErrors(ID, "PROGRAM")) {
        glDeleteShader(vertShader);
        glDeleteShader(fragShader);
        glDeleteProgram(ID);
        ID = 0;
        return false;
    }
    
    // Remove os shaders pois já estão linkados ao programa
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
    
    cout << "Shaders compilados com sucesso! (ID: " << ID << ")" << endl;
    cout << endl;

    return true;
}


unsigned int Shader::compileShader(const string& source, GLenum shaderType) const {

    unsigned int shader = glCreateShader(shaderType);
    const char* sourceCStr = source.c_str();
    glShaderSource(shader, 1, &sourceCStr, NULL);
    glCompileShader(shader);
    
    string type = (shaderType == GL_VERTEX_SHADER) ? "VERTEX" : "FRAGMENT"; // Determina o tipo de shader para 
                                                                            // verificar possiveis erros de compilação
    if (!checkCompileErrors(shader, type)) { // Verifica erros de compilação
        glDeleteShader(shader);              // Limpa o shader com erro
        return 0;
    }
    
    return shader;
}


bool Shader::checkCompileErrors(unsigned int shader, const string& type) const {
    int success;
    char infoLog[1024];
    
    if (type != "PROGRAM") { // = "VERTEX" ou "FRAGMENT"
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            cout << "ERRO DE COMPILACAO DO " << type << " SHADER" << type << "\n" << infoLog << endl;
            return false;
        }
    } else {    // = "PROGRAM"
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            cout << "ERRO DE LINKAGEM DO SHADERS " << "\n" << infoLog << endl;
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
