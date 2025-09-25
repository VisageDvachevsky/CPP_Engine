#include "Object.h"

Object::Object(const std::string& name, ObjectType type) 
    : m_name(name), m_type(type) {
}

void Object::update(float dt) {
    // Basic object updates can go here
    // For now, objects are static
}