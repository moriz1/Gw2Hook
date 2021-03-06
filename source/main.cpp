/**
 * Copyright (C) 2014 Patrick Mours. All rights reserved.
 * License: https://github.com/crosire/reshade#license
 */

#include "log.hpp"
#include "filesystem.hpp"
#include "input.hpp"
#include "runtime.hpp"
#include "hook_manager.hpp"
#include "version.h"
#include <Windows.h>

HMODULE g_module_handle = nullptr;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpvReserved)
{
	UNREFERENCED_PARAMETER(lpvReserved);

	using namespace reshade;

	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:
		{
			DisableThreadLibraryCalls(hModule);

			g_module_handle = hModule;
			runtime::s_reshade_dll_path = filesystem::get_module_path(hModule);
			runtime::s_target_executable_path = filesystem::get_module_path(nullptr);

			CreateDirectory( (runtime::s_target_executable_path.parent_path() +"\\addons").wstring().c_str(), NULL);
			CreateDirectory( (runtime::s_target_executable_path.parent_path() +"\\addons\\Gw2Hook").wstring().c_str(), NULL);
			CreateDirectory((runtime::s_target_executable_path.parent_path() + "\\addons\\Gw2Hook\\Shaders").wstring().c_str(), NULL);
			CreateDirectory((runtime::s_target_executable_path.parent_path() + "\\addons\\Gw2Hook\\Textures").wstring().c_str(), NULL);
			CreateDirectory((runtime::s_target_executable_path.parent_path() + "\\addons\\Gw2Hook\\Presets").wstring().c_str(), NULL);
			CreateDirectory((runtime::s_target_executable_path.parent_path() + "\\addons\\Gw2Hook\\Screenshots").wstring().c_str(), NULL);

			runtime::s_gw2hook_wrkdir_path = runtime::s_target_executable_path.parent_path() + "\\addons\\Gw2Hook\\";


			const filesystem::path system_path = filesystem::get_special_folder_path(filesystem::special_folder::system);

			log::open( filesystem::path(runtime::s_gw2hook_wrkdir_path + "Gw2Hook.log") );

#define VERSION_PLATFORM "64-bit"

			LOG(INFO) << "Initializing crosire's ReShade version '" VERSION_STRING_FILE "' (" << VERSION_PLATFORM << ") built on '" VERSION_DATE " " VERSION_TIME "' loaded from " << runtime::s_reshade_dll_path << " to " << runtime::s_target_executable_path << " ...";
			LOG(INFO) << "Initializing Grenbur's Gw2 hook version'" VERSION_STRING_FILE "' ";

			hooks::register_module(system_path / "d3d9.dll");
			hooks::register_module(system_path / "user32.dll");
			hooks::register_module(system_path / "ws2_32.dll");

			LOG(INFO) << "Initialized.";
			break;
		}
		case DLL_PROCESS_DETACH:
		{
			LOG(INFO) << "Exiting ...";

			input::uninstall();
			hooks::uninstall();

			LOG(INFO) << "Exited.";
			break;
		}
	}

	return TRUE;
}
