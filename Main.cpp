#include "BCHInternals.h"
#include "VersionInfo.h"

extern "C"
{
	bool OBSEPlugin_Query(const OBSEInterface * obse, PluginInfo * info)
	{
		_MESSAGE("Better Cities Helper Initializing...");

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
		g_INIManager->Initialize("Data\\OBSE\\Plugins\\Better Cities Helper.ini", NULL);

		_MESSAGE("Meh, Vorians. Meh...\n\n");
		gLog.Indent();

		HelpVorians();

		return true;
	}
};