//#include "tchar.h"
#include "ippdefs.h"
#include "ippi.h"
#include "ippcore.h"
#include "ipps.h"
#include "ippi.h"
#include "nmpp.h"
#include "math.h"
#include "VShell.h"
#include "memory.h"
#include "dtp/dtp.h"
#include "dtp/file.h"
#include "dtp/mc12101-host.h"
#include "mc12101load.h"
#include "dumpx.h"
#include "stdio.h"
#include "sobel.h"
#include "tracker.h"
#include "crtdbg.h"
#include "vsimg.h"
#include "nmblas.h"
#include "hal/ringbuffert.h"

//#include "hadamard.h"

#define VS_TEXT
#define LOG2DIM 7
#define START_FRAME 1
#define MC12101 1
#define ALIGN 0xFFFF
//#define ALIGN 0xFFF8
//#define ALIGN 0xFFC0
#define WANTED_SIZE 64
//#define MAX_CACHE_FRAMES 0x1000
#define MAX_CACHE_FRAMES 0x40000
#define SCRIPT 1
//#define AVI "..\\..\\..\\Samples\\Road2_256x256(xvid).avi"
//#define AVI "..\\..\\..\\Samples\\strike_640x360(xvid).avi"
//#define AVI "..\\..\\..\\Samples\\demo_640x360.avi"
//#define AVI "..\\..\\..\\Samples\\Windg5.avi"
//#define AVI "..\\..\\..\\Samples\\rocket.avi"

#define AVI "..\\..\\..\\Samples\\stalin5.avi"
struct Mov{
	int start;
	int end;
	int x;
	int y;
	int w;
	char avi[128];
} ;
Mov script[]={
{0,100,	10,156,40,"..\\..\\..\\Samples\\strike_640x360(xvid).avi"},
{206,350,335,199,40,"..\\..\\..\\Samples\\strike_640x360(xvid).avi"},
{0,47,155,227,28,"..\\..\\..\\Samples\\rocket.avi"},
{0,45,211,287,32,"..\\..\\..\\Samples\\rocket2.avi"},
//{0,45,240,269,40,"..\\..\\..\\Samples\\rocket3.avi"},
{0,109,	434,11,48, "..\\..\\..\\Samples\\stalin5.avi"},
{40,177,371,81,48, "..\\..\\..\\Samples\\Windg5.avi"},
//{40,177,371,81,48, "..\\..\\..\\Samples\\Windg5.avi"},
{0,51,	450,7,48, "..\\..\\..\\Samples\\stalin4.avi"},
{0,70,	401,151,48, "..\\..\\..\\Samples\\stalin9.avi"}


};




#define VS_CREATE_IMAGE 
#define VS_SET_DATA 
#define VS_WRITE_IMAGE

//#define AVI "..\\..\\..\\Samples\\victory22_384x360(xvid).avi"
//#define AVI "../Samples/strike(xvid).avi"
//#define AVI "..\\..\\..\\Samples\\victory22_360x360(xvid).avi"
//#define AVI "..\\..\\..\\Samples\\Su25_720x720(xvid).avi"
//#define AVI "..\\..\\..\\Samples\\Su25_256x256(xvid).avi"
//#define AVI "..\\..\\..\\Samples\\Trooping the Colour Flypast 2023_640x360(xvid).avi"
//#define AVI "..\\..\\..\\Samples\\strike(xvid).avi"
//#define AVI "..\\..\\..\\Samples\\strike256(xvid).avi"
//#define AVI "..\\..\\..\\Samples\\victory22_384x360(xvid).avi"
//#define AVI "..\\..\\..\\Samples\\victory22_384x360(xvid).avi"
//#define AVI "c:/SU 57 CRAZY SCARY SOUND COMPILATION_(ms video1).avi"
//#define AVI "../../Samples/SU57(xvid).avi"
//#define AVI "c:\\vel.avi"
//#define AVI "c:\\drift.avi"
//#define AVI "..\\Samples\\MAKS2015_256x256.avi1"
//#define AVI "..\\Samples\\MAKS2015_256x256.avi1"
//#define AVI "..\\Samples\\Heli_forest.avi"
//#define AVI "../../Samples/Su256.avi"
//#define AVI "..\\Samples\\Formula3.avi"
//#define AVI "d:\\video\\films\\256x256\\2xFormula2.avi"

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
//resize(prevOrigin8u + prevFrame.y*WIDTH + prevFrame.x, srcRoiSize, WIDTH, prevImage8u, dimRoiSize, DIM);
//resize(currOrigin8u + currFrame.y*WIDTH + currFrame.x, srcRoiSize, WIDTH, currImage8u, dimRoiSize, DIM);

IppStatus resize(Ipp8u* pSrc, IppiSize srcSize, Ipp32s srcStep, Ipp8u* pDst, IppiSize	dstSize, Ipp32s dstStep)
{
	
	IppiResizeSpec_32f* pSpec = 0;
	int specSize = 0, initSize = 0, bufSize = 0;
	Ipp8u* pBuffer = 0;
	Ipp8u* pInitBuf = 0;
	Ipp32u numChannels = 1;
	IppiPoint dstOffset = { 0, 0 };
	IppStatus status = ippStsNoErr;
	IppiBorderType border = ippBorderRepl;
	/* Spec and init buffer sizes */
	status = ippiResizeGetSize_8u(srcSize, dstSize, ippLanczos, 0, &specSize, &initSize);
	if (status != ippStsNoErr) return status;
	/* Memory allocation */
	pInitBuf = ippsMalloc_8u(initSize);
	pSpec = (IppiResizeSpec_32f*)ippsMalloc_8u(specSize);
	if (pInitBuf == NULL || pSpec == NULL)
	{
		ippsFree(pInitBuf);
		ippsFree(pSpec);
		return ippStsNoMemErr;
	}
	/* Filter initialization */
	status = ippiResizeLanczosInit_8u(srcSize, dstSize, 3, pSpec, pInitBuf);
	ippsFree(pInitBuf);
	if (status != ippStsNoErr)
	{
		ippsFree(pSpec);
		return status;
	}
	/* work buffer size */
	status = ippiResizeGetBufferSize_8u(pSpec, dstSize, numChannels, &bufSize);
	if (status != ippStsNoErr)
	{
		ippsFree(pSpec);
		return status;
	}
	pBuffer = ippsMalloc_8u(bufSize);
	if (pBuffer == NULL)
	{
		ippsFree(pSpec);
		return ippStsNoMemErr;
	}
	/* Resize processing */
	status = ippiResizeLanczos_8u_C1R(pSrc, srcStep, pDst, dstStep, dstOffset, dstSize, border, 0, pSpec, pBuffer);
	ippsFree(pSpec);
	ippsFree(pBuffer);
	return status;
}
extern "C" {
	void halSleep(int ms) {

	}
}
#define NmppSize IppiSize
#define NmppRect IppiRect



//##########################################################
//###############            main        ###################
//##########################################################


int main()
{
	if (!VS_Init())
		return 0;
	int file_desc = 0;
	do {
		file_desc = dtpOpenFile("../exchange.bin", "rb");
	} while (file_desc < 0);
	unsigned ring_addr[6];
	dtpRecv(file_desc, ring_addr, 6);
	dtpClose(file_desc);
	for (int i = 0; i < 3; i++)
		printf("%d: %x\n", i, ring_addr[i]);

	int toNM1;
	int dwImg;
	int drOut;
	if (MC12101) {
		toNM1 = dtpOpenMc12101Ringbuffer(0, 1, ring_addr[0]);
		dwImg = dtpOpenMc12101Ringbuffer(0, 1, ring_addr[1]);
		drOut = dtpOpenMc12101Ringbuffer(0, 0, ring_addr[2]);
	}
	
	//if (!VS_Bind(AVI)) return 1;

	int mv = 0;
	VS_Bind(script[mv].avi);
	VS_Seek(script[mv].start);

	int WIDTH  = VS_GetWidth(VS_SOURCE);
	int HEIGHT = VS_GetHeight(VS_SOURCE);
	int imgFullSize32 = WIDTH * HEIGHT / 4;
	NmppSize frmFullDim = { WIDTH,HEIGHT };
	NmppRect frmFullRoi = { 0,0,WIDTH,HEIGHT };
	Cmd_x86_to_nm1 cmd = { 0,0,0,0 };
	long long  nmCacheSize32=0x100000000;
	if (MC12101) {
		cmd.command = 0x64078086;
		cmd.counter++;
		cmd.frmIndex = 0;
		cmd.frmRoi = { 0,0,DIM,DIM };
		cmd.frmSize = { WIDTH, HEIGHT };
		dtpSend(toNM1, &cmd, sizeof32(cmd));
		dtpRecv(drOut, &nmCacheSize32, sizeof32(nmCacheSize32));
	}
	//nmCacheSize32 = WIDTH * HEIGHT * 8 / 4;
	nm32u	*currOriginC = nmppsMalloc_32u(WIDTH*HEIGHT);
	nm32u	*prevOriginC = nmppsMalloc_32u(WIDTH*HEIGHT);
	nm8u	*prevOrigin8u = nmppsMalloc_8u(WIDTH*HEIGHT);
	nm8u	*currOrigin8u = nmppsMalloc_8u(WIDTH*HEIGHT);

	//IppiFFTSpec_C_32fc *spec;5
	//IppStatus st;

	int size = DIM * DIM;
	Ipp32fc *currImage_fc = (Ipp32fc *)ippMalloc(3 * WIDTH*HEIGHT * sizeof(Ipp32fc));
	nm32fcr *currImage_fcr = (nm32fcr *)malloc32(WIDTH*HEIGHT * sizeof32(nm32fcr));
	Ipp32fc *prevImage_fc = (Ipp32fc *)ippMalloc(3 * WIDTH*HEIGHT * sizeof(Ipp32fc));
	nm32fcr *prevImage_fcr = (nm32fcr *)malloc32(WIDTH*HEIGHT * sizeof32(nm32fcr));

	//Ipp32fc *currFFT_fc = (Ipp32fc *)ippMalloc(3 * size * sizeof(Ipp32fc));
	nm32fcr *currFFT_fcr = (nm32fcr *)malloc32(DIM*DIM * sizeof32(nm32fcr));
	
	//Ipp32fc *wantedImage_fc = (Ipp32fc *)ippMalloc(DIM*DIM* sizeof(Ipp32fc));
	nm32fcr *wantedImage_fcr = (nm32fcr *)malloc32(DIM*DIM* sizeof32(nm32fcr));
	nm8s    *wantedImage8s = nmppsMalloc_8s(DIM*DIM);
	nm8u    *wantedImage8u = nmppsMalloc_8u(DIM*DIM);
	//Ipp32fc *wantedFFT_fc = (Ipp32fc *)ippMalloc(3 * DIM*DIM sizeof(Ipp32fc));
	nm32fcr *wantedFFT_fcr = (nm32fcr *)malloc32(DIM*DIM * sizeof32(nm32fcr));

	//nm32fc  *tmpFFT_fc = (nm32fc *)ippMalloc(3 * size * sizeof(nm32fcr));
	nm32fcr *tmpFFT_fcr = (nm32fcr *)malloc32(size * sizeof32(nm32fcr));

	//Ipp32fc *productFFT_fc = (Ipp32fc *)ippMalloc(3 * size * sizeof(Ipp32fc));
	nm32fcr *productFFT_fcr = (nm32fcr *)malloc32(DIM*DIM * sizeof32(nm32fcr));
	nm32fcr *productIFFT_fcr = (nm32fcr *)malloc32(DIM*DIM * sizeof32(nm32fcr));
	nm32f	*productAbs32f = (nm32f *)malloc32(DIM*DIM * sizeof32(float));
	nm32f	*blurCorr32f = (nm32f *)malloc32(DIM*DIM * sizeof32(float));
	nm32f	*temp32f = (nm32f *)malloc32(size * sizeof32(float));
	//Ipp32fc *productIFFT_fc = (Ipp32fc *)ippMalloc(3 * size * sizeof(Ipp32fc));
	//nm32f   *currInput = nmppsMalloc_32f(3 * size);
	//nm32f   *wantedInput = nmppsMalloc_32f(3 * size);
	nm8u	*currImage8u_ = nmppsMalloc_8u(3 * WIDTH*HEIGHT); memset(currImage8u_, 0, 3 * WIDTH*HEIGHT);
	nm8u	*currImage8u = currImage8u_ + size;

	nm8s	*currImage8s_ = nmppsMalloc_8s(3* WIDTH*HEIGHT);
	nm8s	*currImage8s = currImage8s_+size; // because filter box need pads
	nm8u	*currBlur8u = nmppsMalloc_8u(WIDTH*HEIGHT);
	nm8u	*prevBlur8u = nmppsMalloc_8u(WIDTH*HEIGHT);
	nm8u	*diffBlur8u = nmppsMalloc_8u(WIDTH*HEIGHT);
	//nm32s	*currImage32s = nmppsMalloc_32s(4 * size);
	nm8u	*prevImage8u_ = nmppsMalloc_8u(3 * WIDTH*HEIGHT);
	nm8u	*prevImage8u = prevImage8u_ + size;
	nm8s	*prevImage8s_ = nmppsMalloc_8s(3* WIDTH*HEIGHT);
	nm8s	*prevImage8s  = prevImage8s_+size;
	nm32s	*prevImage32s = nmppsMalloc_32s(3 * size);
	nm32s	*diffImage32s = nmppsMalloc_32s(3 * size);
	nm32s	*blurImage32s = nmppsMalloc_32s(size);
	nm8s    *currFullImage8s_ = nmppsMalloc_8s(3 * WIDTH*HEIGHT);
	nm8s    *currFullImage8s= currFullImage8s_ + WIDTH*HEIGHT;
	nm8s	*currFullBlur8s = nmppsMalloc_8s(WIDTH * HEIGHT);
	nm8s    *prevFullImage8s_ = nmppsMalloc_8s(3 * WIDTH*HEIGHT);
	nm8s    *prevFullImage8s = currFullImage8s_ + WIDTH * HEIGHT;
	nm8s	*prevFullBlur8s_ = nmppsMalloc_8s(3*WIDTH * HEIGHT);
	nm8s	*prevFullBlur8s = prevFullBlur8s_  + WIDTH * HEIGHT;

	nm32s   *tempFull32s    = nmppsMalloc_32s(WIDTH * HEIGHT);
	nm8s    *tempFull8s    = nmppsMalloc_8s(WIDTH * HEIGHT);
	nm8s    *temp2Full8s    = nmppsMalloc_8s(WIDTH * HEIGHT);
	float	*tmp = nmppsMalloc_32f(3 * size);
	float	*tmp2 = nmppsMalloc_32f(3 * size);
	Ipp8u	*blurImage = (Ipp8u*)ippMalloc(3 * size);
	//nm32s	*wantImage32s	= nmppsMalloc_32s(size);
	//nm32s	*hadImage32s = nmppsMalloc_32s(size);
	//nm32s	*hadWanted32s = nmppsMalloc_32s(size);
	//nm32s	*tempImage32s = nmppsMalloc_32s(size);
	//nm32s	*hadMul32s = nmppsMalloc_32s(size);
	//nm32s	*hadConv32s = nmppsMalloc_32s(size);
	nm8u* srcBlur = nmppsMalloc_8u(size);

		//st = ippiFFTInitAlloc_C_32fc(&spec, LOG2DIM, LOG2DIM, IPP_FFT_DIV_INV_BY_N, ippAlgHintNone);

		S_VS_MouseStatus MouseStatus;
		
//#define RADIO_USE
		//VS_CreateRadioGroup("use",RADIO_USE,3,useLabel,) VS_CreateCheckBox
#define SEARCH_IASMAX 0
#define SEARCH_REAL 1

//#define EDIT_SEARCH_MODE 0
	//	VS_CreateEdit("SearchMode", EDIT_SEARCH_MODE);
		//VS_SetEditInt(EDIT_SEARCH_MODE, 0);
#define CHECK_IPP 0
#define CHECK_TRACK_X64 1
#define CHECK_TRACK_NMC 2 
#define CHECK_TRACK_CACHE 3
		VS_CreateCheckBox("Use IPP", CHECK_IPP, true);
		VS_CreateCheckBox("Track by X64", CHECK_TRACK_X64, true);
		VS_CreateCheckBox("Track by NMC", CHECK_TRACK_NMC, MC12101);
		VS_CreateCheckBox("Loop ", CHECK_TRACK_CACHE, false);

#define CHECK_SEARCH_AREA 9
		VS_CreateCheckBox("search area", CHECK_SEARCH_AREA, true);


#define SLIDER_START_FRAME 10
#define SLIDER_CACHE_FRAMES  11
		//int srcFrames =
		//VS_GetSrcFrames();
		VS_CreateSlider("Start frame", SLIDER_START_FRAME, 0, VS_GetSrcFrames(), 1, START_FRAME);
		VS_CreateSlider("Cache frames", SLIDER_CACHE_FRAMES,1, MIN(nmCacheSize32/imgFullSize32,MAX_CACHE_FRAMES), 1, MIN(nmCacheSize32 / imgFullSize32,MAX_CACHE_FRAMES));
#define SLIDER_WANTED_SIZE 1		
#define SLIDER_BLUR_SIZE 2	
		VS_CreateSlider("wantedSize", SLIDER_WANTED_SIZE, 8, DIM, 8, WANTED_SIZE);
		VS_CreateSlider("blurSize", SLIDER_BLUR_SIZE, 1, DIM-1, 2, 15);
		VS_CreateSlider("norm1", 3, 1, 10000, 1, 1);
		VS_CreateSlider("norm2", 4, 1, 10000, 1, 1);
#define SLIDER_BLUR_THRESH 5
		VS_CreateSlider("blurThresh", SLIDER_BLUR_THRESH, 1, 255, 1, 16);

#define SLIDER_DEPTH_SEARCH 6
		VS_CreateSlider("depth search", SLIDER_DEPTH_SEARCH, 1, 255, 1, 1);
#define SLIDER_SEARCH_MODE 8
		VS_CreateSlider("search mode", SLIDER_SEARCH_MODE, 0,2, 1, 0);


#define SLIDER_SCALE 7
		VS_CreateSlider("scale", SLIDER_SCALE, 0.1, 2, 0.01, 1);

#define PREV_ORIGIN_IMG 0
#define CURR_ORIGIN_IMG 1
		VS_CreateImage("Prev Origin", PREV_ORIGIN_IMG, VS_GetWidth(VS_SOURCE), VS_GetHeight(VS_SOURCE), VS_GetType(VS_SOURCE), 0);
		VS_CreateImage("Cirr Origin", CURR_ORIGIN_IMG, VS_GetWidth(VS_SOURCE), VS_GetHeight(VS_SOURCE), VS_GetType(VS_SOURCE), 0);

#define PREV_ORG_BLUR_8S 2
#define CURR_ORG_BLUR_8S 3
		VS_CREATE_IMAGE("Prev Blur", PREV_ORG_BLUR_8S, WIDTH, HEIGHT, VS_RGB8_8, 0);
		VS_CREATE_IMAGE("Curr Blur", CURR_ORG_BLUR_8S, WIDTH, HEIGHT, VS_RGB8_8, 0);

#define CURR_IMG8 4
		VS_CREATE_IMAGE("current Image", CURR_IMG8, WIDTH, HEIGHT, VS_RGB8, 0);
#define PREV_IMG8 5
		VS_CREATE_IMAGE("previous Image", PREV_IMG8, WIDTH, HEIGHT, VS_RGB8, 0);
#define PREV_BLUR 12
#define CURR_BLUR 13
#define DIFF_BLUR 14
#define WANRED 15
		//VS_CreateImage("CurrBlur8", CURR_BLUR, width, height, VS_RGB8_8, 0);
		//VS_CreateImage("PrevBlur8", PREV_BLUR, width, height, VS_RGB8_8, 0);
		//VS_CreateImage("DiffBlur8", DIFF_BLUR, width, height, VS_RGB8_8, 0);
		//VS_CreateImage("Wanted", WANRED, width, height, VS_RGB8_8, 0);

#define CURR_IMG_FC 213
#define PREV_IMG_FC 214
		VS_CREATE_IMAGE("curr input(cmplx)", CURR_IMG_FC, DIM, DIM, VS_RGB32FC, 0);
		VS_CREATE_IMAGE("prev input(cmplx)", PREV_IMG_FC, DIM, DIM, VS_RGB32FC, 0);
#define WANTED_IMG_FC 215
		//VS_CreateImage("wanted input(cmplx)", WANTED_IMG_FC, dim, dim, VS_RGB32FC, 0);

#define CURR_FFT 221
#define WANT_FFT 222
//VS_CreateImage("curr  FFT(cmplx)", CURR_FFT, dim, dim, VS_RGB32FC, 0);
//VS_CreateImage("wanted FFT(cmplx)", WANT_FFT, dim, dim, VS_RGB32FC, 0);
//VS_CreateImage("FFT*FFT(cmplx)", 332, dim, dim, VS_RGB32FC, 0);
#define  IFFT_IMG 333
		VS_CREATE_IMAGE("IFFT   (cmplx)", IFFT_IMG, DIM, DIM, VS_RGB32FC, 0);

#define  DIFF_IMG 334
		//VS_CreateImage("Diff ", DIFF_IMG, width, height, VS_RGB8_32, 0);

		//VS_CreateImage("Had32", 444, width, height, VS_RGB8_32, 0);

	NmppsFFTSpec_32fcr *spec128;
	NmppsFFTSpec_32fcr *specInv128;
	nmppsFFT128FwdInitAlloc_32fcr(&spec128);
	nmppsFFT128InvInitAlloc_32fcr(&specInv128);

	int blurWeights[16*16];
	for (int i = 0; i < 256; i++)
		blurWeights[i] = -1;
	int blurSize = 15;


	blurWeights[blurSize*blurSize / 2] = blurSize * blurSize - 1;
	int blurKernelSize = nmppiGetFilterKernelSize32_8s32s(blurSize, blurSize);
	nm64s* blurKernel = (nm64s*)nmppsMalloc_32s(blurKernelSize);
	nmppiSetFilter_8s32s(blurWeights, blurSize, blurSize, DIM, blurKernel);
	

	NmppPoint caught = {126,59};
	NmppPoint currFrame={ 0,0 };
	NmppPoint prevFrame={ 0,0 };
	NmppPoint caughtNM = { 0,0 };
	NmppPoint caughtPC = { 0,0 };
	NmppPoint caughtOrg = { 120,120 };
	NmppPoint caughtOrgNM = { 0,0 };
	NmppPoint caughtOrgPC = { 0,0 };
	//NmppPoint wanted = { 10,60 };
	NmppPoint wantedOrg = caughtOrg;

	//IppiSize srcRoiSize = { DIM*scale,DIM*scale };
	//IppiSize dimRoiSize = { DIM,DIM };
	int wantedOrgOffsetX = 0;
	//int fr = VS_GetSrcFrameNum();
	bool isCacheTracing = 0;
	//######################################################################
	//####################        VS_Run    ################################
	//######################################################################
	bool loadBlur = true;
	//VS_OpRunForward();
	int startFrame = VS_GetSlider(SLIDER_START_FRAME);
	int lastCachedFrame = -1;
	//int stopCachedFrame = -1;
	//VS_Seek(startFrame-1);
	
	///VS_Seek(START_FRAME-1);

	int cf = 0;

	caughtOrg = { script[mv].x, script[mv].y }; VS_SetSlider(SLIDER_WANTED_SIZE, script[mv].w); 
	VS_Bind(script[mv].avi);
	VS_Seek(script[mv].start);
	VS_Run();
//	VS_Seek(-1);

	VS_Draw(-1);
	int currentFrame = VS_GetSrcFrameNum();
	VS_Bind(script[mv].avi);
	 currentFrame = VS_GetSrcFrameNum();
	 VS_Seek(script[mv].start);
	 int totalCurrentFrame = 1;
	while (int status=VS_Run()) {
		
		int currentFrame = VS_GetSrcFrameNum();
		int cacheFrames= VS_GetSlider(SLIDER_CACHE_FRAMES);
		float scale    = VS_GetSlider(SLIDER_SCALE);
		 	blurSize   = VS_GetSlider(SLIDER_BLUR_SIZE);
		
		
		/*
		if (SCRIPT )switch (currentFrame) {
		case 1:				caughtOrg = { 29, 161 }; VS_SetSlider(SLIDER_WANTED_SIZE, 32); break;
		case 50:				                     VS_SetSlider(SLIDER_WANTED_SIZE, 48); break;
		//case 100:				caughtOrg = { 29, 161 }; VS_SetSlider(SLIDER_WANTED_SIZE, 32); break;
		case 207:			caughtOrg = { 336, 210 }; VS_SetSlider(SLIDER_WANTED_SIZE, 32); break;

		}
			
		*/
			//if (currentFrame == 0) {
			//	VS_Bind(AVI);
			//}



		int wantedSize = (int)VS_GetSlider(1)&ALIGN;
/*
		if (currentFrame >= startFrame + cacheFrames - 1) {
			VS_Seek(startFrame-1);
		}

		if (startFrame != VS_GetSlider(SLIDER_START_FRAME)) {
			startFrame = VS_GetSlider(SLIDER_START_FRAME);
			VS_Seek(startFrame-1);
			lastCachedFrame = -1; // cache reset
			if (VS_GetCheckBox(CHECK_TRACK_NMC)) {
				void* comSpec = dtpGetComSpec(dwImg);
				HalRingBufferConnector<int, 2>* connector = (HalRingBufferConnector<int, 2>*)comSpec;
				//connector->setHead(0);
				//connector->setTail(-1);
			}
			continue;
		}
		if (startFrame + cacheFrames > VS_GetSrcFrames() - 1) {
			cacheFrames = VS_GetSrcFrames() - startFrame - 1;
			VS_SetSlider(SLIDER_CACHE_FRAMES, cacheFrames);
		}
*/
		// ------------------------------- MOUSE HANDLING ----------------------------		
		VS_GetMouseStatus(&MouseStatus);
		if (MouseStatus.nKey == VS_MOUSE_CONTROL) {
			if (MouseStatus.nID == PREV_ORIGIN_IMG || MouseStatus.nID == CURR_ORIGIN_IMG) {
				//wantedOrg.x = MouseStatus.nX>>3<<3;
				wantedOrg.x = MouseStatus.nX&ALIGN;
				wantedOrg.y = MouseStatus.nY;
				//currFrame.x = MIN(WIDTH - DIM, MAX(0, wantedOrg.x + wantedSize / 2  - DIM / 2))&ALIGN;
				//currFrame.y = MIN(HEIGHT - DIM, MAX(0, wantedOrg.y + wantedSize / 2 - DIM / 2));

				currFrame.x = MIN(WIDTH - DIM, MAX(0, wantedOrg.x + wantedSize / 2 - DIM / 2))&ALIGN;
				currFrame.y = MIN(HEIGHT - DIM, MAX(0, wantedOrg.y + wantedSize / 2 - DIM / 2));

				caughtOrg   = wantedOrg;
			}
		}

		if (MouseStatus.nKey == VS_MOUSE_LBUTTON) {
			if (MouseStatus.nID == PREV_ORIGIN_IMG || MouseStatus.nID == CURR_ORIGIN_IMG) {
				//wantedOrg.x = MouseStatus.nX>>3<<3;
				wantedOrg.x = MouseStatus.nX&ALIGN;
				wantedOrg.y = MouseStatus.nY;
				caughtOrg = wantedOrg;
			}
		}
		// ------------------------------- PAUSE HANDLING ----------------------------
		if (!(status&VS_PAUSE)) {	// если не пауза
			memcpy(prevOriginC,    currOriginC, WIDTH*HEIGHT*3);
			memcpy(prevOrigin8u,   currOrigin8u, WIDTH*HEIGHT);
			memcpy(prevFullBlur8s, currFullBlur8s, WIDTH*HEIGHT);
			VS_GetData    (VS_SOURCE, currOriginC);
			VS_GetGrayData(VS_SOURCE, currOrigin8u);
			if ( VS_GetCheckBox(CHECK_TRACK_X64) || 
				(VS_GetCheckBox(CHECK_TRACK_NMC) && (currentFrame > lastCachedFrame)))
			{
				
				if (VS_GetCheckBox(CHECK_IPP)) {
					ippiFilterBox_8u_C1R((Ipp8u*)currOrigin8u, WIDTH, (Ipp8u*)currFullBlur8s, WIDTH, frmFullDim, { blurSize, blurSize }, { blurSize / 2, blurSize / 2 });
					nmppsRShiftC_8u((nm8u*)currFullBlur8s, 1, (nm8u*)currFullBlur8s, WIDTH*HEIGHT);
					nmppsRShiftC_8u(currOrigin8u, 1, (nm8u*)tempFull8s, WIDTH*HEIGHT);
					nmppsSub_8s(currFullBlur8s, tempFull8s, currFullBlur8s, WIDTH*HEIGHT);
				}
				else {
					nmppsSubC_8s((nm8s*)currOrigin8u, 127, currFullImage8s, WIDTH*HEIGHT);
					nmppiFilter_8s32s(currFullImage8s, tempFull32s, WIDTH, HEIGHT, blurKernel);
					nmppsRShiftC_32s(tempFull32s, 8, tempFull32s, WIDTH*HEIGHT);
					nmppsConvert_32s8s(tempFull32s, currFullBlur8s, WIDTH*HEIGHT);
				}
			}
			if (currentFrame == script[mv].start+1){
			//if (currentFrame == startFrame) {
				memcpy(prevOriginC, currOriginC, WIDTH*HEIGHT * 3);
				memcpy(prevOrigin8u, currOrigin8u, WIDTH*HEIGHT);
				memcpy(prevFullBlur8s, currFullBlur8s, WIDTH*HEIGHT);
			}
			
			//VS_GetGrayData(VS_SOURCE, prevOrigin8u);

			//VS_GetData(VS_SOURCE, prevOriginC);
			
			VS_SetData(PREV_ORIGIN_IMG, prevOriginC);
			VS_SetData(CURR_ORIGIN_IMG, currOriginC);
			
			wantedOrg = caughtOrg;
			///currFrame.x = MIN(WIDTH - DIM, MAX(0, wantedOrg.x + wantedSize / 2 - DIM / 2)) >> 3 << 3;
			currFrame.x = MIN(WIDTH - DIM, MAX(0, wantedOrg.x + wantedSize / 2 - DIM / 2))&ALIGN;
			currFrame.y = MIN(HEIGHT- DIM, MAX(0, wantedOrg.y + wantedSize / 2 - DIM / 2));

			// 8-byte alignment
			///wantedOrg.x += wantedOrgOffsetX; // compensation from prev step offset
			///wantedOrgOffsetX = wantedOrg.x % 8;
			///wantedOrg.x >>=3;
			///wantedOrg.x <<=3;
			wantedOrg.x += wantedOrgOffsetX; // compensation from prev step offset
			wantedOrgOffsetX = wantedOrg.x &~ALIGN;
			wantedOrg.x &=ALIGN;
			
		}
	
		//ippiSuperSampling_8u_C1R(currOrigin8u, WIDTH, srcRoiSize, currOrigin8u, dim, dimRoiSize, buffer);
		VS_SET_DATA(PREV_IMG8, prevImage8u);
		VS_SET_DATA(CURR_IMG8, currImage8u);
		
		VS_SET_DATA(PREV_ORG_BLUR_8S, prevFullBlur8s);
		VS_SET_DATA(CURR_ORG_BLUR_8S, currFullBlur8s);

		VS_Rectangle(PREV_ORIGIN_IMG, currFrame.x, currFrame.y, currFrame.x + DIM * scale, currFrame.y + DIM * scale, VS_BLUE, VS_NULL_COLOR);
		VS_Rectangle(CURR_ORIGIN_IMG, currFrame.x, currFrame.y, currFrame.x + DIM * scale, currFrame.y + DIM * scale, VS_BLUE, VS_NULL_COLOR);
		VS_Rectangle(PREV_ORIGIN_IMG, wantedOrg.x, wantedOrg.y, wantedOrg.x + wantedSize * scale, wantedOrg.y + wantedSize * scale, VS_RED, VS_NULL_COLOR);
		//########################################################################
		//#####################         track X64      ###########################
		//########################################################################
		nm32fcr maxPC = { 0,0 };
		
		if (VS_GetCheckBox(CHECK_TRACK_X64))
		{

			// ------------------- wanted preparation --------------
			//VS_SetData(PREV_BLUR, prevBlur8u);
			//ippiFilterBox_8u_C1R((Ipp8u*)prevImage8u, width, blurImage, width, s, { blurSize, blurSize }, { blurSize / 2, blurSize / 2 });
			//sobel(prevImage8u, blurImage, width, height);
		//		sobelCmplx(prevImage8u, prevImage_fcr, width, height);
			memset(wantedImage_fcr, 0, DIM*DIM * 8);
			memset(wantedImage8s, 0, DIM*DIM);
			///_ASSERTE(wantedOrg.x % 8 == 0);
			///_ASSERTE(wantedSize % 8 == 0);
			for (int i = 0; i < wantedSize; i++) {
				for (int j = 0; j < wantedSize; j++) {
					int blurDiff = prevFullBlur8s[WIDTH*(wantedOrg.y + i) + wantedOrg.x + j];
					wantedImage_fcr[i*DIM + j] = { 0, float(blurDiff) };
					wantedImage8s[i*DIM + j] = blurDiff;
				}
			}
			//VS_SetData(PREV_IMG_FC, wantedImage_fcr);

			// ----------------- BLUR  MASK ------------
			//float blurPoint = prevBlur8u[(wanted.y + wantedSize / 2)*DIM + wanted.x + wantedSize / 2];
			//for (int i = 0; i < DIM - wantedSize; i++) {
			//	for (int j = 0; j < DIM - wantedSize; j++) {
			//		diffBlur8u[i*DIM + j] = ABS(int(currBlur8u[(i + wantedSize / 2)*DIM + j + wantedSize / 2]) - int(blurPoint));
			//	}
			//}
	//		VS_SetData(DIFF_BLUR, diffBlur8u);
	//		VS_SetData(WANRED, wantedImage8s);
			//VS_SetData(WANTED_IMG_FC, wantedImage_fc);


			//--------------- prepare current input ----------------

			//ippiFilterBox_8u_C1R((Ipp8u*)currImage8u, width, currBlur8u, width, dimRoiSize, { blurSize, blurSize }, { blurSize /2, blurSize /2 });
			//sobel(currImage8u, blurImage, width,  height);
			//sobelCmplx(currImage8u, currImage_fcr, width, height);
			//VS_SetData(CURR_BLUR, currBlur8u);

			///_ASSERTE(currFrame.x % 8 == 0);
			for (int i = 0, k = 0; i < DIM; i++) {
				for (int j = 0; j < DIM; j++, k++) {
					int blurDiff = currFullBlur8s[WIDTH*(currFrame.y + i) + currFrame.x + j];
					currImage_fcr[k] = { 0, float(blurDiff) };
					currImage8s[k] = blurDiff;
				}
			}
			VS_WRITE_IMAGE("../back/pcwant.vsimg", wantedImage_fcr, DIM, DIM, VS_RGB32FC);
			VS_WRITE_IMAGE("../back/pccurr.vsimg", currImage_fcr, DIM, DIM, VS_RGB32FC);
			//VS_SetData(CURR_IMG_FC, currImage_fcr);

			// ----------forward fft ----------------
			NmppPoint caught0 = { 0,0 };

			//st = ippiFFTFwd_CToC_32fc_C1R(currImage_fc, dim * 8, currFFT_fc, dim * 8, spec, 0);
			//st = ippiFFTFwd_CToC_32fc_C1R(wantedImage_fc, dim * 8, wantedFFT_fc, dim * 8, spec, 0);
			//dump_32f("%f ", (nm32f*)wantedImage_fcr, 16, 16, DIM * 2, 0);
			//printf("---\n");
			for (int i = 0; i < DIM; i++) 	nmppsFFT128Fwd_32fcr(wantedImage_fcr + i * DIM, 1, tmpFFT_fcr + i, DIM, spec128);
			//dump_32f("%f ", (nm32f*)tmpFFT_fcr, 16, 16, DIM * 2, 0);
			//printf("---\n");
			for (int i = 0; i < DIM; i++) 	nmppsFFT128Fwd_32fcr(tmpFFT_fcr + i * DIM, 1, wantedFFT_fcr + i, DIM, spec128);
			//dump_32f("%f ", (nm32f*)wantedFFT_fcr, 16, 16, DIM * 2,0);

			for (int i = 0; i < DIM; i++) 	nmppsFFT128Fwd_32fcr(currImage_fcr + i * DIM, 1, tmpFFT_fcr + i, DIM, spec128);
			for (int i = 0; i < DIM; i++) 	nmppsFFT128Fwd_32fcr(tmpFFT_fcr + i * DIM, 1, currFFT_fcr + i, DIM, spec128);
			//VS_SetData(CURR_FFT, currFFT_fc);
			//VS_SetData(WANT_FFT, wantedFFT_fc);
			//-------- NORMAL ORDER ---------- 
			//for (int i = 0; i < size; i++) {
			//	wantedFFT_fc[i].im *= -1; 
			//}
			//
			//for (int i = 0; i < size; i++) {
			//	productFFT_fc[i].re = currFFT_fc[i].re * wantedFFT_fc[i].re - currFFT_fc[i].im * wantedFFT_fc[i].im;
			//	productFFT_fc[i].im = currFFT_fc[i].re * wantedFFT_fc[i].im + currFFT_fc[i].im * wantedFFT_fc[i].re;
			//}
			////------- FUCKING ORDER ----------- 


			//for (int i = 0; i < size; i++) {
			//	productFFT_fcr[i].re =  currFFT_fcr[i].re * wantedFFT_fcr[i].re + currFFT_fcr[i].im * wantedFFT_fcr[i].im;
			//	productFFT_fcr[i].im = -currFFT_fcr[i].re * wantedFFT_fcr[i].im + currFFT_fcr[i].im * wantedFFT_fcr[i].re;
			//}
			nmppsConjMul_32fcr(currFFT_fcr, wantedFFT_fcr, productFFT_fcr, DIM*DIM);

			//dump_32f("%f ", (nm32f*)productFFT_fcr, 16, 16, DIM * 2, 0);
			//vsWriteImage("../back/pcProfuctFFT.vsimg", productFFT_fcr, DIM, DIM, VS_RGB32FC);
			//VS_SetData(332, productFFT_fcr);

			//----------- inverse fft-------------- 
			//st = ippiFFTInv_CToC_32fc_C1R(productFFT_fc, width * 8, productIFFT_fc, width * 8, spec, 0);

			for (int i = 0; i < DIM; i++) 	nmppsFFT128Inv_32fcr(productFFT_fcr + i * DIM, 1, tmpFFT_fcr + i, DIM, specInv128);
			for (int i = 0; i < DIM; i++) 	nmppsFFT128Inv_32fcr(tmpFFT_fcr + i * DIM, 1, productIFFT_fcr + i, DIM, specInv128);




			//vsWriteImage("../back/pcIFFT.vsimg", productIFFT_fcr, DIM, DIM, VS_RGB32FC);

			VS_SET_DATA(IFFT_IMG, productIFFT_fcr);

			//dump_32f("%.3f ", (nm32f*)productIFFT_fcr, 14, 14, DIM * 2, 0);
			//printf("------\n");
			//dump_32f("%.2e ", (nm32f*)productIFFT_fcr, 14, 14, DIM * 2, 0);

			int blurThresh = VS_GetSlider(SLIDER_BLUR_THRESH);
			VS_SET_DATA(IFFT_IMG, productIFFT_fcr);
			for (int k = 0; k < VS_GetSlider(SLIDER_DEPTH_SEARCH); k++) {
				nm32fcr maxx = { 0,0 };
				NmppPoint caught;
				int searchMode = VS_GetSlider(SLIDER_SEARCH_MODE);
				
				switch (searchMode){

				case 0: {
					int idx = nmblas_isamax(DIM*DIM * 2, (const float*)productIFFT_fcr, 1);
					caught.y = idx >> 8;
					caught.x = (idx % 256) >> 1;
					maxx = productIFFT_fcr[idx >> 1];
				}
				case 1:
				{
					for (int i = 0; i < DIM - wantedSize; i++) {
						for (int j = 0; j < DIM - wantedSize; j++) {
							nm32fcr curr = productIFFT_fcr[i*DIM + j];
							if (maxx.re < curr.re) {
								maxx = curr;
								caught = { i,j };
							}
						}
					}
				}
				case 2 :
				{
					for (int i = 0; i < DIM; i++) {
						for (int j = 0; j < DIM; j++) {
							nm32fcr curr = productIFFT_fcr[i*DIM + j];
							if (maxx.re < curr.re) {
								maxx = curr;
								caught = { i,j };
							}
						}
					}
				}
				}
				if (k == 0) {
					caughtPC = caught;
					maxPC = maxx;
					caughtOrgPC.x = currFrame.x + caughtPC.x;
					caughtOrgPC.y = currFrame.y + caughtPC.y;

				}
				else {
					int spot = wantedSize;
					NmppPoint caughtDepthOrg;
					caughtDepthOrg.x = currFrame.x + caught.x*scale;
					caughtDepthOrg.y = currFrame.y + caught.y*scale;
					for(int i=-spot/2; i<spot/2;i++ )
						for(int j=-spot/2; j<spot/2;j++ )
							productIFFT_fcr[(caught.y+i)*DIM + caught.x+j] = { 0,0 };

					//if (diffBlur8u[caught.y*DIM + caught.x] < blurThresh)
					{
						caughtDepthOrg.x = currFrame.x + caught.x*scale;
						caughtDepthOrg.y = currFrame.y + caught.y*scale;
						VS_Rectangle(CURR_ORIGIN_IMG, caughtDepthOrg.x, caughtDepthOrg.y, caughtDepthOrg.x + wantedSize, caughtDepthOrg.y + wantedSize, VS_GREENYELLOW, VS_NULL_COLOR);
					}
				}
			}
		}
		//########################################################################
		//#####################         track NMC      ###########################
		//########################################################################
		nm32fcr maxNM = { 0,0 };
		
		static int index0;
		static int index1;
		//int currFrameNum = VS_GetSrcFrameNum();
		if (MC12101  && VS_GetCheckBox(CHECK_TRACK_NMC)) {
			if (!(status&VS_PAUSE)) {
				//if (totalCurrentFrame == startFrame ) {
				if (totalCurrentFrame == 1 ) {
					index0 = 0;
					index1 = 0;
				}
				else {
					index1++;
					index0 = index1 - 1;
				}
				if (totalCurrentFrame > lastCachedFrame) {
					printf("0");
					dtpSend(dwImg, currFullBlur8s, imgFullSize32);
					printf("1");
					lastCachedFrame = totalCurrentFrame;
				}
				else{
					//VS_Text("run from cache %d\r\n",currentFrame);
					printf("run from cache %d\n", lastCachedFrame);
				}
				

			}
	
			cmd.command = DO_FFT0;
			cmd.counter++;
			cmd.frmIndex = index0;
			cmd.frmSize = frmFullDim;
			cmd.frmRoi = { wantedOrg.x,wantedOrg.y,wantedSize, wantedSize };
			dtpSend(toNM1, &cmd, sizeof32(cmd));

			cmd.command = DO_FFT1;
			cmd.counter++;
			cmd.frmIndex = index1;
			cmd.frmSize = frmFullDim;
			cmd.frmRoi = {currFrame.x,currFrame.y,DIM,DIM};
			dtpSend(toNM1, &cmd, sizeof32(cmd));
		
			cmd.command = DO_CORR;
			cmd.counter++;
			dtpSend(toNM1, &cmd, sizeof32(cmd));
	
			// take profit :
			
			dtpRecv(drOut, &caughtNM, sizeof32(caughtNM));
			
			//dtpRecv(drOut, &maxNM, sizeof32(maxNM)); //bug
			caughtOrgNM.x = currFrame.x + caughtNM.x;
			caughtOrgNM.y = currFrame.y + caughtNM.y;
		}
		
		if (VS_GetCheckBox(CHECK_TRACK_NMC)) {
			VS_Rectangle(CURR_ORIGIN_IMG, caughtOrgNM.x + 1, caughtOrgNM.y + 1, caughtOrgNM.x + wantedSize - 1, caughtOrgNM.y + wantedSize - 1, VS_YELLOW, VS_NULL_COLOR);
			caughtOrg = caughtOrgNM;
			VS_TEXT("%d NM cx:%d cy:%d max:%f %f\r\n", currentFrame - startFrame, caughtNM.x, caughtNM.y, maxNM.re, maxNM.im);
		}

		if (VS_GetCheckBox(CHECK_TRACK_X64)) {
			VS_Rectangle(CURR_ORIGIN_IMG, caughtOrgPC.x, caughtOrgPC.y, caughtOrgPC.x + wantedSize, caughtOrgPC.y + wantedSize, VS_GREEN, VS_NULL_COLOR);
			caughtOrg = caughtOrgPC;
			VS_TEXT("%d PC cx:%d cy:%d max:%f %f \r\n", currentFrame - startFrame, caughtPC.x, caughtPC.y, maxPC.re, maxPC.im);
		}

		
		_ASSERTE(caughtOrg.x >= 0);
		_ASSERTE(caughtOrg.y >= 0);
		_ASSERTE(caughtOrg.x < WIDTH);
		_ASSERTE(caughtOrg.y < HEIGHT);

		//VS_Rectangle(PREV_IMG8, wanted.x, wanted.y, wanted.x + wantedSize, wanted.y + wantedSize, VS_RED, VS_NULL_COLOR);
		//VS_Rectangle(CURR_IMG8, caught.x, caught.y, caught.x + wantedSize, caught.y + wantedSize, VS_GREEN, VS_NULL_COLOR);
		
		//VS_Rectangle(IFFT_IMG, caught.x-1, caught.y-1, caught.x + wantedSize, caught.y + wantedSize, VS_GREEN, VS_NULL_COLOR);
		//VS_Rectangle(CURR_IMG_FC, caught.x-1, caught.y-1, caught.x + wantedSize, caught.y + wantedSize, VS_GREEN, VS_NULL_COLOR);
		//VS_Text("NM wx:%d wy:%d -> cx:%d cy:%d dx:%d dy:%d\r\n", wantedOrg.x, wantedOrg.y, caughtOrgNM.x, caughtOrgNM.y, caughtOrgNM.x - caughtOrgNM.x, caughtOrgNM.y - wantedOrg.y);
		//VS_Text("PC wx:%d wy:%d -> cx:%d cy:%d dx:%d dy:%d\r\n", wantedOrg.x, wantedOrg.y, caughtOrgPC.x, caughtOrgPC.y, caughtOrgPC.x - caughtOrgNM.x, caughtOrg.y - wantedOrg.y);
		//VS_Text("pc:%f nm:%f \r\n",maxPC , maxNM);
		
		VS_Draw(VS_DRAW_ALL);
		
		if (currentFrame == script[mv].end) {
			mv++;
			if (mv == sizeof(script) / sizeof(Mov)) {

				mv = 0;
				totalCurrentFrame=0;
			}
			VS_Bind(script[mv].avi);
			VS_Seek(script[mv].start);
			caughtOrg = { script[mv].x, script[mv].y }; VS_SetSlider(SLIDER_WANTED_SIZE, script[mv].w);
			currentFrame = VS_GetSrcFrameNum();

			_ASSERTE(currentFrame == script[mv].start);
		}
		totalCurrentFrame++;
		printf("%d %dMb\n", totalCurrentFrame, totalCurrentFrame * 640 * 360 / 1024 / 1024);
		_ASSERTE(totalCurrentFrame < 256 * 1024 * 1024 / 640 / 360);
	}
}

//
			//ippsFFTFwd_CToC_32fc(currImage_fc + i * dim, (Ipp32fc*)tmpFFT_fc + i, sp64, buffer);
			//nmppsFFT64Fwd_32fcr((nm32fcr*)currImage_fc + i * dim, (nm32fcr*)tmpFFT_fc + i, spec64);
			//
			//ippsFFTFwd_CToC_32fc(currImage_fc + i * dim, (Ipp32fc*)tmpFFT_fc + i, sp32, buffer);
			//nmppsFFT32Fwd_32fcr((nm32fcr*)currImage_fc + i * dim, (nm32fcr*)tmpFFT_fc + i, spec32);
			//
			//ippsFFTFwd_CToC_32fc(currImage_fc + i * dim, (Ipp32fc*)tmpFFT_fc + i, sp16, buffer);
			//nmppsFFT16Fwd_32fcr((nm32fcr*)currImage_fc + i * dim, (nm32fcr*)tmpFFT_fc + i, spec16);
			//
			//ippsFFTFwd_CToC_32fc(currImage_fc + i * dim, (Ipp32fc*)tmpFFT_fc + i, sp8, buffer);
			//nmppsDFT8Fwd_32fcr((nm32fcr*)currImage_fc + i * dim, (nm32fcr*)tmpFFT_fc + i, spec16);



		//------------- hadamard --------------
	//	nmppsHadamard(currImage32s, hadImage32s, hadMatrix, tempImage32s, dim);
	//	nmppsHadamard(wantImage32s, hadWanted32s, hadMatrix, tempImage32s, dim);
	//	//hadImage32s[0] = 0;
	//	int norm1 = VS_GetSlider(3);
	//	int norm2 = VS_GetSlider(4);
	//	for (int i = 1; i < size; i++) {
	//		//if (abs(hadImage32s[i]) < ppp)
	//			__int64 a=hadImage32s[i]/norm1;
	//			__int64 b = hadWanted32s[i];// /= norm2;
	//			//hadImage32s[i]*=ppp;
	//			__int64 c = a * b;
	//			if (c != (int)((int)a*(int)b))
	//				c = 0;
	//			//hadMul32s[i] = hadImage32s[i] * hadWanted32s[i];
	//			hadMul32s[i] = c/norm2;
	//	}
	//	hadMul32s[0] = 0;
	//	//VS_SetData(444, hadImage32s);
	//
	//	//nmppsSet_32s(0, currImage32s, size);
	//	nmppsHadamardInverse(hadMul32s, hadConv32s, hadMatrix, tempImage32s, dim);
	//	//VS_SetData(444, currImage32s);
	//	//VS_SetData(444, wantImage32s);
	//	VS_SetData(444, hadConv32s);

			//printf("currFFT_fcr:\n");
			//dump_32f("%.1f ", (float*)currFFT_fcr, 8, 16, dim * 2, 1);
			//printf("wantedFFT_fcr:\n");
			//dump_32f("%.1f ", (float*)wantedFFT_fcr, 8, 16, dim * 2, 1);
			//printf("productFFT_fcr:\n");
			//dump_32f("%.1f ", (float*)productFFT_fcr, 8, 16, dim * 2, 1);
			//printf("productIFFT_fcr:\n");
			//dump_32f("%.1f ", (float*)productIFFT_fcr, 8, 16, dim * 2, 1);

