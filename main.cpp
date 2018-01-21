#include <iostream>

int main() {
    const int nx = 200;
    const int ny = 100;
    
    std::cout << "P3\n" << nx << " " << ny << "\n255\n";
    for (int j = ny-1; j >= 0; j--) {
        for (int i = 0; i < nx; i++) {
            float r = float(i) / float(nx);
            float g = float(j) / float(ny);
            float b = 0.2;
            
            int ir = int(255.999 * r);
            int ig = int(255.999 * g);
            int ib = int(255.999 * b);
            
            std::cout << ir << " " << ig << " " << ib << "\n";
        }
    }
}
