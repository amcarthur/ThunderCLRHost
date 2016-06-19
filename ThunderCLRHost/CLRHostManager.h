#pragma once

namespace thunder
{
	class CLRHostManager
	{
	public:
		CLRHostManager();
		~CLRHostManager();
		int Execute(const filesystem::path& assemblyPath, const std::wstring& className, const std::wstring& methodName, const std::wstring& argument = L"");
		void InitializeCLR();
		void DestroyCLR(bool forceStopExecution = false);

	private:
		LPCWSTR GetLatestFrameworkVersion();

		ICLRMetaHost* _pMetaHost;
		ICLRRuntimeInfo* _pRuntimeInfo;
		ICLRRuntimeHost* _pClrRuntimeHost;
		bool _initialized;
		bool _destroyed;
	};
}