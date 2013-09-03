#ifndef _NX_NWN_STRUCT_CNWSMODULE_
#define _NX_NWN_STRUCT_CNWSMODULE_


struct CNWSModule_s { //: public CResHelper_s { //, public CGameObject {
	uint32_t	*pVTable;	// 0000
	uint32_t bIsRequestPending; // 0004 not quite sure
	void *pRes; // 0008
	char sResRef[16];	// 000C 

    uint32_t obj_type;  // 0x01C   
    nwn_objid_t	obj_id;	// 0x020
    uint32_t field_24;  // 0x024
    uint32_t field_28;  // 0x028
    uint32_t field_2C;  // 0x02C
    uint32_t field_30;  // 0x030
    nwn_objid_t *mod_areas;   // 0x034
    uint32_t mod_areas_len;   // 0x038
    uint32_t field_3C;        // 0x03C
    uint32_t field_40;    // 0x40
    CExoLinkedList mod_PlayerTURDList;  // 0x44, should be just 4 bytes 
    uint32_t field_48; // 0x48 - NWNX Cool puts the turdlist here.  It certaintly looks similar in mem...
	uint32_t field_4C; // pointer to somewhere
    uint32_t field_50;



	CExoString	m_sCustomTLK;  /* 0x54 & 0x58 */

	
    uint32_t field_5C;  // 0x5C; ptr to a weird string heap
    char *start_area_name;  // 0x60
	CExoString currentgame_modulename;  /* "CURRENTGAME:<module name>" */ /* 0x64 & 0x68 */
    uint32_t field_6C;  // 0x6C
    uint32_t field_70; // 0x70
    uint32_t field_74; // 0x74
    uint32_t field_78; // 0x78
	uint32_t field_7C; // 0x7C
	uint32_t field_80; // 0x80; some pointer to somewhere
	uint32_t field_84; // 0x84

	// if the mod has a haklist, the above variables may be an indicator if it does.
	// 12 bytes: pointer to array of CExoStrings, 4 byte count of elements, and 4 byte number of allocated entries.
	CExoArrayList_string HakList; /* 0x88, 0x8C, 0x90 */

	uint32_t field_94;
	uint32_t field_98;
	uint32_t field_9C;
	uint32_t field_A0;


    CNWSScriptVarTable mod_vartable;   // consumes 8 bytes, 0xA4 & 0xA8


	uint32_t field_AC; // No matter the mod I run, this generally seems to be '10h' for me :)
	uint32_t field_B0; // No matter the mod I run, this generally seems to be 0 for me :)

	CExoString	Mod_OnHeartbeat;
	CExoString	Mod_OnUsrDefined;
	CExoString	Mod_OnModLoad;
	CExoString	Mod_OnModStart;
	CExoString	Mod_OnClientEntr;
	CExoString	Mod_OnClientLeav;
	CExoString	Mod_OnActvtItem;
	CExoString	Mod_OnAcquirItem;
	CExoString	Mod_OnUnAqreItem;
	CExoString	Mod_OnPlrDeath;
	CExoString	Mod_OnPlrDying;
	CExoString	Mod_OnSpawnBtnDn;
	CExoString	Mod_OnPlrRest;
	CExoString	Mod_OnPlrLvlUp;
	CExoString	Mod_OnCutsnAbort;
	CExoString	Mod_OnPlrEqItm;
	CExoString	Mod_OnPlrUnEqItm;
	CExoString	Mod_OnPlrChat;

	// Not vetted beyond htis point.

    uint32_t	spacer1[2];	
    
    void	*mod_objlist;	
    uint32_t	mod_objlist_len;	
    uint32_t	mod_objlist_alloc;	
    
    uint32_t	spacer2_01;	
    uint32_t	spacer2_02;	
    uint32_t	spacer2_03;	
    uint32_t	spacer2_04;	
    CLookupTableObject	*lookuptable; 
    uint32_t	lookuptable_len;	
    uint32_t	lookuptable_alloc;	
    uint32_t	spacer2[26];	
    
    uint32_t	mod_date_year;	
    uint32_t	mod_date_month;	
    uint32_t	mod_date_day;	
    uint32_t	mod_time_hour;	
    
    uint32_t	field_D0;
    uint8_t	mod_timeofday;	
    uint8_t	mod_dawnhour;
    uint8_t	mod_duskhour;
    uint8_t	field_no_idea;

    uint32_t	mod_LL_date;
    uint32_t	mod_LL_time;
    
    uint32_t	getModuleTime() {
		return (mod_LL_date * 2880000LL) + mod_LL_time;
    }

	void	AddObjectToLimbo(nwn_objid_t oID);
	int AddObjectToLookupTable(CExoString Tag, nwn_objid_t oID);
	CNWSArea * GetArea(nwn_objid_t oID);
	CNWSPlayerTURD* GetPlayerTURDFromList(CNWSPlayer *Player);
	void SetScriptName(int iScript, CExoString ScriptName);
	int RemoveObjectFromLookupTable(CExoString Tag, nwn_objid_t oID);
	nwn_objid_t	FindObjectByTagOrdinal(CExoString *sTag, int nNth);
};

#endif