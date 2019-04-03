#include "Tracer.h"

Tracer::Tracer()
{
	m_buffWidth = 0;
	m_buffHeight = 0;
	m_renderCount = 0;

	SetTraceLevel(5);
}

Tracer::Tracer(int width, int height)
{
	m_buffWidth = width;
	m_buffHeight = height;
	m_renderCount = 0;

	SetTraceLevel(5);

	m_framebuffer = new Framebuffer(width, height);
}

void Tracer::DoTrace(Scene * pScene) {}

Tracer::~Tracer()
{
	delete m_framebuffer;
}