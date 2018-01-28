#pragma once

#include "ray.hpp"

class camera {
public:
    camera() {
        lower_left_corner = vec3(-2, -1, -1);
        horizontal = vec3(4, 0, 0);
        vertical = vec3(0, 2, 0);
        origin = vec3(0, 0, 0);
    }

    camera(float vfov, float aspect) { // vfov is top to bottom, in degrees
        float theta = vfov * M_PI / 180;
        float half_height = tan(theta / 2);
        float half_width = aspect * half_height;
        lower_left_corner = vec3(-half_width, -half_height, -1);
        horizontal = vec3(2 * half_width, 0, 0);
        vertical = vec3(0, 2 * half_height, 0);
        origin = vec3(0, 0, 0);
    }

    ray get_ray(float u, float v) const {
        return ray(origin, lower_left_corner + u*horizontal + v*vertical - origin);
    }

    vec3 origin;
    vec3 lower_left_corner;
    vec3 horizontal;
    vec3 vertical;
};
