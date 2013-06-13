/***************************************************************************
    NWNXTime.h: interface for the CNWNXTime class.
    (c) 2005 by Earandel Senessa and Ingmar Stieger (Papillon)

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

#if !defined(AFX_NWNXTIME_H__F5FD08FD_D40F_4506_A7ED_986365F88ABB__INCLUDED_)
#define AFX_NWNXTIME_H__F5FD08FD_D40F_4506_A7ED_986365F88ABB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\NWNXDLL\NWNXBase.h"

#pragma warning(disable:4786)
#include <map>
#include <string>

class CNWNXTime : public CNWNXBase  
{
protected:
	typedef std::map<std::string, LARGE_INTEGER> TTimers;

private:
	TTimers  m_Timers;
	LARGE_INTEGER liFrequency;

public:
	CNWNXTime();
	virtual ~CNWNXTime();
	char* OnRequest(char *gameObject, char* Request, char* Parameters);
	BOOL OnCreate(const char* LogDir);
	BOOL OnRelease();
	void WriteLogHeader();

protected:
	void StartTimer(LPCSTR szName);
	LONGLONG StopTimer(LPCSTR szName);
	LONGLONG PeekTimer(LPCSTR szName);
};

#endif // !defined(AFX_NWNXTIME_H__F5FD08FD_D40F_4506_A7ED_986365F88ABB__INCLUDED_)
