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


//#include "hadamard.h"

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


void dimToOrg(int dimX,int dimY,int &orgX,int &orgY,int dim,IppiSize orgSize,float cropFactor) {
		orgX = orgSize.width*cropFactor / dim * ( dimX-dim/2) + orgSize.width / 2;
		orgY = orgSize.height*1.0 / dim * (dimY- dim / 2 ) + orgSize.height / 2;
}
void orgToDim(int orgX, int orgY, int& dimX, int& dimY,  int dim, IppiSize orgSize, float cropFactor) {

	dimX = (orgX - orgSize.width / 2) * dim / orgSize.width / cropFactor + dim / 2 ;
	dimY = (orgY - orgSize.height/ 2) * dim / orgSize.height / 1.0 + dim / 2;
}

struct Pos {
	int x;
	int y;
} caught;

#define FILE "../exchange.bin"

#define ECHO 128
#define X86_TO_NM0_BUFFER_SIZE ECHO		//0
#define X86_TO_NM1_BUFFER_SIZE SIZE*4	//1
#define NM0_TO_X86_BUFFER_SIZE ECHO		//2
#define NM1_TO_X86_BUFFER_SIZE ECHO		//3
#define NM0_TO_NM1_BUFFER_SIZE ECHO		//4
#define NM1_TO_NM0_BUFFER_SIZE SIZE	//5


int main()
{

	int file_desc = 0;
	do {
		file_desc = dtpOpenFile(FILE, "rb");
	} while (file_desc < 0);

	unsigned ring_addr[6];
	//uintptr_t addr_read = 0;

	dtpRecv(file_desc, ring_addr, 6);
	dtpClose(file_desc);
	for (int i = 0; i < 6; i++)
		printf("%d: %x\n", i, ring_addr[i]);


	// -------------------------------------------------------------


	int dw = dtpOpenMc12101Ringbuffer(0, 1, ring_addr[1]);
	int dr = dtpOpenMc12101Ringbuffer(0, 0, ring_addr[2]);



	if (!VS_Init())
		return 0;
	//if (!VS_Bind("..\\Samples\\Road1.avi"))
	//if (!VS_Bind("..\\Samples\\Road2.avi"))
#ifdef _DEBUG
	if (!VS_Bind("..\\..\\..\\Samples\\Road2.avi"))
#else 
	if (!VS_Bind("..\\Samples\\Road2.avi"))
#endif

	//if (!VS_Bind("c:/SU 57 CRAZY SCARY SOUND COMPILATION_(ms video1).avi"))
	//if (!VS_Bind("../../Samples/SU57(xvid).avi"))
	//if (!VS_Bind("../Samples/strike(xvid).avi"))
	
	//if (!VS_Bind("c:\\vel.avi"))
	//if (!VS_Bind("c:\\drift.avi"))

	//if (!VS_Bind("..\\Samples\\MAKS2015_256x256.avi1"))
	//if (!VS_Bind("..\\Samples\\MAKS2015_256x256.avi1"))
	//if (!VS_Bind("..\\Samples\\Heli_forest.avi"))
	//if (!VS_Bind("../../Samples/Su256.avi"))
	//if (!VS_Bind("..\\Samples\\Formula3.avi"))
	//if (!VS_Bind("d:\\video\\films\\256x256\\2xFormula2.avi"))
		return 0;
	int WIDTH = VS_GetWidth(VS_SOURCE);
	int HEIGHT = VS_GetHeight(VS_SOURCE);
	IppiSize orgSize{ WIDTH,HEIGHT };
	
	nm32u	*currOriginC  = nmppsMalloc_32u(WIDTH*HEIGHT);
	nm32u	*prevOriginC  = nmppsMalloc_32u(WIDTH*HEIGHT);
	nm8u	*currOrigin8u = nmppsMalloc_8u(WIDTH*HEIGHT);
	

	IppiFFTSpec_C_32fc *spec;
	IppStatus st;

	int dim = 128;
	#define LOG2DIM 7
	int width = dim;
	int height =dim;

	int size = dim*dim;
	Ipp32fc *currImage_fc	= (Ipp32fc *)ippMalloc(3*size * sizeof(Ipp32fc));
	nm32fcr *currImage_fcr	= (nm32fcr *)malloc32 (size * sizeof32(nm32fcr));
;
	Ipp32fc *prevImage_fc	= (Ipp32fc *)ippMalloc(3*size * sizeof(Ipp32fc));
	nm32fcr *prevImage_fcr  = (nm32fcr *)malloc32(size * sizeof32(nm32fcr));
	
	Ipp32fc *currFFT_fc		= (Ipp32fc *)ippMalloc(3*size * sizeof(Ipp32fc));
	nm32fcr *currFFT_fcr	= (nm32fcr *)malloc32(size * sizeof32(nm32fcr));
	
	
	Ipp32fc *wantedImage_fc = (Ipp32fc *)ippMalloc(3*size * sizeof(Ipp32fc));
	nm32fcr *wantedImage_fcr= (nm32fcr *)malloc32(size * sizeof32(nm32fcr));
	nm8s    *wantedImage8s  = nmppsMalloc_8s(size);
	Ipp32fc *wantedFFT_fc	= (Ipp32fc *)ippMalloc(3*size * sizeof(Ipp32fc));
	nm32fcr *wantedFFT_fcr = (nm32fcr *)malloc32(size * sizeof32(nm32fcr));

	nm32fc  *tmpFFT_fc		= (nm32fc *)ippMalloc(3*size * sizeof(nm32fcr));
	nm32fcr *tmpFFT_fcr		= (nm32fcr *)malloc32( size * sizeof32(nm32fcr));
	
	
	Ipp32fc *productFFT_fc	= (Ipp32fc *)ippMalloc(3*size * sizeof(Ipp32fc));
	nm32fcr *productFFT_fcr = (nm32fcr *)malloc32(size * sizeof32(nm32fcr));
	nm32fcr *productIFFT_fcr= (nm32fcr *)malloc32(size * sizeof32(nm32fcr));
	Ipp32fc *productIFFT_fc = (Ipp32fc *)ippMalloc(3*size * sizeof(Ipp32fc));
	nm32f   *currInput		= nmppsMalloc_32f(3*size);
	nm32f   *wantedInput	= nmppsMalloc_32f(3*size);
	nm8u	*currImage8u	= nmppsMalloc_8u(4 * size);
	nm8s	*currImage8s	= nmppsMalloc_8s( size);
	nm32s	*currImage32s   = nmppsMalloc_32s(4 * size)+size;
	nm8u	*prevImage8u	= nmppsMalloc_8u(4 * size);
	nm8s	*prevImage8s	= nmppsMalloc_8s( size);
	nm32s	*prevImage32s	= nmppsMalloc_32s(4 * size)+size;
	nm32s	*diffImage32s	= nmppsMalloc_32s(4 * size);
	float	*tmp            = nmppsMalloc_32f(3 * size);
	float	*tmp2           = nmppsMalloc_32f(3 * size);
	Ipp8u	*blurImage		= (Ipp8u*)ippMalloc(3 * size);
	//nm32s	*wantImage32s	= nmppsMalloc_32s(size);
	nm32s	*hadImage32s	= nmppsMalloc_32s(size);
	nm32s	*hadWanted32s	= nmppsMalloc_32s(size);
	nm32s	*tempImage32s	= nmppsMalloc_32s(size);
	nm32s	*hadMul32s		= nmppsMalloc_32s(size);
	nm32s	*hadConv32s		= nmppsMalloc_32s(size);
	nm8u	*buffer			= nmppsMalloc_8u(4*size);
	//nm16u	*currImage16 = nmppsMalloc_16u(size);
	//nm8u	*currImage8 = nmppsMalloc_8u(size);
	nm2s	*hadMatrix		= nmppsMalloc_2s(size);
//	nmppsHadamardInit(hadMatrix, dim);


	nm8u* srcBlur = nmppsMalloc_8u(size);

	st = ippiFFTInitAlloc_C_32fc(&spec, LOG2DIM, LOG2DIM, IPP_FFT_DIV_INV_BY_N, ippAlgHintNone);

	VS_CreateSlider("width", 0, 0.1, 1, 0.01, 1);
	//VS_CreateSlider("norm2", 11, 1, 10000, 1, 1);
	//VS_CreateSlider("norm1", 12, 1, 10000, 1, 1);
	//VS_CreateSlider("norm2", 13, 1, 10000, 1, 1);

	VS_CreateSlider("wantedSize", 1, 1, 255, 1, 14);
	VS_CreateSlider("blurSize", 2, 1, 255, 1, 11);
	VS_CreateSlider("norm1", 3, 1, 10000, 1, 1);
	VS_CreateSlider("norm2", 4, 1, 10000, 1, 1);

	#define PREV_ORIGIN_IMG 0
	#define CURR_ORIGIN_IMG 1
	VS_CreateImage("Prev Origin", PREV_ORIGIN_IMG, VS_GetWidth(VS_SOURCE),VS_GetHeight(VS_SOURCE),VS_GetType(VS_SOURCE), 0);
	VS_CreateImage("Cirr Origin", CURR_ORIGIN_IMG, VS_GetWidth(VS_SOURCE),VS_GetHeight(VS_SOURCE),VS_GetType(VS_SOURCE), 0);
	
	#define CURR_IMG8 2
	VS_CreateImage("current Image", CURR_IMG8, width, height, VS_RGB8, 0);
	#define PREV_IMG8 4
	VS_CreateImage("previous Image", PREV_IMG8, width, height, VS_RGB8, 0);
	VS_CreateImage("Blur", 10, width, height, VS_RGB8, 0);
	//VS_CreateImage("Blur32", 11, width, height, VS_RGB8_32, 0);
	
	//VS_CreateImage("Blur16", 12, width, height, VS_RGB8_16, 0);
	//VS_CreateImage("Blur8", 13, width, height, VS_RGB8_8, 0);
	//VS_CreateImage("Wanted", 11, width, height, VS_RGB8_8, 0);
	//VS_CreateImage("Wanted float", 12, width, height, VS_RGB32F, 0);
	
	#define CURR_IMG_FC 213
	#define PREV_IMG_FC 214
	VS_CreateImage("curr input(cmplx)", CURR_IMG_FC, width, height, VS_RGB32FC, 0);
	VS_CreateImage("prev input(cmplx)", PREV_IMG_FC, width, height, VS_RGB32FC, 0);
	#define WANTED_IMG_FC 215
	VS_CreateImage("wanted input(cmplx)", WANTED_IMG_FC, dim, dim, VS_RGB32FC, 0);

	#define CURR_FFT 221
	#define WANT_FFT 222
	VS_CreateImage("curr  FFT(cmplx)", CURR_FFT, dim, dim, VS_RGB32FC, 0);
	VS_CreateImage("wanted FFT(cmplx)", WANT_FFT, dim, dim, VS_RGB32FC, 0);
	
	VS_CreateImage("FFT*FFT(cmplx)", 332, dim, dim, VS_RGB32FC, 0);
	#define  IFFT_IMG 333
	VS_CreateImage("IFFT   (cmplx)", IFFT_IMG, dim, dim, VS_RGB32FC, 0);
	
	#define  DIFF_IMG 334
	VS_CreateImage("Diff ", DIFF_IMG, width, height, VS_RGB8_32, 0);

	//VS_CreateImage("Had32", 444, width, height, VS_RGB8_32, 0);
	
	NmppsFFTSpec_32fcr *spec128;
	nmppsFFT128FwdInitAlloc_32fcr(&spec128);

	NmppsFFTSpec_32fcr *specInv128;
	nmppsFFT128InvInitAlloc_32fcr(&specInv128);

	//IppsFFTSpec_C_32fc *sp128;
	//ippsFFTInitAlloc_C_32fc(&sp128, 7, IPP_FFT_DIV_INV_BY_N, ippAlgHintFast);
	//
	//IppsFFTSpec_C_32fc *sp64;
	//ippsFFTInitAlloc_C_32fc(&sp64, 6, IPP_FFT_DIV_INV_BY_N, ippAlgHintFast);
	//
	//IppsFFTSpec_C_32fc *sp32;
	//ippsFFTInitAlloc_C_32fc(&sp32, 5, IPP_FFT_DIV_INV_BY_N, ippAlgHintFast);
	//
	//IppsFFTSpec_C_32fc *sp16;
	//ippsFFTInitAlloc_C_32fc(&sp16, 4, IPP_FFT_DIV_INV_BY_N, ippAlgHintFast);
	//
	//IppsFFTSpec_C_32fc *sp8;
	//ippsFFTInitAlloc_C_32fc(&sp8, 3, IPP_FFT_DIV_INV_BY_N, ippAlgHintFast);

	int wantedSize = 10;
	int wantedY = 10;
	int wantedX = 60;
	caught.x = 10;
	caught.y = 10;
	
	S_VS_MouseStatus MouseStatus;
	int status;
	float cropFactor;
	//memset(currImage, 0, size);



	//if (1) {
	//
	//	dw = dtpOpenFile("../curr-pc2nm.bin", "wb");
	//	dr = dtpOpenFile("../caught-nm2pc.bin", "wb");	dtpClose(dr);
	//	dr = dtpOpenFile("../caught-nm2pc.bin", "rb");
	//}
	//else {
	


	//	PL_Board *board;
	//	int ok=PL_GetBoardDesc(0, &board);
	//	PL_Access *access;
	//	ok=PL_GetAccess(board, 0, &access);
	//	int desc=dtpOpenPloadFileHost(access, "../handshake.bin");
	//	dr = desc;
	//	
	//	uintptr_t ringAddr=0;
	//	dtpRecv(dr, &ringAddr, 1);
	//	int dwb = dtpOpenPloadRingbuffer(access, ringAddr);
	//	dw = dwb;
	//}




	while (status=VS_Run()) {

		for (int i = 0; i < size; i++) {
			//currImage_fc[i] = { 0,0 };
			//prevImage_fc[i] = { 0,0 };
			wantedImage_fc[i] = { 0,0 };
			wantedImage_fcr[i] = { 0,0 };
			//currImage8u[i] = 0;
			tmp[i] = 0;
			tmp2[i] = 0;
		}

		cropFactor = VS_GetSlider(0);

		if (!(status&VS_PAUSE)) {
			// если не пауза
			swap((void**)&currImage_fc,(void**)&prevImage_fc);
			swap((void**)&currImage8u, (void**)&prevImage8u);
			//memcpy(prevImage8u, currImage8u, dim*dim);
			//swap((void**)&currImage_fc,(void**)&prevImage_fc);
			memcpy(prevOriginC, currOriginC, WIDTH*HEIGHT*3);
			VS_GetData(VS_SOURCE, currOriginC);
			VS_SetData(0, prevOriginC);
			VS_SetData(1, currOriginC);
			VS_GetGrayData(VS_SOURCE, currOrigin8u);
			
			nmppsConvert_8u32u(prevImage8u,(nm32u*)prevImage32s,dim*dim);
			nmppsConvert_8u32u(currImage8u,(nm32u*)currImage32s,dim*dim);
			IppiSize srcRoiSize{ WIDTH*cropFactor,HEIGHT };
			IppiSize dimRoiSize{ dim,dim };
			//memset(currImage8u, 0, size);

			resize(currOrigin8u + int(WIDTH*(1 - cropFactor) / 2), srcRoiSize, WIDTH, currImage8u, dimRoiSize, dim);

			wantedX = caught.x;
			wantedY = caught.y;

			//VS_GetGrayData(VS_SOURCE, currImage8u+size);
			//VS_GetGrayData(VS_SOURCE, currImage8u + 2*size);
		}

		//ippiSuperSampling_8u_C1R(currOrigin8u, WIDTH, srcRoiSize, currOrigin8u, dim, dimRoiSize, buffer);
		VS_SetData(CURR_IMG8, currImage8u);
		VS_SetData(PREV_IMG8, prevImage8u);
		int wantedSize = VS_GetSlider(1);
		int blurSize   = VS_GetSlider(2);
		VS_GetMouseStatus(&MouseStatus);
		if (MouseStatus.nKey == VS_MOUSE_LBUTTON) {
			if (MouseStatus.nID == PREV_IMG8) {
				wantedY = MouseStatus.nY;
				wantedX = MouseStatus.nX;
			}
			if (MouseStatus.nID == PREV_ORIGIN_IMG || MouseStatus.nID == CURR_ORIGIN_IMG) {
				orgToDim(MouseStatus.nX, MouseStatus.nY, wantedX, wantedY, dim, orgSize, cropFactor);
			}
		}
		
		if (wantedY >dim - wantedSize) {
			wantedY = 10;
			wantedX = 40;
		}
		
		IppiSize s = {dim,dim};

//	---- prepare current input ----------------
		ippiFilterBox_8u_C1R((Ipp8u*)currImage8u, width, blurImage, width, s, { blurSize, blurSize }, { blurSize /2, blurSize /2 });
		VS_SetData(10, blurImage);

		for (int i = 0; i < size; i++) {
			float diff = (float)currImage8u[i] - (float)blurImage[i];
			currImage_fc [i] = { diff,0 };
			currImage_fcr[i] = { 0,diff };
			currImage8s[i] = diff / 2;
		}

		VS_SetData(CURR_IMG_FC, currImage_fc);
		
		//	---- prepare wanted input ----------------
		//ippiFilterBox_8u_C1R((Ipp8u*)prevImage8u+size, width, blurImage, width, s, { blurSize, blurSize }, { blurSize / 2, blurSize / 2 });
		ippiFilterBox_8u_C1R((Ipp8u*)prevImage8u, width, blurImage, width, s, { blurSize, blurSize }, { blurSize / 2, blurSize / 2 });
		for (int i = 0; i < size; i++) {
			float diff = (float)prevImage8u[i] - (float)blurImage[i];
			prevImage_fc [i] = { diff, 0 };
			prevImage_fcr[i] = { 0,diff};
			prevImage8s[i] = diff / 2;
			wantedImage8s[i] = 0;
		}

		VS_SetData(PREV_IMG_FC, prevImage_fc);
		
		for (int i = 0; i <  wantedSize; i++) {
			for (int j = 0; j < wantedSize; j++) {
				wantedImage_fc[i*width + j]  =  prevImage_fc [(wantedY + i)*width + wantedX + j];
				wantedImage_fcr[i*width + j] =  prevImage_fcr[(wantedY + i)*width + wantedX + j];
				wantedImage8s[i*width + j] = prevImage8s[(wantedY + i)*width + wantedX + j];
			}
		}
		VS_SetData(WANTED_IMG_FC, wantedImage_fc);


		//dtpSend(dw, currImage_fcr, size * 2);
		//dtpSend(dw, wantedImage_fcr, size * 2);
		dtpSend(dw, currImage8s,   size /4);
		dtpSend(dw, wantedImage8s, size /4);
		

		// ----------forward fft ----------------
		
		
		if (0) {
			//st = ippiFFTFwd_CToC_32fc_C1R(currImage_fc, dim * 8, currFFT_fc, dim * 8, spec, 0);
			//st = ippiFFTFwd_CToC_32fc_C1R(wantedImage_fc, dim * 8, wantedFFT_fc, dim * 8, spec, 0);

			
			//memset(currFFT_fc, 0, dim*dim * 8);

			//spec128->dstStep = dim*2;
			//spec128->dstStep =  dim*2;
			//spec128->dstStep = 2;
			for (int i = 0; i < dim; i++) {
				nmppsFFT128Fwd_32fcr(currImage_fcr+ i * dim, 1, tmpFFT_fcr+i, dim, spec128);
				//nmppsFFT128Fwd_32fcr(currImage_fcr, tmpFFT_fcr, spec128);
			}
			////spec128->dstStep = dim*2;
			for (int i = 0; i < dim; i++) {
				nmppsFFT128Fwd_32fcr(tmpFFT_fcr  + i * dim, 1, currFFT_fcr + i, dim, spec128);
				//nmppsFFT128Fwd_32fcr(tmpFFT_fcr  + i * dim, currFFT_fcr, spec128);
			}
			
			
			//spec128->dstStep = dim * 2;
			for (int i = 0; i < dim; i++) {
				nmppsFFT128Fwd_32fcr(wantedImage_fcr + i * dim, 1,  tmpFFT_fcr + i, dim, spec128);
			}
			
			for (int i = 0; i < dim; i++) {
				nmppsFFT128Fwd_32fcr(tmpFFT_fcr + i * dim, 1, wantedFFT_fcr + i, dim, spec128);
			}
			//

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
			
			for (int i = 0; i < size; i++) {
				wantedFFT_fcr[i].im *= -1;
			}
			
			for (int i = 0; i < size; i++) {
				productFFT_fcr[i].re = currFFT_fcr[i].re * wantedFFT_fcr[i].re - currFFT_fcr[i].im * wantedFFT_fcr[i].im;
				productFFT_fcr[i].im = currFFT_fcr[i].re * wantedFFT_fcr[i].im + currFFT_fcr[i].im * wantedFFT_fcr[i].re;
			}

			//VS_SetData(332, productFFT_fcr);

			//----------- inverse fft-------------- 
			//st = ippiFFTInv_CToC_32fc_C1R(productFFT_fc, width * 8, productIFFT_fc, width * 8, spec, 0);

			specInv128->dstStep = dim * 2;
			for (int i = 0; i < dim; i++) {
				nmppsFFT128Inv_32fcr(productFFT_fcr + i * dim, 1, tmpFFT_fcr + i,  dim,specInv128);
			}
			specInv128->dstStep = dim * 2;
			for (int i = 0; i < dim; i++) {
				nmppsFFT128Inv_32fcr(tmpFFT_fcr + i * dim, 1,  productIFFT_fcr + i, dim,specInv128);
			}
		
		


			VS_SetData(IFFT_IMG, productIFFT_fc);
			
			
			float max = 0;
			
			for (int i = 0; i < dim -  wantedSize ; i++) {
				for (int j = 0; j < dim -  wantedSize; j++) {
					//if (max < productIFFT_fc[i*dim + j].re) {
					//	max = productIFFT_fc[i*dim + j].re;
					if (max < productIFFT_fcr[i*dim + j].re) {
						max = productIFFT_fcr[i*dim + j].re;
			
						caught.x = j;
						caught.y = i;
					}
				}
			}

			printf("currFFT_fcr:\n");
			dump_32f("%.1f ", (float*)currFFT_fcr, 8, 16, dim * 2, 1);
			printf("wantedFFT_fcr:\n");
			dump_32f("%.1f ", (float*)wantedFFT_fcr, 8, 16, dim * 2, 1);
			printf("productFFT_fcr:\n");
			dump_32f("%.1f ", (float*)productFFT_fcr, 8, 16, dim * 2, 1);
			printf("productIFFT_fcr:\n");
			dump_32f("%.1f ", (float*)productIFFT_fcr, 8, 16, dim * 2, 1);


		}
		//Pos caughtNM;
		dtpRecv(dr, &caught, sizeof32(caught));
		//dtpRecv(dr, &caughtNM, sizeof32(caught));

		//nmppsSub_32s(currImage32s, prevImage32s + caught.x- wantedX + (caught.y-wantedY) * dim, diffImage32s, dim*dim);
		VS_SetData(DIFF_IMG, diffImage32s);
		int caughtOrgX, caughtOrgY, caughtOrgXX, caughtOrgYY;
		int wantedOrgX, wantedOrgY, wantedOrgXX, wantedOrgYY;
		
		dimToOrg(caught.x, caught.y, caughtOrgX, caughtOrgY, dim, orgSize, cropFactor);
		dimToOrg(caught.x+wantedSize, caught.y+ wantedSize, caughtOrgXX, caughtOrgYY, dim, orgSize, cropFactor);
		
		dimToOrg(wantedX, wantedY, wantedOrgX, wantedOrgY, dim, orgSize, cropFactor);
		dimToOrg(wantedX + wantedSize, wantedY + wantedSize, wantedOrgXX, wantedOrgYY, dim, orgSize, cropFactor);


		VS_Rectangle(PREV_ORIGIN_IMG, wantedOrgX - 1, wantedOrgY - 1, wantedOrgXX , wantedOrgYY, VS_RED, VS_NULL_COLOR);
		VS_Rectangle(CURR_ORIGIN_IMG, caughtOrgX - 1, caughtOrgY - 1, caughtOrgXX , caughtOrgYY, VS_GREEN, VS_NULL_COLOR);

		VS_Rectangle(PREV_IMG8, wantedX, wantedY, wantedX + wantedSize, wantedY + wantedSize, VS_RED, VS_NULL_COLOR);
		VS_Rectangle(CURR_IMG8,   caught.x-1, caught.y-1, caught.x + wantedSize, caught.y + wantedSize, VS_GREEN, VS_NULL_COLOR);
		VS_Rectangle(IFFT_IMG, caught.x-1, caught.y-1, caught.x + wantedSize, caught.y + wantedSize, VS_GREEN, VS_NULL_COLOR);
		VS_Rectangle(CURR_IMG_FC, caught.x-1, caught.y-1, caught.x + wantedSize, caught.y + wantedSize, VS_GREEN, VS_NULL_COLOR);
		VS_Text("dx:%d dy:%d\r\n", caught.x- wantedX, caught.y- wantedY);

		VS_Draw(VS_DRAW_ALL);
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
