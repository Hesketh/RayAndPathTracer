/*---------------------------------------------------------------------
*
* Copyright © 2015  Minsi Chen
* E-mail: m.chen@derby.ac.uk
*
* The source is written for the Graphics I and II modules. You are free
* to use and extend the functionality. The code provided here is functional
* however the author does not guarantee its performance.
---------------------------------------------------------------------*/
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>


#if defined(WIN32) || defined(_WINDOWS)
#include <Windows.h>
#include <gl/GL.h>
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#endif

#include "RayTracer.h"
#include "Ray.h"
#include "Scene.h"
#include "Camera.h"
#include "perlin.h"

RayTracer::RayTracer()
{
	m_traceflag = (TraceFlags)(TRACE_AMBIENT | TRACE_DIFFUSE_AND_SPEC |
		TRACE_SHADOW | TRACE_REFLECTION | TRACE_REFRACTION);

}

RayTracer::RayTracer(int width, int height) : Tracer(width, height)
{
	//default set default trace flag, i.e. no lighting, non-recursive
	m_traceflag = (TraceFlags)(TRACE_DIFFUSE_AND_SPEC |
		TRACE_SHADOW | TRACE_REFLECTION);
}

void RayTracer::DoTrace( Scene* pScene )
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
	
	int total = m_buffHeight*m_buffWidth;
	int done_count = 0;
	
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
		fprintf(stdout, "Ray Trace started.\n");

		Colour colour;
//TinyRay on multiprocessors using OpenMP!!!
#pragma omp parallel for schedule (dynamic, 1) private(colour)
		for (int i = 0; i < m_buffHeight; i+=1) {
			for (int j = 0; j < m_buffWidth; j+=1) {

				//calculate the metric size of a pixel in the view plane (e.g. framebuffer)
				Vector3 pixel;

				pixel[0] = start[0] + (i + 0.5) * camUpVector[0] * pixelDY
					+ (j + 0.5) * camRightVector[0] * pixelDX;
				pixel[1] = start[1] + (i + 0.5) * camUpVector[1] * pixelDY
					+ (j + 0.5) * camRightVector[1] * pixelDX;
				pixel[2] = start[2] + (i + 0.5) * camUpVector[2] * pixelDY
					+ (j + 0.5) * camRightVector[2] * pixelDX;

				/*
				* setup first generation view ray
				* In perspective projection, each view ray originates from the eye (camera) position 
				* and pierces through a pixel in the view plane
				*/
				Ray viewray;
				viewray.SetRay(camPosition,	(pixel - camPosition).Normalise());
				
				double u = (double)j / (double)m_buffWidth;
				double v = (double)i / (double)m_buffHeight;

				scenebg = pScene->GetBackgroundColour();

				//trace the scene using the view ray
				//default colour is the background colour, unless something is hit along the way
				colour = this->TraceScene(pScene, viewray, scenebg, m_traceLevel);

				/*
				* Draw the pixel as a coloured rectangle
				*/
				m_framebuffer->WriteRGBToFramebuffer(colour, j, i);
			}
		}

		timer = clock() - timer;

		fprintf(stdout, "Ray Trace completed in %f seconds!\n", (float)timer/CLOCKS_PER_SEC);
		m_renderCount++;
	}
}

Colour RayTracer::TraceScene(Scene* pScene, Ray& ray, Colour incolour, int tracelevel, bool shadowray)
{
	RayHitResult result;

	Colour outcolour = incolour; //the output colour based on the ray-primitive intersection

	//Ensure that we still should be Tracing the scene, each successive call of tracescene reduces tracelevel by 1 (prevent stack overflow)
	if (tracelevel >= 0)
	{
		std::vector<Light*> *light_list = pScene->GetLightList();
		Vector3 cameraPosition = pScene->GetSceneCamera()->GetPosition();

		//Intersect the ray with the scene
		//DONE: Scene::IntersectByRay needs to be implemented first
		result = pScene->IntersectByRay(ray);

		if (result.data) //the ray has hit something
		{
			//DONE:
			//1. Non-recursive ray tracing:
			//	 When a ray intersect with an objects, determine the colour at the intersection point
			//	 using CalculateLighting
			
			outcolour = CalculateLighting(light_list, &cameraPosition, &result);

			//2. The following conditions are for implementing recursive ray tracing
			if (m_traceflag & TRACE_REFLECTION && ((Primitive*)result.data)->m_primtype != Primitive::PRIMTYPE_Plane)
			{
				//Calculate the normalised value of the reflection of the ray
				Vector3 vecReflection = ray.GetRay().Reflect(result.normal).Normalise();
				Vector3 vecStartPos = result.point + (vecReflection * 0.001);	//We move a small amount out in the direction of the ray to prevent self reflecting
				
				Ray rayReflected;	//Create a ray from the hit position along the reflected position
				rayReflected.SetRay(vecStartPos, vecReflection);
				
				outcolour = outcolour + (TraceScene(pScene, rayReflected, outcolour, tracelevel - 1, shadowray));
			}

			if (m_traceflag & TRACE_REFRACTION && ((Primitive*)result.data)->m_primtype != Primitive::PRIMTYPE_Plane)
			{
				//DONE: trace the refraction ray from the intersection point
				Vector3 vecRefraction = ray.GetRay().Refract(result.normal, 1.05);
				Vector3 vecStartPos = result.point + (vecRefraction * 0.001);	//We move a small amount out in the direction of the ray to prevent self refracting

				Ray rayRefraction;
				rayRefraction.SetRay(vecStartPos, vecRefraction);

				outcolour = outcolour + (TraceScene(pScene, rayRefraction, outcolour, tracelevel - 1, shadowray));
			}

			if (m_traceflag & TRACE_SHADOW)
			{
				for (Light* lightItem : *light_list)
				{
					//I dont do shadows recursively as I could not determine if an object was between the point and the current light or after the light
					Vector3 vecPointToLight = (lightItem->GetLightPosition() - result.point).Normalise();
					Vector3 vecStartPosition = result.point + (vecPointToLight * 0.001); //We move a small amount out in the direction of the ray to prevent self shadowing

					Ray rayShadow;
					rayShadow.SetRay(vecStartPosition, vecPointToLight);

					RayHitResult resultShadow;
					resultShadow = pScene->IntersectByRay(rayShadow);

					//If there is an object blocking the ray to the light (ray has hit something)
					if (resultShadow.data)
					{
						//Ensure the object is before the point and the lightsource
						float distanceToLight = (result.point - lightItem->GetLightPosition()).Norm();
						float distanceToPoint = (result.point - resultShadow.point).Norm();

						if (distanceToLight > distanceToPoint)
						{
							//We only apply the shadow once ignoring the amount of lights, or else the shadow becomes too dark
							outcolour = outcolour * Vector3(0.1f, 0.1f, 0.1f);
							break;
						}
					}
				}
			}
		}
	}

	return outcolour;
}

Colour RayTracer::CalculateLighting(std::vector<Light*>* lights, Vector3* campos, RayHitResult* hitresult)
{
	Colour outcolour;
	std::vector<Light*>::iterator lit_iter = lights->begin();

	Primitive* prim = (Primitive*)hitresult->data;
	Material* mat = prim->GetMaterial();

	outcolour = mat->GetAmbientColour();
	
	//Generate the grid pattern on the plane
	if (((Primitive*)hitresult->data)->m_primtype == Primitive::PRIMTYPE_Plane)
	{
		int dx = hitresult->point[0]/2.0;
		int dy = hitresult->point[1]/2.0;
		int dz = hitresult->point[2]/2.0;

		if (dx % 2 || dy % 2 || dz % 2 )
		{
			outcolour = Vector3(0.1, 0.1, 0.1);
		}
		else
		{
			outcolour = mat->GetDiffuseColour();
		}

	}
	
	////Go through all lights in the scene
	////Note the default scene only has one light source
	else if (m_traceflag & TRACE_DIFFUSE_AND_SPEC)
	{
		//Vectors
		Vector3 vecNormal = hitresult->normal.Normalise();
		Vector3 vecView = (*campos - hitresult->point).Normalise();

		//Colours of material
		Colour matDiffuse = mat->GetDiffuseColour();
		Colour matSpecular = mat->GetSpecularColour();

		for (Light* lightItem : *lights)
		{
			//Vectors (involving light)
			Vector3 vecLight = (lightItem->GetLightPosition() - hitresult->point).Normalise();
			Vector3 vecReflected = (hitresult->point - lightItem->GetLightPosition()).Normalise().Reflect(vecNormal);

			//Colours of light
			Colour cLight = lightItem->GetLightColour();

			//Diffuse using lambertian model
			float cosAngle = vecNormal.DotProduct(vecLight);
			if (cosAngle < 0) { cosAngle = 0; }			//Clamp values of cosAngle between 0 and 1
			else if (cosAngle > 1) { cosAngle = 1; }

			Colour diffuse = matDiffuse * cLight * cosAngle;

			//Specular using phong model
			int specularPower = mat->GetSpecPower();
			float specAngle = vecView.DotProduct(vecReflected);
			if (specAngle < 0) { specAngle = 0; }			//Clamp values of specAngle between 0 and 1
			else if (specAngle > 1) { specAngle = 1; }

			Colour specular = matSpecular * cLight * pow(specAngle, specularPower);

			//Calculate the effects on the outcolour caused by this light
			outcolour = outcolour + diffuse + specular;
		}
	}

	return outcolour;
}