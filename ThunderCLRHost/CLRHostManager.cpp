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
		if (_initialized || !_destroyed)
			DestroyCLR();
	}

	HRESULT CLRHostManager::Execute(const filesystem::path& assemblyPath, const std::wstring& className, const std::wstring& methodName, const std::wstring& argument, DWORD* returnVal)
	{
		if (!_initialized || _destroyed)
			return E_FAIL;

		if (!filesystem::exists(assemblyPath) || !filesystem::is_regular_file(assemblyPath))
			return E_INVALIDARG;

		auto imageRuntimeVersion = blackbone::ImageNET::GetImageRuntimeVer(assemblyPath.c_str());

		wchar_t versionString[20];
		DWORD versionStringSize = 20;
		HRESULT hr = _pRuntimeInfo->GetVersionString(versionString, &versionStringSize);

		if (FAILED(hr))
			return hr;

		if (std::wcscmp(versionString, imageRuntimeVersion.c_str()) != 0)
		{
			MessageBox(GetDesktopWindow(), imageRuntimeVersion.c_str(), L"Image Runtime Version", MB_OK);
			MessageBox(GetDesktopWindow(), versionString, L"Current Runtime Version", MB_OK);
			return E_FAIL;
		}

		hr = _pClrRuntimeHost->ExecuteInDefaultAppDomain(assemblyPath.c_str(),
			className.c_str(), methodName.c_str(), argument.c_str(), returnVal);

		if (FAILED(hr))
			return hr;

		return S_OK;
	}

	HRESULT CLRHostManager::InitializeCLR()
	{
		if (_initialized)
			return S_OK;

		HRESULT hr = CLRCreateInstance(CLSID_CLRMetaHost, IID_PPV_ARGS(&_pMetaHost));
		if (FAILED(hr))
		{
			DestroyCLR();
			return hr;
		}

		std::wstring latestRuntimeVersion;
		hr = GetLatestRuntimeVersion(latestRuntimeVersion);
		if (FAILED(hr))
		{
			DestroyCLR();
			return hr;
		}
		
		hr = _pMetaHost->GetRuntime(latestRuntimeVersion.c_str(), IID_PPV_ARGS(&_pRuntimeInfo));
		if (FAILED(hr))
		{
			DestroyCLR();
			return hr;
		}

		BOOL fLoadable;
		hr = _pRuntimeInfo->IsLoadable(&fLoadable);
		
		if (FAILED(hr))
		{
			DestroyCLR();
			return hr;
		}

		if (!fLoadable)
		{
			DestroyCLR();
			return E_FAIL;
		}

		hr = _pRuntimeInfo->GetInterface(CLSID_CLRRuntimeHost, IID_PPV_ARGS(&_pClrRuntimeHost));
		if (FAILED(hr))
		{
			DestroyCLR();
			return hr;
		}

		BOOL isStarted;
		hr = _pRuntimeInfo->IsStarted(&isStarted, NULL);
		if (FAILED(hr))
		{
			DestroyCLR();
			return hr;
		}

		if (isStarted == FALSE)
		{
			hr = _pClrRuntimeHost->Start();
			if (FAILED(hr))
			{
				DestroyCLR();
				return hr;
			}
		}

		_initialized = true;
		return S_OK;
	}

	HRESULT CLRHostManager::DestroyCLR(bool forceStopExecution)
	{
		if (!_initialized || !_destroyed)
			return S_OK;

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
		return S_OK;
	}

	// Credit - Mattias Hogstrom http://blog.mattiashogstrom.com/coding/2012/05/26/clr-hosting-api.html
	HRESULT CLRHostManager::GetInstalledClrRuntimes(std::list<std::wstring>& clrRuntimeList)
	{
		HRESULT hr = S_OK;
		clrRuntimeList.clear();
		ICLRMetaHost* metahost = nullptr;
		hr = CLRCreateInstance(CLSID_CLRMetaHost,
			IID_ICLRMetaHost,
			(LPVOID*)&metahost);
		if (FAILED(hr))
			return hr;

		IEnumUnknown* runtimeEnumerator = nullptr;
		hr = metahost->EnumerateInstalledRuntimes(&runtimeEnumerator);
		if (SUCCEEDED(hr))
		{
			WCHAR currentRuntime[50];
			DWORD bufferSize = ARRAYSIZE(currentRuntime);
			IUnknown* runtime = nullptr;
			while (runtimeEnumerator->Next(1, &runtime, NULL) == S_OK)
			{
				ICLRRuntimeInfo* runtimeInfo = nullptr;
				hr = runtime->QueryInterface(IID_PPV_ARGS(&runtimeInfo));
				if (SUCCEEDED(hr))
				{
					hr = runtimeInfo->GetVersionString(currentRuntime, &bufferSize);
					if (SUCCEEDED(hr))
					{
						clrRuntimeList.push_back(std::wstring(currentRuntime));
					}
					runtimeInfo->Release();
				}
				runtime->Release();
			}
			runtimeEnumerator->Release();
			hr = S_OK;
		}
		metahost->Release();
		return hr;
	}

	HRESULT CLRHostManager::GetLatestRuntimeVersion(std::wstring& latestRuntimeVersion)
	{
		HRESULT result = S_OK;
		std::list<std::wstring> installedRuntimes;
		result = GetInstalledClrRuntimes(installedRuntimes);

		if (FAILED(result))
			return result;

		if (installedRuntimes.empty())
			return E_FAIL;

		installedRuntimes.sort();
		latestRuntimeVersion = installedRuntimes.back();
		return result;
	}
}