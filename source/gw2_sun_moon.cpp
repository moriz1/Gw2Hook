#include "gw2_sun_moon.hpp"

coord_set gw2_sun_moon::def_coord = { vec(-90.0f, 43.5f, -2.1f), vec(80.0f, 60.0f, -0.2f) };

coord_set gw2_sun_moon::c[2048] = {NULL};

void gw2_sun_moon::init() {
	c[0] = { vec(0,0,0), vec(0,0,0) };
	c[18] = { NULL, vec(-61.9f, 76.0f, -20.2f) };// DR
	c[20] = { NULL, vec(-84.9f, 52.7f, -2.0f) };// Strie
	c[24] = { NULL, vec(-78.5f, 60.0f, 15.4f) };// Gendarran
	c[29] = { vec(-80.2f, 59.5f, 4.3f), NULL };//Canopée
	c[32] = { NULL, vec(-94.45f, 26.0f, -20.0f) };// Diessa
	c[39] = { vec(-72.8f, 68.5f, -1.8f), NULL };//Mt mstrom
	c[50] = { vec(-68.0f, 62.0f, -39.0f), vec(-60.0f, 71.4f, -37.5f) };//LA
	c[73] = { vec(-71.3f, 70.0f, 3.4f), NULL };//Marée
	c[873] = { vec(-92.8f, 35.6f, 10.41f), NULL };//Sud soleil
	c[1041] = { vec(-78.0f, 60.1f, -17.3f) , NULL };//DS
	c[1052] = { vec(-97.0f, 21.4f, -9.45f), NULL };//Orée
}


D3DXVECTOR3* gw2_sun_moon::sun(int mapid) {
	D3DXVECTOR3* coord = c[mapid].sun;
	return coord==NULL ? def_coord.sun : coord;
}

D3DXVECTOR3* gw2_sun_moon::moon(int mapid) {
	D3DXVECTOR3* coord = c[mapid].moon;
	return coord == NULL ? def_coord.moon : coord;
}

D3DXVECTOR3* gw2_sun_moon::vec(float x, float y, float z) {
	D3DXVECTOR3* coord = new D3DXVECTOR3(x, y, z);
	return coord;
}