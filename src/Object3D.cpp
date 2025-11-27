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
	  curveTimer(0.0f),
	  isAnimated(false),
	  animationSpeed(1.0f),
	  baseRotation(0.0f)
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
	  curveTimer(0.0f),
	  isAnimated(false),
	  animationSpeed(1.0f),
	  baseRotation(0.0f)
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
    
	// Aplica rotações - Ordem alterada para: Yaw (Y) -> Pitch (X) -> Roll (Z)	// Matematicamente: T * Ry * Rx * Rz * v
	// Isso resolveu problemas de inversão da inclinação, à esquerda ou a direita da pista.
	// primeiro inclinar (X) e depois girar (Y) garantiu que a inclinação seja sempre relativa ao corpo do carro.
	// fontes: https://learnopengl.com/Getting-started/Transformations (Matrix multiplication order)
	// 		   https://learnopengl.com/Getting-started/Camera (Euler Angles)
	transform = rotate(transform, rotation.y, vec3(0.0f, 1.0f, 0.0f));	// Yaw
	transform = rotate(transform, rotation.x, vec3(1.0f, 0.0f, 0.0f));	// Pitch
	transform = rotate(transform, rotation.z, vec3(0.0f, 0.0f, 1.0f));	// Roll
    
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


// Atualiza a posição do objeto ao longo da curva de animação com base no deltaTime
// Realiza calculos de orientação do objeto (inclinação/pitch e direção/yaw) para seguir a curva
void Object3D::updateAnimation(float deltaTime) {

	if (!isAnimated || animationPoints.empty()) return; // sem animação ou sem pontos na curva
	
	// Acumula o progresso temporal para transição entre pontos da curva em curveTimer
	// Exemplo: com animationSpeed=2.0 e deltaTime=0.016s (60 FPS), avança 0.32 por frame (32% do caminho entre pontos)
	curveTimer += animationSpeed * deltaTime * 10.0f;
	
	// curveTimer é mantido no intervalo entre 0.0 e 1.0 pelo loop while
	while (curveTimer >= 1.0f) {
		currentCurveIndex = (currentCurveIndex + 1) % animationPoints.size(); // Avança para o próximo ponto usando módulo (%) para criar loop infinito na curva
																			  // Se animationPoints tem 10 elementos (índices 0-9): quando currentCurveIndex=9, (9+1)%10=0, voltando ao início
		
		curveTimer -= 1.0f;	// Subtrai 1.0 mas preserva o excedente para manter precisão temporal do proximo frame
	}
	
	// recupera a próxima posição na curva -> proximo ponto e atualiza a posição do objeto
	vec3 targetPos = animationPoints[currentCurveIndex]; // para onde o objeto deve ir
	setPosition(targetPos);	// atualiza a posição do objeto para o ponto alvo
	
	// Calcula direção para o ponto posterior ao novo ponto, para orientar o objeto
	int nextIndex = (currentCurveIndex + 1) % animationPoints.size();
	vec3 nextPos = animationPoints[nextIndex]; // posição seguinte ao ponto alvo
	vec3 direction = nextPos - targetPos;
	
	if (length(direction) > 0.001f) {	// Evita divisões por zero ou instabilidade
		direction = normalize(direction);	// normaliza o vetor direction, substituindo seu valor original por 1 e mantendo a mesma direção.
		
		float rotationY = atan2(direction.x, direction.z); // Calcula rotação Y (yaw) baseada na direção
		
		// Calcula rotação X (pitch) baseada na inclinação vertical da pista
		float horizontalDistance = sqrt(direction.x * direction.x + direction.z * direction.z);
		float rotationX = 0.0f;
		
		// Evita divisões por zero ou instabilidade em movimentos puramente verticais
		if (horizontalDistance > 0.001f) {
			rotationX = -atan2(direction.y, horizontalDistance); // calcula a rotação X (pitch) baseada na inclinação
		}
		
		// Aplica rotação calculada + rotação base do modelo (rotação base do arquivo de configuração de cena)
		setRotation(vec3(rotationX + baseRotation.x, rotationY + baseRotation.y, baseRotation.z)); 
			// tínhamos problemas de inversão da inclinação à esquerda ou a direita, resolvemos com a troca da ordem de
			// aplicação das rotações no updateTransform(), pitch (inclinação) sendo aplicado antes do yaw (direção)
			// primeiro inclinar (X) e depois girar (Y) garantiu que a inclinação seja sempre relativa ao corpo do carro.
			// fontes: https://learnopengl.com/Getting-started/Transformations (Matrix multiplication order)
			// 		   https://learnopengl.com/Getting-started/Camera (Euler Angles)
	}
}


// Define a velocidade de animação
void Object3D::setAnimationSpeed(float speed) {
	animationSpeed = speed;
}