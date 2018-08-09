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

const std::size_t kB = 1024;
const std::size_t MB = 1024 * kB;
const std::size_t GB = 1024 * MB;

#include <malloc.h>
#include <stdio.h>
#include <Shlobj.h>
#pragma comment(lib,"shell32")

int main()
{
  SHChangeNotify(SHCNE_ASSOCCHANGED,SHCNF_IDLIST,0,0);
  printf("done");
  return 0;
}






























