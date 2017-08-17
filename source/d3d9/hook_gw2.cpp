#include "d3d9_device.hpp"
#include "d3d9_swapchain.hpp"
#include "hash.hpp"
#include "hook_gw2.hpp"
#include "gw2_sun_moon.hpp"
#include <windows.h>

DWORD Stable = 0x4bfaaee9;
DWORD Char = 0x47497604;
DWORD CharHoT = 0x5a00d081;
DWORD Bloom = 0x59b97796;
DWORD Sun = 0x3c3a406a;
DWORD PostLight = 0x99006846;
DWORD Light = 0xbfcddd47;
DWORD Unstable = 0x7db279cf;

void hook_gw2::PresentHook() {
	if (lm == NULL) {
		initMumble();
		moonCoord3D = D3DXVECTOR3(80.0f, 60.0f, -0.2f);
		sunCoord3D = D3DXVECTOR3(-90.0f, 43.5f, -2.1f);
		Position = D3DXVECTOR3(0, 0, 0);
		Up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);

		_device->_orig->GetViewport(&d3dvp);

		D3DXMatrixPerspectiveFovLH(&Projection, 1.22f, (float)d3dvp.Width / (float)d3dvp.Height, 1.0f, 10000.0f);

		D3DXMatrixIdentity(&World);
	} else {
		system("cls");
		Target = D3DXVECTOR3(lm->fCameraFront[0], lm->fCameraFront[1], lm->fCameraFront[2]);
		printf("Map : %d, At : %f, %f, %f\n", lm->context.mapId, lm->fCameraFront[0] * 100, lm->fCameraFront[1] * 100, lm->fCameraFront[2] * 100);
		D3DXMatrixLookAtLH(&View, &Position, &Target, &Up);
		WVP = World * View * Projection;
		D3DXVec3TransformCoord(&sunCoord, gw2_sun_moon::sun(lm->context.mapId), &WVP);
		D3DXVec3TransformCoord(&moonCoord, gw2_sun_moon::moon(lm->context.mapId), &WVP);

		_device->_implicit_swapchain->_runtime->sunCoord[0] = (sunCoord.x + 1.0f) / 2.0f;
		_device->_implicit_swapchain->_runtime->sunCoord[1] = (-sunCoord.y + 1.0f) / 2.0f;

		_device->_implicit_swapchain->_runtime->moonCoord[0] = (moonCoord.x + 1.0f) / 2.0f;
		_device->_implicit_swapchain->_runtime->moonCoord[1] = (-moonCoord.y + 1.0f) / 2.0f;

		_device->_implicit_swapchain->_runtime->facingsun = !(sunCoord.z>1);
		_device->_implicit_swapchain->_runtime->facingmoon = !(moonCoord.z>1);
		_device->_implicit_swapchain->_runtime->onscharselec = lm->fCameraPosition[0] ==  0 && lm->fCameraPosition[1] == 0 && lm->fCameraPosition[2] == 0;
		_device->_implicit_swapchain->_runtime->mapid = lm->context.mapId;
	}

	if (_is_on_char_screen) {
		LPDIRECT3DSURFACE9 l_Surface;
		_device->_implicit_swapchain->_runtime->_lightbuffer_texture->GetSurfaceLevel(0, &l_Surface);
		_device->_orig->ColorFill(l_Surface, nullptr, D3DCOLOR_COLORVALUE(0.2f, 0.2f, 0.2f, 0.2f));
		l_Surface->Release();

		_is_on_char_screen_last_frame = true;
	} else {
		_is_on_char_screen_last_frame = false;
	}
	_device->_implicit_swapchain->_runtime->onscharselec = _is_on_char_screen_last_frame;
	_is_on_char_screen = false;

	if (!_is_fx_done) _device->_implicit_swapchain->_runtime->applyPostFX(_is_on_char_screen_last_frame, _surface_current);
	_is_fx_done = false;
}

void hook_gw2::ResetHook() {
	D3DXMatrixPerspectiveFovLH(&Projection, 1.22f, (float)d3dvp.Width / (float)d3dvp.Height, 1.0f, 10000.0f);
	return;
}

HRESULT hook_gw2::SetRenderTargetHook(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) {
	HRESULT hr = _device->_orig->SetRenderTarget(RenderTargetIndex, pRenderTarget);
	_surface_current = pRenderTarget;
	return hr;
}

HRESULT hook_gw2::CreateVertexHook(const DWORD *pFunction, IDirect3DVertexShader9 **ppShader) {
	HRESULT hr = _device->_orig->CreateVertexShader(pFunction, ppShader);
	int l = getFuncLenght(pFunction);
	if (_pShaderInjection_stable == NULL && getHash(pFunction, l, _bFunction, MAX_TOKENS) == Stable) {
		LOG(INFO) << "Stable injection point found.";
		_pShaderInjection_stable = *ppShader;
		return hr;
	} else if (_pShaderCharScreen == NULL && getHash(pFunction, l, _bFunction, MAX_TOKENS) == CharHoT) {
		LOG(INFO) << "Char. screen VS found.";
		_pShaderCharScreen = *ppShader;
		return hr;
	}
	return hr;
}

HRESULT hook_gw2::SetVertexHook(IDirect3DVertexShader9 *pShader) {
	if (pShader == NULL) return _device->_orig->SetVertexShader(pShader);
	if (!_is_fx_done && isInjectionShaderSt(pShader)) {
		_is_fx_done = true;
		_device->_implicit_swapchain->_runtime->applyPostFX(_is_on_char_screen_last_frame, _surface_current);
	} else if (isInjectionShaderChS(pShader)) {
		_is_on_char_screen = true;
	}

	return _device->_orig->SetVertexShader(pShader);
}
HRESULT hook_gw2::SetVertexFHook(UINT StartRegister, const float *pConstantData, UINT Vector4fCount) {
	return  _device->_orig->SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}


HRESULT hook_gw2::CreatePixelHook(const DWORD *pFunction, IDirect3DPixelShader9 **ppShader) {
	int l = getFuncLenght(pFunction);
	HRESULT hr = _device->_orig->CreatePixelShader(pFunction, ppShader);
	if (_pShaderInjection == NULL && getHash(pFunction, l, _bFunction, MAX_TOKENS) == Unstable) {
		LOG(INFO) << "Unstable injection point found.";
		_pShaderInjection = *ppShader;
		return hr;
	} else if (_pShaderLightMap == NULL && getHash(pFunction, l, _bFunction, MAX_TOKENS) == Light) {
		LOG(INFO) << "Light injection point found.";
		_pShaderLightMap = *ppShader;
		return hr;
	} else if (_pShaderPostLightMap == NULL && getHash(pFunction, l, _bFunction, MAX_TOKENS) == PostLight) {
		LOG(INFO) << "PostLight injection point found.";
		_pShaderPostLightMap = *ppShader;
		return hr;
	}
	if ((_device->_implicit_swapchain->_runtime->_fog_amount == 1)) return hr;

	int _pattern = get_pattern(pFunction, l);
	switch (_pattern) {
	case 1: //Detected pattern 1
		replacePatternFog1(pFunction, l, _device->_implicit_swapchain->_runtime->_fog_amount);
		return _device->_orig->CreatePixelShader(_pFunction, ppShader);
		break;
	case 2: //Detected pattern 2
		replacePatternFog2(pFunction, l, _device->_implicit_swapchain->_runtime->_fog_amount);
		return _device->_orig->CreatePixelShader(_pFunction, ppShader);
		break;
	case 3://Detected bloom
		if (_device->_implicit_swapchain->_runtime->_no_bloom == 0)
			return _device->_orig->CreatePixelShader(pFunction, ppShader);
		replacePatternBloom(pFunction, l);
		return _device->_orig->CreatePixelShader(_pFunction, ppShader);
	case 4://Detected sun
		if (_device->_implicit_swapchain->_runtime->_max_sun == 0)
			return _device->_orig->CreatePixelShader(pFunction, ppShader);
		replacePatternSun(pFunction, l);
		return _device->_orig->CreatePixelShader(_pFunction, ppShader);
	default: //No pattern detected
		return hr;
		break;
	}
}

HRESULT hook_gw2::SetPixelHook(IDirect3DPixelShader9 *pShader) {
	if (pShader == NULL) return _device->_orig->SetPixelShader(pShader);
	if (isInjectionShaderLit(pShader)) {
		_surface_lightmap = _surface_current;
	} else if (_surface_lightmap != NULL && isInjectionShaderPLit(pShader)) {
		LPDIRECT3DSURFACE9 l_Surface;
		_device->_implicit_swapchain->_runtime->_lightbuffer_texture->GetSurfaceLevel(0, &l_Surface);
		_device->_orig->StretchRect(_surface_lightmap, NULL, l_Surface, NULL, D3DTEXF_NONE);
		l_Surface->Release();
		_surface_lightmap = NULL;
	} else if (isInjectionShaderUs(pShader)) {
		if (!_is_fx_done) {
			_is_fx_done = true;
			_device->_implicit_swapchain->_runtime->applyPostFX(_is_on_char_screen_last_frame, _surface_current);
		}
	}
	return _device->_orig->SetPixelShader(pShader);
}

HRESULT hook_gw2::SetPixelFHook(UINT StartRegister, const float *pConstantData, UINT Vector4fCount) {
	return _device->_orig->SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
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
	} else if (getHash(pFunction, l, _bFunction, MAX_TOKENS) == Bloom) {
		return 3;
	} else if (getHash(pFunction, l, _bFunction, MAX_TOKENS) == Sun) {
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

bool hook_gw2::isInjectionShaderChS(void* pShader) {
	return pShader == _pShaderCharScreen;
}

bool hook_gw2::isInjectionShaderSt(void* pShader) {
	return pShader == _pShaderInjection_stable;
}

bool hook_gw2::isInjectionShaderUs(void* pShader) {
	return pShader == _pShaderInjection;
}

bool hook_gw2::isInjectionShaderLit(void* pShader) {
	return pShader == _pShaderLightMap;
}

bool hook_gw2::isInjectionShaderPLit(void* pShader) {
	return pShader == _pShaderPostLightMap;
}

void hook_gw2::logShader(const DWORD * pFunction) {
	LOG(INFO) << "### Shader created : ###";
	int opl = 0;
	int l = 0;
	while (!isEnd(pFunction[l])) {
		LOG(INFO) << std::hex << (DWORD)pFunction[l];
		l += 1;
	}
	LPD3DXBUFFER disassembly;
	D3DXDisassembleShader(pFunction, true, "", &disassembly);
	LOG(INFO) << static_cast<char*>(disassembly->GetBufferPointer());
}

void hook_gw2::initMumble() {
	HANDLE hMapObject = OpenFileMappingW(PAGE_READONLY, FALSE, L"MumbleLink");
	if (hMapObject == NULL) {
		hMapObject = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, sizeof(LinkedMem), L"MumbleLink");
		if (hMapObject == NULL) {
			printf("Error\n");
		}
	}

	lm = (LinkedMem *)MapViewOfFile(hMapObject, PAGE_READONLY, 0, 0, sizeof(LinkedMem));
	if (lm == NULL) {
		CloseHandle(hMapObject);
		hMapObject = NULL;
		return;
	}
}
