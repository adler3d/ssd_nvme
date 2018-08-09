#include <fstream>
#include <chrono>
#include <vector>
#include <cstdint>
#include <numeric>
#include <random>
#include <algorithm>
#include <iostream>
#include <cassert>
#include <string>
#include <windows.h>
#include <time.h>

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

using std::cout;using std::endl;using std::vector;using std::array;

#include "vector_view.inl"

static vector_view<char> data;
static size_t size;


const std::size_t kB = 1024;
const std::size_t MB = 1024 * kB;
const std::size_t GB = 1024 * MB;

#include <malloc.h>
#include <stdio.h>

constexpr size_t SECTOR_SIZE=4096; // system("fsutil fsinfo ntfsinfo c:");

void GenerateData()
{
  std::cout<<"size = "<<(size/MB)<<"MB"<<endl;
  data.p=(char*)_aligned_malloc(size+4096,SECTOR_SIZE);
  cout<<"p = "<<size_t(data.p)<<"\n";
  data.n=size;
  //data.resize(size);
  typedef unsigned short U16;
  static_assert(sizeof(U16)==2,"...hm?");
  for(uint64_t i=0;i<data.size();i+=2)*(U16*)(&data[i])=rand();
  cout<<"generation done\n";
}

void func_stream()
{
  auto myfile = std::fstream("stream.binary", std::ios::out | std::ios::binary);
  myfile.write((char*)&data[0], data.size());
}

void func_fopen_()
{
  FILE* file = fopen("fopen.binary", "wb");
  fwrite(&data[0], 1, data.size(), file);
  fflush(file);
  fclose(file);
  auto endTime = std::chrono::high_resolution_clock::now();
}

void func_sync0_()
{
  auto old=std::ios_base::sync_with_stdio(false);
  auto myfile = std::fstream("sync0.binary", std::ios::out | std::ios::binary);
  myfile.write((char*)&data[0], data.size());
  std::ios_base::sync_with_stdio(old);
}

void func_ms_fnb()
{
  auto h=CreateFile("ms_flagnobuff.binary",GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL|FILE_FLAG_NO_BUFFERING|FILE_FLAG_SEQUENTIAL_SCAN,NULL);
  DWORD dwWritten;
  WriteFile(h,data.p,data.size(),&dwWritten,NULL);
  cout<<"dwWritten = "<<dwWritten<<"\n";
  CloseHandle(h);
}

template<class FUNC>
void f(FUNC&&func,std::string name){
  QapClock clock;
  func();
  auto ms=clock.MS();
  cout<<(name+": speed = ")<<(data.size()/MB/ms)<<" MB/ms"<<endl;
}

#define F(FUNC)D();f(FUNC,#FUNC);
void D(){cout<<"sleep";for(int i=0;i<100;i++){Sleep(100);cout<<".";}cout<<"\n";}
int main()
{
  srand(time(0));
  size=128*MB;GB*1.75;

  F(GenerateData);
  
  auto aligned_on_disk_sector_boundaries=[](const char*buf){
    return (char*)((size_t)(buf+SECTOR_SIZE-1)&~(SECTOR_SIZE-1));
  };
  
  auto*p=aligned_on_disk_sector_boundaries(data.p);
  if(p!=data.p){cout<<"need do aligned_on_disk_sector_boundaries"<<endl;return 0;}

  //system("pause");
  
  F(func_ms_fnb);
  F(func_sync0_);
  F(func_stream);
  F(func_fopen_);

  return 0;
}

/*

C:\Users\Adler\Desktop\delete_it\ssd_test>build

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp
                                                                                                                        C:\Users\Adler\Desktop\delete_it\ssd_test>ssd_test
size = 256MB
generation done
GenerateData: speed = 0.0929211MB/ms
Для продолжения нажмите любую клавишу . . .
func_stream: speed = 0.28642MB/ms
^C
C:\Users\Adler\Desktop\delete_it\ssd_test>build

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp

C:\Users\Adler\Desktop\delete_it\ssd_test>ssd_test
size = 256MB
generation done
GenerateData: speed = 0.0904053 MB/ms
Для продолжения нажмите любую клавишу . . .
func_stream: speed = 0.321098 MB/ms
func_fopen: speed = 0.144074 MB/ms
func_sync0: speed = 0.220186 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>build

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp

C:\Users\Adler\Desktop\delete_it\ssd_test>ssd_test
size = 256MB
generation done
GenerateData: speed = 0.0895415 MB/ms
Для продолжения нажмите любую клавишу . . .
func_stream: speed = 0.269674 MB/ms
func_fopen_: speed = 0.536918 MB/ms
func_sync0_: speed = 0.312315 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>ssd_test
size = 256MB
generation done
GenerateData: speed = 0.0905002 MB/ms
Для продолжения нажмите любую клавишу . . .
func_stream: speed = 0.216081 MB/ms
func_fopen_: speed = 0.162342 MB/ms
func_sync0_: speed = 0.335733 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>ssd_test
size = 256MB
generation done
GenerateData: speed = 0.0905065 MB/ms
Для продолжения нажмите любую клавишу . . .
func_stream: speed = 0.283625 MB/ms
func_fopen_: speed = 0.342717 MB/ms
func_sync0_: speed = 0.100542 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>build

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp

C:\Users\Adler\Desktop\delete_it\ssd_test>ssd_test
size = 1024MB
generation done
GenerateData: speed = 0.0889136 MB/ms
Для продолжения нажмите любую клавишу . . .
func_stream: speed = 0.155408 MB/ms
func_fopen_: speed = 0.0955212 MB/ms
func_sync0_: speed = 0.148023 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>build

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp

C:\Users\Adler\Desktop\delete_it\ssd_test>build

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp

C:\Users\Adler\Desktop\delete_it\ssd_test>ssd_test
size = 2048MB
generation done
GenerateData: speed = 0.0758996 MB/ms
Для продолжения нажмите любую клавишу . . .
func_stream: speed = 0.14715 MB/ms
func_fopen_: speed = 0.147295 MB/ms
func_sync0_: speed = 0.140815 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>build

C:\Users\Adler\Desktop\delete_it\ssd_test>ssd_test
size = 256MB
generation done
GenerateData: speed = 0 MB/ms
Для продолжения нажмите любую клавишу . . .
func_stream: speed = 0 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>build

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp
ssd_test.cpp(53): error C2659: =: функция в качестве левого операнда

C:\Users\Adler\Desktop\delete_it\ssd_test>ssd_test
size = 256MB
generation done
GenerateData: speed = 0 MB/ms
Для продолжения нажмите любую клавишу . . .
func_stream: speed = 0 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>build

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp

C:\Users\Adler\Desktop\delete_it\ssd_test>ssd_test
size = 256MB
generation done
GenerateData: speed = 0.0961576 MB/ms
Для продолжения нажмите любую клавишу . . .
func_stream: speed = 0.312548 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>ssd_test
size = 256MB
generation done
GenerateData: speed = 0.0953038 MB/ms
Для продолжения нажмите любую клавишу . . .
func_stream: speed = 0.342113 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>ssd_test
size = 256MB
generation done
GenerateData: speed = 0.0959768 MB/ms
Для продолжения нажмите любую клавишу . . .
func_stream: speed = 0.274362 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>build

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp

C:\Users\Adler\Desktop\delete_it\ssd_test>ssd_test
size = 512MB
generation done
GenerateData: speed = 0.0931988 MB/ms
Для продолжения нажмите любую клавишу . . .
func_stream: speed = 0.328994 MB/ms
func_fopen_: speed = 0.266278 MB/ms
func_sync0_: speed = 0.178114 MB/ms
func_ms_fnb: speed = 18.3739 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>build

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp

C:\Users\Adler\Desktop\delete_it\ssd_test>ssd_test
size = 512MB
generation done
GenerateData: speed = 0.0918912 MB/ms
Для продолжения нажмите любую клавишу . . .
func_stream: speed = 0.21411 MB/ms
func_fopen_: speed = 0.541904 MB/ms
func_sync0_: speed = 0.0999844 MB/ms
dwWritten = 536870912
func_ms_fnb: speed = 0.17831 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>build

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp

C:\Users\Adler\Desktop\delete_it\ssd_test>ssd_test
size = 2048MB

C:\Users\Adler\Desktop\delete_it\ssd_test>build

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp
ssd_test.cpp(53): error C2065: p: необъявленный идентификатор

C:\Users\Adler\Desktop\delete_it\ssd_test>ssd_test
size = 2048MB
^C
C:\Users\Adler\Desktop\delete_it\ssd_test>build

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp

C:\Users\Adler\Desktop\delete_it\ssd_test>ssd_test
size = 1024MB
p =
^C
C:\Users\Adler\Desktop\delete_it\ssd_test>build

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp

C:\Users\Adler\Desktop\delete_it\ssd_test>ssd_test
size = 1024MB
p = 1748051746816
generation done
GenerateData: speed = 0.0936071 MB/ms
Для продолжения нажмите любую клавишу . . .
func_stream: speed = 0.182044 MB/ms
func_fopen_: speed = 0.150714 MB/ms
func_sync0_: speed = 0.107011 MB/ms
dwWritten = 1073741824
func_ms_fnb: speed = 0.143639 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>build

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp

C:\Users\Adler\Desktop\delete_it\ssd_test>ssd_test
size = 2048MB
p = 2128420298752

C:\Users\Adler\Desktop\delete_it\ssd_test>build

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp

C:\Users\Adler\Desktop\delete_it\ssd_test>ssd_test
size = 512MB
p = 3015643893760
generation done
GenerateData: speed = 0.0887321 MB/ms
Для продолжения нажмите любую клавишу . . .
dwWritten = 536870912
func_ms_fnb: speed = 0.612892 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>build

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp

C:\Users\Adler\Desktop\delete_it\ssd_test>ssd_test
size = 1024MB
p = 2538325729280
generation done
GenerateData: speed = 0.0881192 MB/ms
Для продолжения нажмите любую клавишу . . .
dwWritten = 1073741824
func_ms_fnb: speed = 0.188383 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>ssd_test
size = 1024MB
p = 1823213658112
generation done
GenerateData: speed = 0.090191 MB/ms
Для продолжения нажмите любую клавишу . . .
dwWritten = 1073741824
func_ms_fnb: speed = 0.474372 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>ssd_test
size = 1024MB
p = 2452426354688
generation done
GenerateData: speed = 0.0857474 MB/ms
Для продолжения нажмите любую клавишу . . .
dwWritten = 1073741824
func_ms_fnb: speed = 0.349595 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>build

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp

C:\Users\Adler\Desktop\delete_it\ssd_test>ssd_test
size = 2048MB
p = 2778194919424

C:\Users\Adler\Desktop\delete_it\ssd_test>type build.bat
cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
C:\Users\Adler\Desktop\delete_it\ssd_test>type build.bat

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /Od /DEBUG /EHsc /nologo /std:c++17
ssd_test.cpp

C:\Users\Adler\Desktop\delete_it\ssd_test>ssd_test
size = 2048MB
p = 1543223861248

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /Od /DEBUG /EHsc /nologo /std:c++17
ssd_test.cpp

C:\Users\Adler\Desktop\delete_it\ssd_test>ssd_test
Для продолжения нажмите любую клавишу . . .
size = 2048MB
p = 2648796078080
^C
C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /Od /DEBUG /EHsc /nologo /std:c++17 /Zi
ssd_test.cpp

C:\Users\Adler\Desktop\delete_it\ssd_test>ssd_test
Для продолжения нажмите любую клавишу . . .
size = 2048MB
p = 2245390245888

C:\Users\Adler\Desktop\delete_it\ssd_test>build

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp

C:\Users\Adler\Desktop\delete_it\ssd_test>ssd_test
Для продолжения нажмите любую клавишу . . .
size = 2048MB
p = 2123607007232
generation done
GenerateData: speed = 0.07728 MB/ms
Для продолжения нажмите любую клавишу . . .
dwWritten = 2147483648
func_ms_fnb: speed = 0.039365 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>build

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp

C:\Users\Adler\Desktop\delete_it\ssd_test>ssd_test
Для продолжения нажмите любую клавишу . . .
size = 256MB
p = 2547130540032
generation done
GenerateData: speed = 0.0946343 MB/ms
Для продолжения нажмите любую клавишу . . .
func_stream: speed = 0.325838 MB/ms
func_fopen_: speed = 0.48851 MB/ms
func_sync0_: speed = 0.317249 MB/ms
dwWritten = 268435456
func_ms_fnb: speed = 0.262401 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>ssd_test
Для продолжения нажмите любую клавишу . . .
size = 256MB
p = 3219078004736
generation done
GenerateData: speed = 0.0934562 MB/ms
Для продолжения нажмите любую клавишу . . .
func_stream: speed = 0.27866 MB/ms
func_fopen_: speed = 0.244276 MB/ms
func_sync0_: speed = 0.331982 MB/ms
dwWritten = 268435456
func_ms_fnb: speed = 0.190635 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>build

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp

C:\Users\Adler\Desktop\delete_it\ssd_test>ssd_test
Для продолжения нажмите любую клавишу . . .
size = 512MB
p = 2060102950912
generation done
GenerateData: speed = 0.0939448 MB/ms
Для продолжения нажмите любую клавишу . . .
func_stream: speed = 0.321674 MB/ms
func_fopen_: speed = 0.237443 MB/ms
func_sync0_: speed = 0.0979969 MB/ms
dwWritten = 536870912
func_ms_fnb: speed = 0.215116 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>build

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp

C:\Users\Adler\Desktop\delete_it\ssd_test>ssd_test
size = 512MB
p = 1597948162048
generation done
GenerateData: speed = 0.0963996 MB/ms
Для продолжения нажмите любую клавишу . . .
func_stream: speed = 0.25675 MB/ms
func_fopen_: speed = 0.36217 MB/ms
func_sync0_: speed = 0.0984653 MB/ms
dwWritten = 536870912
func_ms_fnb: speed = 0.233486 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>build

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp

C:\Users\Adler\Desktop\delete_it\ssd_test>ssd_test
size = 512MB
p = 2720896815104
generation done
GenerateData: speed = 0.0960425 MB/ms
dwWritten = 536870912
func_ms_fnb: speed = 0.617339 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>build&&ssd_test

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp
size = 1536MB
p = 1622024949760
generation done
GenerateData: speed = 0.0943207 MB/ms
dwWritten = 1610612736
func_ms_fnb: speed = 0.185587 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>build&&ssd_test

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp
size = 64MB
p = 2203982471168
generation done
GenerateData: speed = 0.0917841 MB/ms
dwWritten = 67108864
func_ms_fnb: speed = 0.597415 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>build&&ssd_test

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp
size = 64MB
p = 1233036410880
generation done
GenerateData: speed = 0.0921588 MB/ms
func_sync0_: speed = 0.349511 MB/ms
dwWritten = 67108864
func_ms_fnb: speed = 0.576217 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>build&&ssd_test

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp
size = 64MB
p = 1905280880640
generation done
GenerateData: speed = 0.088832 MB/ms
func_stream: speed = 0.382322 MB/ms
func_fopen_: speed = 0.593198 MB/ms
func_sync0_: speed = 0.420449 MB/ms
dwWritten = 67108864
func_ms_fnb: speed = 0.570803 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>build&&ssd_test

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp
size = 16MB
p = 2167338401792
generation done
GenerateData: speed = 0.0755746 MB/ms
func_stream: speed = 0.274619 MB/ms
func_fopen_: speed = 0.645362 MB/ms
func_sync0_: speed = 0.3027 MB/ms
dwWritten = 16777216
func_ms_fnb: speed = 0.523626 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>build&&ssd_test

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp
size = 16MB
p = 1715843944448
generation done
GenerateData: speed = 0.0703744 MB/ms
func_stream: speed = 0.22648 MB/ms
func_fopen_: speed = 0.625574 MB/ms
func_sync0_: speed = 0.292373 MB/ms
dwWritten = 16777216
func_ms_fnb: speed = 0.369353 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>build&&ssd_test

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp
size = 16MB
p = 2405843865600
generation done
GenerateData: speed = 0.0934082 MB/ms
func_stream: speed = 0.358646 MB/ms
func_fopen_: speed = 0.994327 MB/ms
func_sync0_: speed = 0.373875 MB/ms
dwWritten = 16777216
func_ms_fnb: speed = 0.377823 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>build&&ssd_test

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp
size = 16MB
p = 2941637599232
generation done
GenerateData: speed = 0.0820469 MB/ms
func_stream: speed = 0.372653 MB/ms
func_fopen_: speed = 1.15923 MB/ms
func_sync0_: speed = 0.380441 MB/ms
dwWritten = 16777216
func_ms_fnb: speed = 0.390288 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>build&&ssd_test

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp
size = 16MB
p = 2981848121344
generation done
GenerateData: speed = 0.0837394 MB/ms
func_stream: speed = 0.296546 MB/ms
func_fopen_: speed = 0.975877 MB/ms
func_sync0_: speed = 0.307287 MB/ms
dwWritten = 16777216
func_ms_fnb: speed = 0.417902 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>build&&ssd_test

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp
size = 16MB
p = 2894265769984
generation done
GenerateData: speed = 0.0719291 MB/ms
func_stream: speed = 0.325304 MB/ms
func_fopen_: speed = 1.05771 MB/ms
func_sync0_: speed = 0.3357 MB/ms
dwWritten = 16777216
func_ms_fnb: speed = 0.459064 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>build&&ssd_test

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp
ssd_test.cpp(107): error C3688: недопустимый литеральный суффикс "MB"; не удалось найти литеральный оператор или шаблон литерального оператора "operator ""MB"

C:\Users\Adler\Desktop\delete_it\ssd_test>build&&ssd_test

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp
size = 50MB
p = 2336223756288
generation done
GenerateData: speed = 0.0860059 MB/ms
func_stream: speed = 0.349703 MB/ms
func_fopen_: speed = 0.706933 MB/ms
func_sync0_: speed = 0.440721 MB/ms
dwWritten = 52428800
func_ms_fnb: speed = 0.573429 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>build&&ssd_test

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp
size = 50MB
p = 1711425372160
generation done
GenerateData: speed = 0.092353 MB/ms
func_stream: speed = 0.376867 MB/ms
func_fopen_: speed = 0.666005 MB/ms
func_sync0_: speed = 0.397941 MB/ms
dwWritten = 52428800
func_ms_fnb: speed = 0.518839 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>build&&ssd_test

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp
size = 50MB
p = 2789449654272
generation done
GenerateData: speed = 0.0945539 MB/ms
func_stream: speed = 0.372969 MB/ms
func_fopen_: speed = 0.722952 MB/ms
func_sync0_: speed = 0.469094 MB/ms
dwWritten = 52428800
func_ms_fnb: speed = 0.590232 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>build&&ssd_test

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp
size = 50MB
p = 2376822046720
generation done
GenerateData: speed = 0.0965222 MB/ms
func_stream: speed = 0.387291 MB/ms
func_fopen_: speed = 0.671039 MB/ms
func_sync0_: speed = 0.435859 MB/ms
dwWritten = 52428800
func_ms_fnb: speed = 0.572373 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>build&&ssd_test

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp
size = 50MB
p = 1387274440704
generation done
GenerateData: speed = 0.0964304 MB/ms
func_stream: speed = 0.358248 MB/ms
func_fopen_: speed = 0.642307 MB/ms
func_sync0_: speed = 0.456478 MB/ms
dwWritten = 52428800
func_ms_fnb: speed = 0.572415 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>build&&ssd_test

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp
size = 512MB
p = 2781092151296
generation done
GenerateData: speed = 0.0932601 MB/ms
func_stream: speed = 0.326615 MB/ms
func_fopen_: speed = 0.214268 MB/ms
func_sync0_: speed = 0.0871686 MB/ms
dwWritten = 536870912
func_ms_fnb: speed = 0.173205 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>build&&ssd_test

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp
size = 512MB
p = 2479489998848
generation done
GenerateData: speed = 0.0969147 MB/ms
dwWritten = 536870912
func_ms_fnb: speed = 0.46699 MB/ms
func_stream: speed = 0.169828 MB/ms
func_fopen_: speed = 0.145615 MB/ms
func_sync0_: speed = 0.0898774 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>build&&ssd_test

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp
sleep..................................................
size = 512MB
p = 2684354592768
generation done
GenerateData: speed = 0.0988407 MB/ms
sleep..................................................
dwWritten = 536870912
func_ms_fnb: speed = 0.618434 MB/ms
sleep..................................................
func_stream: speed = 0.101613 MB/ms
sleep..................................................
func_fopen_: speed = 0.335687 MB/ms
sleep..................................................
func_sync0_: speed = 0.184785 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>build&&ssd_test

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp
sleep....................
size = 512MB
p = 2360084561920
generation done
GenerateData: speed = 0.0990782 MB/ms
sleep....................
func_sync0_: speed = 0.390478 MB/ms
sleep....................
dwWritten = 536870912
func_ms_fnb: speed = 0.369323 MB/ms
sleep....................
func_stream: speed = 0.162785 MB/ms
sleep....................
func_fopen_: speed = 0.141301 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>build&&ssd_test

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp
sleep............................^C
C:\Users\Adler\Desktop\delete_it\ssd_test>build&&ssd_test

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp
sleep....................................................................................................
size = 1024MB
p = 1314260013056
generation done
GenerateData: speed = 0.097327 MB/ms
sleep....................................................................................................
func_sync0_: speed = 0.159005 MB/ms
sleep....................................................................................................
dwWritten = 1073741824
func_ms_fnb: speed = 0.323696 MB/ms
sleep....................................................................................................
func_stream: speed = 0.261121 MB/ms
sleep....................................................................................................
func_fopen_: speed = 0.283002 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>build&&ssd_test

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp
sleep....................................................................................................
size = 1792MB
p = 2258122887168
generation done
GenerateData: speed = 0.0949837 MB/ms
sleep....................................................................................................
dwWritten = 1879048192
func_ms_fnb: speed = 0.17955 MB/ms
sleep....................................................................................................
func_sync0_: speed = 0.173848 MB/ms
sleep....................................................................................................
func_stream: speed = 0.169789 MB/ms
sleep....................................................................................................
func_fopen_: speed = 0.143117 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>

C:\Users\Adler\Desktop\delete_it\ssd_test>build&&ssd_test

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp
sleep....................................................................................................
size = 128MB
p = 2038389710848
generation done
GenerateData: speed = 0.098394 MB/ms
sleep....................................................................................................
dwWritten = 134217728
func_ms_fnb: speed = 0.139213 MB/ms
sleep....................................................................................................
func_sync0_: speed = 0.446275 MB/ms
sleep....................................................................................................
func_stream: speed = 0.461517 MB/ms
sleep....................................................................................................
func_fopen_: speed = 0.521012 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>build&&ssd_test

C:\Users\Adler\Desktop\delete_it\ssd_test>cl ssd_test.cpp /O2 /EHsc /nologo /std:c++17
ssd_test.cpp
sleep....................................................................................................
size = 128MB
p = 2284453785600
generation done
GenerateData: speed = 0.0951381 MB/ms
sleep....................................................................................................
dwWritten = 134217728
func_ms_fnb: speed = 0.589284 MB/ms
sleep....................................................................................................
func_sync0_: speed = 0.362424 MB/ms
sleep....................................................................................................
func_stream: speed = 0.429787 MB/ms
sleep....................................................................................................
func_fopen_: speed = 0.558176 MB/ms

C:\Users\Adler\Desktop\delete_it\ssd_test>

*/