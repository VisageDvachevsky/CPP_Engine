#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform vec3 u_cameraPos;
uniform vec3 u_cameraDir;
uniform vec3 u_cameraUp;
uniform vec3 u_cameraRight;
uniform float u_fov;
uniform vec2 u_resolution;

uniform int u_maxBounces;
uniform int u_samplesPerPixel;
uniform float u_time;

#define MAX_PRIMITIVES 64
#define PI 3.14159265359
#define TWO_PI 6.28318530718
#define INV_PI 0.31830988618
#define EPSILON 0.0001
#define MAX_FLOAT 1e20

struct Sphere {
    vec3 center;
    float radius;
    vec3 color;
    int materialType;
    float roughness;
    float ior;
    float metalness;
    vec3 emission;
};

struct Plane {
    vec3 point;
    vec3 normal;
    vec3 color;
    int materialType;
    float roughness;
    float metalness;
    vec3 emission;
};

struct Cube {
    vec3 center;
    vec3 size;
    vec3 color;
    int materialType;
    float roughness;
    float ior;
    float metalness;
    vec3 emission;
};

uniform Sphere u_spheres[MAX_PRIMITIVES];
uniform Plane u_planes[MAX_PRIMITIVES];
uniform Cube u_cubes[MAX_PRIMITIVES];

uniform int u_numSpheres;
uniform int u_numPlanes;
uniform int u_numCubes;

struct Ray {
    vec3 origin;
    vec3 direction;
};

struct HitInfo {
    bool hit;
    float t;
    vec3 point;
    vec3 normal;
    vec3 color;
    int materialType;
    float roughness;
    float ior;
    float metalness;
    vec3 emission;
};

// Улучшенный RNG
uint g_seed;

uint hash(uint x) {
    x += (x << 10u);
    x ^= (x >> 6u);
    x += (x << 3u);
    x ^= (x >> 11u);
    x += (x << 15u);
    return x;
}

uint hash(uvec2 v) { return hash(v.x ^ hash(v.y)); }
uint hash(uvec3 v) { return hash(v.x ^ hash(v.y) ^ hash(v.z)); }

float floatConstruct(uint m) {
    const uint ieeeMantissa = 0x007FFFFFu;
    const uint ieeeOne = 0x3F800000u;
    m &= ieeeMantissa;
    m |= ieeeOne;
    float f = uintBitsToFloat(m);
    return f - 1.0;
}

float random() {
    g_seed = hash(g_seed);
    return floatConstruct(g_seed);
}

vec2 random2() {
    return vec2(random(), random());
}

vec3 random3() {
    return vec3(random(), random(), random());
}

// Семплинг функции
vec3 sampleCosineWeightedHemisphere(vec3 normal) {
    vec2 r = random2();
    float r1 = r.x;
    float r2 = r.y;
    
    float cosTheta = sqrt(r1);
    float sinTheta = sqrt(1.0 - r1);
    float phi = TWO_PI * r2;
    
    vec3 w = normal;
    vec3 u = normalize(cross(abs(w.x) > 0.1 ? vec3(0, 1, 0) : vec3(1, 0, 0), w));
    vec3 v = cross(w, u);
    
    return cosTheta * w + sinTheta * cos(phi) * u + sinTheta * sin(phi) * v;
}

vec3 sampleHemisphere(vec3 normal) {
    vec2 r = random2();
    float cosTheta = sqrt(1.0 - r.x);
    float sinTheta = sqrt(r.x);
    float phi = TWO_PI * r.y;
    
    vec3 w = normal;
    vec3 u = normalize(cross(abs(w.x) > 0.1 ? vec3(0, 1, 0) : vec3(1, 0, 0), w));
    vec3 v = cross(w, u);
    
    return cosTheta * w + sinTheta * cos(phi) * u + sinTheta * sin(phi) * v;
}

// Intersection functions
bool intersectSphere(Ray ray, Sphere sphere, out float t) {
    vec3 oc = ray.origin - sphere.center;
    float a = dot(ray.direction, ray.direction);
    float b = 2.0 * dot(oc, ray.direction);
    float c = dot(oc, oc) - sphere.radius * sphere.radius;
    float discriminant = b * b - 4.0 * a * c;
    
    if (discriminant < 0.0) return false;
    
    float sqrtD = sqrt(discriminant);
    float t1 = (-b - sqrtD) / (2.0 * a);
    float t2 = (-b + sqrtD) / (2.0 * a);
    
    if (t1 > EPSILON) {
        t = t1;
        return true;
    }
    
    if (t2 > EPSILON) {
        t = t2;
        return true;
    }
    
    return false;
}

bool intersectPlane(Ray ray, Plane plane, out float t) {
    float denom = dot(plane.normal, ray.direction);
    if (abs(denom) < EPSILON) return false;
    
    t = dot(plane.point - ray.origin, plane.normal) / denom;
    return t > EPSILON;
}

bool intersectCube(Ray ray, Cube cube, out float t, out vec3 normal) {
    vec3 halfSize = cube.size * 0.5;
    vec3 minBounds = cube.center - halfSize;
    vec3 maxBounds = cube.center + halfSize;
    
    vec3 invDir = 1.0 / ray.direction;
    
    vec3 t1 = (minBounds - ray.origin) * invDir;
    vec3 t2 = (maxBounds - ray.origin) * invDir;
    
    vec3 tmin = min(t1, t2);
    vec3 tmax = max(t1, t2);
    
    float tNear = max(max(tmin.x, tmin.y), tmin.z);
    float tFar = min(min(tmax.x, tmax.y), tmax.z);
    
    if (tNear > tFar || tFar < 0.0) return false;
    
    t = tNear > EPSILON ? tNear : tFar;
    
    vec3 hitPoint = ray.origin + ray.direction * t;
    vec3 d = hitPoint - cube.center;
    vec3 absD = abs(d);
    
    if (absD.x > absD.y && absD.x > absD.z) {
        normal = vec3(sign(d.x), 0.0, 0.0);
    } else if (absD.y > absD.x && absD.y > absD.z) {
        normal = vec3(0.0, sign(d.y), 0.0);
    } else {
        normal = vec3(0.0, 0.0, sign(d.z));
    }
    
    return true;
}

HitInfo intersectScene(Ray ray) {
    HitInfo closestHit;
    closestHit.hit = false;
    closestHit.t = MAX_FLOAT;
    
    // Spheres
    for (int i = 0; i < u_numSpheres && i < MAX_PRIMITIVES; i++) {
        float t;
        if (intersectSphere(ray, u_spheres[i], t) && t < closestHit.t) {
            closestHit.hit = true;
            closestHit.t = t;
            closestHit.point = ray.origin + t * ray.direction;
            closestHit.normal = normalize(closestHit.point - u_spheres[i].center);
            closestHit.color = u_spheres[i].color;
            closestHit.materialType = u_spheres[i].materialType;
            closestHit.roughness = u_spheres[i].roughness;
            closestHit.ior = u_spheres[i].ior;
            closestHit.metalness = u_spheres[i].metalness;
            closestHit.emission = u_spheres[i].emission;
        }
    }
    
    // Planes
    for (int i = 0; i < u_numPlanes && i < MAX_PRIMITIVES; i++) {
        float t;
        if (intersectPlane(ray, u_planes[i], t) && t < closestHit.t) {
            closestHit.hit = true;
            closestHit.t = t;
            closestHit.point = ray.origin + t * ray.direction;
            closestHit.normal = u_planes[i].normal;
            closestHit.color = u_planes[i].color;
            closestHit.materialType = u_planes[i].materialType;
            closestHit.roughness = u_planes[i].roughness;
            closestHit.ior = 1.5;
            closestHit.metalness = u_planes[i].metalness;
            closestHit.emission = u_planes[i].emission;
        }
    }
    
    // Cubes
    for (int i = 0; i < u_numCubes && i < MAX_PRIMITIVES; i++) {
        float t;
        vec3 normal;
        if (intersectCube(ray, u_cubes[i], t, normal) && t < closestHit.t) {
            closestHit.hit = true;
            closestHit.t = t;
            closestHit.point = ray.origin + t * ray.direction;
            closestHit.normal = normal;
            closestHit.color = u_cubes[i].color;
            closestHit.materialType = u_cubes[i].materialType;
            closestHit.roughness = u_cubes[i].roughness;
            closestHit.ior = u_cubes[i].ior;
            closestHit.metalness = u_cubes[i].metalness;
            closestHit.emission = u_cubes[i].emission;
        }
    }
    
    return closestHit;
}

// Более мягкое и темное небо
vec3 getSkyColor(vec3 direction) {
    // Солнце - менее яркое и более мягкое
    vec3 sunDir = normalize(vec3(0.2, 0.6, 0.4));
    float sunDot = max(0.0, dot(direction, sunDir));
    vec3 sunColor = vec3(1.2, 1.0, 0.8) * pow(sunDot, 128.0) * 0.5;
    
    // Более темный градиент неба
    float t = max(0.0, direction.y);
    vec3 skyGradient = mix(vec3(0.4, 0.6, 0.8), vec3(0.15, 0.3, 0.6), t);
    
    // Более мягкий горизонт
    float horizonFactor = 1.0 - abs(direction.y);
    vec3 horizonColor = vec3(0.6, 0.5, 0.4) * horizonFactor * 0.2;
    
    return (skyGradient + horizonColor + sunColor) * 0.6;
}

// Простая функция scatter без сложной BRDF
bool scatter(Ray inRay, HitInfo hit, out vec3 attenuation, out Ray scattered) {
    vec3 V = -normalize(inRay.direction);
    vec3 N = normalize(hit.normal);
    
    // Убедиться что нормаль смотрит в правильную сторону
    if (dot(N, V) < 0.0) {
        N = -N;
    }
    
    if (hit.materialType == 0) { // Diffuse
        vec3 L = sampleCosineWeightedHemisphere(N);
        scattered = Ray(hit.point + N * EPSILON * 2.0, L);
        attenuation = hit.color * 0.8;
        return dot(N, L) > 0.0;
    }
    else if (hit.materialType == 1) { // Metal
        float safeRoughness = clamp(hit.roughness, 0.02, 1.0);
        
        vec3 reflected = reflect(normalize(inRay.direction), N);
        vec3 fuzz = sampleHemisphere(N) * safeRoughness * 0.5;
        vec3 L = normalize(reflected + fuzz);
        
        if (dot(N, L) > 0.0) {
            scattered = Ray(hit.point + N * EPSILON * 2.0, L);
            
            // Простое отражение для металлов
            float metallic = clamp(hit.metalness, 0.0, 1.0);
            vec3 baseReflection = mix(vec3(0.04), hit.color, metallic);
            attenuation = baseReflection * 0.9;
            return true;
        }
        return false;
    }
    else if (hit.materialType == 2) { // Glass
        float safeIor = clamp(hit.ior, 1.001, 3.0);
        bool entering = dot(inRay.direction, N) < 0.0;
        vec3 normal = entering ? N : -N;
        float eta = entering ? 1.0 / safeIor : safeIor;
        
        vec3 incident = normalize(inRay.direction);
        vec3 refracted = refract(incident, normal, eta);
        
        if (length(refracted) > 0.5) {
            // Простые уравнения Френеля
            float cosTheta = abs(dot(incident, normal));
            float r0 = (1.0 - safeIor) / (1.0 + safeIor);
            r0 *= r0;
            float fresnel = r0 + (1.0 - r0) * pow(1.0 - cosTheta, 5.0);
            
            if (random() < fresnel) {
                // Отражение
                vec3 reflected = reflect(incident, normal);
                scattered = Ray(hit.point + normal * EPSILON * 2.0, reflected);
            } else {
                // Рефракция
                scattered = Ray(hit.point - normal * EPSILON * 2.0, normalize(refracted));
            }
            
            attenuation = vec3(0.95);
            return true;
        } else {
            // Полное внутреннее отражение
            vec3 reflected = reflect(incident, normal);
            scattered = Ray(hit.point + normal * EPSILON * 2.0, reflected);
            attenuation = vec3(0.98);
            return true;
        }
    }
    
    return false;
}

// Упрощенный path tracer
vec3 pathTrace(Ray ray) {
    vec3 radiance = vec3(0.0);
    vec3 throughput = vec3(1.0);
    
    for (int bounce = 0; bounce < u_maxBounces; bounce++) {
        HitInfo hit = intersectScene(ray);
        
        if (!hit.hit) {
            radiance += throughput * getSkyColor(ray.direction);
            break;
        }
        
        // Добавить emission
        if (bounce == 0 || dot(throughput, throughput) > 0.01) {
            radiance += throughput * hit.emission;
        }
        
        // Sample next direction
        vec3 attenuation;
        Ray scattered;
        
        if (!scatter(ray, hit, attenuation, scattered)) {
            break;
        }
        
        // Russian roulette
        if (bounce > 2) {
            float maxComponent = max(max(throughput.r, throughput.g), throughput.b);
            float rrProbability = min(maxComponent * 0.8, 0.9);
            if (random() > rrProbability) {
                break;
            }
            throughput /= rrProbability;
        }
        
        throughput *= clamp(attenuation, vec3(0.0), vec3(2.0));
        ray = scattered;
        
        if (dot(throughput, throughput) < 0.001) {
            break;
        }
    }
    
    return clamp(radiance, vec3(0.0), vec3(50.0));
}

// Камера с DOF
Ray getCameraRay(vec2 coords) {
    float theta = u_fov * 0.5 * PI / 180.0;
    float halfHeight = tan(theta);
    float halfWidth = halfHeight;
    
    vec3 rayDir = normalize(
        coords.x * halfWidth * u_cameraRight +
        coords.y * halfHeight * u_cameraUp +
        u_cameraDir
    );
    
    // Простой DOF
    float aperture = 0.03;
    float focusDistance = 10.0;
    
    if (aperture > 0.0) {
        vec2 rd = aperture * (random2() - 0.5) * 2.0;
        vec3 offset = u_cameraRight * rd.x + u_cameraUp * rd.y;
        vec3 focusPoint = u_cameraPos + rayDir * focusDistance;
        
        return Ray(u_cameraPos + offset, normalize(focusPoint - (u_cameraPos + offset)));
    }
    
    return Ray(u_cameraPos, rayDir);
}

void main() {
    vec2 uv = (2.0 * gl_FragCoord.xy - u_resolution) / u_resolution.y;
    
    // RNG инициализация
    uvec2 pixelCoord = uvec2(gl_FragCoord.xy);
    uint frameHash = uint(u_time * 60.0);
    g_seed = hash(uvec3(pixelCoord, frameHash)) | 1u;
    
    vec3 color = vec3(0.0);
    
    for (int sample = 0; sample < u_samplesPerPixel; sample++) {
        vec2 jitter = (random2() - 0.5) * 0.8 / u_resolution;
        vec2 coords = uv + jitter;
        
        Ray ray = getCameraRay(coords);
        vec3 sampleColor = pathTrace(ray);
        
        // Защита от артефактов
        if (any(isnan(sampleColor)) || any(isinf(sampleColor))) {
            sampleColor = vec3(0.0);
        }
        
        color += clamp(sampleColor, vec3(0.0), vec3(20.0));
    }
    
    color /= float(u_samplesPerPixel);
    
    // ACES Tonemapping с пониженной экспозицией
    color *= 0.8;
    
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    
    color = clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
    
    // Gamma correction
    color = pow(color, vec3(1.0 / 2.2));
    
    // Дополнительное затемнение
    color *= 0.95;
    
    FragColor = vec4(color, 1.0);
}