#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include "helperMethods.h"
#include "Constants.h"
#include <iostream>
#include <ctime>   // For std::clock() and CLOCKS_PER_SEC
#include <cstdio>  // For FILE, fopen, fread, fclose
#include <cstdlib> // For malloc, free

using namespace std;

GLint windowwidth()
{
 static int ret=0;
 if(!ret)ret=glutGet(GLUT_SCREEN_WIDTH)-WINDOW_MARGIN;
 return ret;
}

GLint windowheight()
{
 static int ret=0;
 if(!ret)ret=glutGet(GLUT_SCREEN_HEIGHT)-WINDOW_MARGIN;
 return ret;
}

string timeLeft(const std::function<void(int,int,int,int)>& func, clock_t endTime) {
    std::string strSec, strMn, strHour;

    // Calculate total seconds remaining
    int n = (endTime - std::clock()) / CLOCKS_PER_SEC;

    // Breakdown into hours, minutes, and seconds
    int hour = n / 3600;
    int mn = (n % 3600) / 60; // Fixed: Previous code did not bound minutes to 0-59
    int sec = n % 60;

    // Convert directly to string and concatenate
    strHour = std::to_string(hour);
    strMn = std::to_string(mn);
    strSec = std::to_string(sec);

    func(hour, mn, sec, n);

    return strHour + "::" + strMn + "::" + strSec;
}

void app_assert_success(const char* szz)
{
 if(GLint xerr= glGetError())
 { 
  char szerr[64]; 
  sprintf(szerr,"%s , %d",szz,xerr); 
  fprintf(stderr,"%s",szerr); 
  exit(1);
 }
}

GLuint maketex(const char* tfile,GLint xSize,GLint ySize) //returns tex. no.
{
 GLuint rmesh;
 FILE * file;
 unsigned char * texdata = (unsigned char*) malloc( xSize * ySize * 3 ); //3 is {R,G,B}

 file = fopen(tfile, "rb" );
 fseek(file,BMP_HEADER_SIZE,SEEK_CUR);
 fread( texdata, xSize * ySize * 3, 1, file ); 
 fclose( file );
 glEnable( GL_TEXTURE_2D );

 char* colorbits = new char[ xSize * ySize * 3]; 

 for(GLint a=0; a<xSize * ySize * 3; ++a) colorbits[a]=0xFF; 

 glGenTextures(1,&rmesh);
 glBindTexture(GL_TEXTURE_2D,rmesh);

 glTexImage2D(GL_TEXTURE_2D,0 ,3 , xSize,
 ySize, 0 , GL_RGB, GL_UNSIGNED_BYTE, colorbits);

 app_assert_success("post0_image");

 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);

 app_assert_success("pre_getview");

 //Save viewport and set up new one
 GLint viewport[4]; //4 is {X,Y,Width,Height}
 glGetIntegerv(GL_VIEWPORT,(GLint*)viewport);

 app_assert_success("pre_view");
 glViewport(0,0,xSize,ySize);
 app_assert_success("post0_view");

 //Clear target and depth buffers
 glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT); 

 glPushMatrix(); //Duplicates MODELVIEW stack top
 glLoadIdentity(); //Replace new top with {1}

 app_assert_success("ogl_mvx");

 glDrawPixels(xSize,ySize, GL_BGR_EXT, GL_UNSIGNED_BYTE,texdata);

 app_assert_success("pre_copytext");
 glPopMatrix();
 app_assert_success("copytext2");
 glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
 0,0, xSize, ySize, 0);
 app_assert_success("post_copy");
 
 //Restore viewport
 glViewport(viewport[0],viewport[1],viewport[2],viewport[3]); //{X,Y,Width,Height}
 app_assert_success("ogl_mm1");
 delete[] colorbits;
 free(texdata);
 return rmesh;

}


DisplayTiming initializeDisplayTiming() {
    DisplayTiming timing = {};
    // Request one buffer swap per refresh. Drivers/compositors can override this;
    // these software sequences still require photodiode validation under load.
    typedef BOOL (WINAPI *SwapIntervalProc)(int);
    const PROC swapAddress = wglGetProcAddress("wglSwapIntervalEXT");
    const bool validSwapAddress = swapAddress && swapAddress != reinterpret_cast<PROC>(1)
        && swapAddress != reinterpret_cast<PROC>(2) && swapAddress != reinterpret_cast<PROC>(3)
        && swapAddress != reinterpret_cast<PROC>(-1);
    auto swapInterval = validSwapAddress ? reinterpret_cast<SwapIntervalProc>(swapAddress) : nullptr;
    timing.vsyncEnabled = swapInterval && swapInterval(1);
    if (!timing.vsyncEnabled) {
        std::cerr << "VSync unavailable: flicker paused. F1 enables unvalidated preview.\n";
    } else {
        std::cout << "VSync enabled: flickering targets enabled for the SSVEP experiment.\n";
    }
    MONITORINFOEX monitor = {};
    monitor.cbSize = sizeof(monitor);
    DEVMODE mode = {};
    mode.dmSize = sizeof(mode);
    if (GetMonitorInfo(MonitorFromWindow(GetActiveWindow(), MONITOR_DEFAULTTONEAREST), &monitor)
        && EnumDisplaySettings(monitor.szDevice, ENUM_CURRENT_SETTINGS, &mode)
        && mode.dmDisplayFrequency > 1)
        timing.refreshHz = mode.dmDisplayFrequency;
    return timing;
}
