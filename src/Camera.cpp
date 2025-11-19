#include "Camera.h"

// Construtor padrão com valores iniciais
Camera::Camera(vec3 position, vec3 up, float yaw, float pitch) 
    : Front(vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM) {
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
}

// retorna a matriz de visualização usando a função lookAt do GLM
mat4 Camera::GetViewMatrix() {
    return lookAt(Position, Position + Front, Up);
}

// processa o input recebido do teclado, movendo a câmera de acordo com a direção e a velocidade do movimento 
void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime) {

    float velocity = MovementSpeed * deltaTime;

    if (direction == FORWARD)
        Position += Front * velocity;
    if (direction == BACKWARD)
        Position -= Front * velocity;
    if (direction == LEFT)
        Position -= Right * velocity;
    if (direction == RIGHT)
        Position += Right * velocity;
}


// processa o input recebido do mouse, ajustando os ângulos Yaw (olhar para os lados) e Pitch (olhar para cima e para baixo)
void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch) {

    // aplica a sensibilidade do mouse
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;
    
    // atualiza os ângulos Yaw e Pitch
    Yaw += xoffset;
    Pitch += yoffset;


    // limita o pitch entre -89° e +89°, prevenindo a inversão da câmera que pode causar desorientação.
    if (constrainPitch) {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }
    
    // Atualiza os vetores Front, Right e Up a partir dos ângulos de Euler atualizados
    updateCameraVectors();
}


// processa o input recebido do scroll do mouse, ajustando o zoom da câmera
void Camera::ProcessMouseScroll(float yoffset) {
    Zoom -= (float)yoffset;
    if (Zoom < 1.0f)
        Zoom = 1.0f;    // zoom máximo (campo de visão menor)
    if (Zoom > 45.0f)
        Zoom = 45.0f;   // zoom mínimo (campo de visão maior)
}


// calcula os vetores Front, Right e Up a partir dos ângulos de Euler atualizados
void Camera::updateCameraVectors() {
    
    // calcula o novo vetor Front
    vec3 front;
    front.x = cos(radians(Yaw)) * cos(radians(Pitch));
    front.y = sin(radians(Pitch));
    front.z = sin(radians(Yaw)) * cos(radians(Pitch));
    Front = normalize(front);

    // também recalcula os vetores Right e Up
    Right = normalize(cross(Front, WorldUp));
    Up = normalize(cross(Right, Front));
}
