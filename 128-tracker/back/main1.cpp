﻿#include "nmtype.h"
#include "dtp/dtp.h"
#include "dtp/file.h"
#include "hal/ringbuffert.h"
#include "hal/cache.h"
#include "dtp/mc12101.h"
#include "mc12101load_nm.h"
#include "nmpp.h"
#include "string.h"
#include "dumpx.h"
#include "tracker.h"
#include "nmassert.h"
#include "vsimg.h"

Cmd_x86_to_nm1 cmdIn;
Cmd_nm1_to_nm0 cmdOut = { 0,0 };
 

__attribute__((section(".data.imu3"))) 	int ringBufferLo[DIM*DIM*2];
__attribute__((section(".data.imu1"))) 	int ringBufferHi[DIM*DIM*2];
//__attribute__((section(".data.imu2"))) 	int ringBufferHi2[DIM*DIM* 2];

#define FULL_BANK 32*1024 // 128kB
#define PRINT 
//printf
//__attribute__((section(".data.imu3"))) int x86_to_nm1_buffer[FULL_BANK];


static void* memCopyPop(const void *src, void *dst, unsigned int size32) {
	//if ((int)src & 1 || (int)dst & 1 || size32 & 1)
	//	printf("error\n");
	//printf("%08x %08x %8d\n", src, dst, size32);
	if (size32&1 || size32<4)
		memcpy(dst, src, size32 * sizeof(int));
	else {
		memcpy(dst, src, size32 * sizeof(int));
		//nmppsCopy_32s((nm32s*)src,(nm32s*) dst, size32);
		//halDmaStart(src, dst, size32);
		
		//while (!halDmaIsCompleted());
	}
	return 0;
}
static void* memCopyPush(const void *src, void *dst, unsigned int size32) {
	//memcpy(dst, src, size32 * sizeof(int));
	if ((int)src & 1 || (int)dst & 1 || size32 & 1)
		printf("error\n");
	nmppsCopy_32s((nm32s*)src,(nm32s*) dst, size32);
	return 0;
}

extern "C" {
	void mdelay(int);

};
//}

#define VS_SAVE_IMAGE 
//vsSaveImage
#define USE_SEMIHOSTING 1
#define PRINT0(a)
#define PRINT1(a) 
#define PRINT2(a,b) 
#define PRINT3(a,b,c) 
#define PRINT4(a,b,c,d) 
#define PRINT5(a,b,c,d,e) 
#define PRINT6(a,b,c,d,f,g) 
//#define PRINT 
//#define PRINT0				
//
//#define PRINT0				printf
//#define PRINT1(a) 				printf(a) 
//#define PRINT2(a,b) 			printf(a,b) 
//#define PRINT3(a,b,c) 			printf(a,b,c) 
//#define PRINT4(a,b,c,d) 		printf(a,b,c,d) 
//#define PRINT5(a,b,c,d,e) 		printf(a,b,c,d,e) 
//#define PRINT6(a,b,c,d,f,g) 	printf(a,b,c,d,f,g) 
//

//printf

#define FILE "../exchange.bin"
int blurWeights[16 * 16];
__attribute__((section(".text.nmpp")))
int main(){
	//halSleep(1000);d
	//s
	halInstrCacheEnable();
	if (USE_SEMIHOSTING) printf("nmc1 started\n");
	HalRingBufferData<int, 2>* ring[6];
	
	if (USE_SEMIHOSTING) {
		int file_desc = 0;
		do {
			file_desc = dtpOpenFile(FILE, "rb");
		} while (file_desc < 0);
		dtpRecv(file_desc, ring, 6);
		dtpClose(file_desc);
	}
	else {
		ring[0]=0x000a8184;// data : 000a8020 size : 256 id : 8601cdcd
		ring[1]=0x000a8170;// data : 20022286 size : 8388608 id : 8601b00f
		ring[2]=0x000a815c;// data : 20012286 size : 65536 id : 0186b00f
		ring[3]=0x000a8148;// data : 000a8000 size : 32 id : 0100cdcd
		ring[4]=0x000a8134;// data : 00070000 size : 32768 id : 0100deef
		ring[5]=0x000a8120;// data : 00078000 size : 32768 id : 0001beef
	}

	HalRingBufferData<int, 2>* ring_x86_to_nm1_cmd = ring[0];
	HalRingBufferData<int, 2>* ring_x86_to_nm1_img = ring[1];
	//HalRingBufferData<int, 2>* ring_nm1_to_x86_out = ring[2];
	HalRingBufferData<int, 2>* ring_nm1_to_nm0_cmd = ring[3];
	HalRingBufferData<int, 2>* ring_nm1_to_nm0_diff= ring[4];
	//HalRingBufferData<int, 2>* ring_nm0_to_nm1_corr= ring[5];



		
	//ring_x86_to_nm1_img->data = (int*)ringBufferHi;
	//ring_x86_to_nm1_img->init(32 * 1024);
	if (USE_SEMIHOSTING)
	for (int i = 0; i < 6; i++)
		PRINT6("%d: ring:%08x data:%08x size:%8d id:%08x\n", i, (int)ring[i], (int)ring[i]->data, ring[i]->size, ring[i]->bufferId);
	
	halEnbExtInt();
	halDmaInit();



	
	// -------------------------------------------------------------
	int rbCmdToNm1=dtpOpenRingbufferDefault(ring_x86_to_nm1_cmd);
	int rbCmdToNm0=dtpOpenRingbufferDefault(ring_nm1_to_nm0_cmd);
	//int rbImg=dtpOpenRingbuffer(ring_x86_to_nm1_img, memCopyPush, memCopyPop);
	

	for (int i = 0; i < 256; i++)
		blurWeights[i] = -1;
	int blurSize = 15;

	blurWeights[blurSize*blurSize / 2] = blurSize * blurSize - 1;
	int blurKernelSize = nmppiGetFilterKernelSize32_8s32s(blurSize, blurSize);
	nm64s* blurKernel = (nm64s*)nmppsMalloc_32s(blurKernelSize);
	NMASSERT(blurKernel);
	nmppiSetFilter_8s32s(blurWeights, blurSize, blurSize, DIM, blurKernel);


	
	cmdOut.command = 0x6407600D;
	cmdOut.counter++;
	PRINT0("Sending hanshake to nm1 ... \n");
	dtpSend(rbCmdToNm0, &cmdOut, sizeof32(cmdOut));
	PRINT0("Waiting hanshake from x86 ... \n");
	dtpRecv(rbCmdToNm1, &cmdIn, sizeof32(cmdIn));
	if (cmdIn.command == 0x64078086)
		PRINT0("Handshake ok. Working ... \n");
	else {
		PRINT2("Handshake error %x\n", cmdIn.command);
		return -1;
	}
	
	
	NmppSize imgDim = cmdIn.frmSize;
	int imgSize32 = imgDim.height*imgDim.width/4;
	PRINT2("imfSize32 =%d\n", imgSize32);
	PRINT2("width     =%d\n", cmdIn.frmSize.width);
	PRINT2("height    =%d\n", cmdIn.frmSize.height);
	PRINT2("roi.width =%d\n", cmdIn.frmRoi.width);
	PRINT2("roi.height=%d\n", cmdIn.frmRoi.height);
	PRINT2("roi.x     =%d\n", cmdIn.frmRoi.x);
	PRINT2("roi.y     =%d\n", cmdIn.frmRoi.y);

	//int handshake[8];
	//dtpRecv(rbImg, handshake, 8);
	//for (int i = 0; i < 8; i++)
	//	printf("% d\n", handshake[i]);
	
	PRINT0("Starting nm1 ... \n");


	

	while (1) {
		//printf("<<< 1:\n");
		dtpRecv(rbCmdToNm1, &cmdIn, sizeof32(cmdIn));
		//PRINT4("--------- [in] cnt:%d cmd:0x%x frmIndex:%d------ \n", cmdIn.counter, cmdIn.command, cmdIn.frmIndex);

		if (cmdIn.command == DO_FFT0 || cmdIn.command == DO_FFT1) {

			while (cmdIn.frmIndex >= ring_x86_to_nm1_img->head ) {
				PRINT3("ring_x86_to_nm1_img: head:%d tail:%d\n", ring_x86_to_nm1_img->head, ring_x86_to_nm1_img->tail);
			}

			//cmdIn.frmIndex*imgSize32 +
			//int srcPos8 = cmdIn.frmRoi.y*imgDim.width + cmdIn.frmRoi.x;
			//int src
			//int srcDma=
			//if (cmdIn.frmRoi.x)
			int *roi = ring_x86_to_nm1_img->data +cmdIn.frmIndex*imgSize32 + cmdIn.frmRoi.y*imgDim.width / 4 + cmdIn.frmRoi.x / 4;

			nm8s* blurRoi8s = (nm8s*)ringBufferLo;
			//nm32s* imgAddr =
			nm32s * imgAddr = ring_x86_to_nm1_img->data + cmdIn.frmIndex*imgSize32;
			NMASSERT((int)imgAddr & 0x0F == 0);
			int roi8Pos = cmdIn.frmRoi.y*imgDim.width  + cmdIn.frmRoi.x;	// byte position 
			int roi32PosBase = (roi8Pos / 4) & 0xFFFF0;						// 16x-aligned word position
			int roi32PosDisp = roi8Pos - roi32PosBase * 4;					// byte offset from aligned 
			int roi32EndBase = (cmdIn.frmRoi.y*imgDim.width + cmdIn.frmRoi.x + cmdIn.frmRoi.width) / 4; // right out off roi word position
			roi32EndBase = (roi32EndBase + 15)0xFFF0; // 16-x aligned word position of out off roi

		//		NmppRect srcDma;
	//		srcDma.
//

				//, "srcAdrres not aligned");
			if (cmdIn.command == DO_FFT0)
				nmppsSet_8s(0, blurRoi8s, DIM*DIM);
			//halDma2D_Start(roi, blurRoi8s, cmdIn.frmRoi.height*cmdIn.frmRoi.width / 4, cmdIn.frmRoi.width / 4, cmdIn.frmSize.width / 4, DIM/4);
			
			halDma2D_Start(roiPos, blurRoi8s, cmdIn.frmRoi.height*cmdIn.frmRoi.width / 4, cmdIn.frmRoi.width / 4, cmdIn.frmSize.width / 4, DIM/4);
			
			//mdelay(10);
			while (!halDma2D_IsCompleted());// {
			//	printf(".");
			//};
			
			//if (cmdIn.command == DO_FFT0)
			//	nmppsSet_8s(0, (nm8s*)ringBufferHi, DIM*DIM);
			//

			//nm8s* src = (nm8s*)roi;
			//nm8s* dst = (nm8s*)blurRoi8s;
			//for (int y = 0; y < cmdIn.frmRoi.height; y++) {
			//	memcpy(dst, src, cmdIn.frmRoi.width/ 4);
			//	src = nmppsAddr_8s(src, cmdIn.frmSize.width);
			//	dst = nmppsAddr_8s(dst, DIM);
			//}
			
			//for (int i = 0; i < DIM*DIM; i++) {
			//	if (ringBufferHi[i] != ringBufferLo[i])
			//		printf("DMA error: dma[%d]=%08x mcpy[%d]=08%x\n", i, ringBufferLo[i], i, ringBufferHi[i]);
			//}
			

			
			if (cmdIn.command == DO_FFT0 )	
				VS_SAVE_IMAGE("1_FFT0_8s.vsimg", blurRoi8s, DIM, DIM, VS_RGB8_8);
			else				
				VS_SAVE_IMAGE("1_FFT1_8s.vsimg", blurRoi8s, DIM, DIM, VS_RGB8_8);
			int *tail=ring_x86_to_nm1_img->ptrTail();
			//memcpy(ringBufferLo, roi, DIM*DIM / 4);
			//printf("%x %x %d\n", roi, tail, cmdIn.frmIndex);
			
			//dtpRecv(rbImg, ringBufferLo, DIM*DIM/4);
			//PRINT("in: recv ok\n");

			//ring_x86_to_nm1_img->tail+=DIM*DIM/4;
			//nm8u* img8u= (nm8u*)ringBufferLo;
			//nm8s* img8s= (nm8s*)ringBufferLo;
			
		//if (cmdIn.command== DO_FFT0)
		//	vsSaveImage("fft0_in8s.img", blurRoi8s, DIM, DIM, VS_RGB8_8);
		//else 
		//	vsSaveImage("fft1_in8s.img", blurRoi8s, DIM, DIM, VS_RGB8_8);
		
		//	//nmppsSet_8u(0, img8u,DIM*DIM);
		//	//---------------------------------------
		//	nmppsSubC_8s((nm8s*)img8u, 127, img8s, DIM*DIM);
		//if (cmdIn.command == DO_FFT0)
		//	vsSaveImage("fft0_in8s.img", img8s, DIM, DIM, VS_RGB8_8);
		//else	
		//	vsSaveImage("fft1_in8s.img", img8s, DIM, DIM, VS_RGB8_8);
		//	nmppiFilter_8s32s(img8s, blurImage32s, DIM, DIM, blurKernel);
		//	
		//	//nmppsConvert_8s32s(img8s, blurImage32s, DIM*DIM);
		//
		//	//dump_32s("%d ", blurImage32s, DIM, DIM,DIM, 0);
		//
		//if (cmdIn.command == DO_FFT0)
		//	vsSaveImage("fft0_filter32s.img", blurImage32s, DIM, DIM, VS_RGB8_32);
		//else
		//	vsSaveImage("fft1_filter32s.img", blurImage32s, DIM, DIM, VS_RGB8_32);
		//
			
			nm32s* blurRoi32s = ring_nm1_to_nm0_diff->ptrHead();
			
			while (ring_nm1_to_nm0_diff->isFull()) 
				PRINT("ring_nm1_to_nm0_diff: head:%d tail:%d\n", ring_nm1_to_nm0_diff->head, ring_nm1_to_nm0_diff->tail);
			//dump_8s("%d ", blurRoi8s, 16, 16, DIM, 0);
			//printf("---\n");
			//dump_8s("%d ", blurRoi8s, 16, 16, DIM, 0);
			//printf("---\n");
			nmppsConvert_8s32s(blurRoi8s, blurRoi32s, DIM*DIM);
			//dump_32s("%d ", blurRoi32s, 16, 16, DIM, 0);

			if (cmdIn.command == DO_FFT0)
				VS_SAVE_IMAGE("1_FFT0_32s.vsimg", blurRoi32s, DIM, DIM, VS_RGB8_32);
			else
				VS_SAVE_IMAGE("1_FFT1_32s.vsimg", blurRoi32s, DIM, DIM, VS_RGB8_32);

			//nmppsRShiftC_32s(blurImage32s, 7, toNM0, DIM*DIM);

		//if (cmdIn.command == DO_FFT0)
		//	vsSaveImage("fft0_in32s.img", blurRoi32s, DIM, DIM, VS_RGB8_32);
		//else
		//	vsSaveImage("fft1_in32s.img", blurRoi32s, DIM, DIM, VS_RGB8_32);


			//nmppsConvert_8s32s((nm8s*)ringBufferLo, (nm32s*)ring_nm1_to_nm0_diff->ptrHead(), DIM*DIM);
			//dump_8u("%d ", ringBufferLo, 16, 16, 128, 1);
			//printf("--8s--\n");
			//dump_8s("%d ", ringBufferLo, 16, 16, 128, 1);
			//printf("--32s--\n");
			//dump_32s("%d ", ring_nm1_to_nm0_diff->ptrHead(), 16, 16, 128, 1);
			//printf("--32u--\n");
			//dump_32u("%d ", ring_nm1_to_nm0_diff->ptrHead(), 16, 16, 128, 1);

			ring_nm1_to_nm0_diff->head+=DIM*DIM;
			cmdOut.counter++;
			cmdOut.command = cmdIn.command;
			PRINT("out:%d 0x%x\n", cmdOut.counter, cmdOut.command);
			dtpSend(rbCmdToNm0, &cmdOut, sizeof32(Cmd_nm1_to_nm0));
		}
		
		else if (cmdIn.command == DO_CORR) {
			cmdOut.counter++;
			cmdOut.command = DO_CORR;
			PRINT("out:%d 0x%x\n", cmdOut.counter, cmdOut.command);
			dtpSend(rbCmdToNm0, &cmdOut, sizeof32(Cmd_nm1_to_nm0));
		}
		else{
			PRINT("Don't this fucking command 0x%x\n",cmdIn.command);
		}

		//	ringBufferLo, SIZE/4);
		//
		//printf(">>> 2:\n");
		//dtpSend(rbOut, ringBufferHi, SIZE );
		//printf("<<< 3:\n");
		//dtpRecv(rbIn, ringBufferLo, SIZE /4);
		//nmppsConvert_8s32s((nm8s*)ringBufferLo, (nm32s*)ringBufferHi, SIZE);
		//printf(">>>> 4:\n");
		//dtpSend(rbOut, ringBufferHi, SIZE );
	}

	
    return 0;
}