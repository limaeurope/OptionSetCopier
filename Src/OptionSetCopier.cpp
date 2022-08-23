// *****************************************************************************
// *****************************************************************************

#define	_ELEMENT_TEST_TRANSL_


// ---------------------------------- Includes ---------------------------------

#include	"APIEnvir.h"
#include	"ACAPinc.h"					// also includes APIdefs.h
#include	"APICommon.h"

#include	"OptionSetCopier.h"
#include	"DG.h"


// ---------------------------------- Types ------------------------------------

typedef struct {
	GS::HashTable<Int64, API_PropertyDefinition> definitionsTable;
	Int64 iSource;
	Int64 iTarget;
	Int32 iAppend;
} CntlDlgData;

// ---------------------------------- Variables --------------------------------

//static API_Guid	gGuid = APINULLGuid;
//static API_Guid	gLastRenFiltGuid = APINULLGuid;
static CntlDlgData			cntlDlgData;

// ---------------------------------- Prototypes -------------------------------


//TODO remove
void CopyOptionSet(bool isBoundingBoxConsidered)
{
	GSErrCode				err;
	API_SelectionInfo		selectionInfo;
	GS::Array<API_Neig>		selNeigs;
	API_Element				elementThis, elementOther;
	GS::Array<API_Guid>		guid_Targets, guid_Operators;
	API_Neig				_neig;

	GS::Array<API_PropertyGroup> groups;

	err = ACAPI_Property_GetPropertyGroups(groups);

	GS::Array<API_PropertyDefinition> definitions;

	for (API_PropertyGroup _group : groups)
	{
		GS::Array<API_PropertyDefinition> _defs;

		err = ACAPI_Property_GetPropertyDefinitions(_group.guid, _defs);

		for (auto _def : _defs)
		{
			if (_def.possibleEnumValues.GetSize())
			{
				definitions.Push(_def);
			}
		}
	}

	GS::UniString _t{ "Teszt" };
	auto _g = definitions.Pop();
	GS::UniString _s{};
	for (auto _v : _g.possibleEnumValues)
	{
		_s.Append(_v.displayVariant.uniStringValue + '\n');
	}

	DGAlert(DG_INFORMATION, _t, _g.name, _s, GS::UniString("Ok"));
	auto _g2 = definitions.Pop();

	auto vals = _g.possibleEnumValues;

	_g2.possibleEnumValues.Append(vals);
	GS::UniString _s2{};

	for (auto _v : _g2.possibleEnumValues)
	{
		_s2.Append(_v.displayVariant.uniStringValue + '\n');
	}

	DGAlert(DG_INFORMATION, _t, _g2.name, _s2, GS::UniString("Ok"));

	err = ACAPI_Property_ChangePropertyDefinition(_g2);
}


// -----------------------------------------------------------------------------
// Load a localisable Unicode string from resource
// -----------------------------------------------------------------------------

//static void		GetStringFromResource(GS::UniString* buffer, short resID, short stringID)
//{
//	if (buffer != nullptr && !RSGetIndString(buffer, resID, stringID, ACAPI_GetOwnResModule()))
//		buffer->Clear();
//
//	return;
//}		// GetStringFromResource


static GS::UniString SetOptionsView(Int64 i_iIdx)
{
	if (i_iIdx == 0)
		return GS::UniString{};
	GS::Array<API_SingleEnumerationVariant> _sourceVariants = cntlDlgData.definitionsTable.Get(i_iIdx).possibleEnumValues;
	GS::UniString _text{};

	for (auto _v : _sourceVariants)
	{
		_text += _v.displayVariant.uniStringValue;
		_text += "\n";
	}

	return _text;
}


static void Do_CopyPropertyOptions()
{
	GSErrCode err;
	GS::UniString _text{};

	auto sourceDef = cntlDlgData.definitionsTable.Get(cntlDlgData.iSource);
	auto targetDef = cntlDlgData.definitionsTable.Get(cntlDlgData.iTarget);
	
	if (cntlDlgData.iAppend)
		targetDef.possibleEnumValues.Append(sourceDef.possibleEnumValues);
	else
		targetDef.possibleEnumValues = sourceDef.possibleEnumValues;

	err = ACAPI_Property_ChangePropertyDefinition(targetDef);

	cntlDlgData.definitionsTable[cntlDlgData.iTarget] = targetDef;
	
	_text = SetOptionsView(cntlDlgData.iTarget);

	DGSetItemText(32400, 7, _text);
}


static void RefreshUI(short i_dialID) {
	GS::UniString _text = SetOptionsView(cntlDlgData.iSource);

	DGSetItemText(i_dialID, 6, _text);

	_text = SetOptionsView(cntlDlgData.iTarget);

	DGSetItemText(i_dialID, 7, _text);

	if (cntlDlgData.iSource == 0\
		|| cntlDlgData.iTarget == 0\
		|| cntlDlgData.iSource == cntlDlgData.iTarget)
		DGDisableItem(i_dialID, 4);
	else
		DGEnableItem(i_dialID, 4);

	DGSetItemValLong(i_dialID, 8, cntlDlgData.iAppend);
}


static short DGCALLBACK CntlDlgCallBack(short message, short dialID, short item, DGUserData userData, DGMessageData msgData)
{
	short result = 0;
	GS::UniString _text{};
	API_PropertyDefinition _def;

	switch (message) {
	case DG_MSG_INIT:
	{
		GSErrCode						err;
		GS::Array<API_PropertyGroup>	groups;
		cntlDlgData.iAppend = 1;

		DGPopUpInsertItem(dialID, 3, DG_LIST_BOTTOM);
		DGPopUpInsertItem(dialID, 5, DG_LIST_BOTTOM);
		short _i = 1;

		err = ACAPI_Property_GetPropertyGroups(groups);

		for (API_PropertyGroup _group : groups)
		{
			if (_group.groupType == API_PropertyCustomGroupType)
			{
				GS::Array<API_PropertyDefinition> _defs;
				
				err = ACAPI_Property_GetPropertyDefinitions(_group.guid, _defs);

				for (auto _def : _defs)
				{
					if (_def.possibleEnumValues.GetSize())
					{
						cntlDlgData.definitionsTable.Add(_i++, _def);

						DGPopUpInsertItem(dialID, 3, DG_LIST_BOTTOM);
						DGPopUpSetItemText(dialID, 3, DG_LIST_BOTTOM, _def.name);
						DGPopUpInsertItem(dialID, 5, DG_LIST_BOTTOM);
						DGPopUpSetItemText(dialID, 5, DG_LIST_BOTTOM, _def.name);
					}
				}
			}
		}

		break;
	}
	case DG_MSG_CLICK:
		switch (item) {
		case DG_OK:
		case DG_CANCEL:
			result = item;
			break;
		case 4:
			Do_CopyPropertyOptions();
			RefreshUI(dialID);
			break;
		}

		break;
	case DG_MSG_CLOSE:
		result = item;
		if (item == DG_OK) {
		}
		break;
	case DG_MSG_CHANGE:
		switch (item) {
		case 3:
			cntlDlgData.iSource = DGPopUpGetSelected(dialID, 3) - 1;

			RefreshUI(dialID);

			break;
		case 5 :
			cntlDlgData.iTarget = DGPopUpGetSelected(dialID, 5) - 1;

			RefreshUI(dialID);

			break;
		case 8:
			cntlDlgData.iAppend = DGGetItemValLong(dialID, 8);
			break;
			}
		break;
	}

	return result;
}


static GSErrCode	Do_CopyOptionSets()
{
	GSErrCode		err = NoError;

	err = DGModalDialog(ACAPI_GetOwnResModule(), 32400, ACAPI_GetOwnResModule(), CntlDlgCallBack, (DGUserData)&cntlDlgData);

	return err;
}		// Do_CopyOptionSets


// -----------------------------------------------------------------------------
// Elements: Solid Operations Functions
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL ElementsSolidOperation (const API_MenuParams *menuParams)
{
	return ACAPI_CallUndoableCommand ("Element Test API Function",
		[&] () -> GSErrCode {

			switch (menuParams->menuItemRef.itemIndex) {
				case 1:		Do_CopyOptionSets();				break;
				default:										break;
			}

			return NoError;
		});
}		/* ElementsSolidOperation */


// -----------------------------------------------------------------------------
// Dependency definitions
// -----------------------------------------------------------------------------
API_AddonType __ACENV_CALL	CheckEnvironment (API_EnvirParams* envir)
{
	RSGetIndString (&envir->addOnInfo.name, 32000, 1, ACAPI_GetOwnResModule ());
	RSGetIndString (&envir->addOnInfo.description, 32000, 2, ACAPI_GetOwnResModule ());

	return APIAddon_Preload;
}		/* RegisterAddOn */


// -----------------------------------------------------------------------------
// Interface definitions
// -----------------------------------------------------------------------------
GSErrCode __ACENV_CALL	RegisterInterface (void)
{
	GSErrCode	err;

	//
	// Register menus
	//
	err = ACAPI_Register_Menu (32506, 0, MenuCode_UserDef, MenuFlag_Default);

	return err;
}		/* RegisterInterface */


// -----------------------------------------------------------------------------
// Called when the Add-On has been loaded into memory
// to perform an operation
// -----------------------------------------------------------------------------
GSErrCode __ACENV_CALL	Initialize (void)
{
	GSErrCode err = NoError;

	//
	// Install menu handler callbacks
	//

	err = ACAPI_Install_MenuHandler (32506, ElementsSolidOperation);

	return err;
}		/* Initialize */


// -----------------------------------------------------------------------------
// Called when the Add-On is going to be unloaded
// -----------------------------------------------------------------------------
GSErrCode __ACENV_CALL	FreeData (void)
{
	return NoError;
}		/* FreeData */

