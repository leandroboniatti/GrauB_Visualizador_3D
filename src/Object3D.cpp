#include "Object3D.h"
#include <iostream>

Object3D::Object3D() 
	: transform(1.0f), 
	  position (0.0f), 
	  rotation (0.0f), 
	  scale    (1.0f), 
	  eliminable(true), 
	  name("")
	//  textureID(0),
	//  hasTexture(false)
	{ updateTransform(); }

Object3D::Object3D(string& objName)
	: transform(1.0f),  // matriz identidade
	  position (0.0f),  // posição zero
	  rotation (0.0f),  // sem rotação
	  scale    (1.0f),  // escala unitária
	  eliminable(true),
	  name(objName)
	//  textureID(0),
	//  hasTexture(false)
	{ updateTransform(); }

Object3D::~Object3D() { mesh.cleanup(); }

bool Object3D::loadObject(string& path) {

	if (!mesh.readObjectModel(path)) {
		cerr << "Falha ao carregar arquivo OBJ: " << path << endl;
		return false;
	}

	cout << "Arquivo Object3D \"" << name << "\" carregado com sucesso de: " << path << endl;
	return true;
}

void Object3D::render(const Shader& shader) const {
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, value_ptr(transform));
    
	// Set default object color
	glUniform3f(glGetUniformLocation(shader.ID, "objectColor"), 0.7f, 0.7f, 0.7f); // cinza claro
    
	// A textura agora é gerenciada pelos grupos através dos materiais MTL
	mesh.render(shader);
}

void Object3D::setPosition(const vec3& pos) {
	position = pos;
	updateTransform();
}

void Object3D::setRotation(const vec3& rot) {
	rotation = rot;
	updateTransform();
}

void Object3D::setScale(const vec3& scl) {
	scale = scl;
	updateTransform();
}

void Object3D::setEliminable(bool canEliminate) {
	eliminable = canEliminate;
}

/*
void Object3D::setTexture(const string& texturePath) {
	if (!texturePath.empty()) {
		textureID = Texture::loadTexture(texturePath);
		hasTexture = (textureID != 0);
		if (hasTexture) {
			cout << "Textura carregada para objeto \"" << name << "\": " << texturePath << endl;
		} else {
			cerr << "Falha ao carregar textura para objeto \"" << name << "\": " << texturePath << endl;
		}
	} else {
		hasTexture = false;
		textureID = 0;
	}
}
*/

// Atualiza a matriz de transformação (matrix "model" no pipeline gráfico) com base na posição, rotação e escala
void Object3D::updateTransform() {

	transform = mat4(1.0f); // "zera" a matriz de transformação

	// Aplica transformações na ordem: Escala -> Rotação -> Translação

	// translação
	transform = translate(transform, position);
    
	// Aplica rotações
	transform = rotate(transform, rotation.x, vec3(1.0f, 0.0f, 0.0f));
	transform = rotate(transform, rotation.y, vec3(0.0f, 1.0f, 0.0f));
	transform = rotate(transform, rotation.z, vec3(0.0f, 0.0f, 1.0f));
    
	// Aplica escala
	transform = glm::scale(transform, scale);
}


// Retorna a bounding box do objeto 3D transformada pela matriz de transformação
BoundingBox Object3D::getTransformedBoundingBox() const {
	BoundingBox transformedBB;

	// Transforma todos os 8 cantos da caixa delimitadora
	vec3 corners[8] = {
		mesh.boundingBox.pontoMinimo,
		vec3(mesh.boundingBox.pontoMaximo.x, mesh.boundingBox.pontoMinimo.y, mesh.boundingBox.pontoMinimo.z),
		vec3(mesh.boundingBox.pontoMinimo.x, mesh.boundingBox.pontoMaximo.y, mesh.boundingBox.pontoMinimo.z),
		vec3(mesh.boundingBox.pontoMinimo.x, mesh.boundingBox.pontoMinimo.y, mesh.boundingBox.pontoMaximo.z),
		vec3(mesh.boundingBox.pontoMaximo.x, mesh.boundingBox.pontoMaximo.y, mesh.boundingBox.pontoMinimo.z),
		vec3(mesh.boundingBox.pontoMaximo.x, mesh.boundingBox.pontoMinimo.y, mesh.boundingBox.pontoMaximo.z),
		vec3(mesh.boundingBox.pontoMinimo.x, mesh.boundingBox.pontoMaximo.y, mesh.boundingBox.pontoMaximo.z),
		mesh.boundingBox.pontoMaximo
	};
    
	for (int i = 0; i < 8; i++) {
		vec4 transformedCorner = transform * vec4(corners[i], 1.0f);
		transformedBB.expand(vec3(transformedCorner));
	}
    
	return transformedBB;
}


// Testa interseção do raio com a bounding box (retorna true se houver interseção)
// Se houver interseção, retorna a distância até o ponto de interseção mais próximo
// objetos muito rápidos podem atravessar objetos sem detectar colisão !!!
bool Object3D::rayIntersect(const vec3& rayOrigin, const vec3& rayDirection, float& distance) const {
	// Transforma as informações do "raio" para o espaço do objeto ("Local Space")
	mat4 invTransform = inverse(transform);   // gera a matriz inversa da transformação
	vec4 localOrigin = invTransform * vec4(rayOrigin, 1.0f); // ponto de origem do raio no espaço do objeto
	vec4 localDirection = invTransform * vec4(rayDirection, 0.0f); // direção do raio no espaço do objeto
    
	// verifica interseção com a bounding box da malha no espaço do objeto
	return mesh.rayIntersect(vec3(localOrigin), normalize(vec3(localDirection)), distance);
}
