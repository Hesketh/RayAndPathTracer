#include "PathTracer.h"
#include <time.h>

#define M_PI 3.14159265358979323846

PathTracer::PathTracer()
{
	m_samplesCount = 10;
}

PathTracer::PathTracer(int width, int height) : Tracer(width, height)
{
	m_samplesCount = 10;
}

Ray PathTracer::SampleDiffuse(Ray& ray, RayHitResult result) const
{
	Vector3 orientedNormal = result.normal.DotProduct(ray.GetRay()) < 0 ? result.normal : result.normal * -1; //Front facing normal

	float angle = 2 * M_PI * GetUniformFloat();
	float elevation = GetUniformFloat();
	float elevationSqrt = sqrt(elevation);

	Vector3 w = orientedNormal; //normal at the hit point
	Vector3 u = (fabs(w[0]) > .1 ? Vector3(0, 1, 0) : Vector3(1, 0, 0)).CrossProduct(w);	//Orthogonal (at right angle to) to w 
	Vector3 v = w.CrossProduct(u);	//Orthogonal to both w and u

	Vector3 direction = ((u * cos(angle) * elevationSqrt) + (v * sin(angle) * elevationSqrt) + (w * sqrtf(1 - elevation))).Normalise(); //Sample unit hemisphere
	Vector3 origin = result.point + direction * 0.001;	//offset to prevent self intersection

	Ray outray;
	outray.SetRay(origin, direction);

	return outray;
}

Colour PathTracer::Radiance(Scene* scene, Ray& ray, int depth)
{
	RayHitResult result = scene->IntersectByRay(ray);

	if(result.data)
	{
		Colour diffuse = static_cast<Primitive*>(result.data)->GetMaterial()->GetDiffuseColour();
		Colour emissive = static_cast<Primitive*>(result.data)->GetMaterial()->GetEmissiveColour();
		Colour specular = static_cast<Primitive*>(result.data)->GetMaterial()->GetSpecularColour();

		float random = GetUniformFloat();

		if(depth <= 0)
		{
			float maxReflectance = diffuse[0] > diffuse[1] && diffuse[0] > diffuse[2] ? diffuse[0] : diffuse[1] > diffuse[2] ? diffuse[1] : diffuse[2];
			
			//Russian Roulette to terminate after minimum depth reached
			if (random < maxReflectance)
			{
				diffuse = diffuse * (1.0f / maxReflectance);
			}
			else { return emissive; }
		}

		//Randomly trace Spcular or Diffuse reflectance
		Colour* colourFromMaterial;
		if(random > .5f)
		{
			ray = SampleDiffuse(ray, result);
			colourFromMaterial = &diffuse;
		}		
		else
		{
			Vector3 d = ray.GetRay().Reflect(result.normal);
			ray.SetRay(result.point + d*0.001, d);
			colourFromMaterial= &specular;
		}

		//Compute BRDF for the ray (Assuming lambertian reflection)
		float angle = ray.GetRay().DotProduct(result.normal);
		Colour brdf = *colourFromMaterial * 2 * angle; //bidirectional reflectance distribution function, aka how light is reflected at a surface 
		Colour reflected = Radiance(scene, ray, depth - 1);

		return emissive + (brdf * reflected);
	}
	return Colour(0,0,0);
}

void PathTracer::DoTrace(Scene* pScene)
{
	clock_t timer = clock();

	Camera* cam = pScene->GetSceneCamera();

	Vector3 camRightVector = cam->GetRightVector();
	Vector3 camUpVector = cam->GetUpVector();
	Vector3 camViewVector = cam->GetViewVector();
	Vector3 centre = cam->GetViewCentre();
	Vector3 camPosition = cam->GetPosition();

	double sceneWidth = pScene->GetSceneWidth();
	double sceneHeight = pScene->GetSceneHeight();

	double pixelDX = sceneWidth / m_buffWidth;
	double pixelDY = sceneHeight / m_buffHeight;

	Vector3 start;

	start[0] = centre[0] - ((sceneWidth * camRightVector[0])
		+ (sceneHeight * camUpVector[0])) / 2.0;
	start[1] = centre[1] - ((sceneWidth * camRightVector[1])
		+ (sceneHeight * camUpVector[1])) / 2.0;
	start[2] = centre[2] - ((sceneWidth * camRightVector[2])
		+ (sceneHeight * camUpVector[2])) / 2.0;

	Colour scenebg = pScene->GetBackgroundColour();

	if (m_renderCount == 0)
	{
		fprintf(stdout, "Path Trace started with %d samples.\n", m_samplesCount);

		Colour colour;
//TinyRay on multiprocessors using OpenMP!!!
#pragma omp parallel for schedule (dynamic, 1) private(colour)
		for (int y = 0; y < m_buffHeight; y += 1) 
		{
			for (int x = 0; x < m_buffWidth; x += 1) 
			{
				colour = Colour(0, 0, 0);

				//calculate the metric size of a pixel in the view plane (e.g. framebuffer)
				Vector3 pixel;

				pixel[0] = start[0] + (y + 0.5) * camUpVector[0] * pixelDY
					+ (x + 0.5) * camRightVector[0] * pixelDX;
				pixel[1] = start[1] + (y + 0.5) * camUpVector[1] * pixelDY
					+ (x + 0.5) * camRightVector[1] * pixelDX;
				pixel[2] = start[2] + (y + 0.5) * camUpVector[2] * pixelDY
					+ (x + 0.5) * camRightVector[2] * pixelDX;

				//Sample each pixel an amount of times
				for (int s = 0; s < m_samplesCount; s++)
				{
					Vector3 direction = (pixel - camPosition).Normalise();

					Ray viewray;
					viewray.SetRay(camPosition, direction);

					//trace the scene using the view ray
					//add the average colour calculated from the trace
					//sample count must be cast! Otherwise result will never go beyond black
					colour = colour + Radiance(pScene, viewray, m_traceLevel) * (1.0f / static_cast<float>(m_samplesCount));
				}
				m_framebuffer->WriteRGBToFramebuffer(colour, x, y);
			}
		}

		timer = clock() - timer;

		fprintf(stdout, "Path Trace completed in %f seconds!\n", static_cast<float>(timer)/CLOCKS_PER_SEC);
		m_renderCount++;
	}
}


float PathTracer::GetUniformFloat()
{
	//TODO: Replace with Halton Sequence
	return static_cast<float> (rand()) / static_cast<float>(RAND_MAX);
}
