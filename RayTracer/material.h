#pragma once

#include "hittable.h"

class material
{
public:
	virtual ~material() = default;

	virtual bool scatter(const ray& rIn, const hitRecord& rec, colour& attenuation, ray& scattered) const
	{
		return false;
	}
};

class lambertian : public material
{
public:
	lambertian(const colour& albedo) : albedo(albedo) {}

	bool scatter(const ray& rIn, const hitRecord& rec, colour& attenuation, ray& scattered) const override
	{
		auto scatterDirection = rec.normal + randomUnitVector();

		if (scatterDirection.nearZero())
		{
			scatterDirection = rec.normal;
		}

		scattered = ray(rec.p, scatterDirection);
		attenuation = albedo;
		return true;
	}

private:
	colour albedo;
};

class metal : public material
{
public:
	metal(const colour& albedo, double fuzz) : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1) {}

	bool scatter(const ray& rIn, const hitRecord& rec, colour& attenuation, ray& scattered) const override
	{
		vec3 reflected = reflect(rIn.direction(), rec.normal);
		reflected = unitVector(reflected) + (fuzz * randomUnitVector());
		scattered = ray(rec.p, reflected);
		attenuation = albedo;
		return (dot(scattered.direction(), rec.normal) > 0);
	}

private:
	colour albedo;
	double fuzz;
};

class dielectric : public material
{
public:
	dielectric(double refractionIndex) : refractionIndex(refractionIndex) {}

	bool scatter(const ray& rIn, const hitRecord& rec, colour& attenuation, ray& scattered) const override
	{
		attenuation = colour(1.0, 1.0, 1.0);
		double ri = rec.frontFace ? (1.0 / refractionIndex) : refractionIndex;

		vec3 unitDirection = unitVector(rIn.direction());
		double cosTheta = fmin(dot(-unitDirection, rec.normal), 1.0);
		double sinTheta = sqrt(1.0 - cosTheta * cosTheta);

		bool cannotRefract = ri * sinTheta > 1.0;
		vec3 direction;

		if (cannotRefract || reflectance(cosTheta, ri) > randomDouble())
		{
			direction = reflect(unitDirection, rec.normal);
		}
		else
		{
			direction = refract(unitDirection, rec.normal, ri);
		}

		scattered = ray(rec.p, direction);
		return true;
	}
private:
	double refractionIndex;

	static double reflectance(double cosine, double refractionIndex)
	{
		auto r0 = (1 - refractionIndex) / (1 + refractionIndex);
		r0 = r0 * r0;
		return r0 + (1 - r0) * pow((1 - cosine), 5);
	}
};