#pragma once

#include "d3d9.hpp"
#include "log.hpp"


#define MAX_TOKENS	8192

typedef union {
	DWORD d;
	float f;
} DWFL;

class hook_gw2 {
public:
	hook_gw2(Direct3DDevice9* device) :
		_device(device),
		_pShaderInjection_stable(NULL),
		_pShaderInjection(NULL),
		_pShaderCharScreen(NULL),
		_pShaderLightMap(NULL),
		_pShaderPostLightMap(NULL),
		_surface_lightmap(NULL),
		_surface_current(NULL),
		_is_fx_done(false),
		_is_on_char_screen_last_frame(false),
		_is_on_char_screen(false),
		_pattern_InjectionStable{ 0x800c0001, 0xa0550006, 0x3000009, 0x80010002, 0x80e40001, 0xa0e40004, 0x3000009, 0x80020002, 0x80e40001, 0xa0e40005, 0x2000001, 0xe0030000, 0x80440002 },//l = 13
		_pattern_Injection{ 0x3000005, 0x80280800, 0x80000000, 0x80ff0001, 0x2000001, 0x80270800, 0x80e40001 }, //l = 7
		_pattern_charScreen{ 0x80440002, 0x2000001, 0xe00c0000, 0x90440009, 0x2000001, 0xe0030001, 0x90440008 }, //l = 7
		_pattern_bloom{ 0x3000005, 0x80270000, 0x80e40000, 0xa0ff0000, 0x2000001, 0x802f0800, 0x80e40000, }, //l = 7
		_pattern_sun{ 0x1000041, 0x800f0002, 0x1000041, 0x800f0000, 0x2000001, 0x800f0800, 0xa0000001 }, //l = 7
		_pattern_lightMap{ 0xa0000000, 0x80400000, 0x80950000, 0x3000005, 0x800f0800, 0x80e40000, 0x80e40001 }, //l = 7
		_pattern_lightMapFab{ 0x800f0000, 0x80e40000, 0x80e40001, 0x3000005, 0x800f0800, 0x80e40000, 0xa0000000 },
		_pattern_postLightMap{ 0x0, 0x2000001, 0x802f0000, 0xa0000000, 0x2000001, 0x802f0800, 0x80e40000 } //l = 7
	{ }

	void PresentHook();
	HRESULT CreateVertexHook(const DWORD *pFunction, IDirect3DVertexShader9 **ppShader);
	HRESULT SetVertexHook(IDirect3DVertexShader9 *pShader);
	HRESULT CreatePixelHook(const DWORD *pFunction, IDirect3DPixelShader9 **ppShader);
	HRESULT SetPixelHook(IDirect3DPixelShader9 *pShader);
	HRESULT SetRenderTargetHook(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget);

private:
	void replacePatternFog1(const DWORD *pFunction, int l, float _fog_amount);
	void replacePatternFog2(const DWORD *pFunction, int l, float _fog_amount);
	void replacePatternBloom(const DWORD *pFunction, int l);
	void replacePatternSun(const DWORD *pFunction, int l);

	bool isInjectionShaderChS(void* pShader);
	bool isInjectionShaderSt(void* pShader);
	bool isInjectionShaderUs(void* pShader);
	bool isInjectionShaderLit(void* pShader);
	bool isInjectionShaderPLit(void* pShader);
	bool isEnd(DWORD token);

	int get_pattern(const DWORD *pFunction, int l);
	bool checkPattern(const DWORD *pFunction, int l, DWORD *pattern, int pl);
	int getFuncLenght(const DWORD *pFunction);

	void logShader(const DWORD* pFunction);

	DWORD _pFunction[MAX_TOKENS];
	void* _pShaderInjection_stable;
	void* _pShaderInjection;
	void* _pShaderCharScreen;
	void* _pShaderLightMap;
	void* _pShaderLightMapFab;
	void* _pShaderPostLightMap;

	DWORD _pattern_InjectionStable[13];//l = 13
	DWORD _pattern_Injection[7]; //l = 7
	DWORD _pattern_charScreen[7]; //l = 7
	DWORD _pattern_bloom[7]; //l = 7
	DWORD _pattern_sun[7]; //l = 7
	DWORD _pattern_lightMap[7]; //l = 7
	DWORD _pattern_lightMapFab[7];
	DWORD _pattern_postLightMap[7]; //l = 7

	bool _is_on_char_screen_last_frame;
	bool _is_on_char_screen;
	bool can_use_unstable;
	bool unstable_in_cframe;
	bool _is_fx_done;

	IDirect3DSurface9* _surface_lightmap;
	IDirect3DSurface9* _surface_current;
	Direct3DDevice9 *_device;
};	

