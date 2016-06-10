// ThunderCLRHost.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ThunderCLRHost.h"
#include "CLRHostManager.h"

thunder::CLRHostManager* _hostManager = nullptr;


THUNDERCLRHOST_API int fnThunderCLRHostInit(void)
{
	if (_hostManager != nullptr)
		return -1;

	_hostManager = new thunder::CLRHostManager();
	_hostManager->RequestStart();
	return 0;
}

THUNDERCLRHOST_API int fnThunderCLRHostDestroy(void)
{
	if (_hostManager == nullptr)
		return -1;

	_hostManager->RequestStop();
	delete _hostManager;
	_hostManager = nullptr;
	return 0;
}

THUNDERCLRHOST_API int fnThunderCLRHostExecute(const wchar_t* assemblyPath, const wchar_t* className, const wchar_t* methodName, const wchar_t* argument)
{
	if (_hostManager == nullptr)
		return -1;

	return _hostManager->Execute(assemblyPath, className, methodName, argument);
}