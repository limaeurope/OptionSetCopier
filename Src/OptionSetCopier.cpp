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
	short			dialID;					/* dialog ID of the palette */
	bool			hiddenByUser;			/* palette was hidden from menu by the user */
	bool			hiddenByAC;				/* palette was hidden through the API callback */
	GS::HashTable<Int64, API_PropertyDefinition> definitionsTable;
	Int64 iSource;
	Int64 iTarget;
} CntlDlgData;

// ---------------------------------- Variables --------------------------------

static API_Guid	gGuid = APINULLGuid;
static API_Guid	gLastRenFiltGuid = APINULLGuid;
static CntlDlgData			cntlDlgData;

// ---------------------------------- Prototypes -------------------------------


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

static void		GetStringFromResource(GS::UniString* buffer, short resID, short stringID)
{
	if (buffer != nullptr && !RSGetIndString(buffer, resID, stringID, ACAPI_GetOwnResModule()))
		buffer->Clear();

	return;
}		// GetStringFromResource

static void Do_CopyPropertyOptions(Int64 iSource, Int64 iTarget)
{
	GSErrCode err;
	GS::UniString _text{};

	auto sourceDef = cntlDlgData.definitionsTable.Get(cntlDlgData.iSource);
	auto targetDef = cntlDlgData.definitionsTable.Get(cntlDlgData.iTarget);
	targetDef.possibleEnumValues.Append(sourceDef.possibleEnumValues);

	for (auto _v : targetDef.possibleEnumValues)
	{
		_text += _v.displayVariant.uniStringValue;
		_text += "\n";
	}

	DGSetItemText(32400, 7, _text);

	err = ACAPI_Property_ChangePropertyDefinition(targetDef);
}

static short DGCALLBACK CntlDlgCallBack(short message, short dialID, short item, DGUserData userData, DGMessageData msgData)
{
	short				result = 0;
	GS::UniString _text{};
	API_PropertyDefinition _def;

	switch (message) {
	case DG_MSG_INIT:
	{
		//DGSetItemText(dialID, 3, _teszt);
		//DGSetItemText(dialID, 6, _teszt);
		GSErrCode				err;
		GS::Array<API_PropertyGroup> groups;
		short _i = 0;
		err = ACAPI_Property_GetPropertyGroups(groups);

		for (API_PropertyGroup _group : groups)
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

		break;
	}
	case DG_MSG_CLICK:
		switch (item) {
		case DG_OK:
		case DG_CANCEL:
			result = item;
			break;
		case 4:
			Do_CopyPropertyOptions(cntlDlgData.iSource, cntlDlgData.iTarget);
			break;
		}
	case DG_MSG_CLOSE:
		result = item;
		if (item == DG_OK) {
		}
		break;
	case DG_MSG_CHANGE:
		switch (item) {
		case 3:
			cntlDlgData.iSource = msgData;

			_def = cntlDlgData.definitionsTable.Get(cntlDlgData.iSource);

			for (auto _v : _def.possibleEnumValues)
			{
				_text += _v.displayVariant.uniStringValue;
				_text += "\n";
			}

			DGSetItemText(dialID, 6, _text);

			break;
		case 5 :
			cntlDlgData.iTarget = msgData;

			_def = cntlDlgData.definitionsTable.Get(cntlDlgData.iTarget);

			for (auto _v : _def.possibleEnumValues)
			{
				_text += _v.displayVariant.uniStringValue;
				_text += "\n";
			}

			DGSetItemText(dialID, 7, _text);

			break;
			}
		break;
	}

	return result;
}

static GSErrCode	Do_PaletteInit()
{
	GSErrCode		err = NoError;

	err = DGModalDialog(ACAPI_GetOwnResModule(), 32400, ACAPI_GetOwnResModule(), CntlDlgCallBack, (DGUserData)&cntlDlgData);

	return err;
}		// Do_PaletteInit

static void		Do_PaletteClose(void)
{
	if (cntlDlgData.dialID != 0 && DGIsDialogOpen(cntlDlgData.dialID))
		DGModelessClose(cntlDlgData.dialID);

	cntlDlgData.dialID = 0;

	return;
}		// Do_PaletteClose


// -----------------------------------------------------------------------------
// Elements: Solid Operations Functions
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL ElementsSolidOperation (const API_MenuParams *menuParams)
{
	return ACAPI_CallUndoableCommand ("Element Test API Function",
		[&] () -> GSErrCode {

			switch (menuParams->menuItemRef.itemIndex) {
				case 1:		CopyOptionSet(false);				break;
				case 2:		Do_PaletteInit();					break;
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

