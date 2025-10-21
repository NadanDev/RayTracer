#pragma once

class hitRecord
{
public:
	point3 p;
	vec3 normal;
	double t;
	bool frontFace;

	void setFaceNormal(const ray& r, const vec3& outwardNormal)
	{
		// Sets the hit record normal vector.
		// NOTE: The parameter 'outwordNormal' is assumed to have unit length

		frontFace = dot(r.direction(), outwardNormal) < 0;
		normal = frontFace ? outwardNormal : -outwardNormal;
	}
};

class hittable
{
public:
	virtual ~hittable() = default;

	virtual bool hit(const ray& r, interval rayT, hitRecord& rec) const = 0;
};