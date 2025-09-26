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

// Dynamic object counts
#define MAX_PRIMITIVES 64
#define PI 3.14159265359
#define EPSILON 0.001

struct Sphere {
    vec3 center;
    float radius;
    vec3 color;
    int materialType; // 0=diffuse, 1=metal, 2=dielectric
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

// Dynamic arrays of primitives
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

// PCG Random Number Generator
uint g_seed;

uint pcg(inout uint state) {
    uint prev = state;
    state = state * 747796405u + 2891336453u;
    uint result = ((prev >> ((prev >> 28) + 4u)) ^ prev) * 277803737u;
    return result >> 22u;
}

float random() {
    g_seed = pcg(g_seed);
    return float(g_seed) / 4294967295.0;
}

vec3 randomInUnitSphere() {
    vec3 p;
    do {
        p = 2.0 * vec3(random(), random(), random()) - 1.0;
    } while (dot(p, p) >= 1.0);
    return p;
}

vec3 randomInUnitDisk() {
    vec3 p;
    do {
        p = 2.0 * vec3(random(), random(), 0.0) - vec3(1.0, 1.0, 0.0);
    } while (dot(p, p) >= 1.0);
    return p;
}

vec3 randomUnitVector() {
    float z = random() * 2.0 - 1.0;
    float a = random() * 2.0 * PI;
    float r = sqrt(1.0 - z * z);
    float x = r * cos(a);
    float y = r * sin(a);
    return vec3(x, y, z);
}

// GGX Distribution function
float D_GGX(float NoH, float roughness) {
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float NoH2 = NoH * NoH;
    float denom = NoH2 * (alpha2 - 1.0) + 1.0;
    return alpha2 / (PI * denom * denom);
}

// Schlick Fresnel approximation
vec3 F_Schlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// GGX Geometry function
float G_Smith(float NoV, float NoL, float roughness) {
    float alpha = roughness * roughness;
    float k = alpha / 2.0;
    float G1V = NoV / (NoV * (1.0 - k) + k);
    float G1L = NoL / (NoL * (1.0 - k) + k);
    return G1V * G1L;
}

vec3 importanceSampleGGX(vec2 Xi, float roughness, vec3 N) {
    float alpha = roughness * roughness;
    
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (alpha*alpha - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    
    // Convert from spherical to cartesian
    vec3 H = vec3(
        sinTheta * cos(phi),
        sinTheta * sin(phi),
        cosTheta
    );
    
    // Tangent space to world space
    vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);
    
    return tangent * H.x + bitangent * H.y + N * H.z;
}

// Ray-sphere intersection
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

// Ray-plane intersection
bool intersectPlane(Ray ray, Plane plane, out float t) {
    float denom = dot(plane.normal, ray.direction);
    if (abs(denom) < EPSILON) return false;
    
    t = dot(plane.point - ray.origin, plane.normal) / denom;
    return t > EPSILON;
}

// Ray-AABB (cube) intersection
bool intersectCube(Ray ray, Cube cube, out float t, out vec3 normal) {
    vec3 halfSize = cube.size * 0.5;
    vec3 min = cube.center - halfSize;
    vec3 max = cube.center + halfSize;
    
    vec3 invDir = 1.0 / ray.direction;
    
    vec3 t1 = (min - ray.origin) * invDir;
    vec3 t2 = (max - ray.origin) * invDir;
    
    vec3 tmin = min(t1, t2);
    vec3 tmax = max(t1, t2);
    
    float tNear = max(max(tmin.x, tmin.y), tmin.z);
    float tFar = min(min(tmax.x, tmax.y), tmax.z);
    
    if (tNear > tFar || tFar < 0.0) return false;
    
    t = tNear > EPSILON ? tNear : tFar;
    
    // Calculate the normal based on which face was hit
    vec3 hitPoint = ray.origin + ray.direction * t;
    vec3 center = cube.center;
    vec3 d = normalize(hitPoint - center);
    vec3 absD = abs(d);
    
    // Find the dominant axis to determine which face was hit
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
    closestHit.t = 1e30;
    
    // Intersect with spheres
    for (int i = 0; i < u_numSpheres; i++) {
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
    
    // Intersect with planes
    for (int i = 0; i < u_numPlanes; i++) {
        float t;
        if (intersectPlane(ray, u_planes[i], t) && t < closestHit.t) {
            closestHit.hit = true;
            closestHit.t = t;
            closestHit.point = ray.origin + t * ray.direction;
            closestHit.normal = u_planes[i].normal;
            closestHit.color = u_planes[i].color;
            closestHit.materialType = u_planes[i].materialType;
            closestHit.roughness = u_planes[i].roughness;
            closestHit.ior = 1.0; // Planes don't use IOR
            closestHit.metalness = u_planes[i].metalness;
            closestHit.emission = u_planes[i].emission;
        }
    }
    
    // Intersect with cubes
    for (int i = 0; i < u_numCubes; i++) {
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

vec3 getSkyColor(vec3 direction) {
    float t = 0.5 * (direction.y + 1.0);
    return mix(vec3(1.0, 1.0, 1.0), vec3(0.5, 0.7, 1.0), t) * 0.8;
}

bool scatter(Ray inRay, HitInfo hit, out vec3 attenuation, out Ray scattered, out vec3 emission) {
    emission = hit.emission;
    
    if (hit.materialType == 0) { // Diffuse
        vec3 target = hit.point + hit.normal + randomUnitVector();
        scattered = Ray(hit.point, normalize(target - hit.point));
        attenuation = hit.color;
        return true;
    }
    else if (hit.materialType == 1) { // Metal
        vec3 reflected = reflect(normalize(inRay.direction), hit.normal);
        
        if (hit.roughness > 0.0) {
            reflected += hit.roughness * randomInUnitSphere();
        }
        
        scattered = Ray(hit.point, normalize(reflected));
        
        // Metalness determines color of reflection
        vec3 F0 = mix(vec3(0.04), hit.color, hit.metalness);
        float cosTheta = max(0.0, dot(-normalize(inRay.direction), hit.normal));
        attenuation = F_Schlick(cosTheta, F0);
        
        return dot(scattered.direction, hit.normal) > 0.0;
    }
    else if (hit.materialType == 2) { // Dielectric
        attenuation = vec3(1.0); // Glass doesn't absorb light
        
        float refraction_ratio = hit.ior;
        bool front_face = dot(inRay.direction, hit.normal) < 0.0;
        vec3 normal = front_face ? hit.normal : -hit.normal;
        
        if (!front_face) {
            refraction_ratio = 1.0 / hit.ior;
        }
        
        vec3 unit_direction = normalize(inRay.direction);
        float cos_theta = min(dot(-unit_direction, normal), 1.0);
        float sin_theta = sqrt(1.0 - cos_theta * cos_theta);
        
        // Schlick approximation for reflectance
        float r0 = (1.0 - refraction_ratio) / (1.0 + refraction_ratio);
        r0 = r0 * r0;
        float reflectance = r0 + (1.0 - r0) * pow(1.0 - cos_theta, 5.0);
        
        bool cannot_refract = refraction_ratio * sin_theta > 1.0;
        vec3 direction;
        
        if (cannot_refract || reflectance > random()) {
            // Reflect
            direction = reflect(unit_direction, normal);
        } else {
            // Refract
            direction = refract(unit_direction, normal, refraction_ratio);
        }
        
        scattered = Ray(hit.point, normalize(direction));
        return true;
    }
    
    return false;
}

// Path tracing
vec3 rayColor(Ray ray) {
    vec3 color = vec3(1.0);
    vec3 emission = vec3(0.0);
    
    for (int bounce = 0; bounce < u_maxBounces; bounce++) {
        HitInfo hit = intersectScene(ray);
        
        if (!hit.hit) {
            color *= getSkyColor(ray.direction);
            break;
        }
        
        vec3 emitted;
        vec3 attenuation;
        Ray scattered;
        
        if (scatter(ray, hit, attenuation, scattered, emitted)) {
            emission += color * emitted;
            color *= attenuation;
            ray = scattered;
        } else {
            emission += color * emitted;
            color = vec3(0.0);
            break;
        }
    }
    
    return color + emission;
}

// Depth of field
Ray getRayWithDOF(vec3 origin, vec3 direction, float lensRadius, float focalDistance) {
    vec3 rd = lensRadius * randomInUnitDisk();
    vec3 offset = u_cameraRight * rd.x + u_cameraUp * rd.y;
    
    vec3 focusPoint = origin + direction * focalDistance;
    return Ray(origin + offset, normalize(focusPoint - (origin + offset)));
}

void main() {
    vec2 uv = (2.0 * gl_FragCoord.xy - u_resolution) / u_resolution.y;
    
    // Initialize random seed
    g_seed = uint(gl_FragCoord.x) + uint(gl_FragCoord.y) * uint(u_resolution.x) + uint(u_time * 1000.0);
    
    vec3 totalColor = vec3(0.0);
    
    float aperture = 0.05; // Adjustable depth of field
    float focusDistance = 10.0; // Distance to perfect focus
    
    for (int sample = 0; sample < u_samplesPerPixel; sample++) {
        // Anti-aliasing jitter
        vec2 jitter = vec2(random() - 0.5, random() - 0.5) / u_resolution;
        vec2 coords = uv + jitter;
        
        // Calculate ray direction
        float theta = u_fov * 0.5 * PI / 180.0;
        float halfHeight = tan(theta);
        float halfWidth = halfHeight;
        
        vec3 rayDir = normalize(
            coords.x * halfWidth * u_cameraRight +
            coords.y * halfHeight * u_cameraUp +
            u_cameraDir
        );
        
        // Apply depth of field if aperture > 0
        Ray ray;
        if (aperture > 0.0) {
            ray = getRayWithDOF(u_cameraPos, rayDir, aperture, focusDistance);
        } else {
            ray = Ray(u_cameraPos, rayDir);
        }
        
        totalColor += rayColor(ray);
    }
    
    vec3 color = totalColor / float(u_samplesPerPixel);
    
    // Tone mapping (ACES filmic)
    color = (color * (2.51 * color + 0.03)) / (color * (2.43 * color + 0.59) + 0.14);
    
    // Gamma correction
    color = pow(color, vec3(1.0 / 2.2));
    
    FragColor = vec4(color, 1.0);
}