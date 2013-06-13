/***************************************************************************
    NWNXTime.cpp: implementation of the CNWNXTime class.
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

#include "stdafx.h"
#include "NWNXTime.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNWNXTime::CNWNXTime()
{
	QueryPerformanceFrequency(&liFrequency);
}

CNWNXTime::~CNWNXTime()
{
	OnRelease();
}

char* CNWNXTime::OnRequest(char *gameObject, char* Request, char* Parameters)
{
	char* function = &Request[0];
	if (!function)
	{
		Log("* Function not specified.\n");
		return NULL;
	}

	char* timerName = strchr(function, '!');
	if (!timerName)
	{
		Log("* Timer name not specified.\n");
		return NULL;
	}
	*timerName = 0x0;
	timerName++;

	if (strcmp(function, "START") == 0)
	{
		Log("o Starting timer on %s\n", timerName);
		StartTimer(timerName);
	}
	else if (strcmp(function, "QUERY") == 0)
	{
		LONGLONG Result = PeekTimer(timerName);
		Log("o Elapsed on %s: %I64i µs / %.3f msec / %.3f sec\n", timerName, Result, (float) Result / 1000, (float) Result / 1000 / 1000);
		sprintf_s(Parameters, 256, "%I64i", Result);
	}
	else if (strcmp(function, "STOP") == 0)
	{
		LONGLONG Result = StopTimer(timerName);
		Log("o Stopping timer on %s, elapsed: %I64i µs / %.3f msec / %.3f sec\n", timerName, Result, (float) Result / 1000, (float) Result / 1000 / 1000);
		sprintf_s(Parameters, 256, "%I64i", Result);
	}

	return NULL;
}

void CNWNXTime::StartTimer(LPCSTR szName)
{
	LARGE_INTEGER liLast;
	QueryPerformanceCounter(&liLast);
	m_Timers[std::string(szName)] = liLast;
}

LONGLONG CNWNXTime::StopTimer(LPCSTR szName)
{
	LONGLONG result = PeekTimer(szName);
	TTimers::iterator it = m_Timers.find(std::string(szName));
	if (it != m_Timers.end())
		m_Timers.erase(it);
	return result;
}

LONGLONG CNWNXTime::PeekTimer(LPCSTR szName)
{
	LARGE_INTEGER liCurrent;
	TTimers::iterator it;

	QueryPerformanceCounter(&liCurrent);
	it = m_Timers.find(std::string(szName));
	if (it != m_Timers.end())
		return (((liCurrent.QuadPart - it->second.QuadPart) * 1000000) / liFrequency.QuadPart);
	else
		return 0;
}

BOOL CNWNXTime::OnCreate(const char* LogDir)
{
	// call the base class function
	char log[MAX_PATH];
	sprintf_s(log, sizeof(char) * MAX_PATH,  "%s\\nwnx_time.txt", LogDir);
	if (!CNWNXBase::OnCreate(log))
		return false;

	WriteLogHeader();
	return true;
}


BOOL CNWNXTime::OnRelease()
{
	Log("o Shutting down\n");
	m_Timers.clear();

	// call base class function
	return CNWNXBase::OnRelease();
}

void CNWNXTime::WriteLogHeader()
{
	fprintf(m_fFile, "NWNX Time plugin V.1.0.\n");
	fprintf(m_fFile, "(c) 2005 by Earandel Senessa and Ingmar Stieger (Papillon)\n");
	fprintf(m_fFile, "visit us at http://www.nwnx.org\n\n");
	fprintf(m_fFile, "o Performance counter frequency: %I64i (1/s) \n", liFrequency.QuadPart);
}