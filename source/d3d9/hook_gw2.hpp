#pragma once

#include "log.hpp"
#include "d3d9_device.hpp"
#include "d3d9_swapchain.hpp"

#define MAX_TOKENS	8192

typedef union {
	DWORD d;
	float f;
} DWFL;

class hook_gw2 {
public:
	static void PresentHook(Direct3DSwapChain9* _implicit_swapchain);
	static HRESULT CreateVertexHook(const DWORD *pFunction, IDirect3DVertexShader9 **ppShader, IDirect3DDevice9* _orig, Direct3DSwapChain9* _implicit_swapchain);
	static HRESULT SetVertexHook(IDirect3DVertexShader9 *pShader, IDirect3DDevice9* _orig, Direct3DSwapChain9* _implicit_swapchain);
	static HRESULT CreatePixelHook(const DWORD *pFunction, IDirect3DPixelShader9 **ppShader, IDirect3DDevice9* _orig, Direct3DSwapChain9* _implicit_swapchain);
	static HRESULT SetPixelHook(IDirect3DPixelShader9 *pShader, IDirect3DDevice9* _orig, Direct3DSwapChain9* _implicit_swapchain);
private:
	static void replacePatternFog1(const DWORD *pFunction, int l, float _fog_amount);
	static void replacePatternFog2(const DWORD *pFunction, int l, float _fog_amount);
	static void replacePatternBloom(const DWORD *pFunction, int l);
	static void replacePatternSun(const DWORD *pFunction, int l);
	static bool isInjectionShaderChS(void* pShader);
	static bool isInjectionShaderSt(void* pShader);
	static bool isInjectionShaderUs(void* pShader);
	static bool isEnd(DWORD token);
	static int get_pattern(const DWORD *pFunction, int l);
	static bool checkPattern(const DWORD *pFunction, int l, DWORD *pattern, int pl);
	static int getFuncLenght(const DWORD *pFunction);
	static void logShader(const DWORD* pFunction);

	static DWORD _pFunction[MAX_TOKENS];
	static void* _pShaderSt;
	static void* _pShaderUs;
	static void* _pShaderChS;

	static DWORD patternStable[];//l = 13
	static DWORD patternUnstable[]; //l=24
	static DWORD patternCharSelec[]; //l = 7
	static DWORD patternBloom[]; //l = 7
	static DWORD patternSun[]; //l = 7

	static int vs_count; //Used to save ref of second Us pattern and not first (second is 241th vs created)
	static bool onCharSelecLastFrame;
	static bool onCharSelec;
	static bool can_use_unstable;
	static bool unstable_in_cframe;
	static bool fx_applied;
};

