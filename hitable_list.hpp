#pragma once

#include "hitable.hpp"

class hitable_list : public hitable {
public:
    hitable_list() {}
    virtual ~hitable_list();
    hitable_list(std::vector<hitable *> v) { vecarr = v; }
    virtual bool hit(const ray &r, float t_min, float t_max, hit_record &rec) const;

    std::vector<hitable *> vecarr;
};

bool hitable_list::hit(const ray &r, float t_min, float t_max, hit_record &rec) const {
    hit_record temp_rec;
    bool hit_anything = false;
    double closest_so_far = t_max;
    for (std::vector<hitable *>::const_iterator it = vecarr.begin(); it != vecarr.end(); it++) {
        hitable *thing = *it;
        if (thing->hit(r, t_min, closest_so_far, temp_rec)) {
            hit_anything = true;
            closest_so_far = temp_rec.t;
            rec = temp_rec;
        }
    }

    return hit_anything;
}

hitable_list::~hitable_list() {
    std::cout << "Cleaning up hitable_list" << std::endl;
    for (std::vector<hitable *>::const_iterator it = vecarr.begin(); it != vecarr.end(); it++) {
        delete *it;
    }
}