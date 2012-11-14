// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

uniform float maxHeight;
uniform float seaLevel;
uniform int fracnum;

uniform int octaves[10];
uniform float amplitude[10];
uniform float lacunarity[10];
uniform float frequency[10];

uniform sampler2D texHeightmap;
	

float GetHeight(in vec3 p)
{
	float latitude = -asin(p.y);
	if (p.y < -1.0) latitude = -0.5*M_PI;
	if (p.y > 1.0) latitude = 0.5*M_PI;
//	if (!isfinite(latitude)) {
//		// p.y is just n of asin domain [-1,1]
//		latitude = (p.y < 0 ? -0.5*M_PI : M_PI*0.5);
//	}
	float longitude = atan2(p.x, p.z);
	float px = (((m_heightMapSizeX-1) * (longitude + M_PI)) / (2.0*M_PI));
	float py = ((m_heightMapSizeY-1)*(latitude + 0.5*M_PI)) / M_PI;
	int ix = int(floor(px));
	int iy = int(floor(py));
	ix = clamp(ix, 0, m_heightMapSizeX-1);
	iy = clamp(iy, 0, m_heightMapSizeY-1);
	float dx = px-ix;
	float dy = py-iy;

	// p0,3 p1,3 p2,3 p3,3
	// p0,2 p1,2 p2,2 p3,2
	// p0,1 p1,1 p2,1 p3,1
	// p0,0 p1,0 p2,0 p3,0
	float map[4][4];
	for (int x=-1; x<3; x++) {
		for (int y=-1; y<3; y++) {
			vec2 hmuv = vec2(clamp(ix+x, 0, m_heightMapSizeX-1), clamp(iy+y, 0, m_heightMapSizeY-1));
			map[x+1][y+1] = texture2D(texHeightmap, hmuv).x;
			//map[x+1][y+1] = m_heightMapScaled[clamp(iy+y, 0, m_heightMapSizeY-1)*m_heightMapSizeX + clamp(ix+x, 0, m_heightMapSizeX-1)];
		}
	}

	float c[4];
	for (int j=0; j<4; j++) {
		float d0 = map[0][j] - map[1][j];
		float d2 = map[2][j] - map[1][j];
		float d3 = map[3][j] - map[1][j];
		float a0 = map[1][j];
		float a1 = -(1.0/3.0)*d0 + d2 - (1.0/6.0)*d3;
		float a2 = 0.5*d0 + 0.5*d2;
		float a3 = -(1.0/6.0)*d0 - 0.5*d2 + (1.0/6.0)*d3;
		c[j] = a0 + a1*dx + a2*dx*dx + a3*dx*dx*dx;
	}

	{
		float d0 = c[0] - c[1];
		float d2 = c[2] - c[1];
		float d3 = c[3] - c[1];
		float a0 = c[1];
		float a1 = -(1.0/3.0)*d0 + d2 - (1.0/6.0)*d3;
		float a2 = 0.5*d0 + 0.5*d2;
		float a3 = -(1.0/6.0)*d0 - 0.5*d2 + (1.0/6.0)*d3;
		float v = 0.1 + a0 + a1*dy + a2*dy*dy + a3*dy*dy*dy;

		//v = (v<0 ? 0 : v);

		v=v*m_heightScaling+m_minh; // v = v*height scaling+min height
		v/=m_planetRadius;

		v += 0.1;
		float h = 1.5*v*v*v*ridged_octavenoise(16, 4.0*v, 4.0, p, 1.0, 1.0);
		h += 30000.0*v*v*v*v*v*v*v*ridged_octavenoise(16, 5.0*v, 20.0*v, p, 1.0, 1.0);
		h += v;
		h -= 0.09;

		return (h > 0.0 ? h : 0.0);

	}

}

