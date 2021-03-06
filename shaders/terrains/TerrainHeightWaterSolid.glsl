// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

uniform float maxHeight;
uniform float seaLevel;
uniform int fracnum;

uniform int octaves[10];
uniform float amplitude[10];
uniform float lacunarity[10];
uniform float frequency[10];

float GetHeight(in vec3 p)
{
	float continents = 0.7*river_octavenoise(octaves[2], 0.5, lacunarity[2], p, 1.0, 1.0)-seaLevel;
	continents = amplitude[0] * ridged_octavenoise(octaves[0],
		clamp(continents, 0.0, 0.6), lacunarity[0], p, 1.0, 1.0);
	float mountains = ridged_octavenoise(octaves[2], 0.5, lacunarity[2], p, 1.0, 1.0);
	float hills = octavenoise(octaves[2], 0.5, lacunarity[2], p, 1.0, 1.0) *
		amplitude[1] * river_octavenoise(octaves[1], 0.5, lacunarity[1], p, 1.0, 1.0);
	float n = continents - (amplitude[0]*seaLevel);
	// craters
	n += crater_function(octaves[5], amplitude[5], frequency[5], lacunarity[5], p);
	if (n > 0.0) {
		// smooth in hills at shore edges
		if (n < 0.05) {
			n += hills * n * 4.0 ;
			n += n * 20.0 * (billow_octavenoise(octaves[3], 0.5*
				ridged_octavenoise(octaves[2], 0.5, lacunarity[2], p, 1.0, 1.0), lacunarity[3], p, 1.0, 1.0) +
				river_octavenoise(octaves[4], 0.5*
				ridged_octavenoise(octaves[3], 0.5, lacunarity[3], p, 1.0, 1.0), lacunarity[4], p, 1.0, 1.0) +
				billow_octavenoise(octaves[3], 0.6*
				ridged_octavenoise(octaves[4], 0.55, lacunarity[4], p, 1.0, 1.0), lacunarity[3], p, 1.0, 1.0));
		} else {
			n += hills * 0.2 ;
			n += billow_octavenoise(octaves[3], 0.5*
				ridged_octavenoise(octaves[2], 0.5, lacunarity[2], p, 1.0, 1.0), lacunarity[3], p, 1.0, 1.0) +
				river_octavenoise(octaves[4], 0.5*
				ridged_octavenoise(octaves[3], 0.5, lacunarity[3], p, 1.0, 1.0), lacunarity[4], p, 1.0, 1.0) +
				billow_octavenoise(octaves[3], 0.6*
				ridged_octavenoise(octaves[4], 0.55, lacunarity[4], p, 1.0, 1.0), lacunarity[3], p, 1.0, 1.0);
		}
		// adds mountains hills craters
		mountains = octavenoise(octaves[3], 0.5, lacunarity[3], p, 1.0, 1.0) *
			amplitude[2] * mountains*mountains*mountains;
		if (n < 0.4) n += 2.0 * n * mountains;
		else n += mountains * 0.8;
	}
	n = maxHeight*n;
	n = (n<0.0 ? -n : n);
	n = (n>1.0 ? 2.0-n : n);
	return n;
}
