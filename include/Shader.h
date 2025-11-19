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
    Shader(const string& vertexPath, const string& fragmentPath);
    ~Shader();

    bool loadFromFiles(const string& vertexPath, const string& fragmentPath);
    bool loadFromStrings(const string& vertexSource, const string& fragmentSource);

    void use() const;
    
    // Funções utilitárias para uniformes
    void setBool (const string& name, bool value) const;
    void setInt  (const string& name, int value) const;
    void setFloat(const string& name, float value) const;
    void setVec3 (const string& name, const vec3& value) const;
    void setVec3 (const string& name, float x, float y, float z) const;
    void setMat4 (const string& name, const mat4& mat) const;
    
private:
    string readFile(const string& filePath) const;
    unsigned int compileShader(const string& source, GLenum shaderType) const;
    bool checkCompileErrors(unsigned int shader, const string& type) const;
    void cleanup();
};

#endif
