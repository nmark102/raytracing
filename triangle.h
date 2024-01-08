#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "hittable.h"
#include "vec3.h"

class triangle : public hittable {
    public:
        triangle(point3& _p1, point3& _p2, point3& _p3, shared_ptr<material> _material)
            : p1(_p1), p2(_p2), p3(_p3), mat(_material) {
            
            // set the normal vector
            this->normal = cross(vec3(_p1 - _p2), vec3(_p2 - _p3));
            
            // set the "d" factor so that we can present the polygon as a plane
            d = dot(normal, vec3(_p1));

            // set the area
            double a    = vec3(_p1 - _p2).length();
            double b    = vec3(_p2 - _p3).length();
            double c    = vec3(_p3 - _p1).length();
            double s    = (a + b + c) / 2;
            this->area  = sqrt(s * (s-a) * (s-b) * (s-c));
        }

        // interval prevents hits behind the camera to be registered
        bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
            // disregard if the ray is parallel to/on the plane
            if (dot(r.direction(), normal) == 0) {
                return false;
            }

            // find the intersection point between the ray and the plane
            // first solve for t
            double t = (this->d - dot(this->normal, r.origin())) 
                            / dot(this->normal, r.direction());
            // std::cout << "In hit(): t = " << t << std::endl;

            // using t, compute the intersection point
            point3 intersection = r.at(t);

            // using this intersection point and the corners of the polygon,
            // we can create 3 inner triangles
            // if the area of these 3 new triangles add up to the area of the
            // original triangle (within tolerance), hit

            double diff = triangle(p1, p2, intersection, nullptr).getarea() 
                    + triangle(p2, p3, intersection, nullptr).getarea()
                    + triangle(p3, p1, intersection, nullptr).getarea()
                    - this->area;

            // std::cout << "Section size difference = " << diff << std::endl;

            if (ray_t.surrounds(t) 
                && -hit_tolerance <= diff && diff <= hit_tolerance) {

                rec.p       = intersection;
                rec.t       = t;
                rec.mat     = this->mat;
                rec.set_face_normal(r, unit_vector(this->normal));
                // std::cout << "hit\n";
                return true;
            }
            return false;
        }

        double getarea() const {
            return this->area;
        }

        void dump() const {
            std::cout << "Normal:  " << this->normal << std::endl;
            std::cout << "Area:    " << this->area << std::endl;
        }

    private:
        constexpr static double hit_tolerance = 1e-3;

        point3  &p1, &p2, &p3;
        vec3    normal;             // note: this vector is NOT normalized
        double  d;
        double  area;
        shared_ptr<material> mat;
};

#endif