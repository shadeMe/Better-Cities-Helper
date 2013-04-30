#pragma once

#include "obse/PluginAPI.h"
#include "obse/CommandTable.h"
#include "obse/GameAPI.h"
#include "obse/GameObjects.h"
#include "obse/GameData.h"

#include "[Libraries]\SME Sundries\SME_Prefix.h"
#include "[Libraries]\SME Sundries\MemoryHandler.h"
#include "[Libraries]\SME Sundries\INIManager.h"
#include "[Libraries]\SME Sundries\StringHelpers.h"

using namespace SME;
using namespace SME::MemoryHandler;

extern IDebugLog					gLog;
extern PluginHandle					g_pluginHandle;
extern OBSEMessagingInterface*		g_msgIntfc;
extern HINSTANCE					g_ModuleInstance;
extern bool							g_OpenCitiesMode;

class BCHINIManager : public INI::INIManager
{
public:
	void								Initialize(const char* INIPath, void* Parameter);

	static BCHINIManager				Instance;
};

void OBSEMessageHandler(OBSEMessagingInterface::Message* msg);

_DeclareMemHdlr(PersistentDoorFixerUpper, "this one is directly ripped off from scruggsy's code");

void HelpVorians(void);		// must...annoy...instead