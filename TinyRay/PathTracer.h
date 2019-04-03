#pragma once

#include "Tracer.h"

class PathTracer : public Tracer
{
private:
	int m_samplesCount;
private:
	Ray SampleDiffuse(Ray& ray, RayHitResult result) const;
	Colour Radiance(Scene* scene, Ray& ray, int depth);

	//Returns a random number of type double
	static float GetUniformFloat();
public:
	PathTracer();
	PathTracer(int width, int height);

	inline void SetSampleCount(int count)
	{
		m_samplesCount = count;
	}

	//Trace a given Scene
	//Params: 
	//	Scene* pScene		Pointer to the scene to be path traced
	//	int numberOfSamples	The number of times to sample the scene
	void DoTrace(Scene* pScene) override;
};