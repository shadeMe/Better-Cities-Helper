#pragma once

#include "obse/PluginAPI.h"
#include "obse/CommandTable.h"
#include "obse/GameAPI.h"
#include "obse/GameObjects.h"
#include "obse/GameData.h"

#include "[Libraries]\SME Sundries\SME_Prefix.h"
#include "[Libraries]\SME Sundries\MemoryHandler.h"
#include "[Libraries]\SME Sundries\INIManager.h"

using namespace SME;
using namespace SME::MemoryHandler;

extern IDebugLog					gLog;
extern PluginHandle					g_pluginHandle;
extern OBSEMessagingInterface*		g_msgIntfc;

extern SME::INI::INIManager*		g_INIManager;

class BCHINIManager : public INI::INIManager
{
public:
	void								Initialize(const char* INIPath, void* Parameter);
};

void OBSEMessageHandler(OBSEMessagingInterface::Message* msg);

_DeclareMemHdlr(PersistentDoorFixerUpper, "this one is directly ripped off from scruggsy's code");

void HelpVorians(void);		// must...annoy...instead

/*
#include "obse/PluginAPI.h"
#include "obse/GameObjects.h"

#if OBLIVION
#include "obse/GameAPI.h"

#else
#include "obse_editor/EditorAPI.h"
#endif

#include "obse/GameForms.h"
#include "obse_common/SafeWrite.h"

	IDebugLog gLog("open_cities.log");
PluginHandle g_pluginHandle = kPluginHandle_Invalid;

// hooks TESObjectREFR::SetParentCell(cell) virtual fn
// when this is invoked for persistent references in exterior cells, the cell argument is compared to
// the ref's current cell, which is of course NULL. Since the current  cell does not match the new cell,
// the game marks the reference as having been moved - erroneously, but generally harmlessly.
// For Open Cities, we want to prevent this as otherwise usage of SetPos_T will cause the refr's
// changed position to be saved in the .ess file
static __declspec(naked) void PersistentDoorHook(void)
{
	// on entry: esi = TESObjectREFR* this
	//  edi = TESObjectCELL* cellParam
	static const UInt32 s_retnAddr = 0x004D8ABA;

	__asm {
		mov edx, [eax+0x40] // MarkAsModified()

		test edi, edi // new cell is null?
			jz JUMP

			mov eax, [edi+0x50] // cell->worldspace
		test eax, eax
			jz JUMP // not an exterior

			mov eax, [esi+0x1C] // base form
		test eax, eax // base is null?
			jz JUMP

			mov al, [eax+4] // type ID
		cmp al, kFormType_Door
			jnz JUMP // not a door, we don't care

			// it's a door, pop the arg and don't mark as modified
			jmp [s_retnAddr]

JUMP:
		// mark as modified (overwritten code)
		push 4
			call edx

			// bye now
			jmp [s_retnAddr]
	}
}

UInt32 s_doorIDs[] =
{
	// Tamriel
	0x0000092F,
	0x00001442,
	0x00003614,
	0x00005605,
	0x00006FB8,
	0x00006FBA,
	0x00006FBC,
	0x0000799C,
	0x00007A67,
	0x00007DE6,
	0x00007DEA,
	0x0000887E,
	0x0000887F,
	0x0000BC98,
	0x0000BC99,
	0x0000BCB9,
	0x0000C155,
	0x0001BC3D,
	0x0001C356,
	0x000288F5,
	0x000308DE,
	0x000308DF,
	0x00030BA1,
	0x00030BBE,
	0x00034713,
	0x00035A98,
	0x00035D3B,
	0x00035D3C,
	0x00035DBE,
	0x0004D80C,
	0x0005102D,
	0x00053F1F,
	0x0006A814,
	0x0008698B,
	0x0009493A,
	0x001771BB,

	// Shivering Isles
	0x00012F6F,
	0x00012F78
};

UInt32 s_numDoors = SIZEOF_ARRAY(s_doorIDs, UInt32);

// this is a bit hacky, haven't bothered to fully decode the ChangesMap structures yet
bool RemoveMovedFlag(UInt32 refID)
{
	TESObjectREFR* refr = OBLIVION_CAST(LookupFormByID(refID), TESForm, TESObjectREFR);
	if (!refr) {
		// door doesn't exist - will see this if Shivering Isles not installed
		_MESSAGE("Ref %08X does not exist", refID);
		return false;
	}

	UInt32** savegame = (UInt32**)*g_createdBaseObjList;
	if (savegame && *savegame) {
		UInt32* changeFlag;
		if (NiTPointerMap_Lookup(*savegame, (void*)refID, (void**)&changeFlag)) {
			// an entry exists for this object
			if ((*changeFlag) & TESObjectREFR::kChanged_Move) {
				// reset to starting position
				ExtraStartingPosition* start = (ExtraStartingPosition*)refr->baseExtraList.GetByType(kExtraData_StartingPosition);
				if (start && refr->posZ != start->z) {
					refr->posX = start->x;
					refr->posY = start->y;
					refr->posZ = start->z;
					refr->Update3D();
				}

				// reset the change flag
				(*changeFlag) &= ~TESObjectREFR::kChanged_Move;
				_MESSAGE("Reset MOVED flag on %08X.", refID);
				return true;
			}
			else {
				_MESSAGE("%08X is not flagged as MOVED - no need to reset.", refID);
			}
		}
		else {
			_MESSAGE("No change entry exists for %08X - no need to reset MOVED flag.", refID);
		}
	}
	else {
		_MESSAGE("Error: Null savegame pointer.");
	}

	return false;
}

void MessageHandlerProc(OBSEMessagingInterface::Message* msg)
{
	if (msg->type == OBSEMessagingInterface::kMessage_PostLoadGame) {
		_MESSAGE("Loading saved game");
		gLog.Indent();
		for (UInt32 i = 0; i < s_numDoors; i++) {
			RemoveMovedFlag(s_doorIDs[i]);
		}
		gLog.Outdent();
	}
}

extern "C" {
	bool OBSEPlugin_Query(const OBSEInterface * obse, PluginInfo * info)
	{
		_MESSAGE("query");

		// fill out the info structure
		info->infoVersion = PluginInfo::kInfoVersion;
		info->name = "open_cities";
		info->version = 1;

		// version checks
		if(!obse->isEditor)
		{
#if OBLIVION
			if(obse->oblivionVersion != OBLIVION_VERSION)
			{
				_ERROR("incorrect Oblivion version (got %08X need %08X)", obse->oblivionVersion, OBLIVION_VERSION);
				return false;
			}
#endif
		}
		else
		{
			// no version checks needed for editor
		}

		// version checks pass

		return true;
	}

	bool OBSEPlugin_Load(const OBSEInterface * obse)
	{
		_MESSAGE("load");

		g_pluginHandle = obse->GetPluginHandle();

		// register commands
		// obse->SetOpcodeBase(0x2000);

		// set up serialization callbacks when running in the runtime
		if(!obse->isEditor)
		{
			// install our hook
			WriteRelJump(0x004D8AB3, (UInt32)&PersistentDoorHook);

			// register for messages
			OBSEMessagingInterface* msg = (OBSEMessagingInterface*)obse->QueryInterface(kInterface_Messaging);
			if (msg) {
				msg->RegisterListener(g_pluginHandle, "OBSE", &MessageHandlerProc);
			}
			else {
				_MESSAGE("Couldn't register with messaging API.");
			}

			_MESSAGE("Patched.");
		}

		return true;
	}
};
*/