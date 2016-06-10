#include "stdafx.h"
#include "CLRHostManager.h"

namespace thunder
{
	CLRHostManager::CLRHostManager()
	{
		_started = false;
		_startRequested = false;
		_stopped = false;
		_stopRequested = false;
		_executing = false;

		_pMetaHost = NULL;
		_pRuntimeInfo = NULL;
		_pClrRuntimeHost = NULL;

		_thread = std::thread(&CLRHostManager::HostThreadProc, this);
	}

	CLRHostManager::~CLRHostManager()
	{
		RequestStop();
	}

	void CLRHostManager::RequestStart()
	{
		_mutex.lock();
		if (!_started && !_startRequested && !_stopRequested)
			_startRequested = true;
		_mutex.unlock();
	}

	void CLRHostManager::RequestStop()
	{
		bool requestedStop = false;
		_mutex.lock();
		if ((_started || _startRequested) && !_stopped && !_stopRequested)
		{
			_stopRequested = true;
			requestedStop = true;
		}
		_mutex.unlock();

		if (requestedStop)
			_thread.join();
	}

	int CLRHostManager::Execute(const filesystem::path& assemblyPath, const std::wstring& className, const std::wstring& methodName, const std::wstring& argument)
	{
		if (!filesystem::exists(assemblyPath) || !filesystem::is_regular_file(assemblyPath))
			return -1;

		bool started = false;
		bool stopRequested = false;
		bool executing = false;
		_mutex.lock();
		started = _started;
		stopRequested = _stopRequested;
		executing = _executing;
		_mutex.unlock();

		if (!started || stopRequested || executing)
			return -1;

		_mutex.lock();
		_executing = true;
		_mutex.unlock();

		// Note: This might not work.
		// I may need to do a QueueExecution design instead so that the callee is in the worker thread.
		// This would make it difficult to pass the method's return value back.

		DWORD ret;
		HRESULT hr = _pClrRuntimeHost->ExecuteInDefaultAppDomain(assemblyPath.c_str(),
			className.c_str(), methodName.c_str(), argument.c_str(), &ret);

		_mutex.lock();
		_executing = false;
		_mutex.unlock();

		if (FAILED(hr))
		{
			return -1;
		}

		return ret;
	}

	void CLRHostManager::InitializeCLR()
	{
		HRESULT hr;
		MessageBox(GetDesktopWindow(), L"Pause", L"ThunderCLRHost", MB_OK);
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

		MessageBox(GetDesktopWindow(), L"Initialized CLR!", L"ThunderCLRHost", MB_OK);
	}

	void CLRHostManager::DestroyCLR(bool forceStopExecution)
	{
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

		MessageBox(GetDesktopWindow(), L"Destroyed CLR!", L"ThunderCLRHost", MB_OK);
	}

	void CLRHostManager::HostThreadProc()
	{
		while (true)
		{
			bool stopRequested = false;
			bool startRequested = false;
			bool started = false;
			bool executing = false;

			_mutex.lock();
			started = _started;
			stopRequested = _stopRequested;
			_stopRequested = false;
			startRequested = _startRequested;
			_startRequested = false;
			executing = _executing;
			_mutex.unlock();

			if (stopRequested)
			{
				if (started)
					DestroyCLR(executing);

				break;
			}

			if (startRequested && !started)
			{
				InitializeCLR();
				_mutex.lock();
				_started = true;
				_mutex.unlock();
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		_mutex.lock();
		_stopped = true;
		_mutex.unlock();
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