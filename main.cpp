#include <iostream>
#include "float.h"

#include "hitable_list.hpp"
#include "sphere.hpp"
#include "camera.hpp"
#include "util.hpp"
#include "material.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

vec3 color(const ray& r, hitable *world, int depth) {
    hit_record rec;
    if (world->hit(r, 0.001, MAXFLOAT, rec)) {
        ray scattered;
        vec3 attenuation;
        if (depth < 50 && rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
            return attenuation * color(scattered, world, depth + 1);
        } else {
            return vec3(0, 0, 0);
        }
    } else {
        vec3 unit_direction = unit_vector(r.direction());
        float t = 0.5 * (unit_direction.y() + 1.0);
        return (1.0 - t) * vec3(1, 1, 1) + t * vec3(0.5, 0.7, 1.0);
    }
}

int main() {
    const int nx = 200;
    const int ny = 100;
    const int ns = 100;

    hitable *list[4];
    list[0] = new sphere(vec3(0, 0, -1), 0.5, new lambertian(vec3(0.8, 0.3, 0.3)));
    list[1] = new sphere(vec3(0, -100.5, -1), 100, new lambertian(vec3(0.8, 0.8, 0)));
    list[2] = new sphere(vec3(1, 0, -1), 0.5, new metal(vec3(0.8, 0.6, 0.2), 0.3));
    list[3] = new sphere(vec3(-1, 0, -1), 0.5, new metal(vec3(0.8, 0.8, 0.8), 1.0));
    
    hitable *world = new hitable_list(list, 4);

    std::cout << "Begin Tracing " << nx << " " << ny << std::endl;
    

    camera cam;

    const int num_pixel_bytes = 3;
    unsigned char *data = new unsigned char [nx * ny * num_pixel_bytes];

    for (int j = ny-1; j >= 0; j--) {
        std::cout << "tracing line " << (j+1) << "/" << ny << std::endl;
        for (int i = 0; i < nx; i++) {

            vec3 col(0, 0, 0);
            for (int s = 0; s < ns; s++) {
                float u = float(i + drand48()) / float(nx);
                float v = float(j + drand48()) / float(ny);
                ray r = cam.get_ray(u, v);
                vec3 p = r.point_at_parameter(2.0);
                col += color(r, world, 0);
            }
            col /= float(ns);
            col = vec3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2])); // simple gamma fix
            int ir = int(255 * col.r());
            int ig = int(255 * col.g());
            int ib = int(255 * col.b());
            
            const int pixel_idx = (i + (ny - 1 - j) * nx) * num_pixel_bytes;

            data[pixel_idx + 0] = (unsigned char) ir;
            data[pixel_idx + 1] = (unsigned char) ig;
            data[pixel_idx + 2] = (unsigned char) ib;
        }
    }

    const char *filename = "foo.png";
    std::cout << std::endl << std::endl << "Tracing complete, writing " << filename << std::endl;
    
    // using num_pixel_bytes for comp parameter, where 1=Y, 2=YA, 3=RGB, 4=RGBA
    stbi_write_png(filename, nx, ny, num_pixel_bytes, data, 0);

}
