#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "hittable.h"
#include "vec3.h"

class triangle : public hittable {
    public:
        triangle(point3& _p1, point3& _p2, point3& _p3, shared_ptr<material> _material)
            : p1(_p1), p2(_p2), p3(_p3), mat(_material) {
            
            // set the normal vector
            normal = cross(vec3(_p1 - _p2), vec3(_p2 - _p3));
            
            // set the "d" factor so that we can present the polygon as a plane
            d = dot(normal, vec3(_p1));

            // set the area
            double a    = vec3(_p1 - _p2).length();
            double b    = vec3(_p2 - _p3).length();
            double c    = vec3(_p3 - _p1).length();
            double s    = (a + b + c) / 2;
            area        = sqrt(s * (s-a) * (s-b) * (s-c));
        }

        bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
            if (dot(r.direction(), normal) == 0) {
                return false;
            }

            // find the intersection between the ray and the plane
            point3& intersection;

            return triangle(p1, p2, intersection).area() 
                    + triangle(p2, p3, intersection).area()
                    + triangle(p3, p1, intersection).area()
                    == area;
        }

        double getarea() const {
            return area;
        }

        void dump() const {
            std::cout << "Normal:  " << normal << std::endl;
            std::cout << "Area:    " << area << std::endl;
        }

    private:
        point3  &p1, &p2, &p3;
        vec3    normal;             // note: this vector is NOT normalized
        double  d;
        double  area;
        shared_ptr<material> mat;
};

#endif