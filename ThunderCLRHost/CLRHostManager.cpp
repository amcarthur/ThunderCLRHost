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

		auto imageRuntimeVersion = blackbone::ImageNET::GetImageRuntimeVer(assemblyPath.c_str());

		wchar_t versionString[20];
		DWORD versionStringSize = 20;
		HRESULT hr = _pRuntimeInfo->GetVersionString(versionString, &versionStringSize);

		if (std::wcscmp(versionString, imageRuntimeVersion.c_str()) != 0)
		{
			MessageBox(GetDesktopWindow(), imageRuntimeVersion.c_str(), L"Image Runtime Version", MB_OK);
			MessageBox(GetDesktopWindow(), versionString, L"Current Runtime Version", MB_OK);
			return -1;
		}

		DWORD ret;
		hr = _pClrRuntimeHost->ExecuteInDefaultAppDomain(assemblyPath.c_str(),
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

		auto frameworkVersion = GetLatestFrameworkVersion();
		
		hr = _pMetaHost->GetRuntime(frameworkVersion.c_str(), IID_PPV_ARGS(&_pRuntimeInfo));
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

		BOOL isStarted;
		hr = _pRuntimeInfo->IsStarted(&isStarted, NULL);
		if (FAILED(hr))
		{
			DestroyCLR();
			return;
		}

		if (isStarted == FALSE)
		{
			hr = _pClrRuntimeHost->Start();
			if (FAILED(hr))
			{
				DestroyCLR();
				return;
			}
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
	}

	std::wstring CLRHostManager::GetLatestFrameworkVersion()
	{
		IEnumUnknown* installedRuntimes;
		HRESULT hr = _pMetaHost->EnumerateInstalledRuntimes(&installedRuntimes);

		ICLRRuntimeInfo* runtimeInfo = NULL;
		ULONG fetched = 0;
		std::wstring version(L"");
		while ((hr = installedRuntimes->Next(1, (IUnknown **)&runtimeInfo, &fetched)) == S_OK && fetched > 0) 
		{
			wchar_t versionString[20];
			DWORD versionStringSize = 20;
			hr = runtimeInfo->GetVersionString(versionString, &versionStringSize);

			if (versionStringSize >= 2 && versionString[1] == '4') 
			{
				version = std::wstring(versionString);
				break;
			}
		}

		runtimeInfo->Release();
		installedRuntimes->Release();
		return version;
	}
}