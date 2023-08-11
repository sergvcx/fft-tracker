//------------------------------------------------------------------------
//  snow_white progect
//
//  Author: S.Mushkaev 
//
//  Copyright (c) 2017 RC Module Inc.
//------------------------------------------------------------------------

//#include "ippdefs.h"
//#include "ippi.h"
//#include "ippcore.h"
//#include "ipps.h"
#include "nmpp.h"
#include "math.h"
#include "VShell.h"
#include "hal_host.h"
#include "ringbuffer_host.h"
#include "stdio.h"
#include "tracker.h"

void swap(void** ptr0, void** ptr1) {
	void* tmp = *ptr1;
	*ptr1 = *ptr0;
	*ptr0 = tmp;
}



#define PROGRAM0 "main0.abs"
#define PROGRAM1 "main1.abs"
int main()
{
	if (!VS_Init())
		return 0;
	//if (!VS_Bind("d:\\video\\films\\MAKS2015_256x2561.avi"))
	//if (!VS_Bind("d:\\video\\films\\256x256\\2xFormula2.avi"))
	if (!VS_Bind("d:\\video\\films\\256x256\\Formula30.avi"))
		//if (!VS_Bind("Formula30.avi"))
		return 0;
	int width = VS_GetWidth(VS_SOURCE);
	int height = VS_GetHeight(VS_SOURCE);
	int size = width * height;

	// Access and loading program to nm-board
	if (halOpen(PROGRAM0, PROGRAM1, NULL)) {
		printf("Connection to mc12101 error!");
		return -1;
	}


	//----------------------------------------------
	int handshake, ok;


	//----------------------- 0 ---------------------------
	handshake = halSync(0xC0DE0086, 0);
	if (handshake != 0xC0DE6406) {
		printf("Handshake with mc12101-nmc0 error!");
		return -1;
	}
	ok = halSync(0, 0);	// Get	status of memory allocation from nm
	if (ok != 0x600DB11F) {
		printf("Memory allocation error on nmc0!");
	}
	TrackingObject object;
	unsigned nmc0_messageAddr = halSync(4, 0);
	unsigned nmc0_objectAddr = halSync(3, 0);
	
	


	//----------------------- 1 ---------------------------
	handshake = halSync(0xC0DE0086, 1);
	if (handshake != 0xC0DE6416) {
		printf("Handshake with mc12101-nmc1 error!");
		return -1;
	}
	ok = halSync(0, 1);	// Get	status of memory allocation from nm
	if (ok != 0x600DB11F) {
		printf("Memory allocation error on nmc1!");
	}
	HalHostRingBuffer* srcRingBuffer1 = new HalHostRingBuffer;
	//HalHostRingBuffer* dstRingBuffer1 = new HalHostRingBuffer;
	//HalHostRingBuffer* outRingBuffer0 = new HalHostRingBuffer;
	unsigned nmc1_messageAddr = halSync(4, 1);
	unsigned nmc1_srcRingBufferAddr = halSync(2, 1);

	//unsigned nmc1_DstRingBufferAddr = halSync(3, 1);
	//unsigned nmc0_OutRingBufferAddr = halSync(4, 0);
	//unsigned nmc0_OutRingBufferAddr = halSync(4, 0);

	
	


	halHostRingBufferInit(srcRingBuffer1, nmc1_srcRingBufferAddr, 1);
	//halHostRingBufferInit(dstRingBuffer1, nmc1_DstRingBufferAddr, 1);
	//halHostRingBufferInit(outRingBuffer0, nmc0_OutRingBufferAddr, 0);



	//----------------------------------------------------

	nm8u* srcBlur = nmppsMalloc_8u(size);

	//VS_CreateSlider("delay", 0, 15, 0, 40, 1);
	VS_CreateSlider("wantedSize", 1, 15, 255, 1, 27);
	VS_CreateSlider("delay", 2, 0, 1000, 1, 0);


	VS_CreateImage("current Image", 1, width, height, VS_RGB24, 0);
	VS_CreateImage("current Image bw", 2, width, height, VS_RGB8, 0);
	//VS_CreateImage("productFFT_fc", 3, width, height, VS_RGB8_32, 0);
	VS_CreateImage("previous Image", 4, width, height, VS_RGB8, 0);
	//VS_CreateImage("Blur", 10, width, height, VS_RGB8_32, 0);

	nm8u* currImage = nmppsMalloc_8u(size);
	nm8u* prevImage = nmppsMalloc_8u(size);
	nm32s* blurImage = nmppsMalloc_32s(size);
	nm32s* outImage = nmppsMalloc_32s(size);

	nm32u* currImageC = nmppsMalloc_32u(size);


	S_VS_MouseStatus MouseStatus;
	int status;
	halReadMemBlock(&object, nmc0_objectAddr, sizeof32(TrackingObject), 0);
	VS_SetSlider(1, object.width);
	while (status = VS_Run()) {
		int delay = VS_GetSlider(2);


		VS_GetMouseStatus(&MouseStatus);
		if (MouseStatus.nKey == VS_MOUSE_LBUTTON) {
			object.wantedY = MouseStatus.nY;
			object.wantedX = MouseStatus.nX;
			object.width = VS_GetSlider(1);
			object.height = VS_GetSlider(1);
			halWriteMemBlock(&object, nmc0_objectAddr, sizeof32(TrackingObject), 0);
		}


		if (!(status & VS_PAUSE)) {
			memcpy(prevImage, currImage, size);
			VS_GetData(VS_SOURCE, currImageC);
			VS_GetGrayData(VS_SOURCE, currImage);
			halHostRingBufferPush(srcRingBuffer1, currImage, 1);

		}
		halSleep(delay);
		halReadMemBlock(&object, nmc0_objectAddr, sizeof32(TrackingObject), 0);
		VS_SetData(1, currImageC);
		VS_SetData(2, currImage);
		VS_SetData(4, prevImage);


		VS_Rectangle(4, object.wantedX, object.wantedY, object.wantedX + object.width, object.wantedY + object.height, VS_RED, VS_NULL_COLOR);


		//VS_SetData(10, blurImage);
		//VS_SetData(10, blurImage);

	//	VS_SetData(3, productIFFT_32s);


		VS_Rectangle(3, object.caughtX, object.caughtY, object.caughtX + object.width, object.caughtY + object.height, VS_GREEN, VS_NULL_COLOR);
		VS_Rectangle(1, object.caughtX, object.caughtY, object.caughtX + object.width, object.caughtY + object.height, VS_GREEN, VS_NULL_COLOR);
		//VS_Text("dx:%d dy:%d\r\n", caughtX - wantedX, caughtY - wantedY);

		VS_Draw(VS_DRAW_ALL);
	}
	unsigned closeMessage  = STOP;
	halWriteMemBlock(&closeMessage, nmc0_messageAddr, 1, 0);
	halWriteMemBlock(&closeMessage, nmc1_messageAddr, 1, 1);
	unsigned replyMessage0 = halSync(666, 0);
	unsigned replyMessage1 = halSync(666, 1);
	halSleep(500);
	halClose();
	return 0;
}
