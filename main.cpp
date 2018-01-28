#include <thread>
#include <iostream>
#include <vector>
#include <float.h>

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

const int num_pixel_bytes = 3;
const int nx = 400;
const int ny = 200;
const int ns = 100;

void trace_line(int t_id, int j, const camera& cam, hitable *world, unsigned char *data) {
    std::cout << t_id << ") tracing line " << (j+1) << "/" << ny << std::endl;
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

void trace(int t_id, const camera& cam, hitable *world, unsigned char *data) {
    std::cout << "Begin Tracing thread_id: " << t_id << ", " << nx << "x" << ny << std::endl;

    for (int j = ny-1; j >= 0; j--) {
        if ((j & 0b111) == t_id) {
            trace_line(t_id, j, cam, world, data);
        }
    }

}

std::vector<hitable *> random_scene() {
    std::vector<hitable *> vecarr;
    vecarr.push_back(new sphere(vec3(0, -1000, 0), 1000, new lambertian(vec3(0.5, 0.5, 0.5))));
    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            float choose_mat = drand48();
            vec3 center(a + 0.9*drand48(), 0.2, b + 0.9*drand48());
            if ((center - vec3(4, 0.2, 0)).length() > 0.9) {
                sphere *s;
                if (choose_mat < 0.8) { // diffuse
                    vec3 albedo(drand48()*drand48(), drand48()*drand48(), drand48()*drand48());
                    s = new sphere(center, 0.2, new lambertian(albedo));
                } else if (choose_mat < 0.5) { // metal
                    vec3 albedo(0.5 * (1 + drand48()), 0.5 * (1 + drand48()), 0.5 * (1 + drand48()));
                    float fuzz = 0.5 * drand48();
                    s = new sphere(center, 0.2, new metal(albedo, fuzz));
                } else { // glass
                    s = new sphere(center, 0.2, new dielectric(1.5));
                }
                vecarr.push_back(s);
            }
        }
    }
    vecarr.push_back(new sphere(vec3(0, 1, 0), 1, new dielectric(1.5)));
    vecarr.push_back(new sphere(vec3(-4, 1, 0), 1, new lambertian(vec3(0.4, 0.2, 0.1))));
    vecarr.push_back(new sphere(vec3(4, 1, 0), 1, new metal(vec3(0.7, 0.6, 0.5), 0)));

    std::cout << "random scene of " << vecarr.size() << " spheres" << std::endl;
    return vecarr;
}

std::vector<hitable *> simple_scene() {
    std::vector<hitable *> vecarr;

    vecarr.push_back(new sphere(vec3(0, 0, -1), 0.5, new lambertian(vec3(0.1, 0.2, 0.5))));
    vecarr.push_back(new sphere(vec3(0, -100.5, -1), 100, new lambertian(vec3(0.8, 0.8, 0))));
    vecarr.push_back(new sphere(vec3(1, 0, -1), 0.5, new metal(vec3(0.8, 0.6, 0.2), 0.3)));
    vecarr.push_back(new sphere(vec3(-1, 0, -1), 0.5, new dielectric(1.5)));
    vecarr.push_back(new sphere(vec3(-1, 0, -1), -0.45, new dielectric(1.5)));

    std::cout << "simple scene of " << vecarr.size() << " spheres" << std::endl;
    return vecarr;
}

int main() {
    hitable *world = new hitable_list(simple_scene());
    // hitable *world = new hitable_list(random_scene());

    vec3 lookfrom(13, 2, 3);
    vec3 lookat(0, 0, 0);
    float dist_to_focus = 10.0;
    float aperture = 0.1;
    camera cam(lookfrom, lookat, vec3(0, 1, 0), 20, float(nx)/float(ny), aperture, dist_to_focus);

    unsigned char *data = new unsigned char [nx * ny * num_pixel_bytes];

    unsigned int hardware_concurrency = std::thread::hardware_concurrency();
    std::cout << hardware_concurrency << " concurrent threads are supported.\n";

    std::thread* threads[hardware_concurrency];
    for (int t_id = 0; t_id < hardware_concurrency; t_id++) {
        threads[t_id] = new std::thread(trace, t_id, cam, world, data);
    }

    for (int t_id = 0; t_id < hardware_concurrency; t_id++) {
        threads[t_id]->join();
    }

    delete world;

    const char *filename = "foo.png";
    std::cout << std::endl << std::endl << "Tracing complete, writing " << filename << std::endl;
    
    // using num_pixel_bytes for comp parameter, where 1=Y, 2=YA, 3=RGB, 4=RGBA
    stbi_write_png(filename, nx, ny, num_pixel_bytes, data, 0);
}
