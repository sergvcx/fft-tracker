
//////////////////////////////////
// fft-tracker                  //
// Copyright (c) RC Module Inc. //
// Author : A. Brodyazhenko     //
// Year 2017					//
//////////////////////////////////

#include "dma.h"
#include "cache.h"
#include "hal_target.h"
#include "hal.h"
#include "nmpp.h"
#include "nmtype.h"
#include "fft_32fcr.h"
//#include <stdio.h>

#define SIZE_BUF			 256

#define IMG_WIDTH			 256
#define IMG_HEIGHT           256
#define PTRN_WIDTH           10
#define PTRN_HEIGHT          10


//#ifdef __GNUC__
__attribute__((section(".data.imu1"))) nm32f currImgBuff[SIZE_BUF];
__attribute__((section(".data.imu2"))) nm32f wantedImgBuff[SIZE_BUF];
__attribute__((section(".data.imu3"))) nm32fcr currFFTBuff[SIZE_BUF];
__attribute__((section(".data.imu4"))) nm32fcr wantedFFTBuff[SIZE_BUF];
__attribute__((section(".data.imu5"))) nm32fcr IFFTBuff[SIZE_BUF];


__attribute__((section(".data.emi"))) nm32fcr currImage_fc[IMG_WIDTH * IMG_HEIGHT];
__attribute__((section(".data.emi"))) nm32fcr wantedImage_fc[IMG_WIDTH * IMG_HEIGHT];
__attribute__((section(".data.emi"))) nm32fcr productFFT_fc[IMG_WIDTH * IMG_HEIGHT];

__attribute__((section(".data.emi"))) nm32u currImage[IMG_WIDTH * IMG_HEIGHT];
__attribute__((section(".data.emi"))) nm32u wantedImage[IMG_WIDTH * IMG_HEIGHT];

__attribute__((section(".data.emi"))) nm32u currImage_8u[IMG_WIDTH * IMG_HEIGHT / 4];
__attribute__((section(".data.emi"))) nm32u wantedImage_8u[PTRN_WIDTH * PTRN_HEIGHT / 4];


int  nmppmCopyRisc_32x(void*  src, int  width, int  height, int srcStride32, void* dst, int dstStride32) {
	int i, j;
	int* ptr2src = (int*)src;
	int* ptr2dst = (int*)dst;
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			ptr2dst[j] = ptr2src[j];
		}
		ptr2dst += dstStride32;
		ptr2src += srcStride32;
	}
	return 0;
};

void nmppsCopyRisc_32x(const void* pSrcVec, void* pDstVec, int nSize){
	nmppsCopyRisc_32f((const float*)pSrcVec, (float*) pDstVec, nSize);
}

void*  halCopyDMA(const void* src, void* dst, unsigned size32) {
	halDmaStart((void*)src, dst, size32);
	while (!halDmaIsCompleted()) {
		//halSleep(10);
	}
	return dst;
}

//halDma2D_StartC(in, out, width*height, width, srcStride, dstStride);
//int  halInitStatusMatrixDMA(void* src, int width, int height, int srcStride32, void* dst, int dstStride32);

int  halCopyDMA2D(void* src, int width, int height, int srcStride32, void* dst, int dstStride32) {

	halDma2D_Start(src, dst, width * height, width, srcStride32, dstStride32);
	while (!halDma2D_IsCompleted());

}



/*
#else
 #pragma data_section ".data_imu1"
 	nm32f currImgBuff[SIZE_BUF];
 #pragma data_section ".data_imu2"
 	nm32f wantedImgBuff[SIZE_BUF];
 #pragma data_section ".data_imu3"
 	nm32fcr currFFTBuff[SIZE_BUF];
 #pragma data_section ".data_imu4"
 	nm32fcr wantedFFTBuff[SIZE_BUF];
 #pragma data_section ".data_imu5"
 	nm32fcr IFFTBuff[SIZE_BUF];


 #pragma data_section ".data_DDR"
	nm32fcr currImage_fc[IMG_WIDTH * IMG_HEIGHT];
	nm32fcr wantedImage_fc[IMG_WIDTH * IMG_HEIGHT];
	nm32fcr productFFT_fc[IMG_WIDTH * IMG_HEIGHT];

	nm32f currImage[IMG_WIDTH * IMG_HEIGHT];
	nm32f wantedImage[IMG_WIDTH * IMG_HEIGHT];
	//nm32u wantedImageExt[IMG_WIDTH * IMG_HEIGHT];
	//nm32f wantedImage[PTRN_WIDTH * PTRN_HEIGHT];

	nm32u currImage_8u[IMG_WIDTH * IMG_HEIGHT / 4];
	nm32u wantedImage_8u[IMG_WIDTH * IMG_HEIGHT / 4];
#endif
	*/
//int blur(nm32u *img, int width, int height, int size, int x, int y) {
//	int halfsize = size / 2;
//	int sum = 0;
//	int k = 0;
//	for (int i = MAX(0, y - halfsize); i <= MIN(height - 1, y + halfsize); i++) {
//		for (int j = MAX(0, x - halfsize); j <= MIN(width - 1, x + halfsize); j++) {
//			sum += img[i*width + j];
//			k++;
//		}
//	}
//	sum /= k;
//	return sum;
//}

extern "C"
{
	int comp_imax(int N, nm32fcr *X, int INCX);
}

extern "C" {
	int user_callback()
	{
		//index = 999;
		return 777;
	}
};


int main()
{
	printf("Hello from nmc0\n");
	int width = IMG_WIDTH;
	int height = IMG_WIDTH;
	int wantedX = PTRN_WIDTH;
	int wantedY = PTRN_WIDTH;
	int size = width * height;
	int wantedSize = wantedX * wantedY;

	//---------- start nm program ------------

	int fromHost = halHostSync(0xC0DE6407);		// 0 send handshake to host
	if (fromHost != 0xC0DE0086) {					// get handshake from host
		return -1;
	}

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

	//halHostSync((int)currImage); //2
	//halHostSync((int)wantedImage); //3
	
	//nm32f* currImage =   (nm32f*)halMalloc32(size);
	//nm32f* wantedImage = (nm32f*)halMalloc32(size);

	halHostSyncAddr(currImage);
	halHostSyncAddr(wantedImage);
	//for(int i = 0; i < size; i++) {
	//	wantedImageExt[i] = 0;
	//}

	halOpenDMA();
	halSetCallbackDMA(user_callback);
	halInstrCacheEnable();
	halHostSync(0xA); //2
	while(1) {			// Start
		wantedSize = halHostSync(0xA); //2

		int offs = 0;

		//nmppsConvertRisc_8u32u((nm8u *)currImage_8u, currImage, size);
		//nmppsConvertRisc_8u32u((nm8u *)wantedImage_8u, wantedImage, size);
		/*for(int i = 0; i < wantedSize; i += wantedX) {
			halInitStatusSingleDMA((nm32u*)(wantedImage + i), (nm32u*)(wantedImageExt + offs), wantedX);
			while(halStatusDMA());
			offs += width;
		}
		offs = 0;*/
//		halHostSync(0xA); //2

		//for (int i = 0; i < size; i++) {
		//	currImage_s[i] = nm32s(currImage[i]) - blur(currImage, width, height, wantedSize, i%width, i / width);
		//}

/////////////////////////////FFT2D_512(2  )///////////////////////////////////////////
		for(int k = 0; k < size; k += width) {
			//halInitStatusSingleDMA((nm32f*)(currImage + k), (nm32f*)((size_t)currImgBuff + MEM_BIAS), width);
			//halInitSingleDMA((nm32f*)(currImage + k), (nm32f*)((size_t)currImgBuff + MEM_BIAS), width);
			//while(halStatusDMA());
			//nmppsCopyRisc_32f((nm32f*)(currImage + k), (nm32f*)((size_t)currImgBuff + MEM_BIAS), width);
			halCopyDMA((nm32f*)(currImage + k), (nm32f*)((size_t)currImgBuff + MEM_BIAS), width);

			nmppsConvert_32f32fcr(currImgBuff, currFFTBuff, width);
			
			nmppsFFT256Fwd_32fcr(currFFTBuff, currFFTBuff, spec);

			//halInitStatusSingleDMA((nm32f*)(wantedImage + k), (nm32f*)((size_t)wantedImgBuff + MEM_BIAS), width);
			//while(halStatusDMA());
			//nmppsCopyRisc_32f((nm32f*)(wantedImage + k), (nm32f*)((size_t)wantedImgBuff + MEM_BIAS), width);
			halCopyDMA((nm32f*)(wantedImage + k), (nm32f*)((size_t)wantedImgBuff + MEM_BIAS), width);

			//halInitStatusSingleDMA((nm32fcr*)((size_t)currFFTBuff + MEM_BIAS), (nm32fcr*)(currImage_fc + k), 2 * width);
			//while(halStatusDMA());
			//nmppsCopyRisc_32f((nm32f*)((size_t)currFFTBuff + MEM_BIAS), (nm32f*)(currImage_fc + k), 2 * width);
			halCopyDMA((nm32f*)((size_t)currFFTBuff + MEM_BIAS), (nm32f*)(currImage_fc + k), 2 * width);

			nmppsConvert_32f32fcr(wantedImgBuff, wantedFFTBuff, width);

			nmppsFFT256Fwd_32fcr(wantedFFTBuff, wantedFFTBuff, spec);


			halCopyDMA((nm32fcr*)((size_t)wantedFFTBuff + MEM_BIAS), (nm32fcr*)(wantedImage_fc + k), 2 * width);
			//nmppsCopyRisc_32x((nm32fcr*)((size_t)wantedFFTBuff + MEM_BIAS), (nm32fcr*)(wantedImage_fc + k), 2 * width);
			//halInitStatusSingleDMA((nm32fcr*)((size_t)wantedFFTBuff + MEM_BIAS), (nm32fcr*)(wantedImage_fc + k), 2 * width);
			//while(halStatusDMA());
			

		}

		if (1) for(int k = 0; k < width; k++) {
			//nmppmCopyRisc_32x((nm32fcr*)(currImage_fc + k), 2, height, 2 * width, (nm32fcr*)((size_t)currFFTBuff + MEM_BIAS), 2);
			halCopyDMA2D((nm32fcr*)(currImage_fc + k), 2, height, 2 * width, (nm32fcr*)((size_t)currFFTBuff + MEM_BIAS), 2);
			//halDma2D_StartC(in, out, width*height, width, srcStride, dstStride);

			//halInitStatusMatrixDMA((nm32fcr*)(currImage_fc + k), 2, height, 2 * width, (nm32fcr*)((size_t)currFFTBuff + MEM_BIAS), 2);
			//while(halStatusDMA());

			//nmppmCopyRisc_32x((nm32fcr*)(wantedImage_fc + k), 2, height, 2 * width, (nm32fcr*)((size_t)wantedFFTBuff + MEM_BIAS), 2);
			halCopyDMA2D((nm32fcr*)(wantedImage_fc + k), 2, height, 2 * width, (nm32fcr*)((size_t)wantedFFTBuff + MEM_BIAS), 2);
			//halInitStatusMatrixDMA((nm32fcr*)(wantedImage_fc + k), 2, height, 2 * width, (nm32fcr*)((size_t)wantedFFTBuff + MEM_BIAS), 2);
			//while(halStatusDMA());

			nmppsFFT256Fwd_32fcr(currFFTBuff, currFFTBuff, spec);
			
			nmppsFFT256Fwd_32fcr(wantedFFTBuff, wantedFFTBuff, spec);

			nmppsConjMul_32fcr(currFFTBuff, wantedFFTBuff, IFFTBuff, width);
			nmppsFFT256Inv_32fcr(IFFTBuff, IFFTBuff, ispec);

			
			//nmppsCopyRisc_32x((nm32fcr*)((size_t)IFFTBuff + MEM_BIAS), (nm32fcr*)(productFFT_fc + offs), 2 * width);
			halCopyDMA((nm32fcr*)((size_t)IFFTBuff + MEM_BIAS), (nm32fcr*)(productFFT_fc + offs), 2 * width);
			//halInitStatusSingleDMA((nm32fcr*)((size_t)IFFTBuff + MEM_BIAS), (nm32fcr*)(productFFT_fc + offs), 2 * width);
			//while(halStatusDMA());

			offs += width;
		}
		offs = 0;
		int max_indx;
		int max1 = 0;
		int index = 0;
		double max_val = 0;
		for(int k = 0; k < width; k++) {
			//nmppmCopyRisc_32x((nm32fcr*)(productFFT_fc + k), 2, height, 2 * width, (nm32fcr*)((size_t)IFFTBuff + MEM_BIAS), 2);
			halCopyDMA2D((nm32fcr*)(productFFT_fc + k), 2, height, 2 * width, (nm32fcr*)((size_t)IFFTBuff + MEM_BIAS), 2);
			//halInitStatusMatrixDMA((nm32fcr*)(productFFT_fc + k), 2, height, 2 * width, (nm32fcr*)((size_t)IFFTBuff + MEM_BIAS), 2);
			//while(halStatusDMA());
					
			nmppsFFT256Inv_32fcr(IFFTBuff, IFFTBuff, ispec);
			max_indx = comp_imax(width, IFFTBuff, 1);
			if(max_val < IFFTBuff[max_indx].re) {
				max_val = IFFTBuff[max_indx].re;
				index = k;
				max1 = max_indx;
			}
		}
		halHostSync(max1); // Send x
		halHostSync(index); // Send y
	}
	return 1; 
}
