#include "BCHInternals.h"

IDebugLog					gLog("Open-Better Cities Helper.log");

PluginHandle				g_pluginHandle = kPluginHandle_Invalid;
OBSEMessagingInterface*		g_msgIntfc = NULL;
HINSTANCE					g_ModuleInstance = NULL;
bool						g_OpenCitiesMode = false;

BCHINIManager				BCHINIManager::Instance;

_DefineHookHdlr(PersistentDoorFixerUpper, 0x004D8AB3);

void BCHINIManager::Initialize( const char* INIPath, void* Parameter )
{
	this->INIFilePath = INIPath;
	_MESSAGE("INI Path: %s", INIPath);

	if (PopulateFromINI() == false)
		_MESSAGE("Couldn't populate INI manager from INI!");
}


void HelpVorians( void )
{
	_MemHdlr(PersistentDoorFixerUpper).WriteJump();
}

void ResetDoorRef(UInt32 FormID)
{
	// scruggsy's code
	TESObjectREFR* refr = OBLIVION_CAST(LookupFormByID(FormID), TESForm, TESObjectREFR);
	if (refr == NULL)
	{
		_MESSAGE("Reference %08X does not exist", FormID);
		return;
	}

	UInt32** savegame = (UInt32**)*g_createdBaseObjList;
	if (savegame && *savegame)
	{
		UInt32* changeFlag = NULL;
		if (NiTPointerMap_Lookup(*savegame, (void*)FormID, (void**)&changeFlag))
		{
			// an entry exists for this object
			if (changeFlag && ((*changeFlag) & TESObjectREFR::kChanged_Move))
			{
				// reset to starting position
				ExtraStartingPosition* start = (ExtraStartingPosition*)refr->baseExtraList.GetByType(kExtraData_StartingPosition);
				if (start && refr->posZ != start->z)
				{
					refr->posX = start->x;
					refr->posY = start->y;
					refr->posZ = start->z;
					refr->Update3D();
				}

				// reset the change flag
				(*changeFlag) &= ~TESObjectREFR::kChanged_Move;
				_MESSAGE("Reset MOVED flag on %08X", FormID);
			}
			else
				_MESSAGE("%08X is not flagged as MOVED - no need to reset.", FormID);
		}
		else
			_MESSAGE("No change entry exists for %08X - no need to reset MOVED flag.", FormID);
	}
	else
		_MESSAGE("Error: Null savegame pointer.");
}

void OBSEMessageHandler(OBSEMessagingInterface::Message* msg)
{
	switch (msg->type)
	{
	case OBSEMessagingInterface::kMessage_PostLoadGame:
		{
			_MESSAGE("PostLoadGame message received!");
			gLog.Indent();

			for (SME::INI::INIManagerIterator Itr(&BCHINIManager::Instance); Itr.GetDone() == 0; Itr.GetNextSetting())
			{
				const SME::INI::INISetting* Setting = Itr.GetCurrentSetting();
				const ModEntry* Plugin = (*g_dataHandler)->LookupModByName(Setting->GetSection());

				if (Plugin && Plugin->IsLoaded())
				{
					UInt32 FormID = 0;
					SME_ASSERT(Setting->GetType() == SME::INI::INISetting::kType_String);
					sscanf_s(Setting->GetData().s, "%08X", &FormID);

					_MESSAGE("Resetting reference %08X for plugin %s", FormID, Setting->GetSection());

					ResetDoorRef(FormID);
				}
			}

			gLog.Outdent();
			_MESSAGE("The Doors have been made relevant once again!");
		}

		break;
	}
}

#define _hhName	PersistentDoorFixerUpper
_hhBegin()
{
	_hhSetVar(Retn, 0x004D8ABA);
	__asm
	{
		mov		edx, [eax + 0x40]	// MarkAsModified()
		test	edi, edi			// new cell is null?
		jz		SKIP

		mov		eax, [edi + 0x50]	// cell->worldspace
		test	eax, eax
		jz		SKIP				// not an exterior

		mov		eax, [esi + 0x1C]	// base form
		test	eax, eax			// base is null?
		jz		SKIP

		mov		al, [eax + 0x4]		// type ID
		cmp		al, kFormType_Door
		jnz		SKIP				// not a door, we don't care

		// it's a door, pop the arg and don't mark as modified
		jmp		_hhGetVar(Retn)
	SKIP:
		push	4
		call	edx

		jmp		_hhGetVar(Retn)
	}
}