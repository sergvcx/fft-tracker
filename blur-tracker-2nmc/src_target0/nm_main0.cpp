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
#include "fft_32fcr.h"
#include "tracker.h"
//#include "nmprofiler.h"


//#define SIZE_RING_BUFFER 4

//#define WIDTH 256
//#define SIZE WIDTH*WIDTH
//#define IMAGE_SIZE WIDTH*WIDTH
//struct Image32s {
//	int data[SIZE];
//};

//typedef tHalRingBuffer<Image32s, 4, 100> RingBufferImage32;

//#define SIZE_BUF			 256
//#define IMG_WIDTH			 256
//#define IMG_HEIGHT           256
//#define PTRN_WIDTH           15
//#define PTRN_HEIGHT          15


#ifdef __GNUC__
__attribute__((section(".data_imu1"))) nm32u buffer_f1[SIZE_BUF];
__attribute__((section(".data_imu2"))) nm32u buffer_f2[SIZE_BUF];
__attribute__((section(".data_imu3"))) nm32fcr buffer[SIZE_BUF];
__attribute__((section(".data_imu4"))) nm32fcr bufferF[SIZE_BUF];
__attribute__((section(".data_imu5"))) nm32fcr bufferDST[SIZE_BUF];


__attribute__((section(".data_DDR"))) nm32fcr SrcDst[IMG_WIDTH * IMG_HEIGHT];
__attribute__((section(".data_DDR"))) nm32fcr SrcDstFind[IMG_WIDTH * IMG_HEIGHT];
__attribute__((section(".data_DDR"))) nm32fcr Cor[IMG_WIDTH * IMG_HEIGHT];

__attribute__((section(".data_DDR"))) nm32u Src[IMG_WIDTH * IMG_HEIGHT];
__attribute__((section(".data_DDR"))) nm32u SrcPExt[IMG_WIDTH * IMG_HEIGHT];
__attribute__((section(".data_DDR"))) nm32u SrcP[PTRN_WIDTH * PTRN_HEIGHT];

__attribute__((section(".data_DDR"))) nm32u SrcByte[IMG_WIDTH * IMG_HEIGHT / 4];
__attribute__((section(".data_DDR"))) nm32u SrcPByte[PTRN_WIDTH * PTRN_HEIGHT / 4];

#else
#pragma data_section ".data_imu1"
//nm32s currImgBuff[SIZE_BUF];
nm32fcr bufferBank1[2][WIDTH];

#pragma data_section ".data_imu2"
nm32fcr bufferBank2[2][WIDTH];

#pragma data_section ".data_imu3"
nm32fcr bufferBank3[2][WIDTH];

#pragma data_section ".data_imu4"
//nm32fcr wantedFFTBuff[SIZE_BUF];
nm32fcr bufferBank4[2][WIDTH];
#pragma data_section ".data_imu5"

nm32fcr bufferBank5[2][WIDTH];
//nm32fcr IFFTBuff[SIZE_BUF];


#pragma data_section ".data_DDR"

nm32fcr bufferImg_fc[2][IMAGE_SIZE];	// здесь храним два последних изображения (отфильтрованных)
nm32fcr wantedImg_fc[IMAGE_SIZE];		// полное изображение (квадрат дополненный нулями)
nm32fcr currFFT_fc[IMAGE_SIZE];			// полное FFT (горизонтальное) от CurrImg_fc
nm32fcr wantedFFT_fc[IMAGE_SIZE];		// полное FFT (горизонтальное) от wantedImg_fc
nm32fcr productIFFT_fc[IMAGE_SIZE];		// полное обратное (горизонтальное) IFFT от произведения спектров


//nm32fcr currImage_fc[IMG_WIDTH * IMG_HEIGHT];
//nm32fcr prevImage_fc[IMG_WIDTH * IMG_HEIGHT];
//nm32fcr wantedImg_fc[IMG_WIDTH * IMG_HEIGHT];
//nm32fcr productFFT_fc[IMG_WIDTH * IMG_HEIGHT];

//nm32s currImage[IMG_WIDTH * IMG_HEIGHT];
//nm32s currImage_s[IMG_WIDTH * IMG_HEIGHT];
//nm32s wantedImage[IMG_WIDTH * IMG_HEIGHT];
//nm32s* wantedImage;// [IMG_WIDTH * IMG_HEIGHT];
//nm32u wantedImageExt[IMG_WIDTH * IMG_HEIGHT];
//nm32f wantedImage[PTRN_WIDTH * PTRN_HEIGHT];

//nm32u currImage_8u[IMG_WIDTH * IMG_HEIGHT / 4];
//nm32u wantedImage_8u[IMG_WIDTH * IMG_HEIGHT / 4];
//nm32fcr currImg_fc[IMG_WIDTH * IMG_HEIGHT];
//nm32fcr prevImg_fc[IMG_WIDTH * IMG_HEIGHT];

#endif

extern "C"
{
	int comp_imax(int N, nm32fcr *X, int INCX);
}



int main()
{
	//int wantedX = PTRN_WIDTH;
	//int wantedY = PTRN_WIDTH;
	//int size = width * height;
	//int wantedSize = 15; 

	halSleep(100);
	halSetProcessorNo(0);
	//---------- start nm program ------------
	int fromHost = halHostSync(0xC0DE6406);		// send handshake to host
	if (fromHost != 0xC0DE0086) {					// get  handshake from host

		return -1;
	}
	int ok = 0;

	//	halSetActiveHeap(0);
	//	int size = 0;
	//	halSetActiveHeap(1);

	//nm8u* finalImg = (nm8u*)halMalloc32(SIZE);
	//PtrnsLeft  = nmppsAddr_32s(PtrnsRight,SRC_PTRNS_SIZE_32/2); 
	//int imageSize;
	int err = 0;

	TrackingObject* object = (TrackingObject*)halMalloc32(sizeof32(TrackingObject));
	Message*     message   = (Message*)halMalloc32(sizeof32(Message));
	Message*     perf      = (Message*)halMalloc32(sizeof32(Message));

	//RingBufferTracking         *trackRingBuffer = (RingBufferTracking*)halMalloc32(sizeof32(RingBufferTracking));
	//trackRingBuffer->init(0, 0, 0);


	message->command = -1;
	message->reply = 0x6407;
	object->wantedX = 160;
	object->wantedY = 160;
	object->width = 27;
	object->height = 27;
	object->caughtX = 0;
	object->caughtY = 0;
	
	if (object==0) {
		halHostSync((int)0xDEADB00F);	// send error to host
		return -1;
	}
	else
		halHostSync(0x600DB11F);	// send ok to host

									//halHostSync((int)srcRingBufferPool);	// Send source buffer address to host//0
	
	
	

	clock_t t0, t1;
	halHostSync((int)message);
	halHostSync((int)object);
	//halHostSync((int)trackRingBuffer);
	RingBufferImage256x256_32s *blurRingBuffer = (RingBufferImage256x256_32s*)halSyncAddr(0, 1);
	

#ifdef __NM__
#define MEM_BIAS 0x40000
#else
#define MEM_BIAS 0
#endif

	NmppsFFTSpec_32fcr *spec;
	NmppsFFTSpec_32fcr *ispec;

	int st1, st2;
	st1 = 0;
	st2 = 0;

	st1 = nmppsFFT256FwdInitAlloc_32fcr(&spec);
	st2 = nmppsFFT256InvInitAlloc_32fcr(&ispec);

	halOpenDMA();

	//-------------------------------
	Image256x256_32s* blurImage;
	
	for (int i = 0; i < IMAGE_SIZE; i++) {
		wantedFFT_fc[i].re = 0;
		wantedFFT_fc[i].im = 0;
	}

	halLedOn(0);
	halSleep(1000);
	halLedOff(0);

	int counter = 0;
	while(1){
		nm32fcr* currImg_fc = bufferImg_fc[counter & 1];
		nm32fcr* prevImg_fc = bufferImg_fc[(counter+1) & 1];
		counter++;

		//t0 = clock();
		while (blurRingBuffer->isEmpty() ) {
			halSleep(2);
			if (message->command == STOP) break;
		}
		blurImage = blurRingBuffer->getTail();
		
		halLedOn(0);
		
		//------------ FFT of current image  ----------------
		nm32s*   currImgStr    = (nm32s*)bufferBank1[0];
		nm32fcr* currImgStr_fc = bufferBank2[0];
		nm32fcr* currFFTStr_fc = bufferBank1[0];
		
		for (int k = 0; k < IMAGE_SIZE; k += WIDTH) {
			halInitStatusSingleDMA(blurImage->data + k, (void*)((int)currImgStr + MEM_BIAS), WIDTH);
			while (halStatusDMA());

			nmppsConvert_32s32fcr(currImgStr, currImgStr_fc, WIDTH);

			halInitStatusSingleDMA((void*)((int)currImgStr_fc + MEM_BIAS), currImg_fc + k, WIDTH <<1);  // INT => DDR
			while (halStatusDMA());

			nmppsFFT256Fwd_32fcr(currImgStr_fc, currFFTStr_fc, spec);

			halInitStatusSingleDMA((void*)((int)currFFTStr_fc + MEM_BIAS), (void*)(currFFT_fc + k), WIDTH <<1);
			while (halStatusDMA());
		}

		halLedOn(1);
		
		//-------- FFT of wanted fragment ------------------
		nm32fcr* wantedImgStr_fc = bufferBank1[1]; // private buffer
		nm32fcr* wantedFFTStr_fc = bufferBank2[0];
		for (int i = 0; i < WIDTH; i++) {
			wantedImgStr_fc[i].re = 0;
			wantedImgStr_fc[i].im = 0;
		}

		for (int k = 0; k < WIDTH*object->height; k += WIDTH) {
			halInitStatusSingleDMA((void*)(prevImg_fc + object->wantedY*WIDTH + object->wantedX + k), (void*)((int)wantedImgStr_fc + MEM_BIAS), object->width<<1);
			while (halStatusDMA());

			nmppsFFT256Fwd_32fcr(wantedImgStr_fc, wantedFFTStr_fc, spec);

			halInitStatusSingleDMA((void*)((int)wantedFFTStr_fc + MEM_BIAS), wantedFFT_fc + k , WIDTH <<1);
			while (halStatusDMA());
		}
		halLedOn(2);
		// ---------- 2FFT & IFFT ---------------------------
		         currFFTStr_fc    = bufferBank1[0];
		nm32fcr* curr2FFTStr_fc   = bufferBank2[0];
		         wantedFFTStr_fc  = bufferBank1[0];
		nm32fcr* wanted2FFTStr_fc = bufferBank3[0];
		nm32fcr* product2FFTStr_fc= bufferBank1[0];
		nm32fcr* productIFFTStr_fc= bufferBank2[0];
	
		int offs = 0;
		for (int k = 0; k < WIDTH; k++) {
			halInitMatrixDMA((void*)(currFFT_fc + k), 2, HEIGHT, 2 * WIDTH, (void*)((int)currFFTStr_fc + MEM_BIAS), 2);
			while (halStatusDMA());

			nmppsFFT256Fwd_32fcr(currFFTStr_fc, curr2FFTStr_fc, spec);

			halInitMatrixDMA((void*)(wantedFFT_fc + k), 2, HEIGHT, 2 * WIDTH, (void*)((int)wantedFFTStr_fc + MEM_BIAS), 2);
			while (halStatusDMA());
			
			nmppsFFT256Fwd_32fcr(wantedFFTStr_fc, wanted2FFTStr_fc, spec);

			nmppsConjMul_32fcr(curr2FFTStr_fc, wanted2FFTStr_fc, product2FFTStr_fc, WIDTH);
			nmppsFFT256Inv_32fcr(product2FFTStr_fc, productIFFTStr_fc, ispec);

			halInitStatusSingleDMA((void*)((int)productIFFTStr_fc + MEM_BIAS), productIFFT_fc + offs , WIDTH <<1);
			while (halStatusDMA());

			offs += HEIGHT;
		}
		halLedOn(3);
		// --------- 2IFFT & max index search ------------- 

		         productIFFTStr_fc  = bufferBank1[0];
		nm32fcr* productI2FFTStr_fc = bufferBank2[0];

		
		int   maxIndx;
		float maxVal = 0;
		
		for (int k = 0; k < WIDTH; k++) {
			halInitMatrixDMA((void*)(productIFFT_fc + k), 2, HEIGHT, 2 * WIDTH, (void*)((int)productIFFTStr_fc + MEM_BIAS), 2);
			while (halStatusDMA());

			nmppsFFT256Inv_32fcr(productIFFTStr_fc, productI2FFTStr_fc, ispec);
			maxIndx = comp_imax(WIDTH, productI2FFTStr_fc, 1);
			if (maxVal < productI2FFTStr_fc[maxIndx].re) {
				maxVal = productI2FFTStr_fc[maxIndx].re;
				object->caughtY = k;
				object->caughtX = maxIndx;
			}
		}
		//--------------------------------------------------
		blurRingBuffer->tail++;
		object->wantedY = object->caughtY;
		object->wantedX = object->caughtX;
		
		halLedOff(0);
		halLedOff(1);
		halLedOff(2);
		halLedOff(3);

		
		if (message->command == STOP)
			break;
		//t1 = clock();

	}

	//message->reply = STOPPED;
	halLedOn(0);
	halHostSync(777);
	return 0;
}
