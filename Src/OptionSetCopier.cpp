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
	API_PropertyGroup group;
	GS::HashTable<Int64, API_PropertyDefinition> defsTable;
} GroupWithDefs;

typedef struct {
	GS::HashTable<Int64, GroupWithDefs> groupsTable;
	Int64 iSourceGroup;
	Int64 iTargetGroup;
	Int64 iSource;
	Int64 iTarget;
	Int32 iAppend;
} CntlDlgData;

// ---------------------------------- Variables --------------------------------

static CntlDlgData			cntlDlgData;
#define OK_BUTTON			1
#define SOURCE_GROUP_POPUP	2
#define SOURCE_POPUP		3
#define SOURCE_VIEW			4
#define COPY_BUTTON			5
#define TARGET_GROUP_POPUP	6
#define TARGET_POPUP		7
#define TARGET_VIEW			8
#define CHECKBOX			9

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

static void		GetStringFromResource(GS::UniString* buffer, short resID, short stringID)
{
	if (buffer != nullptr && !RSGetIndString(buffer, resID, stringID, ACAPI_GetOwnResModule()))
		buffer->Clear();

	return;
}		// GetStringFromResource


static GS::UniString SetOptionsView(Int64 i_idxGroup, Int64 i_idx)
{
	if (i_idxGroup == 0
		|| i_idx == 0)
		return GS::UniString{};
	auto _i = cntlDlgData
		.groupsTable.Get(i_idxGroup);
	auto _j = _i.defsTable.Get(i_idx);
	GS::Array<API_SingleEnumerationVariant> _sourceVariants = _j.possibleEnumValues;
	GS::UniString _text{};

	for (auto _v : _sourceVariants)
	{
		_text += _v.displayVariant.uniStringValue;
		_text += "\n";
	}

	return _text;
}


static GS::Array<GS::UniString> SetPropSelector(Int64 i_iIdx)
{
	GS::Array<GS::UniString> result{};

	auto group = cntlDlgData.groupsTable[i_iIdx];

	for (auto _def : group.defsTable.Values())
	{
		result.Push(_def.name);
	}

	return result;
}


static void Do_CopyPropertyOptions()
{
	GSErrCode err;
	GS::UniString _text{};

	auto sourceDef = cntlDlgData.groupsTable.Get(cntlDlgData.iSourceGroup).defsTable.Get(cntlDlgData.iSource);
	auto targetDef = cntlDlgData.groupsTable.Get(cntlDlgData.iTargetGroup).defsTable.Get(cntlDlgData.iTarget);
	
	if (cntlDlgData.iAppend)
		targetDef.possibleEnumValues.Append(sourceDef.possibleEnumValues);
	else
		targetDef.possibleEnumValues = sourceDef.possibleEnumValues;

	err = ACAPI_Property_ChangePropertyDefinition(targetDef);

	cntlDlgData.groupsTable.Get(cntlDlgData.iTargetGroup).defsTable.Get(cntlDlgData.iTarget) = targetDef;
	
	_text = SetOptionsView(cntlDlgData.iTargetGroup, cntlDlgData.iTarget);

	DGSetItemText(32400, TARGET_VIEW, _text);
}


static void RefreshUI(short i_dialID, short i_item = 0) {
	if (i_item == SOURCE_GROUP_POPUP)
	{
		GS::Array<GS::UniString> groupOps;

		DGPopUpDeleteItem(i_dialID, SOURCE_POPUP, DG_ALL_ITEMS);
		DGPopUpInsertItem(i_dialID, SOURCE_POPUP, DG_LIST_BOTTOM);
		if (cntlDlgData.iSourceGroup > 0)
			groupOps = SetPropSelector(cntlDlgData.iSourceGroup);
		for (auto _name : groupOps)
		{
			DGPopUpInsertItem(i_dialID, SOURCE_POPUP, DG_LIST_BOTTOM);
			DGPopUpSetItemText(i_dialID, SOURCE_POPUP, DG_LIST_BOTTOM, _name);
		}
	}

	if (i_item == TARGET_GROUP_POPUP)
	{
		GS::Array<GS::UniString> groupOps;

		DGPopUpDeleteItem(i_dialID, TARGET_POPUP, DG_ALL_ITEMS);
		DGPopUpInsertItem(i_dialID, TARGET_POPUP, DG_LIST_BOTTOM);
		if (cntlDlgData.iTargetGroup > 0)
			groupOps = SetPropSelector(cntlDlgData.iTargetGroup);
		for (auto _name : groupOps)
		{
			DGPopUpInsertItem(i_dialID, TARGET_POPUP, DG_LIST_BOTTOM);
			DGPopUpSetItemText(i_dialID, TARGET_POPUP, DG_LIST_BOTTOM, _name);
		}
	}

	GS::UniString _text;

	_text = SetOptionsView(cntlDlgData.iSourceGroup, cntlDlgData.iSource);
	DGSetItemText(i_dialID, SOURCE_VIEW, _text);

	_text = SetOptionsView(cntlDlgData.iTargetGroup, cntlDlgData.iTarget);
	DGSetItemText(i_dialID, TARGET_VIEW, _text);

	if (cntlDlgData.iSource == 0\
		|| cntlDlgData.iTarget == 0\
		|| cntlDlgData.iSource == cntlDlgData.iTarget && cntlDlgData.iSourceGroup== cntlDlgData.iTargetGroup)
		DGDisableItem(i_dialID, COPY_BUTTON);
	else
		DGEnableItem(i_dialID, COPY_BUTTON);

	DGSetItemValLong(i_dialID, CHECKBOX, cntlDlgData.iAppend);
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

		DGPopUpInsertItem(dialID, SOURCE_GROUP_POPUP, DG_LIST_BOTTOM);
		DGPopUpInsertItem(dialID, TARGET_GROUP_POPUP, DG_LIST_BOTTOM);
		short _i = 1;

		err = ACAPI_Property_GetPropertyGroups(groups);

		for (API_PropertyGroup _group : groups)
		{
			if (_group.groupType == API_PropertyCustomGroupType)
			{
				GroupWithDefs gwd{};
				gwd.group = _group;

				GS::Array<API_PropertyDefinition> _defs;
				
				err = ACAPI_Property_GetPropertyDefinitions(_group.guid, _defs);

				short _j = 1;
				bool hasGroupEnumValues = false;

				for (auto _def : _defs)
				{
					if (_def.possibleEnumValues.GetSize())
					{
						gwd.defsTable.Add(_j++, _def);
						hasGroupEnumValues = true;
					}
				}

				if (hasGroupEnumValues)
				{
					DGPopUpInsertItem(dialID, SOURCE_GROUP_POPUP, DG_LIST_BOTTOM);
					DGPopUpSetItemText(dialID, SOURCE_GROUP_POPUP, DG_LIST_BOTTOM, _group.name);
					DGPopUpInsertItem(dialID, TARGET_GROUP_POPUP, DG_LIST_BOTTOM);
					DGPopUpSetItemText(dialID, TARGET_GROUP_POPUP, DG_LIST_BOTTOM, _group.name);
						
					cntlDlgData.groupsTable.Add(_i++, gwd);
				}
			}
		}
		RefreshUI(dialID);

		break;
	}
	case DG_MSG_CLICK:
		switch (item) {
		case OK_BUTTON:
		//case DG_CANCEL:
			result = item;
			break;
		case COPY_BUTTON:
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
		case SOURCE_GROUP_POPUP:
			cntlDlgData.iSourceGroup = (GS::Int64)DGPopUpGetSelected(dialID, SOURCE_GROUP_POPUP) - 1;
			//cntlDlgData.iSource = 0;

			RefreshUI(dialID, item);

			break;
		case SOURCE_POPUP:
			cntlDlgData.iSource = (GS::Int64)DGPopUpGetSelected(dialID, SOURCE_POPUP) - 1;

			RefreshUI(dialID, item);

			break;
		case TARGET_GROUP_POPUP:
			cntlDlgData.iTargetGroup = (GS::Int64)DGPopUpGetSelected(dialID, TARGET_GROUP_POPUP) - 1;
			//cntlDlgData.iTarget = 0;

			RefreshUI(dialID, item);

			break;
		case TARGET_POPUP:
			cntlDlgData.iTarget = (GS::Int64)DGPopUpGetSelected(dialID, TARGET_POPUP) - 1;

			RefreshUI(dialID, item);

			break;
		case CHECKBOX:
			cntlDlgData.iAppend = DGGetItemValLong(dialID, CHECKBOX);
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
	//err = DGModelessInit(ACAPI_GetOwnResModule(), 32401, ACAPI_GetOwnResModule(), CntlDlgCallBack, (DGUserData)&cntlDlgData, 1);
	
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

