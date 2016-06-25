#pragma once

namespace thunder
{
	class CLRHostManager
	{
	public:
		CLRHostManager();
		~CLRHostManager();
		HRESULT Execute(const filesystem::path& assemblyPath, const std::wstring& className, const std::wstring& methodName, const std::wstring& argument = L"", DWORD* returnVal = nullptr);
		HRESULT InitializeCLR();
		HRESULT DestroyCLR(bool forceStopExecution = false);

	private:
		HRESULT GetInstalledClrRuntimes(std::list<std::wstring>& clrRuntimeList);
		HRESULT GetLatestRuntimeVersion(std::wstring& latestRuntimeVersion);

		ICLRMetaHost* _pMetaHost;
		ICLRRuntimeInfo* _pRuntimeInfo;
		ICLRRuntimeHost* _pClrRuntimeHost;
		bool _initialized;
		bool _destroyed;
	};
}