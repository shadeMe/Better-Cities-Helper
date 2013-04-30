#include "BCHInternals.h"
#include "VersionInfo.h"

extern "C"
{
	bool OBSEPlugin_Query(const OBSEInterface * obse, PluginInfo * info)
	{
		SME_ASSERT(g_ModuleInstance != NULL);

		char Buffer[MAX_PATH] = {0};
		GetModuleFileName((HMODULE)g_ModuleInstance, Buffer, sizeof(Buffer));
		std::string FileName(Buffer);
		
		SME::StringHelpers::MakeLower(FileName);
		FileName = FileName.substr(FileName.rfind("\\") + 1);
		FileName.erase(FileName.rfind("."), 4);

		if (FileName.find("open") == 0)
			g_OpenCitiesMode = true;

		if (g_OpenCitiesMode == false)
			_MESSAGE("Better Cities Helper Initializing...");
		else
			_MESSAGE("Open Cities Helper Initializing...");
		
		info->infoVersion =	PluginInfo::kInfoVersion;
		info->name =		"Better Cities Helper";
		info->version =		PACKED_SME_VERSION;

		g_pluginHandle = obse->GetPluginHandle();

		if (obse->isEditor)
			return false;
		else if (obse->oblivionVersion != OBLIVION_VERSION)
		{
			_MESSAGE("Unsupported runtime version %08X", obse->oblivionVersion);
			return false;
		}
		else if(obse->obseVersion < OBSE_VERSION_INTEGER)
		{
			_ERROR("OBSE version too old (got %08X expected at least %08X)", obse->obseVersion, OBSE_VERSION_INTEGER);
			return false;
		}

		g_msgIntfc = (OBSEMessagingInterface*)obse->QueryInterface(kInterface_Messaging);
		if (g_msgIntfc == NULL)
		{
			_MESSAGE("OBSE messaging interface couldn't be initialized");
			return false;
		}

		return true;
	}

	bool OBSEPlugin_Load(const OBSEInterface * obse)
	{
		_MESSAGE("Registering OBSE Message Handler");
		g_msgIntfc->RegisterListener(g_pluginHandle, "OBSE", OBSEMessageHandler);

		_MESSAGE("Initializing INI Manager");
		if (g_OpenCitiesMode == false)
			BCHINIManager::Instance.Initialize("Data\\OBSE\\Plugins\\Better Cities Helper.ini", NULL);
		else
			BCHINIManager::Instance.Initialize("Data\\OBSE\\Plugins\\open_cities.ini", NULL);

		_MESSAGE("Meh, Vorians. Meh...\n\n");
		gLog.Indent();

		HelpVorians();

		return true;
	}

	BOOL WINAPI DllMain(HANDLE hDllHandle, DWORD dwReason, LPVOID lpreserved)
	{
		switch (dwReason)
		{
		case DLL_PROCESS_ATTACH:
			g_ModuleInstance = (HINSTANCE)hDllHandle;
			break;
		}

		return TRUE;
	}
};

