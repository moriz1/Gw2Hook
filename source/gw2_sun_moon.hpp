#pragma once

#include <d3d9.h>
#include <d3dx9shader.h>

struct coord_set {
	D3DXVECTOR3* sun;
	D3DXVECTOR3* moon;
};

class gw2_sun_moon {
public:
	static void init();
	static D3DXVECTOR3* sun(int mapid);
	static D3DXVECTOR3* moon(int mapid);
private:
	static coord_set c[];
	static D3DXVECTOR3* vec(float x, float y, float z);
	static coord_set def_coord;
};
