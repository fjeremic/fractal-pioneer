// Original Copyright HackerPoet/MarbleMarcher (https://github.com/HackerPoet/MarbleMarcher).
//
// The following code is a derivative work of the code from the MarbleMarcher project,
// which is licensed GPLv2. This code therefore is also licensed under the terms
// of the GNU Public License, verison 2.
//
// For information on the license of this code when distributed with and used
// in conjunction with the other modules in the MarbleMarcher project, please see
// the root-level LICENSE file.

#version 120

#define MIN_DIST 1e-5
#define MAX_DIST 30.0
#define MAX_MARCHES 1000
#define MAX_ITERATIONS 16

uniform vec3 in_camera_position;
uniform mat3 in_camera_rotation;

uniform float in_fractal_scale;
uniform vec3 in_fractal_shift;
uniform vec3 in_fractal_rotation;
uniform vec3 in_fractal_color;
uniform float in_fractal_exposure;

uniform float in_scene_ambient_occlusion_delta;
uniform float in_scene_ambient_occlusion_strength;
uniform float in_scene_anti_aliasing_samples;
uniform vec3 in_scene_background_color;
uniform bool in_scene_diffuse_lighting;
uniform bool in_scene_filtering;
uniform float in_scene_focal_distance;
uniform bool in_scene_fog;
uniform vec3 in_scene_light_color;
uniform vec3 in_scene_light_direction;
uniform bool in_scene_shadows;
uniform float in_scene_shadow_darkness;
uniform float in_scene_shadow_sharpness;
uniform float in_scene_specular_highlight;
uniform float in_scene_specular_multiplier;

uniform vec2 in_resolution;

void mengerFold(inout vec4 p) {
    float dxy = min(p.x - p.y, 0.0);
    p += vec4(-dxy, +dxy, 0.0, 0.0);
    float dxz = min(p.x - p.z, 0.0);
    p += vec4(-dxz, 0.0, +dxz, 0.0);
    float dyz = min(p.y - p.z, 0.0);
    p += vec4(0.0, -dyz, +dyz, 0.0);
}

void rotateX(inout vec4 p, float a) {
    float s = sin(a);
    float c = cos(a);

    p.yz = vec2(c * p.y + s * p.z, c * p.z - s * p.y);
}

void rotateZ(inout vec4 p, float a) {
    float s = sin(a);
    float c = cos(a);

    p.xy = vec2(c * p.x + s * p.y, c * p.y - s * p.x);
}

float fractalDistanceEstimate(vec4 p) {
    for (int i = 0; i < MAX_ITERATIONS; ++i) {
        p = abs(p);
        rotateZ(p, in_fractal_rotation.z);
        mengerFold(p);
        rotateX(p, in_fractal_rotation.x);

        p *= vec4(in_fractal_scale);
        p += vec4(in_fractal_shift, 0.0);
    }

    vec3 a = abs(p.xyz) - 6.0;

    // Distance estimate to a 6.0 box
    return (min(max(max(a.x, a.y), a.z), 0.0) + length(max(a, 0.0))) / p.w;
}

vec4 fractalColour(vec4 p) {
    vec3 orbit = vec3(0.0);
    for (int i = 0; i < MAX_ITERATIONS; ++i) {
        p = abs(p);
        rotateZ(p, in_fractal_rotation.z);
        mengerFold(p);
        rotateX(p, in_fractal_rotation.x);

        p *= vec4(in_fractal_scale);
        p += vec4(in_fractal_shift, 0.0);

        orbit = max(orbit, p.xyz * in_fractal_color);
    }

    return vec4(orbit, 0.0);
}

vec4 rayMarch(inout vec4 p, vec4 ray, float sharpness) {
    float d = fractalDistanceEstimate(p);
    float s = 0.0;
    float t = 0.0;
    float m = 1.0;

    for (; s < MAX_MARCHES; s += 1.0) {
        // If the distance from the surface is less than the distance per pixel we stop
        float minDistance = max(1.0 / in_resolution.x * t, MIN_DIST);

        if (d < minDistance) {
            s += d / minDistance;
            break;
        } else if (t > MAX_DIST) {
            break;
        }

        t += d;
        p += ray * d;
        m = min(m, sharpness * d / t);
        d = fractalDistanceEstimate(p);
    }

    return vec4(d, s, t, m);
}

vec4 scene(vec4 p, vec4 ray) {
    vec4 colour = vec4(0.0);

    vec4 dstm = rayMarch(p, ray, 1.0f);

    float d = dstm.x;
    float s = dstm.y;
    float t = dstm.z;

    float minDistance = max(1.0 / in_resolution.x * t, MIN_DIST);
    if (d < minDistance) {
        // Calculate the surfrance normal
        // http://www.iquilezles.org/www/articles/normalsSDF/normalsSDF.htm
        const vec3 h = vec3(1.0, -1.0, 0.0);
        vec3 n = normalize(h.xyy * fractalDistanceEstimate(p + h.xyyz * minDistance) +
                           h.yyx * fractalDistanceEstimate(p + h.yyxz * minDistance) +
                           h.yxy * fractalDistanceEstimate(p + h.yxyz * minDistance) +
                           h.xxx * fractalDistanceEstimate(p + h.xxxz * minDistance));

        // Find closest surface point because without this we get weird colouring artifacts
        p.xyz -= n * d;

        if (in_scene_filtering) {
            // Cross product between the ray and the surface normal, should be parallel to the surface
            vec3 s1 = normalize(cross(ray.xyz, n));

            // Cross product between s1 and the surface normal
            vec3 s2 = cross(s1, n);

            // Find the average color of the fractal in a radius dx in plane s1 - s2
            colour = (fractalColour(p + vec4(s1, 0.0) * minDistance) +
                      fractalColour(p - vec4(s1, 0.0) * minDistance) +
                      fractalColour(p + vec4(s2, 0.0) * minDistance) +
                      fractalColour(p - vec4(s2, 0.0) * minDistance)) / 4;
        } else {
            colour = fractalColour(p);
        }

        colour = clamp(colour, 0.0, 1.0);

        // Shadow scaling factor
        float shadow = 1.0;

        if (in_scene_shadows) {
            vec4 lightPoint = vec4(p.xyz + n * MIN_DIST * 100, p.w);

            // March a ray from the surface normal towards to light source and check if we hit it via the minimum distance
            dstm = rayMarch(lightPoint, vec4(in_scene_light_direction, 0.0), in_scene_shadow_sharpness);

            float lt = dstm.z;
            float lm = dstm.w;
            shadow = lm * min(lt, 1.0);
        }

        if (in_scene_specular_highlight > 0) {
            vec3 reflectedRay = ray.xyz - 2.0 * dot(ray.xyz, n) * n;
            float specular = max(dot(reflectedRay, in_scene_light_direction), 0.0);
            specular = pow(specular, in_scene_specular_highlight);
            colour.xyz += specular * in_scene_light_color * (shadow * in_scene_specular_multiplier);
        }

        if (in_scene_diffuse_lighting) {
            shadow = min(shadow, in_scene_shadow_darkness * 0.5 * (dot(n, in_scene_light_direction) - 1.0) + 1.0);
        }

        // Don't make shadows entirely dark
        shadow = max(shadow, 1.0 - in_scene_shadow_darkness);

        // Actually apply the shadow
        colour.xyz *= in_scene_light_color * shadow;

        // Add small amount of ambient occlusion
        float a = 1.0 / (1.0 + s * in_scene_ambient_occlusion_strength);
        colour.xyz += (1.0 - a) * vec3(in_scene_ambient_occlusion_delta);

        if (in_scene_fog) {
            a = t / MAX_DIST;
            colour.xyz = (1.0 - a) * colour.xyz + a * in_scene_background_color;
        }
    } else {
        // Ray missed so set the colour to the background colour
        colour.xyz = in_scene_background_color;
    }

    return colour;
}

void main() {
    vec4 colour = vec4(0.0);

    for (int i = 0; i < in_scene_anti_aliasing_samples; ++i) {
        for (int j = 0; j < in_scene_anti_aliasing_samples; ++j) {
            // Get normalized screen coordinate
            vec2 aaDelta = vec2(i, j) / in_scene_anti_aliasing_samples;
            vec2 screenPosition = (gl_FragCoord.xy + aaDelta) / in_resolution.xy;

            vec2 uv = 2.0 * screenPosition - 1;
            uv.x *= in_resolution.x / in_resolution.y;

            // Convert screen coordinate into a ray
            vec4 ray = vec4(in_camera_rotation * normalize(vec3(uv.x, uv.y, -in_scene_focal_distance)), 0.0);

            // Reflect the light if the ray intersects the fractal
            colour += scene(vec4(in_camera_position, 1.0), ray);
        }
    }

    // Apply exposure
    colour *= in_fractal_exposure / (in_scene_anti_aliasing_samples * in_scene_anti_aliasing_samples);

    gl_FragColor = vec4(clamp(colour.xyz, 0.0, 1.0), 1.0);
}
