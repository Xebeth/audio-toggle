/**************************************************************************
	created		:	2012-10-26
	filename	: 	main.cpp
	author		:	Xebeth`
	copyright	:	North Edge (2012)
	purpose		:	Application entry point
**************************************************************************/
#include "stdafx.h"

#include "SettingsIniFile.h"
#include "EndPointController.h"

string_t GetIniFile(const TCHAR *pFilename_in)
{
	TCHAR ExePath[_MAX_PATH + 1];
	string_t::size_type Pos;
	string_t IniPath;

	ZeroMemory(ExePath, sizeof(ExePath));
	GetModuleFileName(NULL, ExePath, _MAX_PATH);
	IniPath = ExePath;

	Pos = IniPath.rfind('\\');

	if (Pos != string_t::npos)
		IniPath = IniPath.substr(0, Pos + 1);
	else
		IniPath.clear();

	IniPath += pFilename_in;

	return IniPath;
}

// EndPointController.exe [NewDefaultDeviceID]
int APIENTRY _tWinMain(HINSTANCE hInstance,
					   HINSTANCE hPrevInstance,
					   LPTSTR    lpCmdLine,
					   int       nCmdShow)
{
	try
	{
		Settings::SettingsIniFile Settings(GetIniFile(_T("endpoints.ini")));
		string_t Section = _T("AudioEndPoints");
		EndPointController Controller;		

		// load the settings
		if (Settings.Load() == false)
		{
			if (Controller.EnumerateEndPoints() > 0U)
			{
				string_t Key, Comment;
				EndPointInfo Info;
				int Index = 0;			

				Settings.CreateSection(Section);

				while (Controller.Next(Info))
				{
					format(Key, _T("EndPoint%i"), Index++);
					format(Comment, _T("; %s"), Info.Name.c_str());
					Settings.SetString(Section, Key, Info.ID, Comment.c_str());
				}

				Settings.Save();
			}
		}
		else
		{
			const CSimpleIni::TKeyVal *pSection = Settings.GetValues(Section);

			if (pSection != NULL)
			{
				CSimpleIni::TKeyVal::const_iterator ValueIt = pSection->begin();
				CSimpleIni::TKeyVal::const_iterator EndIt = pSection->end();

 				for(; ValueIt != EndIt; ++ValueIt)
 					Controller.AddEndPoint(ValueIt->first.pComment, ValueIt->second);

				Controller.ToggleEndPoint();
			}
		}
		
		return 0;
	}
	catch (std::exception &e)
	{
		::MessageBoxA(NULL, e.what(), "Fatal error", MB_OK | MB_ICONERROR);

		return 2;
	}

	return 1;
}