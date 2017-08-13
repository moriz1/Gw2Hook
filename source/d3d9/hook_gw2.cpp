#include "hook_gw2.hpp"
#include <d3dx9shader.h>
#include <sstream>

DWORD hook_gw2::patternStable[] = { 0x800c0001, 0xa0550006, 0x3000009, 0x80010002, 0x80e40001, 0xa0e40004, 0x3000009,
									0x80020002, 0x80e40001, 0xa0e40005, 0x2000001, 0xe0030000, 0x80440002 };//l = 13

DWORD hook_gw2::patternUnstable[] = { 0x3000005, 0x80280800, 0x80000000, 0x80ff0001, 0x2000001, 0x80270800, 0x80e40001 }; //l = 7 PS !!

/*DWORD hook_gw2::patternUnstable[] = { 0x80070000, 0xa0000004, 0x2000001, 0x80070000, 0xa0000004, 0x3000009, 0xe0010001,
0x90e40000, 0xa0e40000, 0x3000009, 0xe0020001, 0x90e40000, 0xa0e40001, 0x3000009,
0xe0040001, 0x90e40000, 0xa0e40002, 0x3000009, 0xe0080001, 0x90e40000, 0xa0e40003,
0x2000001, 0xe0030000, 0x90e40007 }; //l=24*/

DWORD hook_gw2::patternCharSelec[] = { 0x80440002, 0x2000001, 0xe00c0000, 0x90440009, 0x2000001, 0xe0030001, 0x90440008 }; //l = 7

DWORD hook_gw2::patternBloom[] = { 0x3000005, 0x80270000, 0x80e40000, 0xa0ff0000, 0x2000001, 0x802f0800, 0x80e40000, }; //l = 7

DWORD hook_gw2::patternSun[] = { 0x1000041, 0x800f0002, 0x1000041, 0x800f0000, 0x2000001, 0x800f0800, 0xa0000001 }; //l = 7

DWORD hook_gw2::patternLight[] = { 0xa0000000, 0x80400000, 0x80950000, 0x3000005, 0x800f0800, 0x80e40000, 0x80e40001 }; //l = 7

DWORD hook_gw2::patternPostLight[] = { 0x0, 0x2000001, 0x802f0000, 0xa0000000, 0x2000001, 0x802f0800, 0x80e40000 }; //l = 7

DWORD hook_gw2::_pFunction[MAX_TOKENS];
void* hook_gw2::_pShaderSt = NULL;
void* hook_gw2::_pShaderUs = NULL;
void* hook_gw2::_pShaderChS = NULL;
void* hook_gw2::_pShaderLit = NULL;
void* hook_gw2::_pShaderPLit = NULL;

bool hook_gw2::can_use_unstable = false;
bool hook_gw2::unstable_in_cframe = false;
bool hook_gw2::fx_applied = false;
bool hook_gw2::onCharSelecLastFrame = false;
bool hook_gw2::onCharSelec = false;

IDirect3DSurface9* hook_gw2::lightSurface = NULL;
IDirect3DSurface9* hook_gw2::currentSurface = NULL;

void hook_gw2::PresentHook(Direct3DSwapChain9* _implicit_swapchain, IDirect3DDevice9* _orig) {
	if (onCharSelec) {
		LPDIRECT3DSURFACE9 l_Surface;
		_implicit_swapchain->_runtime->_lightbuffer_texture->GetSurfaceLevel(0, &l_Surface);
		_orig->ColorFill(l_Surface, nullptr, D3DCOLOR_COLORVALUE(0.2f, 0.2f, 0.2f, 0.2f));
		l_Surface->Release();

		onCharSelecLastFrame = true;
	}
	else onCharSelecLastFrame = false;
	onCharSelec = false;

	if (!fx_applied) {
		_implicit_swapchain->_runtime->applyPostFX(onCharSelecLastFrame, currentSurface);
	}
	fx_applied = false;
	if (unstable_in_cframe) {
		can_use_unstable = true;
	} else {
		can_use_unstable = false;
	}
	unstable_in_cframe = false;
}

HRESULT hook_gw2::SetRenderTargetHook(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget, IDirect3DDevice9* _orig) {
	HRESULT hr = _orig->SetRenderTarget(RenderTargetIndex, pRenderTarget);
	currentSurface = pRenderTarget;
	return hr;
}

HRESULT hook_gw2::CreateVertexHook(const DWORD *pFunction, IDirect3DVertexShader9 **ppShader, IDirect3DDevice9* _orig, Direct3DSwapChain9* _implicit_swapchain) {
	HRESULT hr = _orig->CreateVertexShader(pFunction, ppShader);

	if (_pShaderChS == NULL || _pShaderSt == NULL) {
		int l = getFuncLenght(pFunction);
		if (_pShaderSt == NULL && checkPattern(pFunction, l, patternStable, 13)) {
			LOG(INFO) << "Stable injection point found.";
			_pShaderSt = *ppShader;
			return hr;
		} else if (_pShaderChS == NULL && checkPattern(pFunction, l, patternCharSelec, 7)) {
			LOG(INFO) << "Char. screen VS found.";
			_pShaderChS = *ppShader;
			return hr;
		}
	}
	return hr;
}

HRESULT hook_gw2::SetVertexHook(IDirect3DVertexShader9 *pShader, IDirect3DDevice9* _orig, Direct3DSwapChain9* _implicit_swapchain) {
	if (pShader == NULL) return _orig->SetVertexShader(pShader);

	if (!fx_applied && isInjectionShaderSt(pShader)) {
		fx_applied = true;
		_implicit_swapchain->_runtime->applyPostFX(onCharSelecLastFrame, currentSurface);
	} else if (isInjectionShaderChS(pShader)) {
		onCharSelec = true;
	}

	return _orig->SetVertexShader(pShader);
}

HRESULT hook_gw2::CreatePixelHook(const DWORD *pFunction, IDirect3DPixelShader9 **ppShader, IDirect3DDevice9* _orig, Direct3DSwapChain9* _implicit_swapchain) {
	int l = getFuncLenght(pFunction);
	HRESULT hr = _orig->CreatePixelShader(pFunction, ppShader);
	if (_pShaderUs == NULL && checkPattern(pFunction, l, patternUnstable, 7)) {
		LOG(INFO) << "Unstable injection point found.";
		_pShaderUs = *ppShader;
		return hr;
	} else if (_pShaderLit == NULL && checkPattern(pFunction, l, patternLight, 7)) {
		LOG(INFO) << "Light injection point found.";
		_pShaderLit = *ppShader;
		return hr;
	} else if (_pShaderPLit == NULL && checkPattern(pFunction, l, patternPostLight, 7)) {
		LOG(INFO) << "PostLight injection point found.";
		_pShaderPLit = *ppShader;
		return hr;
	}
	if ((_implicit_swapchain->_runtime->_fog_amount == 1)) return hr;

	int _pattern = get_pattern(pFunction, l);
	switch (_pattern) {
		case 1: //Detected pattern 1
			replacePatternFog1(pFunction, l, _implicit_swapchain->_runtime->_fog_amount);
			return _orig->CreatePixelShader(_pFunction, ppShader);
			break;
		case 2: //Detected pattern 2
			replacePatternFog2(pFunction, l, _implicit_swapchain->_runtime->_fog_amount);
			return _orig->CreatePixelShader(_pFunction, ppShader);
			break;
		case 3://Detected bloom
			if(_implicit_swapchain->_runtime->_no_bloom == 0)
				return _orig->CreatePixelShader(pFunction, ppShader);
			replacePatternBloom(pFunction, l);
			return _orig->CreatePixelShader(_pFunction, ppShader);
		case 4://Detected bloom
			if (_implicit_swapchain->_runtime->_max_sun == 0)
				return _orig->CreatePixelShader(pFunction, ppShader);
			replacePatternSun(pFunction, l);
			return _orig->CreatePixelShader(_pFunction, ppShader);
		default: //No pattern detected
			return hr;
			break;
	}
}

HRESULT hook_gw2::SetPixelHook(IDirect3DPixelShader9 *pShader, IDirect3DDevice9* _orig, Direct3DSwapChain9* _implicit_swapchain) {
	if(pShader == NULL) return _orig->SetPixelShader(pShader);
	if (isInjectionShaderLit(pShader)) {
		lightSurface = currentSurface;
	} else if (lightSurface != NULL && isInjectionShaderPLit(pShader)) {
		LPDIRECT3DSURFACE9 l_Surface;
		_implicit_swapchain->_runtime->_lightbuffer_texture->GetSurfaceLevel(0, &l_Surface);
		_orig->StretchRect(lightSurface, NULL, l_Surface, NULL, D3DTEXF_NONE);
		l_Surface->Release();
		lightSurface = NULL;
	} else if (isInjectionShaderUs(pShader)) {
		unstable_in_cframe = true;
		if (!fx_applied) {
			fx_applied = true;
			_implicit_swapchain->_runtime->applyPostFX(onCharSelecLastFrame, currentSurface);
		}
	}
	return _orig->SetPixelShader(pShader);
}

int hook_gw2::get_pattern(const DWORD *pFunction, int l) {
	//If mad oC0 -> mov oC0 -> end
	if (pFunction[l - 4] == 0x2000001 && pFunction[l - 3] == 0x80280800 && pFunction[l - 9] == 0x4000004 && pFunction[l - 8] == 0x80270800) {
		//If last arg of move is one of the following
		if (pFunction[l - 2] == 0xa0000001 || pFunction[l - 2] == 0xa0000000 || pFunction[l - 2] == 0xa0000002 || pFunction[l - 2] == 0xa0ff0000 || pFunction[l - 2] == 0xa0ff0001) {
			//If op above are lerp and add, it's map ps so ignore
			if (pFunction[l - 13] == 0x3000002 && pFunction[l - 18] == 0x4000012) {
				return -1;
			}
			//Else it need to be edited with pattern 1
			return 1;
		}
		//If it's only mad oC0 -> end
	} else if (pFunction[l - 6] == 0x4000004 && pFunction[l - 5] == 0x80270800) {
		//If it's mad -> mad, it's UI ps
		if (pFunction[l - 11] == 0x4000004) return -1;
		//Else it need to be edited with pattern 2
		return 2;
	} else if (checkPattern(pFunction, l, patternBloom, 7)) {
		return 3;
	} else if (checkPattern(pFunction, l, patternSun, 7)) {
		return 4;
	}
	return -1;
}

void hook_gw2::replacePatternFog1(const DWORD * pFunction, int l, float _fog_amount) {
	DWFL hexFloat;
	hexFloat.f = _fog_amount;
	DWORD constant[] = { 0x5000051, 0xa00f00df, hexFloat.d, hexFloat.d, hexFloat.d, hexFloat.d };

	_pFunction[0] = pFunction[0];
	for (int i = 1; i < 7; ++i) _pFunction[i] = constant[i - 1];
	for (int i = 1; i < l; ++i) {
		//0x5000051
		_pFunction[i + 6] = pFunction[i];
	}
	l += 6;
	//Trying to find a better way to reduce fog.
	_pFunction[l + 4] = _pFunction[l - 1]; //END>>5
	_pFunction[l + 3] = _pFunction[l - 2];//c1>>4
	_pFunction[l + 2] = _pFunction[l - 3];//oC0.w>>4
	_pFunction[l + 1] = _pFunction[l - 4];//mov>>4

	_pFunction[l - 4] = 0x4000012; //lrp
	_pFunction[l - 3] = _pFunction[l - 8];//oC0.xyz
	_pFunction[l - 2] = 0xa00000df;//c223
	_pFunction[l - 1] = 0x80e40001;//r1
	_pFunction[l] = _pFunction[l - 7];//r0

	_pFunction[l - 8] = 0x800f0001;//oC0.xyz > r1
}

void hook_gw2::replacePatternFog2(const DWORD * pFunction, int l, float _fog_amount) {
	DWFL hexFloat;
	hexFloat.f = _fog_amount;
	DWORD constant[] = { 0x5000051, 0xa00f00df, hexFloat.d, hexFloat.d, hexFloat.d, hexFloat.d };

	for (int i = 1; i < 7; ++i) _pFunction[i] = constant[i - 1];
	for (int i = 1; i < l; ++i) {
		//0x5000051
		_pFunction[i + 6] = pFunction[i];
	}
	l += 6;

	_pFunction[l + 4] = _pFunction[l - 1]; //END>>5

	_pFunction[l - 1] = 0x4000012; //lrp
	_pFunction[l] = _pFunction[l - 5];//oC0.xyz
	_pFunction[l + 1] = 0xa00000df;//c223
	_pFunction[l + 2] = 0x80e40001;//r1
	_pFunction[l + 3] = _pFunction[l - 4];//r0

	_pFunction[l - 5] = 0x800f0001;//oC0.xyz > r1
}

void hook_gw2::replacePatternBloom(const DWORD * pFunction, int l) {
	LOG(INFO) << "Bloom shader edited.";
	for (int i = 0; i < l; ++i) _pFunction[i] = pFunction[i];
	//After defs and dcls, put the last op (mov oC0 r0) and End
	_pFunction[38] = pFunction[l - 4];
	_pFunction[39] = pFunction[l - 3];
	_pFunction[40] = pFunction[l - 2];
	_pFunction[41] = 0xffff;
}

void hook_gw2::replacePatternSun(const DWORD * pFunction, int l) {
	LOG(INFO) << "Sun shader edited.";
	DWFL hexFloat;
	hexFloat.f = 100.0f;
	for (int i = 0; i < l; ++i) _pFunction[i] = pFunction[i];
	_pFunction[3] = hexFloat.d;
}

bool hook_gw2::checkPattern(const DWORD *pFunction, int l, DWORD *pattern, int pl) {
	for (int i = 0; i < pl; ++i) {
		if (pFunction[l - 2 - i] != pattern[pl - 1 - i]) {
			return false;
		}
	}
	return true;
}

int hook_gw2::getFuncLenght(const DWORD *pFunction) {
	int op = 0, l = 1;
	bool test = true;
	while (!isEnd(pFunction[op++]))  l++;
	return l;
}

bool hook_gw2::isEnd(DWORD token) {
	return (token & D3DSI_OPCODE_MASK) == D3DSIO_END;
}

bool hook_gw2::hook_gw2::isInjectionShaderChS(void* pShader) {
	return pShader == _pShaderChS;
}

bool hook_gw2::hook_gw2::isInjectionShaderSt(void* pShader) {
	return pShader == _pShaderSt;
}

bool hook_gw2::hook_gw2::isInjectionShaderUs(void* pShader) {
	return pShader == _pShaderUs;
}

bool hook_gw2::hook_gw2::isInjectionShaderLit(void* pShader) {
	return pShader == _pShaderLit;
}

bool hook_gw2::hook_gw2::isInjectionShaderPLit(void* pShader) {
	return pShader == _pShaderPLit;
}

void hook_gw2::logShader(const DWORD * pFunction) {
	LOG(INFO) << "### Shader created : ###";
	int opl = 0;
	int l = 0;
	while (!isEnd(pFunction[l])) {
		/*opl = pFunction[l] & 0x0F000000 >> 24;
		printf("l:%d\n", opl);
		std::stringstream stream;
		for (int i = 0; i <= opl+1; ++i) {
			stream << std::hex << (DWORD)pFunction[l+i] << ", ";
		}*/
		LOG(INFO) << std::hex << (DWORD)pFunction[l];//stream.str();
		l += 1;//2 + opl;
	}
	LPD3DXBUFFER disassembly;
	D3DXDisassembleShader(pFunction, true, "", &disassembly);
	LOG(INFO) << static_cast<char*>(disassembly->GetBufferPointer());
}
