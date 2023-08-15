#include "nmtype.h"
#include "dtp/dtp.h"
#include "dtp/file.h"
#include "hal/ringbuffert.h"
#include "dtp/mc12101.h"
#include "mc12101load_nm.h"
#include "nmpp.h"
#include "string.h"
#include "dumpx.h"
#include "tracker.h"
#include "nmassert.h"

Cmd_x86_to_nm1 cmdIn;
Cmd_nm1_to_nm0 cmdOut = { 0,0 };

__attribute__((section(".data.imu3"))) 	int ringBufferLo[DIM*DIM*2];
__attribute__((section(".data.imu1"))) 	int ringBufferHi[DIM*DIM*2];
__attribute__((section(".data.imu2"))) 	int ringBufferHi2[DIM*DIM* 2];

#define FULL_BANK 32*1024 // 128kB
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

//extern "C" {
//void halSleep(int);

	//}
//}


#define VS_RGB1 1
#define VS_RGB4 2
#define VS_RGB8 3
#define VS_RGB16 4
#define VS_RGB24 5
#define VS_RGB32 6
#define VS_RGB8_8 7
#define VS_RGB8_16 8
#define VS_RGB8_32 9
#define VS_RGB32F 10
#define VS_RGB32FC 11

void	vsSaveImage(char* filename, void* data, int width, int height, int type) {
	
	int* data32 = (int*)data;
	FILE* f = fopen(filename,"wb");
	int size= width * height;
	int size32=0;
	switch (type) {
	case VS_RGB1 :
		size32 = size/32; break;
	case VS_RGB4:
		size32 = size/8; break;
	case VS_RGB8_8:
	case VS_RGB8:
		size32 = size/4; break;
	case VS_RGB16 :
	case VS_RGB8_16:
		size32 = size/2; break;
	case VS_RGB24 :
		size32 = size*3/4; break;
	case VS_RGB32:
	case VS_RGB8_32:
	case VS_RGB32F:
		size32 = size; break;
	case VS_RGB32FC:
		size32 = size*2; break;
	}
	if (f) {
		int id=0x00006407;
		fwrite(&id, 1, 1, f);
		fwrite(&type, 1, 1, f);
		fwrite(&width, 1, 1, f);
		fwrite(&height, 1, 1, f);
		//fwrite(data, 1, size32, f);

		int block32 = 4*1024;
		if (1) while (size32) {
			size_t rsize;
			if (size32 < block32) {
				rsize = fwrite(data32,  sizeof(int),size32, f);
				size32 -= size32;
				
			}
			else {
				rsize = fwrite(data32,  sizeof(int), block32, f);
				size32 -= block32;
				data32 += block32;
			}
		}
		fclose(f);
	}
}
void	vsReadImage(char* filename, void* data, int* width, int* height, int* type) {
	FILE* f = fopen(filename, "wb");
	
	if (f) {
		
		fseek(f, 0, SEEK_END);
		int size = ftell(f);
		fseek(f, 0, SEEK_SET);
		int id;// = 0x00006407;
		fread(&id, 1, 1, f);
		fread(&type, 1, 1, f);
		fread(&width, 1, 1, f);
		fread(&height, 1, 1, f);
		//int size = *width * height;


		fread(data, 1, size - 16, f);
		fclose(f);
	}
}

#define FILE "../exchange.bin"
int blurWeights[16 * 16];
int main(){
	halSleep(100);
	
	printf("nmc1 started\n");
	int file_desc = 0;
	do {
		file_desc = dtpOpenFile(FILE, "rb");
	} while (file_desc < 0);
	
	HalRingBufferData<int, 2>* ring[6];
	dtpRecv(file_desc, ring, 6);
	dtpClose(file_desc);
	
	HalRingBufferData<int, 2>* ring_x86_to_nm1_cmd = ring[0];
	HalRingBufferData<int, 2>* ring_x86_to_nm1_img = ring[1];
	//HalRingBufferData<int, 2>* ring_nm1_to_x86_out = ring[2];
	HalRingBufferData<int, 2>* ring_nm1_to_nm0_cmd = ring[3];
	HalRingBufferData<int, 2>* ring_nm1_to_nm0_diff= ring[4];
	//HalRingBufferData<int, 2>* ring_nm0_to_nm1_corr= ring[5];

	//ring_x86_to_nm1_img->data = (int*)ringBufferHi;
	//ring_x86_to_nm1_img->init(32 * 1024);
	for (int i = 0; i < 6; i++)
		printf("%d: ring:%08x data:%08x size:%8d id:%08x\n", i, (int)ring[i], (int)ring[i]->data, ring[i]->size, ring[i]->bufferId);
	
	halDmaInit();
	//halEnbExtInt();



	
	// -------------------------------------------------------------
	int rbCmdToNm1=dtpOpenRingbufferDefault(ring_x86_to_nm1_cmd);
	
	int rbCmdToNm0=dtpOpenRingbufferDefault(ring_nm1_to_nm0_cmd);
	
	int rbImg=dtpOpenRingbuffer(ring_x86_to_nm1_img, memCopyPush, memCopyPop);
	


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
	printf("Sending hanshake to nm1 ... \n");
	dtpSend(rbCmdToNm0, &cmdOut, sizeof32(cmdOut));
	printf("Waiting hanshake from x86 ... \n");
	dtpRecv(rbCmdToNm1, &cmdIn, sizeof32(cmdIn));
	if (cmdIn.command == 0x64078086)
		printf("Handshake ok. Working ... \n");
	else {
		printf("Handshake error %x\n", cmdIn.command);
		return -1;
	}
	
	
	NmppSize imgDim = cmdIn.frmSize;
	int imgSize32 = imgDim.height*imgDim.width/4;
	printf("imfSize32 =%d\n", imgSize32);
	printf("width     =%d\n", cmdIn.frmSize.width);
	printf("height    =%d\n", cmdIn.frmSize.height);
	printf("roi.width =%d\n", cmdIn.frmRoi.width);
	printf("roi.height=%d\n", cmdIn.frmRoi.height);
	printf("roi.x     =%d\n", cmdIn.frmRoi.x);
	printf("roi.y     =%d\n", cmdIn.frmRoi.y);

	//int handshake[8];
	//dtpRecv(rbImg, handshake, 8);
	//for (int i = 0; i < 8; i++)
	//	printf("% d\n", handshake[i]);
	
	printf("Starting nm1 ... \n");


	


	while (1) {
		//printf("<<< 1:\n");
		dtpRecv(rbCmdToNm1, &cmdIn, sizeof32(cmdIn));
		printf("--------- [in] cnt:%d cmd:0x%x frmIndex:%d------ \n", cmdIn.counter, cmdIn.command, cmdIn.frmIndex);

		if (cmdIn.command == DO_FFT0 || cmdIn.command == DO_FFT1) {

			while (cmdIn.frmIndex >= ring_x86_to_nm1_img->head ) {
				printf("ring_x86_to_nm1_img: head:%d tail:%d\n", ring_x86_to_nm1_img->head, ring_x86_to_nm1_img->tail);
			}
			int *roi = ring_x86_to_nm1_img->data +cmdIn.frmIndex*imgSize32 + cmdIn.frmRoi.y*imgDim.width / 4 + cmdIn.frmRoi.x / 4;
			nm8s* blurRoi8s = (nm8s*)ringBufferLo;
			if (cmdIn.command == DO_FFT0)
				nmppsSet_8s(0, blurRoi8s, DIM*DIM);
			halDma2D_Start(roi, blurRoi8s, cmdIn.frmRoi.height*cmdIn.frmRoi.width / 4, cmdIn.frmRoi.width / 4, cmdIn.frmSize.width / 4, DIM/4);
			while (!halDmaIsCompleted());
			int *tail=ring_x86_to_nm1_img->ptrTail();
			//memcpy(ringBufferLo, roi, DIM*DIM / 4);
			//printf("%x %x %d\n", roi, tail, cmdIn.frmIndex);
			
			//dtpRecv(rbImg, ringBufferLo, DIM*DIM/4);
			printf("in: recv ok\n");

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
				printf("ring_nm1_to_nm0_diff: head:%d tail:%d\n", ring_nm1_to_nm0_diff->head, ring_nm1_to_nm0_diff->tail);

			nmppsConvert_8s32s(blurRoi8s, blurRoi32s, DIM*DIM);
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
			printf("out:%d 0x%x\n", cmdOut.counter, cmdOut.command);
			dtpSend(rbCmdToNm0, &cmdOut, sizeof32(cmdOut));
		}
		
		else if (cmdIn.command == DO_CORR) {
			cmdOut.counter++;
			cmdOut.command = DO_CORR;
			printf("out:%d 0x%x\n", cmdOut.counter, cmdOut.command);
			dtpSend(rbCmdToNm0, &cmdOut, sizeof32(cmdOut));
		}
		else{
			printf("Don't this fucking command 0x%x\n",cmdIn.command);
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

	
	printf("---------\n");
    return 0;
}