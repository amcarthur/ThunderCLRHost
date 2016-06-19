#include "stdafx.h"
#include "CLRHostManager.h"

namespace thunder
{
	CLRHostManager::CLRHostManager()
	{
		_pMetaHost = NULL;
		_pRuntimeInfo = NULL;
		_pClrRuntimeHost = NULL;
		_initialized = false;
		_destroyed = false;
	}

	CLRHostManager::~CLRHostManager()
	{
		if (_initialized && !_destroyed)
			DestroyCLR();
	}

	int CLRHostManager::Execute(const filesystem::path& assemblyPath, const std::wstring& className, const std::wstring& methodName, const std::wstring& argument)
	{
		if (!_initialized || _destroyed)
			return -1;

		if (!filesystem::exists(assemblyPath) || !filesystem::is_regular_file(assemblyPath))
			return -1;

		DWORD ret;
		HRESULT hr = _pClrRuntimeHost->ExecuteInDefaultAppDomain(assemblyPath.c_str(),
			className.c_str(), methodName.c_str(), argument.c_str(), &ret);

		if (FAILED(hr))
		{
			return -1;
		}

		return ret;
	}

	void CLRHostManager::InitializeCLR()
	{
		if (_initialized)
			return;

		HRESULT hr;
		hr = CLRCreateInstance(CLSID_CLRMetaHost, IID_PPV_ARGS(&_pMetaHost));
		if (FAILED(hr))
		{
			DestroyCLR();
			return;
		}

		hr = _pMetaHost->GetRuntime(L"v2.0.50727", IID_PPV_ARGS(&_pRuntimeInfo));
		if (FAILED(hr))
		{
			DestroyCLR();
			return;
		}

		BOOL fLoadable;
		hr = _pRuntimeInfo->IsLoadable(&fLoadable);
		if (FAILED(hr))
		{
			DestroyCLR();
			return;
		}

		if (!fLoadable)
		{
			DestroyCLR();
			return;
		}

		hr = _pRuntimeInfo->GetInterface(CLSID_CLRRuntimeHost, IID_PPV_ARGS(&_pClrRuntimeHost));
		if (FAILED(hr))
		{
			DestroyCLR();
			return;
		}

		hr = _pClrRuntimeHost->Start();
		if (FAILED(hr))
		{
			DestroyCLR();
		}

		_initialized = true;
	}

	void CLRHostManager::DestroyCLR(bool forceStopExecution)
	{
		if (!_initialized || !_destroyed)
			return;

		if (_pMetaHost)
		{
			_pMetaHost->Release();
			_pMetaHost = NULL;
		}
		if (_pRuntimeInfo)
		{
			_pRuntimeInfo->Release();
			_pRuntimeInfo = NULL;
		}
		if (_pClrRuntimeHost)
		{
			if (forceStopExecution)
				_pClrRuntimeHost->Stop();

			_pClrRuntimeHost->Release();
			_pClrRuntimeHost = NULL;
		}

		_destroyed = true;

		MessageBox(GetDesktopWindow(), L"Destroyed CLR!", L"ThunderCLRHost", MB_OK);
	}

	LPCWSTR CLRHostManager::GetLatestFrameworkVersion()
	{
		CComPtr<IEnumUnknown> installedRuntimes;
		HRESULT hr = _pMetaHost->EnumerateInstalledRuntimes(&installedRuntimes.p);

		CComPtr<ICLRRuntimeInfo> runtimeInfo = NULL;
		ULONG fetched = 0;
		std::wstring version;
		while ((hr = installedRuntimes->Next(1, (IUnknown **)&runtimeInfo.p, &fetched)) == S_OK && fetched > 0) 
		{
			wchar_t versionString[20];
			DWORD versionStringSize = 20;
			hr = runtimeInfo->GetVersionString(versionString, &versionStringSize);

			if (versionStringSize >= 2 && versionString[1] == '4') 
			{
				version = versionString;
				break;
			}
		}

		//runtimeInfo->Release();
		//installedRuntimes->Release();
		return version.c_str();
	}
}