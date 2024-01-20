#ifndef CAMERA_H
#define CAMERA_H

#include <memory>
#include <vector>

#include "rtweekend.h"

#include "color.h"
#include "hittable.h"
#include "hittable_list.h"
#include "material.h"

typedef struct thread_args {
    hittable_list world;
    int thread_id;
} thread_args;

class camera {
  public:
    double aspect_ratio      = 1.0;  // Ratio of image width over height
    int    image_width       = 100;  // Rendered image width in pixel count
    int    samples_per_pixel = 20;   // Count of random samples for each pixel
    int    max_depth         = 10;   // Maximum number of ray bounces into scene

    double vfov = 90;  // Vertical view angle (field of view)
    point3 lookfrom = point3(0,0,-1);  // Point camera is looking from
    point3 lookat   = point3(0,0,0);   // Point camera is looking at
    vec3   vup      = vec3(0,1,0);     // Camera-relative "up" direction

    double defocus_angle = 0;  // Variation angle of rays through each pixel
    double focus_dist = 10;    // Distance from camera lookfrom point to plane of perfect focus

    int num_worker_threads   = 1;

    /**
     * Multi-threaded rendering notes:
     * 
     * 1) Pre-initialize frame buffer
     * 2) Set thread arguments:
     *  - reference to world
     *  - thread index
     *  - # of threads
     *  - samples per pixel
     * 3) Have each thread work on an independent pixel
     * 4) Export when all threads terminate
    */
    void render(const hittable_list& world) {
        initialize();

        thread_args * args = new thread_args[this->num_worker_threads];
        pthread_t * threads = new pthread_t[this->num_worker_threads];

        for (int tid = 0; tid < 16; ++tid) {
            args[tid].world = world;
            args[tid].thread_id = tid;

            pthread_create(&threads[tid], NULL, &thread_render, (void *) &args[tid]);
        }

        for (int j = 0; j < image_height; ++j) {
            std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
            for (int i = 0; i < image_width; ++i) {
                // color pixel_color(0, 0, 0);
                for (int sample = 0; sample < samples_per_pixel; ++sample) {
                    ray r = get_ray(i, j);
                    frame_buffer[j][i] += ray_color(r, max_depth, world);
                }
                // write_color(std::cout, pixel_color, samples_per_pixel);

            }
        }

        std::clog << "\rDone.                 \n";
        export_image();
    }

  private:
    int    image_height;   // Rendered image height
    point3 center;         // Camera center
    point3 pixel00_loc;    // Location of pixel 0, 0
    vec3   pixel_delta_u;  // Offset to pixel to the right
    vec3   pixel_delta_v;  // Offset to pixel below
    vec3   u, v, w;        // Camera frame basis vectors
    vec3   defocus_disk_u;  // Defocus disk horizontal radius
    vec3   defocus_disk_v;  // Defocus disk vertical radius

    color ** frame_buffer;

    const int MIN_THREADS = 1, MAX_THREADS = 128;

    inline void initialize() {
        image_height = static_cast<int>(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height;

        frame_buffer = new color * [this->image_height];
        
        for (int j = 0; j < this->image_height; ++j) {
            frame_buffer[j] = new color[image_width];
            for (int i = 0; i < this->image_width; ++i) {
                frame_buffer[j][i] = color(0, 0, 0);
            }
        }

        center = lookfrom;

        // Determine viewport dimensions.
        auto theta = degrees_to_radians(vfov);
        auto h = tan(theta/2);
        auto viewport_height = 2 * h * focus_dist;
        auto viewport_width = viewport_height * (static_cast<double>(image_width)/image_height);

        // Calculate the u,v,w unit basis vectors for the camera coordinate frame.
        w = unit_vector(lookfrom - lookat);
        u = unit_vector(cross(vup, w));
        v = cross(w, u);


        // Calculate the vectors across the horizontal and down the vertical viewport edges.
        vec3 viewport_u = viewport_width * u;    // Vector across viewport horizontal edge
        vec3 viewport_v = viewport_height * -v;  // Vector down viewport vertical edge


        // Calculate the horizontal and vertical delta vectors from pixel to pixel.
        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;

        // Calculate the location of the upper left pixel.
        auto viewport_upper_left = center - (focus_dist * w) - viewport_u/2 - viewport_v/2;
        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

        // Calculate the camera defocus disk basis vectors.
        auto defocus_radius = focus_dist * tan(degrees_to_radians(defocus_angle / 2));
        defocus_disk_u = u * defocus_radius;
        defocus_disk_v = v * defocus_radius;

        // error checking number of worker threads
        if (this->num_worker_threads < MIN_THREADS) {
            this->num_worker_threads = MIN_THREADS;
        }
        if (this->num_worker_threads > MAX_THREADS) {
            this->num_worker_threads = MAX_THREADS;
        }
    }

    static void * thread_render(void * args) {
        thread_args * targs = (thread_args *) args;

        const hittable_list world = targs->world;

        return nullptr;
    }
    

    inline void export_image() {

        std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

        for (int j = 0; j < this->image_height; ++j) {
            for (int i = 0; i < this->image_width; ++i) {
                auto r = frame_buffer[j][i].x();
                auto g = frame_buffer[j][i].y();
                auto b = frame_buffer[j][i].z();

                // Divide the color by the number of samples.
                auto scale = 1.0 / this->samples_per_pixel;
                r *= scale;
                g *= scale;
                b *= scale;

                // Apply the linear to gamma transform.
                r = linear_to_gamma(r);
                g = linear_to_gamma(g);
                b = linear_to_gamma(b);

                // Write the translated [0,255] value of each color component.
                static const interval intensity(0.000, 0.999);
                std::cout << static_cast<int>(256 * intensity.clamp(r)) << ' '
                    << static_cast<int>(256 * intensity.clamp(g)) << ' '
                    << static_cast<int>(256 * intensity.clamp(b)) << '\n';
            }
        }
    }

    point3 defocus_disk_sample() const {
        // Returns a random point in the camera defocus disk.
        auto p = random_in_unit_disk();
        return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
    }

    color ray_color(const ray& r, int depth, const hittable& world) const {
        hit_record rec;

        // If we've exceeded the ray bounce limit, no more light is gathered.
        if (depth <= 0)
            return color(0,0,0);

        if (world.hit(r, interval(0.001, infinity), rec)) {
            ray scattered;
            color attenuation;
            if (rec.mat->scatter(r, rec, attenuation, scattered))
                return attenuation * ray_color(scattered, depth-1, world);
            return color(0,0,0);
        }

        vec3 unit_direction = unit_vector(r.direction());
        auto a = 0.5*(unit_direction.y() + 1.0);
        return (1.0-a)*color(0.5, 0.7, 1.0) + a*color(1.0, 1.0, 1.0);
    }

    ray get_ray(int i, int j) const {
        // Get a randomly-sampled camera ray for the pixel at location i,j, originating from
        // the camera defocus disk.

        auto pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
        auto pixel_sample = pixel_center + pixel_sample_square();

        auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
        auto ray_direction = pixel_sample - ray_origin;

        return ray(ray_origin, ray_direction);
    }

    vec3 pixel_sample_square() const {
        // Returns a random point in the square surrounding a pixel at the origin.
        auto px = -0.5 + random_double();
        auto py = -0.5 + random_double();
        return (px * pixel_delta_u) + (py * pixel_delta_v);
    }
};

#endif