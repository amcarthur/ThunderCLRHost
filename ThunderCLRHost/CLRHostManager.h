#pragma once

namespace thunder
{
	class CLRHostManager
	{
	public:
		CLRHostManager();
		~CLRHostManager();

		void RequestStart();
		void RequestStop();
		int Execute(const filesystem::path& assemblyPath, const std::wstring& className, const std::wstring& methodName, const std::wstring& argument = L"");

	private:
		bool _started;
		bool _startRequested;
		bool _stopped;
		bool _stopRequested;
		bool _executing;
		std::thread _thread;
		std::mutex _mutex;

	private:
		void HostThreadProc();
		void InitializeCLR();
		void DestroyCLR(bool forceStopExecution = false);
		LPCWSTR GetLatestFrameworkVersion();

		ICLRMetaHost* _pMetaHost;
		ICLRRuntimeInfo* _pRuntimeInfo;
		ICLRRuntimeHost* _pClrRuntimeHost;
	};
}