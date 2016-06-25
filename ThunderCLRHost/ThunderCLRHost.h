// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the THUNDERCLRHOST_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// THUNDERCLRHOST_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef THUNDERCLRHOST_EXPORTS
#define THUNDERCLRHOST_API extern "C" __declspec(dllexport)
#else
#define THUNDERCLRHOST_API __declspec(dllimport)
#endif

THUNDERCLRHOST_API HRESULT fnThunderCLRHostInit(void);
THUNDERCLRHOST_API HRESULT fnThunderCLRHostDestroy(void);
THUNDERCLRHOST_API HRESULT fnThunderCLRHostExecute(const wchar_t* assemblyPath, const wchar_t* className, const wchar_t* methodName, const wchar_t* argument = L"", DWORD* returnVal = nullptr);