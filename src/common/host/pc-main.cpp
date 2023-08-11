//////////////////////////////////
// fft-tracker                  //
// Copyright (c) RC Module Inc. //
// Author : A. Brodyazhenko     //
// Year 2017					//
//////////////////////////////////

#include "VShell.h"
#include "hal.h"
#include "ippi.h"
#include "ippcore.h"
#include "ipps.h"
#include "hal_host.h"
#include "math.h"
#include "stdio.h"
#include "nmpp.h"
#include "windows.h"
#include <vector>
int blur(nm8u *img, int width, int height, int size, int x, int y) {
	int halfsize = size / 2;
	int sum = 0;
	int k = 0;
	for (int i = MAX(0, y - halfsize); i <= MIN(height - 1, y + halfsize); i++) {
		for (int j = MAX(0, x - halfsize); j <= MIN(width - 1, x + halfsize); j++) {
			sum += img[i*width + j];
			k++;
		}
	}
	sum /= k;
	return sum;
}

void swap(void** ptr0, void** ptr1) {
	void* tmp = *ptr1;
	*ptr1 = *ptr0;
	*ptr0 = tmp;
}

int main()
{
	if (halOpen("board-nm.abs", NULL)) {			// Load executable file to board, connect to shared memory
		::Sleep(10000);
		return -1;
	}
	int handshake = halSync(0xC0DE0086, 0); //0 
											//int handshake1= halSync(0xC0DE0086,1); //0 
	if (handshake != 0xC0DE6407) {
		printf("Handshake with mc12101 error!");
		return  -1;
	}

	if (!VS_Init())
		return 0;
	if (!VS_Bind("..\\Samples\\Road1.avi"))
	//if (!VS_Bind("..\\Samples\\MAKS2015_256x256.avi"))
	//if (!VS_Bind("..\\Samples\\2xFormula2.avi"))
	//if (!VS_Bind("..\\Samples\\Formula3.avi"))
		//if (!VS_Bind("d:\\video\\films\\256x256\\2xFormula2.avi"))
		return 0;
	int width = VS_GetWidth(VS_SOURCE);
	int height = VS_GetHeight(VS_SOURCE);
	int size = width * height;

	VS_CreateSlider("wantedSize", 1, 32, 255, 1, 32);

	VS_CreateImage("current Image", 1, width, height, VS_RGB24, 0);
	VS_CreateImage("current Image bw", 2, width, height, VS_RGB8, 0);
	VS_CreateImage("previous Image", 3, width, height, VS_RGB8, 0);
	VS_CreateImage("previous ImageDiff", 4, width, height, VS_RGB32, 0);

	//nm8u* currImage = nmppsMalloc_8u(size);
	//nm8u* prevImage = nmppsMalloc_8u(size);
	//nm8u* wantedImage = nmppsMalloc_8u(size);
	
	nm8u* currImage = nmppsMalloc_8u(size);
	nm32s* diffImage = nmppsMalloc_32s(size);
	//std::vector<nm8u> currImage(size);
	nm8u* prevImage = nmppsMalloc_8u(size);
	nm8u* wantedImage = nmppsMalloc_8u(size);

	nm32f* currImageBuf_f = nmppsMalloc_32f(size*3);
	nm32f* currImage_f = currImageBuf_f+size;
	nm32f* prevImageBuf_f = nmppsMalloc_32f(size*3);
	nm32f* prevImage_f = prevImageBuf_f+size;
	nm32f* wantedImage_f = nmppsMalloc_32f(size);

	Ipp8u* blurImage = nmppsMalloc_8u(size);  // = (Ipp8u*)ippMalloc(size); 

	nm32u* currImageC = nmppsMalloc_32u(size);

	//unsigned currImgDDR = halSync(0);
	//unsigned wantedImgDDR = halSync(0);

	size_t currImgDDR   = halSyncAddr(0);
	
	size_t wantedImgDDR = halSyncAddr(0);
	nm32f* ci = (nm32f*)currImgDDR;
	nm32f* wi = (nm32f*)wantedImgDDR;

	//int wantedSize = 10;
	int wantedY = 10;
	int wantedX = 10;
	int caughtX = 10;
	int caughtY = 10;

	S_VS_MouseStatus MouseStatus;
	int status;
	int ookk = halSync(0xABC);

	//currImage[size] = 5;
	while (status = VS_Run()) {
		int frame = VS_GetSrcFrameNum();

		if (!(status&VS_PAUSE)) {
			swap((void**)&currImage, (void**)&prevImage);
			swap((void**)&currImage_f, (void**)&prevImage_f);
			VS_GetData(VS_SOURCE, currImageC);
			VS_GetGrayData(VS_SOURCE, currImage);
			wantedX = caughtX;
			wantedY = caughtY;
		}

		VS_SetData(1, currImageC);
		VS_SetData(2, currImage);
		VS_SetData(3, prevImage);
		int wantedSize = VS_GetSlider(1);

		VS_GetMouseStatus(&MouseStatus);
		if (MouseStatus.nKey == VS_MOUSE_LBUTTON) {
			wantedY = MouseStatus.nY;
			wantedX = MouseStatus.nX;
		}

		VS_Rectangle(3, wantedX, wantedY, wantedX + wantedSize, wantedY + wantedSize, VS_RED, VS_NULL_COLOR);

		IppiSize s;
		s.width = width;
		s.height = height;

		ippiFilterBox_8u_C1R((Ipp8u*)currImage, width, blurImage, width, s, { wantedSize, wantedSize }, { 10, 10 });

		for (int i = 0; i < size; i++) {
			//currImage_f[i] = (float)currImage[i] - blur(currImage, width, height, wantedSize, i%width, i / width);
			currImage_f[i] = (float)currImage[i] -blurImage[i];
			//currImage_f[i] = (float)currImage[i] / 16;// -blurImage[i];
			//currImage_f[i] =  blurImage[i];
			diffImage[i] = currImage_f[i];
			currImage_f[i - size] = currImage_f[i];
			currImage_f[i + size] = currImage_f[i];
			wantedImage_f[i] = 0;
		}
		VS_SetData(4, diffImage);

		for (int i = 0; i < height * wantedSize; i += width) {
			for (int j = 0; j < wantedSize; j++) {
				wantedImage_f[i + j] = prevImage_f[wantedY*width + wantedX + i + j];
			}
		}

		VS_Text("1");
		halWriteMemBlock((unsigned int*)currImage_f,   (size_t)currImgDDR, size);
		VS_Text("2");
		halWriteMemBlock((unsigned int*)wantedImage_f, (size_t)wantedImgDDR, size);
		VS_Text("3 %d ", wantedSize);
		int ok = halSync(wantedSize);
		VS_Text("4");
		caughtX = halSync(0xABC);
		VS_Text("5");
		caughtY = halSync(0xEDF);  
		VS_Text("6");
		
		//VS_Rectangle(1, caughtX, caughtY, caughtX + wantedSize, caughtY + wantedSize, VS_BLUE, VS_NULL_COLOR);

		VS_Rectangle(3, caughtX, caughtY, caughtX + wantedSize, caughtY + wantedSize, VS_GREEN, VS_NULL_COLOR);
		VS_Rectangle(1, caughtX, caughtY, caughtX + wantedSize, caughtY + wantedSize, VS_GREEN, VS_NULL_COLOR);
		
		VS_Text(" %d dx:%d dy:%d\r\n", frame, caughtX - wantedX, caughtY - wantedY);

		VS_Draw(VS_DRAW_ALL);
	}
}