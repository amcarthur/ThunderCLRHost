// ThunderCLRHost.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ThunderCLRHost.h"
#include "CLRHostManager.h"

thunder::CLRHostManager* _hostManager = nullptr;


THUNDERCLRHOST_API HRESULT fnThunderCLRHostInit(void)
{
	if (_hostManager != nullptr)
		return E_FAIL;

	_hostManager = new thunder::CLRHostManager();
	return _hostManager->InitializeCLR();
}

THUNDERCLRHOST_API HRESULT fnThunderCLRHostDestroy(void)
{
	if (_hostManager == nullptr)
		return -1;

	HRESULT hr = _hostManager->DestroyCLR();
	delete _hostManager;
	_hostManager = nullptr;
	return hr;
}

THUNDERCLRHOST_API HRESULT fnThunderCLRHostExecute(const wchar_t* assemblyPath, const wchar_t* className, const wchar_t* methodName, const wchar_t* argument, DWORD* returnVal)
{
	if (_hostManager == nullptr)
		return E_FAIL;

	return _hostManager->Execute(assemblyPath, className, methodName, argument, returnVal);
}