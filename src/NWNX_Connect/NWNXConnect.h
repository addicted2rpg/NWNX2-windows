/***************************************************************************
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ***************************************************************************/

#if !defined(NWNXCONNECT_H_)
#define NWNXCONNECT_H_
#pragma once
#define WIN32_LEAN_AND_MEAN	
#include <Windows.h>
#include "../NWNXdll/NWNXBase.h"
#include "nwn_internals.h"
#include "types.h"

#define OBJECT_INVALID 0x7F000000

class CNWNXConnect : public CNWNXBase  
{
public:
	// A proper constructor
	CNWNXConnect();
	~CNWNXConnect();

	BOOL OnCreate(const char* LogDir);
	char* OnRequest(char *gameObject, char* Request, char* Parameters);
	//unsigned long OnRequestObject (char *gameObject, char* Request);
	void WriteLogHeader();
	BOOL OnRelease();

	void SendHakList(CNWSMessage *pMessage, int nPlayerID);


	//nwns malloc routine
	//void * (__cdecl *nwnx_malloc)(size_t cb);
	//nwns free routine
	//void (__cdecl *nwnx_free)(void * cb);


protected:

private:


};

#endif