# RayAndPathTracer
![Ray and Path Tracing](https://github.com/Hesketh/RayAndPathTracer/blob/master/RayAndPath.png?raw=true)

## Ray Tracing
Ray tracing is a rendering technique for generating an image by tracing the path of light in a scene ([Wikipedia](https://en.wikipedia.org/wiki/Ray_tracing_(graphics))). My ray tracer was completed as a part of my Graphics I modules during my 2nd year at the University of Derby.

## Path Tracing
Path tracing is a Monte Carlo method of generating an image of a scene by integrating over the illuminance arriving at each point of the scene. ([Wikipedia](https://en.wikipedia.org/wiki/Path_tracing)). My path tracer was completed as a part of my Graphics II modules during my 2nd year at the University of Derby.

### Sampling Quality
The accuracy of the Path Tracing algorithm at generating life like lighting is greatly impacted by the amount of times each pixel's lighting is sampled, as each iteration brings the average closer to the correct colour. As we can see below; lower samples per pixel (SPP) have much more noise.

![Noise Comparison](https://github.com/Hesketh/RayAndPathTracer/blob/master/SamplesPerPath.png?raw=true)
*Left: 50SPP, Middle: 100SPP, Right: 500SPP*

If we compare the path tracing alogrithm with the ray tracing algorithm we can see a notable difference with the shadows. Ray tracing produces hard and solid shadows whereas path tracing produces a much softer shadow.

![Shadow Comparison](https://github.com/Hesketh/RayAndPathTracer/blob/master/ShadowComp.png?raw=true)

The main advantage of path tracing is the diffuse to diffuse reflectance. Light rays receive a fraction of any object's diffuse they collider with makes the final colour of all light rays be slightly tinted like in real life. 

![Diffuse Diffuse Reflectance](https://github.com/Hesketh/RayAndPathTracer/blob/master/ColourBlendTint.png?raw=true)

### Performance
The significant downside of my path tracing algorithm is how much longer it takes compared to the Ray Tracing algorithm (<5 seconds). Below features some benchmarks on a couple of my machines. All results that are measured are the render time of the scene in seconds, columns are different rendering resolutions and rows are different amounts of samples per pixel. 

The following measurements of rendering time when ran on a PC with the follow specifications;
Intel Core i5-7600k @ 3.80 GHz, 16 GB DDR4 2100Mhz, NVIDIA GeForce GTX 770 2GB VRAM,
Windows 10 Home 64 Bit. 

SPP | 160\*120 | 480\*360 | 1280\*720
--- | --- | --- | ---
50 | 2.61 | 23.87 | 124.07
50 | 5.13 | 46.29 | 248.98
50 | 25.88 | 236.03 | 1252.09

The following measurements of rendering time where ran on a PC with the follow specifications;
Intel Core i5-6200U @ 2.40 GHz, 12 GB DDR4 1600Mhz, Windows 10 Pro 64 Bit. 

SPP | 160\*120 | 480\*360 | 1280\*720
--- | --- | --- | ---
50 | 5.74 | 52.58 | 277.15
50 | 11.29 | 102.08 | 540.51
50 | 56.94 | 519.31 | 2756.07
