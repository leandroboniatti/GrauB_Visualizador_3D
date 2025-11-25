#include "Object3D.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>

Object3D::Object3D() 
	: transform(1.0f), 
	  position (0.0f), 
	  rotation (0.0f), 
	  scale    (1.0f), 
	  eliminable(true), 
	  name(""),
	  currentCurveIndex(0),
	  curveProgress(0.0f),
	  isAnimated(false),
	  animationSpeed(1.0f)
	//  textureID(0),
	//  hasTexture(false)
	{ updateTransform(); }

Object3D::Object3D(string& objName)
	: transform(1.0f),  // matriz identidade
	  position (0.0f),  // posição zero
	  rotation (0.0f),  // sem rotação
	  scale    (1.0f),  // escala unitária
	  eliminable(true),
	  name(objName),
	  currentCurveIndex(0),
	  curveProgress(0.0f),
	  isAnimated(false),
	  animationSpeed(1.0f)
	//  textureID(0),
	//  hasTexture(false)
	{ updateTransform(); }

Object3D::~Object3D() { mesh.cleanup(); }


// Carrega um objeto 3D a partir de um arquivo
// uso: System::loadSceneObjects -> Object3D::loadObject -> Mesh::readObjectModel -> OBJReader::readFileOBJ
bool Object3D::loadObject(string& path) {

	if (!mesh.readObjectModel(path)) {
		cerr << "Falha ao carregar arquivo OBJ: " << path << endl;
		return false;
	}

	cout << "Object3D \"" << name << "\" carregado com sucesso de: " << path << endl;
	return true;
}


// Renderiza o objeto 3D usando o shader fornecido
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

	// Obtém os 8 cantos da bounding box original
	vec3 corners[8];
	mesh.boundingBox.getCorners(corners);
    
	// Transforma todos os 8 cantos pela matriz de transformação
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


// Carrega os pontos da curva de animação a partir de um arquivo
// Aplica as transformações da pista (posição, rotação, escala) aos pontos da curva
bool Object3D::loadAnimationCurve(const string& curveFilePath,
                                  const vec3& trackPosition,
                                  const vec3& trackRotation,
                                  const vec3& trackScale) {

	ifstream file(curveFilePath);
	
	if (!file.is_open()) {
		cerr << "Falha ao abrir arquivo de curva: " << curveFilePath << endl;
		return false;
	}
	
	this->animationPoints.clear(); // limpa pontos anteriores
	string line;
	
	while (getline(file, line)) {
		// Ignora linhas vazias e comentários
		if (line.empty() || line[0] == '#') continue;
		
		istringstream iss(line);
		vec3 point;
		
		if (iss >> point.x >> point.y >> point.z) {
			// Aplica transformações da pista ao ponto
			// 1. Escala
			point = point * trackScale;
			
			// 2. Rotação (apenas Y por simplicidade, se necessário adicionar X e Z)
			if (length(trackRotation) > 0.001f) {
				mat4 rotMatrix = mat4(1.0f);
				rotMatrix = rotate(rotMatrix, trackRotation.x, vec3(1.0f, 0.0f, 0.0f));
				rotMatrix = rotate(rotMatrix, trackRotation.y, vec3(0.0f, 1.0f, 0.0f));
				rotMatrix = rotate(rotMatrix, trackRotation.z, vec3(0.0f, 0.0f, 1.0f));
				vec4 rotatedPoint = rotMatrix * vec4(point, 1.0f);
				point = vec3(rotatedPoint);
			}
			
			// 3. Translação
			point = point + trackPosition;
			
			this->animationPoints.push_back(point);
		}
	}
	
	file.close();
	
	if (!this->animationPoints.empty()) {
		isAnimated = true;
		currentCurveIndex = 0;
		position = this->animationPoints[0];
		updateTransform();
		cout << "Curva de animacao carregada com " << this->animationPoints.size() << " pontos" << endl;
		return true;
	}
	
	return false;
}


// Atualiza a posição do objeto ao longo da curva
void Object3D::updateAnimation(float deltaTime) {
	if (!isAnimated || animationPoints.empty()) return;
	
	// Avança no progresso baseado na velocidade (pontos por segundo)
	curveProgress += animationSpeed * deltaTime * 10.0f; // 10 pontos/segundo como base
	
	// Atualiza o índice quando passar de 1.0
	while (curveProgress >= 1.0f) {
		currentCurveIndex = (currentCurveIndex + 1) % animationPoints.size();
		curveProgress -= 1.0f;
	}
	
	// Atualiza posição
	vec3 currentPos = animationPoints[currentCurveIndex];
	
	// Calcula direção para o próximo ponto para orientar o objeto
	int nextIndex = (currentCurveIndex + 1) % animationPoints.size();
	vec3 nextPos = animationPoints[nextIndex];
	vec3 direction = nextPos - currentPos;
	
	if (length(direction) > 0.001f) {
		direction = normalize(direction);
		// Calcula rotação Y baseada na direção
		float angleY = atan2(direction.x, direction.z);
		setRotation(vec3(0.0f, angleY, 0.0f));
	}
	
	setPosition(currentPos);
}


// Define a velocidade de animação
void Object3D::setAnimationSpeed(float speed) {
	animationSpeed = speed;
}
