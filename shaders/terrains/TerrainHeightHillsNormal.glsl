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

	float continents = octavenoise(octaves[3-fracnum], 0.65, lacunarity[3-fracnum], p, 1.0, 1.0) * (1.0-seaLevel) - (seaLevel*0.1);
	if (continents < 0.0) return 0.0;
	float n = continents;
	float distrib = octavenoise(octaves[4-fracnum], 0.5, lacunarity[4-fracnum], p, 1.0, 1.0);
	distrib *= distrib;
	float m = 0.5*amplitude[3-fracnum] * octavenoise(octaves[4-fracnum], 0.55*distrib, lacunarity[4-fracnum], p, 1.0, 1.0) * amplitude[5-fracnum];
	m += 0.25*billow_octavenoise(octaves[5-fracnum], 0.55*distrib, lacunarity[5-fracnum], p, 1.0, 1.0);
	//hill footings
	m -= octavenoise(octaves[2-fracnum], 0.6*(1.0-distrib), lacunarity[2-fracnum], p, 1.0, 1.0)
         * clamp(0.05-m, 0.0, 0.05) * clamp(0.05-m, 0.0, 0.05);
	//hill footings
	m += voronoiscam_octavenoise(octaves[6-fracnum], 0.765*distrib, lacunarity[6-fracnum], p, 1.0, 1.0)
         * clamp(0.025-m, 0.0, 0.025) * clamp(0.025-m, 0.0, 0.025);
	// cliffs at shore
	if (continents < 0.01) n += m * continents * 100.0;
	else n += m;

	return (n > 0.0 ? n*maxHeight : 0.0);
}
