#include <vector>

#include "rtweekend.h"

#include "camera.h"
#include "color.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"
#include "triangle.h"

#define CHECKERBOARD_FLOOR
#define ADD_POLYGONS
#define GENERATE_RANDOM_SPHERES

int main() {
    
    const double ground_y = -0.2;
    hittable_list world;


    auto ground_material_white = make_shared<metal>(color(0.9, 0.7, 0.2), 0);
    auto ground_material_black = make_shared<metal>(color(0.1, 0.1, 0.1), 0.1);
    
    point3 c0 = point3(-10000, ground_y, -10000), c1 = point3(-10000, ground_y, 10000),
            c2 = point3(10000, ground_y, 10000), c3 = point3(10000, ground_y, -10000);

    world.add(make_shared<triangle>(c0, c1, c3, ground_material_black));
    world.add(make_shared<triangle>(c2, c1, c3, ground_material_white));
    
   
    #ifdef ADD_POLYGONS
    // add a "sail"
    point3 sail_c0 = point3(2.7, -0.2, -1.7), sail_c1 = point3(4.7, -0.2, -1.7), 
            sail_c2 = point3(4.7, 2.2, -2.2), sail_c3 = point3(2.5, 2.2, -2.2);

    point3 sail_center = point3((sail_c0.x() + sail_c1.x() + sail_c2.x() + sail_c3.x()) / 4,
                                (sail_c0.y() + sail_c1.y() + sail_c2.y() + sail_c3.y()) / 4,
                                (sail_c0.z() + sail_c1.z() + sail_c2.z() + sail_c3.z()) / 4);

    auto sail_q0_mat = make_shared<metal>(color(0.2, 1, 0.2), 0);
    auto sail_q1_mat = make_shared<metal>(color(0.2, 0.2, 1), 0);
    auto sail_q2_mat = make_shared<metal>(color(0.8, 0.8, 0.8), 0);
    auto sail_q3_mat = make_shared<metal>(color(1, 0.2, 0.2), 0);

   
    triangle sail_quadrant_0 = triangle(sail_c0, sail_c1, sail_center, sail_q0_mat);
    triangle sail_quadrant_1 = triangle(sail_c1, sail_c2, sail_center, sail_q1_mat);
    triangle sail_quadrant_2 = triangle(sail_c2, sail_c3, sail_center, sail_q2_mat);
    triangle sail_quadrant_3 = triangle(sail_c3, sail_c0, sail_center, sail_q3_mat);

    world.add(make_shared<triangle>(sail_quadrant_0));
    world.add(make_shared<triangle>(sail_quadrant_1));
    world.add(make_shared<triangle>(sail_quadrant_2));
    world.add(make_shared<triangle>(sail_quadrant_3));
    #endif

    #ifdef GENERATE_RANDOM_SPHERES
    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = random_double();
            point3 center(a + 0.9*random_double(), 0.2, b + 0.9*random_double());

            if ((center - point3(4, 0.2, 0)).length() > 0.9) {
                shared_ptr<material> sphere_material;

                if (choose_mat < 0.5) {
                    // diffuse
                    auto albedo = color::random() * color::random();
                    sphere_material = make_shared<lambertian>(albedo);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                } else if (choose_mat < 0.7) {
                    // metal
                    auto albedo = color::random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = make_shared<metal>(albedo, fuzz);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                } else {
                    // glass
                    sphere_material = make_shared<dielectric>(1.5);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }
    #endif

    auto material1 = make_shared<metal>(color(0.2, 0.5, 0.6), 0);
    world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material1));

    auto material2 = make_shared<metal>(color(0.9, 0.7, 0.6), 0.0);
    world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material2));    

    auto material3 = make_shared<dielectric>(1.5);
    world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

    camera cam;

    cam.aspect_ratio      = 16.0 / 10.0;
    cam.image_width       = 1920;
    cam.samples_per_pixel = 500;
    cam.max_depth         = 50;

    cam.vfov     = 25;
    cam.lookfrom = point3(13,2,3);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    // cam.defocus_angle = 0.6;
    cam.focus_dist    = 10.0;

    cam.render(world);
}