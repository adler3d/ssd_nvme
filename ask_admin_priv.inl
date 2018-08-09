
// http://www.dinop.com/vc/service_ctrl.html (ja)
#include <winsvc.h>
#include "atlstr.h"

class	CDnpService
{
	class CServiceThread
	{
	public:
		CServiceThread()
		{
			_bCancel = false;
		}

	private:

		bool					_bCancel;		
		CComAutoCriticalSection	_secbCancel;		

	public:

		bool	IsCancel(bool bSave=false,bool bNewValue=false)
		{
			bool	ret;

			_secbCancel.Lock();
				if(bSave)
				{
					_bCancel = bNewValue;
					ret = true;
				}
				else
					ret = _bCancel;
			_secbCancel.Unlock();

			return	ret;
		}

		bool EasyStartStop(LPCTSTR pszName, bool b)
		{
			bool			ret = false;
			BOOL			bRet = FALSE;
			SC_HANDLE		hManager = NULL;
			SC_HANDLE		hService = NULL;
			SERVICE_STATUS	sStatus;

			hManager = ::OpenSCManager(NULL,NULL,GENERIC_EXECUTE);
			if(hManager == NULL)
			{
				return false;
			}
	
			hService = ::OpenService(hManager, pszName, SERVICE_START | SERVICE_QUERY_STATUS);
			if(hService == NULL)
			{
				if(hManager){::CloseServiceHandle(hManager);}
				return false;
			}
	
			::ZeroMemory(&sStatus,sizeof(SERVICE_STATUS));
			bRet = ::QueryServiceStatus(hService,&sStatus);
			if(bRet == FALSE)
			{
				if(hService){::CloseServiceHandle(hService);}
				if(hManager){::CloseServiceHandle(hManager);}
				return false;
			}

			if(sStatus.dwCurrentState == SERVICE_RUNNING)
			{
				if(hService){::CloseServiceHandle(hService);}
				if(hManager){::CloseServiceHandle(hManager);}
				return true;
			}

			/*CString cstr;
			cstr.Format(_T("sStatus.dwCurrentState:%08X"), sStatus.dwCurrentState);
			DebugPrint(cstr);*/

			DebugPrint(_T("StartService - 1"));
			bRet = ::StartService(hService, NULL, NULL);

			DebugPrint(_T("QueryServiceStatus - 1"));
			int count = 0;
			while(::QueryServiceStatus(hService, &sStatus))
			{
				if(count >= 4)
				{
					break;
				}

				if(sStatus.dwCurrentState == SERVICE_RUNNING)
				{
					DebugPrint(_T("StartService Completed : SERVICE_RUNNING"));
					if(hService){::CloseServiceHandle(hService);}
					if(hManager){::CloseServiceHandle(hManager);}
					return true;
				}
					
				::Sleep(100 * count);
				DebugPrint(_T("Sleep"));
				count++;
			}
				
			ShellExecute(NULL, NULL, _T("sc"), _T("config Winmgmt start= auto"), NULL, SW_HIDE);
			count = 0;
			DebugPrint(_T("QueryServiceStatus - 2"));
			while(::QueryServiceStatus(hService, &sStatus))
			{
				DebugPrint(_T("StartService - 2"));
				::StartService(hService, NULL, NULL);

				if(count >= 10)
				{
					break;
				}

				if(sStatus.dwCurrentState == SERVICE_RUNNING)
				{
					DebugPrint(_T("StartService Completed : SERVICE_RUNNING"));
					if(hService){::CloseServiceHandle(hService);}
					if(hManager){::CloseServiceHandle(hManager);}
					return true;
				}
					
				::Sleep(500);
				DebugPrint(_T("Sleep"));
				count++;
			}

			if(hService){::CloseServiceHandle(hService);}
			if(hManager){::CloseServiceHandle(hManager);}
			return false;
		}
	};


public:

	bool	EasyStartStop(LPCTSTR pszName,bool bStart)
	{
		CServiceThread	cThread;

		return	cThread.EasyStartStop(pszName,bStart);
	}

	bool	EasyStart(LPCTSTR pszName)
	{
		return	EasyStartStop(pszName,true);
	}

	bool	EasyStop(LPCTSTR pszName)
	{
		return	EasyStartStop(pszName,false);
	}

	bool	EasyRestart(LPCTSTR pszName)
	{
		bool			ret;
		CServiceThread	cThread;

		ret = cThread.EasyStartStop(pszName,false);
		if(ret)
			ret = cThread.EasyStartStop(pszName,true);

		return	ret;
	}

	bool	IsServiceRunning(LPCTSTR pszName)
	{
		bool			ret;
		BOOL			bRet;
		SC_HANDLE		hManager;
		SC_HANDLE		hService;
		SERVICE_STATUS	sStatus;

		ret = false;
		hManager = NULL;
		hService = NULL;
		while(1)			//–іЊАѓ‹Ѓ[ѓv‚Е‚Н‚И‚ўЃI
		{
			hManager = ::OpenSCManager(NULL,NULL,GENERIC_EXECUTE);
      if(hManager == NULL)ErrorExit("OpenSCManager");
			ATLASSERT(hManager);
			if(hManager == NULL)
				break;

			hService = ::OpenService(hManager,pszName,SERVICE_QUERY_STATUS);
			ATLASSERT(hService);
			if(hService == NULL)
				break;

			::ZeroMemory(&sStatus,sizeof(SERVICE_STATUS));
			bRet = ::QueryServiceStatus(hService,&sStatus);
			ATLASSERT(bRet);
			if(bRet == FALSE)
				break;

			if(sStatus.dwCurrentState == SERVICE_RUNNING)
				ret = true;

			break;		//•Kђ{
		}

		if(hService)
			::CloseServiceHandle(hService);
		if(hManager)
			::CloseServiceHandle(hManager);

		return	ret;
	}
};


void run_wmi()
{
	CDnpService	cService;
	
	if(! cService.IsServiceRunning(_T("Winmgmt")))
	{
		DebugPrint(_T("Waiting... Winmgmt"));
		auto initWmi = cService.EasyStart(_T("Winmgmt"));
	}
}

namespace StorageQuery {
	typedef enum {
		StorageDeviceProperty = 0,
		StorageAdapterProperty,
		StorageDeviceIdProperty,
		StorageDeviceUniqueIdProperty,
		StorageDeviceWriteCacheProperty,
		StorageMiniportProperty,
		StorageAccessAlignmentProperty,
		StorageDeviceSeekPenaltyProperty,
		StorageDeviceTrimProperty,
		StorageDeviceWriteAggregationProperty,
		StorageDeviceDeviceTelemetryProperty,
		StorageDeviceLBProvisioningProperty,
		StorageDevicePowerProperty,
		StorageDeviceCopyOffloadProperty,
		StorageDeviceResiliencyProperty,
		StorageDeviceMediumProductType,
		StorageDeviceRpmbProperty,
		StorageDeviceIoCapabilityProperty = 48,
		StorageAdapterProtocolSpecificProperty,
		StorageDeviceProtocolSpecificProperty,
		StorageAdapterTemperatureProperty,
		StorageDeviceTemperatureProperty,
		StorageAdapterPhysicalTopologyProperty,
		StorageDevicePhysicalTopologyProperty,
		StorageDeviceAttributesProperty,
	} TStoragePropertyId;

	typedef enum {
		PropertyStandardQuery = 0,
		PropertyExistsQuery,
		PropertyMaskQuery,
		PropertyQueryMaxDefined
	} TStorageQueryType;

	typedef struct {
		TStoragePropertyId PropertyId;
		TStorageQueryType QueryType;
	} TStoragePropertyQuery;

	typedef enum {
		ProtocolTypeUnknown = 0x00,
		ProtocolTypeScsi,
		ProtocolTypeAta,
		ProtocolTypeNvme,
		ProtocolTypeSd,
		ProtocolTypeProprietary = 0x7E,
		ProtocolTypeMaxReserved = 0x7F
	} TStroageProtocolType;

	typedef struct {
		TStroageProtocolType ProtocolType;
		DWORD   DataType;
		DWORD   ProtocolDataRequestValue;
		DWORD   ProtocolDataRequestSubValue;
		DWORD   ProtocolDataOffset;
		DWORD   ProtocolDataLength;
		DWORD   FixedProtocolReturnData;
		DWORD   Reserved[3];
	} TStorageProtocolSpecificData;

	typedef enum {
		NVMeDataTypeUnknown = 0,
		NVMeDataTypeIdentify,
		NVMeDataTypeLogPage,
		NVMeDataTypeFeature,
	} TStorageProtocolNVMeDataType;

	typedef struct {
		TStoragePropertyQuery Query;
		TStorageProtocolSpecificData ProtocolSpecific;
		BYTE Buffer[4096];
	} TStorageQueryWithBuffer;
}
#include <WbemCli.h>
#pragma comment(lib, "wbemuuid.lib")
template<class TYPE>void SAFE_RELEASE(TYPE*p){if(p){(p)->Release();p=NULL;}}
void ask_admin_priv()
{
	try
	{
		IWbemLocator*			pIWbemLocator = NULL;
		IWbemServices*			pIWbemServices = NULL;
		IEnumWbemClassObject*	pEnumCOMDevs = NULL;
		IEnumWbemClassObject*	pEnumCOMDevs2 = NULL;
		IWbemClassObject*		pCOMDev = NULL;
	//	DebugPrint(_T("CoInitialize()"));
		CoInitialize(NULL);
		//DebugPrint(_T("CoInitializeSecurity()"));
		CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT,
			RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
		//DebugPrint(_T("CoCreateInstance()"));
		//CLSID_WbemAdministrativeLocator /
		if(FAILED(CoCreateInstance(CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER,
			IID_IWbemLocator, (LPVOID *)&pIWbemLocator)))
		{
		//	CoUninitialize();
			DebugPrint(_T("NG:WMI Init fail 1"));
		}
		else 
		{
			long securityFlag = WBEM_FLAG_CONNECT_USE_MAX_WAIT;

			//DebugPrint(_T("ConnectServer()"));
			if (FAILED(pIWbemLocator->ConnectServer(L"\\\\.\\root\\cimv2",
				NULL, NULL, 0L,
				securityFlag,
				NULL, NULL, &pIWbemServices)))
			{
			//	CoUninitialize();
				DebugPrint(_T("NG:WMI Init fail 2"));
			}
			else
			{
				//DebugPrint(_T("CoSetProxyBlanket()"));
				auto hRes = CoSetProxyBlanket(pIWbemServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
					NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
				if(FAILED(hRes))
				{
					DebugPrint("NG:WMI Init - "+std::to_string(hRes));
				}
				else
				{
					auto IsEnabledWmi = TRUE;
					//DebugPrint(_T("OK:WMI Init done"));
				}
			}
			SAFE_RELEASE(pIWbemLocator);
		}
	}
	catch(...)
	{
		DebugPrint(_T("EX:WMI Init fail 3"));
	}
  #undef DebugPrint
}
