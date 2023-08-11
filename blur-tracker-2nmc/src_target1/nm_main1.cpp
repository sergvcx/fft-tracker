//#include "malloc32.h"
#include <time.h>
#include "hal_target.h"
#include "hal.h"
#include "dma.h"
#include "led.h"
#include "nmpp.h"
#include "stdio.h"
#include "cache.h"
#include "ringbuffer.h"
#include "tringbuffer.h"
#include "nmpli/iFilter.h"
#include "tracker.h"
//#include "nmprofiler.h"



int main()
{  
	halSleep(100);
	halSetProcessorNo(1);	
	//---------- start nm program ------------
	int fromHost=halHostSync(0xC0DE6416);		// send handshake to host
	if (fromHost!=0xC0DE0086){					// get  handshake from host
		
		return -1;
	}
	int ok=0;

//	halSetActiveHeap(0);
//	int size = 0;
//	halSetActiveHeap(1);
	
	//nm8u* finalImg = (nm8u*)halMalloc32(SIZE);
	//PtrnsLeft  = nmppsAddr_32s(PtrnsRight,SRC_PTRNS_SIZE_32/2); 
	//int imageSize;
	int err = 0;
	
	Message*       message = (Message*)halMalloc32(sizeof32(Message));
	message->command = -1;
	message->reply = 0x6417;

	nm8u*          srcRingBufferPool = (nm8u*)halMalloc32(SIZE / 4 * SIZE_RING_BUFFER);
	HalRingBuffer* srcRingBuffer = (HalRingBuffer*)halMalloc32(sizeof32(HalRingBuffer));
	err = halRingBufferInit(srcRingBuffer, srcRingBufferPool, SIZE/4, SIZE_RING_BUFFER, 0, 0, 0);
	
	//nm32s*         blurRingBufferPool = (nm32s*) halMalloc32(SIZE*SIZE_RING_BUFFER);
	//HalRingBuffer* blurRingBuffer     = (HalRingBuffer*)halMalloc32(sizeof(HalRingBuffer));
	//err += halRingBufferInit(blurRingBuffer, blurRingBufferPool, SIZE, SIZE_RING_BUFFER, 0, 0, 0);

	RingBufferImage256x256_32s *blurRingBuffer = (RingBufferImage256x256_32s*) halMalloc32(sizeof32(RingBufferImage256x256_32s));
	blurRingBuffer->init(0,0,0);

	// Check memory allocation
	if (blurRingBuffer ==0 || err){
		halHostSync((int)0xDEADB00F);	// send error to host
		return -1;
	}
	else 
		halHostSync(0x600DB11F);	// send ok to host

	halHostSync((int)message);
	halHostSync((int)srcRingBuffer);		
	
	//halHostSync((int)blurRingBufferPool);	
	//halHostSync((int)blurRingBuffer);		
     
	halSyncAddr(blurRingBuffer, 0);

	clock_t t0,t1;
	int widthImg = 1;// WIDTH_IMAGE;
	int counter=0;
	//halHostSync((int)0xbabadeda);
	

	//halInitSingleDMA(dydx_table, dydx_bank0, 2*WIDTH_PTRN*HEIGHT_PTRN);
	//DMA_STATUS
		//-------------init-all--------
	
	
	//nmppsSet_8s((nm8s*)overlap_Y_null, BACKGROUND, OVERLAP_SEG*WIDTH_SEG * 4);
	//nmppsSet_16s((nm16s*)overlap_Y_ZBuff_null, 0   , OVERLAP_SEG*WIDTH_SEG * 4);
	//halHostSync(0x600D600D);
	int blurWeights[256]; 
	for (int i = 0; i < 256;i ++)
		blurWeights[i] =-1;
	int blurSize = 15;
	blurWeights[blurSize*blurSize / 2] =  blurSize*blurSize-1;
	int blurKernelSize = IMG_GetFilterKernelSize32_8s32s(blurSize, blurSize);
	nm64s* blurKernel = (nm64s*)nmppsMalloc_32s(blurKernelSize);

	IMG_SetFilter_8s32s(blurWeights, blurSize, blurSize,256, blurKernel);

	nm8s* signedImageExtra = nmppsMalloc_8s(SIZE+WIDTH*16*2);
	nm8s* signedImage =nmppsAddr_8s(signedImageExtra, WIDTH * 16);
	nm32s* signedImage32 = nmppsMalloc_32s(SIZE);

	halLedOn(4);
	halSleep(100);
	halLedOff(4);

//-------------------------------
	while(1){
		//t0=clock();
		while (halRingBufferIsEmpty(srcRingBuffer) || blurRingBuffer->isFull() ) {
			halSleep(2);
			if (message->command == STOP) break;
		}
		//while (halRingBufferIsBusy(infRingBuffer)) {
		//	halSleep(2);
		//}
		/*while (halRingBufferIsFull(blurRingBuffer)) {
			halSleep(2);
		}

		while (blurRingBuffer->isFull()) {
			halSleep(2);
		}
*/
		

		nm8u*  currImage = (nm8u*) halRingBufferTail(srcRingBuffer);
		//nm32s* blurImage = (nm32s*)halRingBufferHead(blurRingBuffer);
		Image256x256_32s* blurImage32 = blurRingBuffer->getHead();
		
		halLedOn(5);
		
	
		nmppsSubC_8s((nm8s*)currImage, 127, signedImage, SIZE);
		//nmppsConvert_8s32s(signedImage, signedImage32, SIZE);
		//nmppsConvert_8s32s(signedImage, blurImage32->data, SIZE);
		//IMG_Filter((nm8s*)signedImage, (nm32s*)blurImage32, 256,256, blurKernel);
		IMG_Filter((nm8s*)signedImage, blurImage32->data, 256, 256, blurKernel);
		nmppsRShiftC_32s(blurImage32->data, 7, blurImage32->data, SIZE);
		//nmppsAddC_32s(blurImage, 8, blurImage, SIZE);
		//nmppsSub_32s(signedImage32,blurImage, blurImage, SIZE);
		//nmppsConvert_8u32u(currImage, blurImage, SIZE);
		srcRingBuffer->tail++;
		blurRingBuffer->head++;


		halLedOn(6);

		//nmprofiler_print2tbl();
		//t1=clock();
		//printf("ammount=%d, time=%u\n", inf->allAmount, t1 - t0);

		//dstRingBuffer->head++;
		counter++;
		halLedOff(4);
		halLedOff(5);
		halLedOff(6);
		halLedOff(7);

		if (message->command == STOP)
			break;

	}
	//halDisExtInt();
	//message->reply = STOPPED;
	halLedOn(4);
	halHostSync(777);
	
	//halSleep(1000);
	return 0;
} 
