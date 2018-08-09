#include <time.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#ifndef _MSC_VER
#include <unistd.h>
#include <sys/time.h>
#else
#include <windows.h>
#endif

using std::string;
using std::vector;
static string join(const vector<string>&arr,const string&glue)
{
  string out;
  size_t c=0;
  size_t dc=glue.size();
  for(size_t i=0;i<arr.size();i++){if(i)c+=dc;c+=arr[i].size();}
  out.reserve(c);
  for(size_t i=0;i<arr.size();i++){if(i)out+=glue;out+=arr[i];}
  return out;
}
#ifndef _MSC_VER
double get_ms(){
  struct timeval tv;
  gettimeofday(&tv,NULL);
  auto v=(tv.tv_sec*1000+tv.tv_usec/1000.0);
  static auto prev=v;
  return v-prev;
}
#else
struct QapClock{
  typedef __int64 int64;
  int64 freq,beg,tmp;bool run;
public:
  QapClock(){QueryPerformanceFrequency((LARGE_INTEGER*)&freq);run=false;tmp=0;Start();}
  void Start(){QueryPerformanceCounter((LARGE_INTEGER*)&beg);run=true;}
  void Stop(){QueryPerformanceCounter((LARGE_INTEGER*)&tmp);run=false;tmp-=beg;}
  double Time(){if(run)QueryPerformanceCounter((LARGE_INTEGER*)&tmp);return run?double(tmp-beg)/double(freq):double(tmp)/double(freq);}
  double MS(){
    double d1000=1000.0;
    if(run)QueryPerformanceCounter((LARGE_INTEGER*)&tmp);
    if(run)return (double(tmp-beg)*d1000)/double(freq);
    if(!run)return (double(tmp)*d1000)/double(freq);
    return 0;
  }
};
double get_ms(){static QapClock c;return c.MS();}
#endif

#define QapDebugMsg(TEXT)std::cout<<string(TEXT)+"\n";

void ErrorExit(const std::string_view&func) 
{ 
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message and exit the process

    QapDebugMsg(string(func)+" failed with error "+std::to_string(dw)+": "+string((char*)lpMsgBuf));

    /*
    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR)); 
    StringCchPrintf((LPTSTR)lpDisplayBuf, 
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"), 
        lpszFunction, dw, lpMsgBuf); 
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 
    
    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);*/


    ExitProcess(dw);
}

// http://www.dinop.com/vc/service_ctrl.html (ja)
#include <winsvc.h>
#include "atlstr.h"

auto _T=[](auto x){return x;};
#define DebugPrint(A)QapDebugMsg(A);

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

#include <winioctl.h>

//int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)


inline string to_hex(unsigned char v){
  char buf[3]={0,0,0};
  sprintf(buf,"%02x",v);
  return buf;
}

inline char from_hex(char c1,char c2){
  char buf[3]={c1,c2,0};
  size_t v;
  sscanf(buf,"%02x",&v);
  return (char&)v;
}

inline string str2hex(const string&s)
{
  string out;
  out.reserve(s.size()*2);
  for(size_t i=0;i<s.size();i++)out+=to_hex((unsigned char&)s[i]);
  return out;
}

inline string hex2str(const string&s){
  string out;
  out.reserve(s.size()/2);
  for(size_t i=0;i<s.size();i+=2){
    out.push_back(from_hex(s[i],s[i+1]));
  }
  return out;
}

static bool file_put_contents(const string&FN,const string&mem)
{
  using namespace std;
  std::fstream f;
  f.open(FN.c_str(),ios::out|ios::binary);
  if(!f)return false;
  if(!mem.empty())f.write(&mem[0],mem.size());
  f.close();
  return true;
}

static string file_get_contents(const string&FN)
{
  using namespace std;
  std::fstream f;
  f.open(FN.c_str(),ios::in|ios::binary);
  if(!f)return "";
  f.seekg(0,std::ios::end);
  auto n=f.tellg();
  f.seekg(0,std::ios::beg);
  if(!n)return "";
  string mem;
  mem.resize(n);
  f.read(&mem[0],mem.size());
  f.close();
  return mem;
};

struct t_state{
  uint64_t r=0,w=0;
};

int main()
{
  //system("pause");
  QapDebugMsg("nvme_main.cpp");
  run_wmi();
  ask_admin_priv();
  auto h=CreateFile("\\\\.\\PhysicalDrive0",GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
  if(auto err=size_t(h)==size_t(-1))QapDebugMsg("h = "+std::to_string((size_t)h)+"\n(h==-1) is "+std::to_string(err));


	StorageQuery::TStorageQueryWithBuffer nptwb;
	ZeroMemory(&nptwb, sizeof(nptwb));
  
  // include <linux/nvme.h>

  nptwb.ProtocolSpecific.ProtocolType = StorageQuery::ProtocolTypeNvme;
  nptwb.ProtocolSpecific.DataType = StorageQuery::NVMeDataTypeLogPage; // nvme_admin_opcode::nvme_admin_get_log_page
  nptwb.ProtocolSpecific.ProtocolDataRequestValue = 2; // SMART Health Information
  nptwb.ProtocolSpecific.ProtocolDataRequestSubValue = 0xFFFFFFFF;
  nptwb.ProtocolSpecific.ProtocolDataOffset = sizeof(StorageQuery::TStorageProtocolSpecificData);
  nptwb.ProtocolSpecific.ProtocolDataLength = 4096;
  nptwb.Query.PropertyId = StorageQuery::StorageAdapterProtocolSpecificProperty;
  nptwb.Query.QueryType = StorageQuery::PropertyStandardQuery;
  DWORD dwReturned = 0;

  auto bRet = DeviceIoControl(h, IOCTL_STORAGE_QUERY_PROPERTY,
	  &nptwb, sizeof(nptwb), &nptwb, sizeof(nptwb), &dwReturned, NULL);
  CloseHandle(h);

  // nptwb.Buffer is nvme_smart_log // https://github.com/mirror/smartmontools/blob/fac3b72ddc5224cbe490544844bc326ed50e5246/nvmecmds.h#L187

	auto HostReads= *(uint64_t*)(void*)&nptwb.Buffer[32];
	auto HostWrites=*(uint64_t*)(void*)&nptwb.Buffer[48];
  auto HostReads_GB=  HostReads*512e3/(1024*1024*1024);
  auto HostWrites_GB=HostWrites*512e3/(1024*1024*1024);
  t_state cur={HostReads,HostWrites};
  auto rub_per_GBW=4750.0/72e3;

  auto fn="prev_state.bin";auto content=file_get_contents(fn);
  if(content.size()==sizeof(t_state))
  {
    auto&prev=*(t_state*)&content[0];auto k=512e3/(1024*1024*1024);
    QapDebugMsg("HostReads_delta = "+std::to_string(cur.r-prev.r));
    QapDebugMsg("HostWrites_delta = "+std::to_string(cur.w-prev.w));
    QapDebugMsg("HostReads_delta_GB = "+std::to_string(k*(cur.r-prev.r)));
    QapDebugMsg("HostWrites_delta_GB = "+std::to_string(k*(cur.w-prev.w)));
  }
  auto beg=((char*)&cur);
  file_put_contents(fn,string(beg,beg+sizeof(cur)));

  QapDebugMsg("HostReads_GB = "+std::to_string(HostReads_GB)+"\nHostWrites_GB = "+std::to_string(HostWrites_GB)+"\nrub_wasted = "+std::to_string(HostWrites_GB*rub_per_GBW));
  string buff(&nptwb.Buffer[0],(&nptwb.Buffer[0])+dwReturned);
  QapDebugMsg("RAW: "+str2hex(buff));

  //system("pause");
  return 0;
}
