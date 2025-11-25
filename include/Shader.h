#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;
using namespace glm;

class Shader {
public:
    unsigned int ID;
    
    Shader();
    ~Shader();

    bool loadShaders(const string& vertexSource, const string& fragmentSource);
    
private:
    unsigned int compileShader(const string& source, GLenum shaderType) const;
    bool checkCompileErrors(unsigned int shader, const string& type) const;
    void cleanup();
};

#endif
