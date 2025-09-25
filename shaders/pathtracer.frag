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

#define MAX_SPHERES 16
#define MAX_PLANES 4

struct Sphere {
    vec3 center;
    float radius;
    vec3 color;
    int materialType; // 0=diffuse, 1=metal, 2=dielectric
    float roughness;
    float ior;
};

struct Plane {
    vec3 point;
    vec3 normal;
    vec3 color;
    int materialType;
    float roughness;
};

uniform Sphere u_spheres[MAX_SPHERES];
uniform Plane u_planes[MAX_PLANES];
uniform int u_numSpheres;
uniform int u_numPlanes;

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
};

uint g_seed;

uint hash(uint x) {
    x += x << 10u;
    x ^= x >> 6u;
    x += x << 3u;
    x ^= x >> 11u;
    x += x << 15u;
    return x;
}

float random() {
    g_seed = hash(g_seed);
    return float(g_seed) / 4294967295.0;
}

vec3 randomInUnitSphere() {
    vec3 p;
    do {
        p = 2.0 * vec3(random(), random(), random()) - 1.0;
    } while (dot(p, p) >= 1.0);
    return p;
}

vec3 randomUnitVector() {
    return normalize(randomInUnitSphere());
}

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
    
    t = (t1 > 0.001) ? t1 : t2;
    return t > 0.001;
}

bool intersectPlane(Ray ray, Plane plane, out float t) {
    float denom = dot(plane.normal, ray.direction);
    if (abs(denom) < 0.0001) return false;
    
    t = dot(plane.point - ray.origin, plane.normal) / denom;
    return t > 0.001;
}

HitInfo intersectScene(Ray ray) {
    HitInfo closestHit;
    closestHit.hit = false;
    closestHit.t = 1e30;
    
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
        }
    }
    
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
        }
    }
    
    return closestHit;
}

vec3 getSkyColor(vec3 direction) {
    float t = 0.5 * (direction.y + 1.0);
    return mix(vec3(1.0, 1.0, 1.0), vec3(0.5, 0.7, 1.0), t) * 0.8;
}

bool scatter(Ray inRay, HitInfo hit, out vec3 attenuation, out Ray scattered) {
    if (hit.materialType == 0) { // Diffuse
        vec3 target = hit.point + hit.normal + randomUnitVector();
        scattered = Ray(hit.point, normalize(target - hit.point));
        attenuation = hit.color;
        return true;
    }
    else if (hit.materialType == 1) { // Metal
        vec3 reflected = reflect(normalize(inRay.direction), hit.normal);
        reflected += hit.roughness * randomInUnitSphere();
        scattered = Ray(hit.point, normalize(reflected));
        attenuation = hit.color;
        return dot(scattered.direction, hit.normal) > 0.0;
    }
    else if (hit.materialType == 2) { // Dielectric
        attenuation = vec3(1.0);
        float refraction_ratio = hit.ior;
        
        bool front_face = dot(inRay.direction, hit.normal) < 0.0;
        vec3 normal = front_face ? hit.normal : -hit.normal;
        
        if (!front_face) {
            refraction_ratio = 1.0 / hit.ior;
        }
        
        vec3 unit_direction = normalize(inRay.direction);
        float cos_theta = min(dot(-unit_direction, normal), 1.0);
        float sin_theta = sqrt(1.0 - cos_theta * cos_theta);
        
        bool cannot_refract = refraction_ratio * sin_theta > 1.0;
        vec3 direction;
        
        if (cannot_refract) {
            direction = reflect(unit_direction, normal);
        } else {
            direction = refract(unit_direction, normal, refraction_ratio);
        }
        
        scattered = Ray(hit.point, direction);
        return true;
    }
    
    return false;
}

// Path tracing
vec3 rayColor(Ray ray) {
    vec3 color = vec3(1.0);
    
    for (int bounce = 0; bounce < u_maxBounces; bounce++) {
        HitInfo hit = intersectScene(ray);
        
        if (!hit.hit) {
            color *= getSkyColor(ray.direction);
            break;
        }
        
        vec3 attenuation;
        Ray scattered;
        
        if (scatter(ray, hit, attenuation, scattered)) {
            color *= attenuation;
            ray = scattered;
        } else {
            color = vec3(0.0);
            break;
        }
    }
    
    return color;
}

void main() {
    vec2 uv = (2.0 * gl_FragCoord.xy - u_resolution) / u_resolution.y;
    
    g_seed = uint(gl_FragCoord.x) + uint(gl_FragCoord.y) * uint(u_resolution.x) + uint(u_time * 1000.0);
    
    vec3 totalColor = vec3(0.0);
    
    for (int sample = 0; sample < u_samplesPerPixel; sample++) {
        // Anti-aliasing jitter
        vec2 jitter = vec2(random() - 0.5, random() - 0.5) / u_resolution;
        vec2 coords = uv + jitter;
        
        // Calculate ray direction
        float theta = u_fov * 0.5;
        float halfHeight = tan(theta);
        float halfWidth = halfHeight;
        
        vec3 rayDir = normalize(
            coords.x * halfWidth * u_cameraRight +
            coords.y * halfHeight * u_cameraUp +
            u_cameraDir
        );
        
        Ray ray = Ray(u_cameraPos, rayDir);
        totalColor += rayColor(ray);
    }
    
    vec3 color = totalColor / float(u_samplesPerPixel);
    
    // Gamma correction
    color = sqrt(color);
    
    FragColor = vec4(color, 1.0);
}