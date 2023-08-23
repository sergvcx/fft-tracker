#include "nmtype.h"
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

#define NW "c:\\git\\nmw\\bin\\nmdisplay_source.nw"
#define VS_SAVE_IMAGE vsSaveImage
#define USE_SEMIHOSTING 1
#define PRINT(...) printf(__VA_ARGS__)
#define PRINTRT(...)
//printf(__VA_ARGS__)
#define EXCHANGE "../exchange.bin"



Cmd_x86_to_nm1 cmdIn;
Cmd_nm1_to_nm0 cmdOut = { 0,0 };
 
/*
#include "nmw/nmw-client.h"

void buffer_release_listener(void *data, NMWBuffer *buffer) {
	int *is_released = (int *)data;
	*is_released = 1;
}

int g_is_released;
NMWBufferListener buffer_listener;


NMWSurface *nmwCreateSurfaceFromBuffer(NMWDisplay *display, void* buffer, int offset, int width, int height, int stride, int format) {
	NMWShmemPool *shm = nmwCreateShmemPoolData(display, buffer, width * height * 4);
	NMWBuffer *nmw_buffer = nmwCreateBuffer(shm, offset, width, height, stride, format);

	g_is_released = 0;
	buffer_listener.release = buffer_release_listener;
	nmwBufferSetListener(nmw_buffer, &buffer_listener, &g_is_released);

	NMWSurface *surface = nmwCreateSurface(display);
	nmwSurfaceAttach(surface, nmw_buffer);
	return surface;
}

void nmwDrawAll(NMWDisplay *display, NMWSurface *surface, int width, int height) {
	nmwSurfaceDamage(surface, 0, 0, width, height);
	nmwSurfaceCommit(surface);
	while (g_is_released == 0) {
		nmwDisplayDispatch(display);
	}
	g_is_released = 0;
}
*/



__attribute__((section(".data.imu3"))) 	int ringBufferLo[DIM*DIM*2];
__attribute__((section(".data.imu1"))) 	int ringBufferHi[DIM*DIM*2];
//__attribute__((section(".data.imu2"))) 	int ringBufferHi2[DIM*DIM* 2];

#define FULL_BANK 32*1024 // 128kB
//#define PRINT 
//printf
//__attribute__((section(".data.imu3"))) int x86_to_nm1_buffer[FULL_BANK];



extern "C" {
	void mdelay(int);

};

int blurWeights[16 * 16];


void nmppmCopy_8si8si(nm8s* pSrc, int srcBeginIndex,   int srcStep, nm8s* pDst, int dstBeginIndex, int dstStep, int width, int height) {

	nm8s* rowSrc = pSrc;
	nm8s* rowDst = pDst;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int val = nmppsGet_8s(rowSrc, srcBeginIndex +x);
			nmppsPut_8s(rowDst, dstBeginIndex + x, val);
		}
		rowSrc = nmppsAddr_8s(rowSrc, srcStep);
		rowDst = nmppsAddr_8s(rowDst, dstStep);
	}
}

__attribute__((section(".text.nmpp")))
int main(){
	halInstrCacheEnable();
	if (USE_SEMIHOSTING) printf("nmc1 started\n");
	HalRingBufferData<int, 2>* ring[6];
	
	if (USE_SEMIHOSTING) {
		int file_desc = 0;
		do {
			file_desc = dtpOpenFile(EXCHANGE, "rb");
		} while (file_desc < 0);
		dtpRecv(file_desc, ring, 6);
		dtpClose(file_desc);
	}
	else {
		ring[0]=(HalRingBufferData<int, 2>*) 0x000a8184;// data : 000a8020 size : 256 id : 8601cdcd
		ring[1]=(HalRingBufferData<int, 2>*) 0x000a8170;// data : 20022286 size : 8388608 id : 8601b00f
		ring[2]=(HalRingBufferData<int, 2>*) 0x000a815c;// data : 20012286 size : 65536 id : 0186b00f
		ring[3]=(HalRingBufferData<int, 2>*) 0x000a8148;// data : 000a8000 size : 32 id : 0100cdcd
		ring[4]=(HalRingBufferData<int, 2>*) 0x000a8134;// data : 00070000 size : 32768 id : 0100deef
		ring[5]=(HalRingBufferData<int, 2>*) 0x000a8120;// data : 00078000 size : 32768 id : 0001beef
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
		PRINT("%d: ring:%08x data:%08x size:%8d id:%08x\n", i, (int)ring[i], (int)ring[i]->data, ring[i]->size, ring[i]->bufferId);
	


	PRINT("NMW running ... \n");

	//NMWDisplay *display = nmwOpenDisplay(NW);
	//
	//NMWSurface *surface[8];
	//for (int i = 0; i < 8; i++) {
	//	//surface[i] = nmwCreateSurfaceFromBuffer(display, ring_x86_to_nm1_img->data + i * 256*256, 0, cmdIn.frmRoi.width, cmdIn.frmRoi.height, 0, NMW_FORMAT_GRAYSCALE_8);
	//	//surface[i] = nmwCreateSurfaceFromBuffer(display, ring_x86_to_nm1_img->data + i * 256*256, 0, cmdIn.frmRoi.width, cmdIn.frmRoi.height, 0, NMW_FORMAT_GRAYSCALE_8);
	//	memset(ring_x86_to_nm1_img->data+ i * 256 * 256 / 4, 0x456789, 256 * 256 / 4);
	//	surface[i] = nmwCreateSurfaceFromBuffer(display, ring_x86_to_nm1_img->data + i * 256*256/4, 0, 256, 256, 0, NMW_FORMAT_GRAYSCALE_8);
	//	//surface[i] = nmwCreateSurfaceFromBuffer(display, ring_x86_to_nm1_img->data + i * 256*256/4, 0, 256, 256, 0, NMW_FORMAT_XRGB_8888);
	//	NMASSERT(surface[i]);
	//	nmwDrawAll(display, surface[i], 256, 256);
	//}
	//
	//while (1);
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
	PRINT("Sending hanshake to nm1 ... \n");
	dtpSend(rbCmdToNm0, &cmdOut, sizeof32(cmdOut));
	PRINT("Waiting hanshake from x86 ... \n");
	dtpRecv(rbCmdToNm1, &cmdIn, sizeof32(cmdIn));
	if (cmdIn.command == 0x64078086)
		PRINT("Handshake ok. Working ... \n");
	else {
		PRINT("Handshake error %x\n", cmdIn.command);
		return -1;
	}
	

	
	NmppSize imgDim = cmdIn.frmSize;
	int imgSize32 = imgDim.height*imgDim.width/4;
	PRINT("imgSize32 =%d\n", imgSize32);
	PRINT("width     =%d\n", cmdIn.frmSize.width);
	PRINT("height    =%d\n", cmdIn.frmSize.height);
	PRINT("roi.width =%d\n", cmdIn.frmRoi.width);
	PRINT("roi.height=%d\n", cmdIn.frmRoi.height);
	PRINT("roi.x     =%d\n", cmdIn.frmRoi.x);
	PRINT("roi.y     =%d\n", cmdIn.frmRoi.y);

	//int handshake[8];
	//dtpRecv(rbImg, handshake, 8);
	//for (int i = 0; i < 8; i++)
	//	printf("% d\n", handshake[i]);
	
		
	

	PRINT("NM1 running ... \n");

	

	while (1) {
		//printf("<<< 1:\n");
		dtpRecv(rbCmdToNm1, &cmdIn, sizeof32(cmdIn));
		PRINTRT("--------- [in] cnt:%d cmd:0x%x frmIndex:%d------ \n", cmdIn.counter, cmdIn.command, cmdIn.frmIndex);

		if (cmdIn.command == DO_FFT0 || cmdIn.command == DO_FFT1) {

			while (cmdIn.frmIndex >= ring_x86_to_nm1_img->head ) {
				PRINT("ring_x86_to_nm1_img: head:%d tail:%d\n", ring_x86_to_nm1_img->head, ring_x86_to_nm1_img->tail);
			}

			
			//cmdIn.frmIndex*imgSize32 +
			//int srcPos8 = cmdIn.frmRoi.y*imgDim.width + cmdIn.frmRoi.x;
			//int src
			//int srcDma=
			//if (cmdIn.frmRoi.x)
			int *roi = ring_x86_to_nm1_img->data +cmdIn.frmIndex*imgSize32 + cmdIn.frmRoi.y*imgDim.width / 4 + cmdIn.frmRoi.x / 4;

			//nmwDrawAll(display, surface[cmdIn.frmIndex], cmdIn.frmRoi.width, cmdIn.frmRoi.height);

			nm8s* blurRoi8s = (nm8s*)ringBufferLo;
			if (cmdIn.command == DO_FFT0) {
				nmppsSet_8s(0, blurRoi8s, DIM*DIM);
				nmppsSet_8s(0, (nm8s*)ringBufferHi, DIM*DIM);

			}
			
			nm8s* src = (nm8s*)(ring_x86_to_nm1_img->data+ cmdIn.frmIndex*imgSize32 + cmdIn.frmRoi.y*imgDim.width/4 + cmdIn.frmRoi.x/4 );
			nm8s* dst = (nm8s*)ringBufferLo;
			int copyMode = 3;
			if (copyMode==0) {
				for (int y = 0; y < cmdIn.frmRoi.height; y++) {
					memcpy(dst, src, cmdIn.frmRoi.width/ 4);
					src = nmppsAddr_8s(src, cmdIn.frmSize.width);
					dst = nmppsAddr_8s(dst, DIM);
				}
			}
			//else if (copyMode==1)
			//	nmppmCopy_8s((nm8s*)roi, cmdIn.frmRoi.width, blurRoi8s, DIM , cmdIn.frmRoi.height, cmdIn.frmRoi.width);
			else if (copyMode == 2) {

				halDma2D_Start(roi, blurRoi8s, cmdIn.frmRoi.height*cmdIn.frmRoi.width / 4, cmdIn.frmRoi.width / 4, cmdIn.frmSize.width / 4, DIM/4);
				while (!halDma2D_IsCompleted());
	
				// It works!
				// halDma2D_Start(roi, ringBufferHi, cmdIn.frmRoi.height*cmdIn.frmRoi.width / 4, cmdIn.frmRoi.width / 4, cmdIn.frmSize.width / 4, DIM/4);
				// while (!halDma2D_IsCompleted());
				// nmppmCopy_8si8si((nm8s*)ringBufferHi, 0, DIM , blurRoi8s, 0, DIM, cmdIn.frmRoi.width , cmdIn.frmRoi.height);

				// It works!
				// halDma2D_Start(roi, ringBufferHi, cmdIn.frmRoi.height*cmdIn.frmRoi.width / 4, cmdIn.frmRoi.width / 4, cmdIn.frmSize.width / 4, cmdIn.frmRoi.width / 4);
				// while (!halDma2D_IsCompleted());
				//nmppmCopy_8si8si((nm8s*)ringBufferHi, 0, cmdIn.frmRoi.width , blurRoi8s, 0, DIM, cmdIn.frmRoi.width , cmdIn.frmRoi.height);



			}
			else if (copyMode ==3){
				nm32s * imgAddr = ring_x86_to_nm1_img->data + cmdIn.frmIndex*imgSize32;
				int roi8Left		= cmdIn.frmRoi.y*imgDim.width  + cmdIn.frmRoi.x;	// roi byte-position 
				int roi8LeftAlign64	= roi8Left & 0xFFFFFFC0;					// 64-byte-aligned roi byte-position
				int roi8LeftDisp    = roi8Left & 0x3F;							// byte-offset from aligned position
			
				int roi8Right		= roi8Left + cmdIn.frmRoi.width;			// right from roi byte-position
				int roi8RightAlign64= (roi8Right + 63)&0xFFFFFFC0;				// 64-byte-aligned byte-position of right from roi 
				int roi8Width		= roi8RightAlign64- roi8LeftAlign64;		// aligned roi byte-width 
				int roi8Size		= roi8Width * cmdIn.frmRoi.height;

				int* srcDma = imgAddr + roi8LeftAlign64/4;
				//int* dstDma = ringBufferLo;
				int* dstDma = ringBufferHi;
				
				NMASSERT(((int)imgAddr& 0x0F) == 0);
				NMASSERT(((int)srcDma & 0x0F) == 0);
				NMASSERT(((int)dstDma & 0x0F) == 0);
				NMASSERT(roi8Width    % 64 == 0);
				
				
				src = (nm8s*)srcDma;
				dst = (nm8s*)dstDma;

				// It works!		
				halDma2D_Start(srcDma, ringBufferHi, roi8Size>>2, roi8Width>>2, cmdIn.frmSize.width >>2, roi8Width >>2);
				while (!halDma2D_IsCompleted());
				//nmppmCopy_8si8si((nm8s*)ringBufferHi, roi8LeftDisp, roi8Width , blurRoi8s, 0, DIM, cmdIn.frmRoi.width , cmdIn.frmRoi.height);
				nmppmCopyua_8s((nm8s*)ringBufferHi, roi8Width, roi8LeftDisp, blurRoi8s, DIM, cmdIn.frmRoi.height, cmdIn.frmRoi.width);

				//cmdIn.info();
				//printf("roi8Size:%d roi8Width:%d roi8LeftDisp:%d \n", roi8Size, roi8Width, roi8LeftDisp);
			}

			//mdelay(10);
			//	printf(".");
			//};
			
			//if (cmdIn.command == DO_FFT0)
			//	nmppsSet_8s(0, (nm8s*)ringBufferHi, DIM*DIM);
			//

			//nm8s* src = (nm8s*)roi;
			//nm8s* dst = (nm8s*)blurRoi8s;
			
			
			//for (int i = 0; i < DIM*DIM; i++) {
			//	if (ringBufferHi[i] != ringBufferLo[i])
			//		printf("DMA error: dma[%d]=%08x mcpy[%d]=08%x\n", i, ringBufferLo[i], i, ringBufferHi[i]);
			//}
			

			
			//if (cmdIn.command == DO_FFT0) {
			//	VS_SAVE_IMAGE("1_roi0_8s.vsimg", ring_x86_to_nm1_img->data + cmdIn.frmIndex*imgSize32, cmdIn.frmSize.width, cmdIn.frmSize.height, VS_RGB8_8);
			//	VS_SAVE_IMAGE("1_FFT0_8s.vsimg", blurRoi8s, DIM, DIM, VS_RGB8_8);
			//}
			//else {
			//	VS_SAVE_IMAGE("1_roi1_8s.vsimg", ring_x86_to_nm1_img->data + cmdIn.frmIndex*imgSize32, cmdIn.frmSize.width, cmdIn.frmSize.height, VS_RGB8_8);
			//	VS_SAVE_IMAGE("1_FFT1_8s.vsimg", blurRoi8s, DIM, DIM, VS_RGB8_8);
			//}
			//int *tail=ring_x86_to_nm1_img->ptrTail();
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
				PRINT("ring_nm1_to_nm0_diff:full head:%d tail:%d\n", ring_nm1_to_nm0_diff->head, ring_nm1_to_nm0_diff->tail);
			//dump_8s("%d ", blurRoi8s, 16, 16, DIM, 0);
			//printf("---\n");
			//dump_8s("%d ", blurRoi8s, 16, 16, DIM, 0);
			//printf("---\n");
			nmppsConvert_8s32s(blurRoi8s, blurRoi32s, DIM*DIM);
			//dump_32s("%d ", blurRoi32s, 16, 16, DIM, 0);

			//if (cmdIn.command == DO_FFT0)
			//	VS_SAVE_IMAGE("1_FFT0_32s.vsimg", blurRoi32s, DIM, DIM, VS_RGB8_32);
			//else
			//	VS_SAVE_IMAGE("1_FFT1_32s.vsimg", blurRoi32s, DIM, DIM, VS_RGB8_32);

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
			PRINTRT("out:%d 0x%x\n", cmdOut.counter, cmdOut.command);
			dtpSend(rbCmdToNm0, &cmdOut, sizeof32(Cmd_nm1_to_nm0));
		}
		
		else if (cmdIn.command == DO_CORR) {
			cmdOut.counter++;
			cmdOut.command = DO_CORR;
			PRINTRT("out:%d 0x%x\n", cmdOut.counter, cmdOut.command);
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
	vsReadImage(0, 0, 0, 0, 0);


}



/*
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
*/