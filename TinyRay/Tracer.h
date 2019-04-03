#pragma once

#include "Scene.h"
#include "Framebuffer.h"

//Base class for Tracers
class Tracer
{
protected:
	Framebuffer		*m_framebuffer;
	int				m_buffWidth;
	int				m_buffHeight;
	int				m_traceLevel;		//default:5
	int				m_renderCount;
public:
	Tracer();
	Tracer(int width, int height);

	//Trace a given scene
	//Params: Scene* pScene   Pointer to the scene to be traced
	virtual void DoTrace(Scene* pScene);

	inline void SetTraceLevel(int level)
	{
		m_traceLevel = level;
	}

	inline void ResetRenderCount()
	{
		m_renderCount = 0;
	}

	inline Framebuffer *GetFramebuffer() const
	{
		return m_framebuffer;
	}

	~Tracer();
};