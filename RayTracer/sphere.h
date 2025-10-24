#pragma once

#include "hittable.h"

class sphere : public hittable
{
public:
	sphere(const point3& center, double radius, shared_ptr<material> mat) : center(center), radius(fmax(0, radius)), mat(mat) {}

	bool hit(const ray& r, interval rayT, hitRecord& rec) const override
	{
		vec3 oc = center - r.origin();
		auto a = r.direction().lengthSquared();
		auto h = dot(r.direction(), oc);
		auto c = oc.lengthSquared() - radius * radius;

		auto discriminant = h * h - a * c;
		if (discriminant < 0)
		{
			return false;
		}

		auto sqrtd = sqrt(discriminant);

		auto root = (h - sqrtd) / a;
		if (!rayT.surrounds(root))
		{
			root = (h + sqrtd) / a;
			if (!rayT.surrounds(root))
			{
				return false;
			}
		}

		rec.t = root;
		rec.p = r.at(rec.t);
		vec3 outwardNormal = (rec.p - center) / radius;
		rec.setFaceNormal(r, outwardNormal);
		rec.mat = mat;

		return true;
	}
private:
	point3 center;
	double radius;
	shared_ptr<material> mat;
};