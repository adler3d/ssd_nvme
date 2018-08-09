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

    ExitProcess(dw);
}

auto _T=[](auto x){return x;};
#define DebugPrint(A)QapDebugMsg(A);

#include "ask_admin_priv.inl"

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
