#include "EditorCamera.h"
#include "core/Input.h"
#include "core/Logger.h"
#include "scene/Camera.h"
#include "math/Math.h"
#include <GLFW/glfw3.h>
#include <algorithm>

EditorCamera::EditorCamera(Camera& camera) : m_camera(camera) {
    reset();
    LOG_INFO("EditorCamera initialized in orbit mode");
}

void EditorCamera::update(float dt, bool isViewportFocused) {
    if (!isViewportFocused) {
        return;
    }
    
    // Переключение между режимами Orbit и Fly по нажатию Tab
    if (Input::isKeyPressed(GLFW_KEY_TAB)) {
        CameraMode oldMode = m_mode;
        if (m_mode == CameraMode::Orbit) {
            m_mode = CameraMode::Fly;
            
            // Сохраняем текущую позицию и направление при переключении в режим полета
            Vec3 position = m_camera.getPosition();
            Vec3 direction = m_camera.getDirection();
            
            // Вычисляем новые yaw и pitch из текущего направления
            float yaw = std::atan2(direction.z, direction.x) * (180.0f / 3.14159f);
            float pitch = std::asin(direction.y) * (180.0f / 3.14159f);
            
            // Устанавливаем углы камеры
            m_orbitYaw = yaw;
            m_orbitPitch = pitch;
            
            LOG_INFO("Switched to FLY mode at position [{:.2f}, {:.2f}, {:.2f}], yaw={:.1f}, pitch={:.1f}",
                     position.x, position.y, position.z, m_orbitYaw, m_orbitPitch);
        } else {
            m_mode = CameraMode::Orbit;
            
            // При переключении в режим орбиты сохраняем текущий таргет
            Vec3 position = m_camera.getPosition();
            Vec3 direction = m_camera.getDirection();
            
            // Целевая точка находится на некотором расстоянии перед камерой
            m_orbitTarget = position + direction * m_orbitDistance;
            
            LOG_INFO("Switched to ORBIT mode around target [{:.2f}, {:.2f}, {:.2f}], distance={:.1f}",
                     m_orbitTarget.x, m_orbitTarget.y, m_orbitTarget.z, m_orbitDistance);
            
            // Обновляем положение камеры после изменения режима
            updateOrbitCameraPosition();
        }
    }
    
    // Обработка ввода в зависимости от режима
    switch (m_mode) {
        case CameraMode::Orbit:
            updateOrbitCamera(dt);
            break;
        case CameraMode::Fly:
            updateFlyCamera(dt);
            break;
        case CameraMode::Focus:
            break;
    }
    
    // Отладочная информация о позиции камеры
    Vec3 pos = m_camera.getPosition();
    LOG_DEBUG("Camera position: [{:.2f}, {:.2f}, {:.2f}]", pos.x, pos.y, pos.z);
}

void EditorCamera::updateOrbitCamera(float dt) {
    Vec2 mouseDelta = Input::getMouseDelta();
    float scrollDelta = Input::getScrollDelta();
    
    LOG_DEBUG("Orbit camera: mouse delta [{:.1f}, {:.1f}], scroll delta {:.1f}", 
              mouseDelta.x, mouseDelta.y, scrollDelta);
    
    // Вращение с правой кнопкой мыши
    if (Input::isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
        if (!m_isRotating) {
            m_isRotating = true;
            Input::setMouseCursorEnabled(false);
            LOG_DEBUG("Started orbit rotation");
        }
        
        // Применяем вращение
        if (mouseDelta.lengthSq() > 0) {
            m_orbitYaw += mouseDelta.x * m_mouseSensitivity;
            m_orbitPitch -= mouseDelta.y * m_mouseSensitivity;
            
            // Ограничиваем наклон для избежания переворота камеры
            m_orbitPitch = std::clamp(m_orbitPitch, -89.0f, 89.0f);
            LOG_DEBUG("Rotating: yaw={:.1f}, pitch={:.1f}", m_orbitYaw, m_orbitPitch);
        }
    } else if (m_isRotating) {
        m_isRotating = false;
        Input::setMouseCursorEnabled(true);
        LOG_DEBUG("Stopped orbit rotation");
    }
    
    // Панорамирование с средней кнопкой мыши или Alt+ЛКМ
    bool altPressed = Input::isKeyHeld(GLFW_KEY_LEFT_ALT);
    bool leftMousePressed = Input::isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);
    bool middleMousePressed = Input::isMouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE);
    
    if ((altPressed && leftMousePressed) || middleMousePressed) {
        if (!m_isPanning) {
            m_isPanning = true;
            LOG_DEBUG("Started panning");
        }
        
        // Смещение вдоль осей right и up камеры
        if (mouseDelta.lengthSq() > 0) {
            Vec3 right = m_camera.getRight();
            Vec3 up = m_camera.getUp();
            
            // Масштабируем скорость панорамирования с расстоянием для более естественного поведения
            float panSpeed = m_orbitDistance * 0.002f;
            Vec3 panDelta = right * (-mouseDelta.x * panSpeed) + up * (mouseDelta.y * panSpeed);
            m_orbitTarget += panDelta;
            LOG_DEBUG("Panning: delta=[{:.2f}, {:.2f}, {:.2f}]", panDelta.x, panDelta.y, panDelta.z);
        }
    } else {
        m_isPanning = false;
    }
    
    // Масштабирование с колесом мыши
    if (scrollDelta != 0.0f) {
        // Логарифмическое масштабирование для лучшего контроля
        float zoomFactor = 0.1f * m_orbitDistance;
        float oldDistance = m_orbitDistance;
        m_orbitDistance -= scrollDelta * m_zoomSpeed * zoomFactor;
        m_orbitDistance = std::max(0.1f, m_orbitDistance);
        LOG_DEBUG("Zooming: {:.1f} -> {:.1f} (delta={:.1f})", oldDistance, m_orbitDistance, scrollDelta);
    }
    
    // Обновляем позицию камеры на основе орбитальных значений
    updateOrbitCameraPosition();
}

void EditorCamera::updateFlyCamera(float dt) {
    Vec2 mouseDelta = Input::getMouseDelta();
    
    // Поворот камеры с правой кнопкой мыши
    if (Input::isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
        if (!m_isRotating) {
            m_isRotating = true;
            Input::setMouseCursorEnabled(false);
            
            // ВАЖНО: При первом нажатии ПКМ пропускаем первый кадр delta
            // это предотвращает скачок при начале поворота
            if (mouseDelta.lengthSq() > 0) {
                LOG_DEBUG("Resetting mouse delta on first rotation frame");
                return; // Пропускаем первый кадр с дельтой
            }
        }
        
        // Применяем вращение с ограниченной скоростью
        if (mouseDelta.lengthSq() > 0) {
            // Ограничиваем максимальное движение за кадр
            float maxDelta = 10.0f; // Максимальное движение за кадр
            Vec2 clampedDelta = mouseDelta;
            
            if (clampedDelta.length() > maxDelta) {
                clampedDelta = clampedDelta.normalized() * maxDelta;
                LOG_DEBUG("Clamping large mouse delta from [{:.1f}, {:.1f}] to [{:.1f}, {:.1f}]", 
                          mouseDelta.x, mouseDelta.y, clampedDelta.x, clampedDelta.y);
            }
            
            m_orbitYaw += clampedDelta.x * m_mouseSensitivity;
            m_orbitPitch -= clampedDelta.y * m_mouseSensitivity;
            m_orbitPitch = std::clamp(m_orbitPitch, -89.0f, 89.0f);
            LOG_DEBUG("Fly mode looking: yaw={:.1f}, pitch={:.1f}", m_orbitYaw, m_orbitPitch);
            
            updateFlyCameraOrientation();
        }
    } else if (m_isRotating) {
        m_isRotating = false;
        Input::setMouseCursorEnabled(true);
        LOG_DEBUG("Stopped fly mode rotation");
    }
    
    // Обработка движения WASD
    Vec3 movement{0, 0, 0};
    Vec3 forward = m_camera.getDirection();
    Vec3 right = m_camera.getRight();
    Vec3 up = Vec3{0, 1, 0}; // Мировой вектор вверх
    
    if (Input::isKeyHeld(GLFW_KEY_W)) movement += forward;
    if (Input::isKeyHeld(GLFW_KEY_S)) movement -= forward;
    if (Input::isKeyHeld(GLFW_KEY_A)) movement -= right;
    if (Input::isKeyHeld(GLFW_KEY_D)) movement += right;
    if (Input::isKeyHeld(GLFW_KEY_E) || Input::isKeyHeld(GLFW_KEY_SPACE)) movement += up;
    if (Input::isKeyHeld(GLFW_KEY_Q) || Input::isKeyHeld(GLFW_KEY_LEFT_CONTROL)) movement -= up;
    
    // Регулируем скорость
    float speed = m_movementSpeed;
    if (Input::isKeyHeld(GLFW_KEY_LEFT_SHIFT)) speed *= 3.0f;
    if (Input::isKeyHeld(GLFW_KEY_LEFT_ALT)) speed *= 0.3f;
    
    if (movement.lengthSq() > 0) {
        movement.normalize();
        Vec3 currentPos = m_camera.getPosition();
        Vec3 newPos = currentPos + movement * speed * dt;
        m_camera.setPosition(newPos);
        LOG_DEBUG("Fly Mode: Moving to [{:.2f}, {:.2f}, {:.2f}]", newPos.x, newPos.y, newPos.z);
    }
}

void EditorCamera::updateOrbitCameraPosition() {
    // Преобразуем из сферических в декартовы координаты
    float yawRad = m_orbitYaw * (3.14159f / 180.0f);
    float pitchRad = m_orbitPitch * (3.14159f / 180.0f);
    
    Vec3 offset;
    offset.x = cos(yawRad) * cos(pitchRad);
    offset.y = sin(pitchRad);
    offset.z = sin(yawRad) * cos(pitchRad);
    
    // Вычисляем позицию камеры
    Vec3 position = m_orbitTarget + offset * m_orbitDistance;
    m_camera.setPosition(position);
    m_camera.lookAt(m_orbitTarget);
    
    LOG_DEBUG("Orbit position updated: pos=[{:.2f}, {:.2f}, {:.2f}], target=[{:.2f}, {:.2f}, {:.2f}]", 
              position.x, position.y, position.z,
              m_orbitTarget.x, m_orbitTarget.y, m_orbitTarget.z);
}

void EditorCamera::updateFlyCameraOrientation() {
    float yawRad = m_orbitYaw * (3.14159f / 180.0f);
    float pitchRad = m_orbitPitch * (3.14159f / 180.0f);
    
    Vec3 direction;
    direction.x = cos(yawRad) * cos(pitchRad);
    direction.y = sin(pitchRad);
    direction.z = sin(yawRad) * cos(pitchRad);
    
    // Нормализуем направление, чтобы быть уверенными в единичной длине
    direction.normalize();
    
    Vec3 position = m_camera.getPosition();
    m_camera.lookAt(position + direction);
    
    LOG_DEBUG("Fly orientation updated: dir=[{:.2f}, {:.2f}, {:.2f}]", 
              direction.x, direction.y, direction.z);
}

void EditorCamera::focusOnObject(const Vec3& position, float radius) {
    m_orbitTarget = position;
    m_orbitDistance = std::max(radius * 3.0f, 2.0f);
    updateOrbitCameraPosition();
    LOG_INFO("Focused on object at [{:.2f}, {:.2f}, {:.2f}], distance={:.1f}", 
             position.x, position.y, position.z, m_orbitDistance);
}

void EditorCamera::frameSelection(const Vec3& minBounds, const Vec3& maxBounds) {
    Vec3 center = (minBounds + maxBounds) * 0.5f;
    Vec3 size = maxBounds - minBounds;
    float radius = size.length() * 0.5f;
    
    focusOnObject(center, radius);
    LOG_INFO("Framed selection: center=[{:.2f}, {:.2f}, {:.2f}], radius={:.1f}", 
             center.x, center.y, center.z, radius);
}

void EditorCamera::reset() {
    m_orbitTarget = Vec3{0, 0, 0};
    m_orbitDistance = 10.0f;
    m_orbitYaw = -90.0f;
    m_orbitPitch = 20.0f;
    m_mode = CameraMode::Orbit;
    
    updateOrbitCameraPosition();
    LOG_INFO("Camera reset to default position");
}