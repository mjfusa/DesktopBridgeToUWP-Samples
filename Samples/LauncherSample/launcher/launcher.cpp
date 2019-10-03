//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "stdafx.h"

using namespace winrt;

// trim whitespace in front and back of string
void trim(std::wstring& value)
{
    const wchar_t* whitespace = L"\n\r\t";
    size_t startpos = value.find_first_not_of(whitespace);
    if (std::wstring::npos != startpos)
    {
        value = value.substr(startpos);
    }

    size_t endpos = value.find_last_not_of(whitespace);
    if (std::wstring::npos != endpos)
    {
        value = value.substr(0, endpos + 1);
    }
}

void slash(std::wstring& value)
{
    if (value[0] != '\\' || value[0] != '/')
        value = L'\\' + value;
}

int GetPreviousRun()
{
	DWORD exitCode = -1;
	try {
		auto localFolder = Windows::Storage::ApplicationData::Current().LocalFolder().Path();
		auto bFileExists = std::filesystem::exists((std::wstring) localFolder + L"\\launcher.out");
		if (!bFileExists)
			return -1;
		std::ifstream infile((std::wstring) localFolder + L"\\launcher.out");

		infile >> exitCode;
		infile.close();
	}
	catch (winrt::hresult_error const& ex)
	{
		winrt::hresult hr = ex.to_abi(); // HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND).
		std::wcout << (std::wstring) L"Error : " + (std::wstring) ex.message() << std::endl; // The system cannot find the file specified.
	}
	return exitCode;
}

// Create a string with last error message
std::wstring message()
{
    DWORD error = GetLastError();
    if (error)
    {
        LPVOID buffer = nullptr;
        DWORD length = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&buffer, 0, nullptr);
        if (length)
        {
            LPCWSTR cstr = (LPCWSTR)buffer;
            std::wstring result(cstr, cstr + length);
            LocalFree(buffer);
            trim(result);
            return result;
        }
    }
    return std::wstring();
}

int wmain()
{
    wchar_t* exePath = 0;
    if (_get_wpgmptr(&exePath) != 0)
        return 1;

    std::wstring launcherDirectory = exePath;
    if (launcherDirectory.empty())
    {
        std::wcout << L"error -- failed to get exe path: " << message() << L" (" << GetLastError() << L")" << std::endl;
        return 1;
    }

    auto pos = launcherDirectory.find_last_of('\\');
    if (pos == std::wstring::npos)
    {
        std::wcout << L"error -- failed to get exe path: " << message() << L" (" << GetLastError() << L")" << std::endl;
        return 1;
    }

    launcherDirectory = launcherDirectory.substr(0, pos);

    std::wifstream infile(launcherDirectory + L"\\launcher.cfg");
    if (!infile)
    {
        std::wcout << L"error -- unable to find 'launcher.cfg'!" << std::endl;
        return 1;
    }

    const size_t length = 256;
    wchar_t buffer[length];
    if (!infile.getline(buffer, length))
    {
        std::wcout << L"error -- unable to read executable name from line 1 of 'launcher.cfg'!" << std::endl;
        std::wcout << L"(example exe: 'age2.exe')" << std::endl;
        return 1;
    }

    std::wstring executable = buffer;

    if (!infile.getline(buffer, length))
    {
        std::wcout << L"error -- unable to read current directory from line 2 of 'launcher.cfg'!" << std::endl;
        std::wcout << L"(example package-relative current directory: 'bin\\x64')" << std::endl;
        return 1;
    }

    std::wstring currentDirectory = buffer;
	
	trim(executable);
	trim(currentDirectory);

	// Read third line for EXE to launch prior to main app.
	std::wstring preLaunchExe;
	std::wstring preLaunchExePath;
	int argc = 0;
	LPWSTR* szArglist=NULL;
	if (!infile.getline(buffer, length))
	{
		std::wcout << L"info -- unable to read current directory from line 3 of 'launcher.cfg'!" << std::endl;
		std::wcout << L"(example exe: CrickActivate.exe param1 param2')" << std::endl;
	}
	else {

		preLaunchExe = buffer;
		trim(preLaunchExe);

		argc = 0;
		szArglist = CommandLineToArgvW(preLaunchExe.c_str(), &argc);
		if (szArglist != NULL)
			preLaunchExe = szArglist[0];
	}

    if (executable.empty())
    {
        std::wcout << L"error -- invalid executable specified on line 1 of 'launcher.cfg'!" << std::endl;
        return 1;
    }

    if (currentDirectory.empty())
    {
        std::wcout << L"error -- invalid current directory specified on line 2 of 'launcher.cfg'!" << std::endl;
        return 1;
    }
	
	slash(executable);
    slash(currentDirectory);
	if (!preLaunchExe.empty())
		slash(preLaunchExe);

    std::wstring currentDirectoryPath = launcherDirectory + currentDirectory;
	std::wstring executablePath = currentDirectoryPath + executable;
	if (!preLaunchExe.empty())
		preLaunchExePath = currentDirectoryPath + preLaunchExe;

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if (!preLaunchExe.empty())
	{
		DWORD previousRunExitCode = GetPreviousRun();
		if (previousRunExitCode == -1)
		{

			if (argc > 0)
			{
				std::array<std::wstring, 10> launchArgs;

				for (int i = 0; i < argc - 1; i++)
				{
					if (szArglist != NULL)
						launchArgs[i] = szArglist[i + 1];
				}

				// Launch pre app launch EXE
				BOOL created = CreateProcessW(preLaunchExePath.c_str(), (LPWSTR)&launchArgs, 0, 0, FALSE, 0, 0, currentDirectoryPath.c_str(), &si, &pi);
				if (FALSE == created) {
					std::wcout << L"error -- failed to create pre launch process: " << message() << L" (" << GetLastError() << L")" << std::endl;
					std::wcout << L"path: " << preLaunchExePath << std::endl;
					return 1;
				}
				else {
					WaitForProcess(pi);

					CloseHandle(pi.hProcess);
					CloseHandle(pi.hThread);
				}
			}
		}
	}

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	// launch main exe
	BOOL created = CreateProcessW(executablePath.c_str(), 0, 0, 0, FALSE, 0, 0, currentDirectoryPath.c_str(), &si, &pi);
    if (FALSE == created)
    {
        std::wcout << L"error -- failed to create process: " << message() << L" (" << GetLastError() << L")" << std::endl;
        std::wcout << L"path: " << executablePath << std::endl;
        return 1;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return 0;
}

int WaitForProcess(PROCESS_INFORMATION pi)
{
	// Successfully created the process.  Wait for it to finish.
	WaitForSingleObject(pi.hProcess, INFINITE);
	DWORD exitCode = 0;
	// Get the exit code.
	auto result = GetExitCodeProcess(pi.hProcess, &exitCode);
	try {
		auto localFolder = Windows::Storage::ApplicationData::Current().LocalFolder().Path();
		std::ofstream outfile((std::wstring) localFolder + L"\\launcher.out");
		outfile << exitCode << std::endl;
		outfile.flush();
		outfile.close();
	} 
	catch (winrt::hresult_error const& ex)
	{
		winrt::hresult hr = ex.to_abi(); // HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND).
		std::wcout << (std::wstring) L"Error : " + (std::wstring) ex.message() << std::endl; // The system cannot find the file specified.
	}
	return exitCode;
}