#include "ippdefs.h"
#include "ippi.h"
#include "ippcore.h"
#include "ipps.h"
#include "nmpp.h"
#include "math.h"
#include "VShell.h"

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
	if (!VS_Init())
		return 0;
	//if (!VS_Bind("..\\Samples\\Road1.avi"))
	if (!VS_Bind("..\\Samples\\MAKS2015_256x256.avi1"))
	//if (!VS_Bind("d:\\video\\films\\256x256\\2xFormula2.avi"))
		return 0;
	int width  = VS_GetWidth(VS_SOURCE);
	int height = VS_GetHeight(VS_SOURCE);
	int size = width * height;


	IppiFFTSpec_C_32fc *spec;
	IppStatus st;

	Ipp32fc *currImage_fc	= (Ipp32fc *)ippMalloc(size * sizeof(Ipp32fc));
	Ipp32fc *prevImage_fc	= (Ipp32fc *)ippMalloc(size * sizeof(Ipp32fc));
	
	Ipp32fc *currFFT_fc		= (Ipp32fc *)ippMalloc(size * sizeof(Ipp32fc));
	//Ipp32fc *prevFFT_fc		= (Ipp32fc *)ippMalloc(size * sizeof(Ipp32fc));

	Ipp32fc *wantedImage_fc = (Ipp32fc *)ippMalloc(size * sizeof(Ipp32fc));
	Ipp32fc *wantedFFT_fc	= (Ipp32fc *)ippMalloc(size * sizeof(Ipp32fc));
	
	Ipp32fc *productFFT_fc	= (Ipp32fc *)ippMalloc(size * sizeof(Ipp32fc));
	Ipp32fc *productIFFT_fc = (Ipp32fc *)ippMalloc(size * sizeof(Ipp32fc));
	nm32s   *productIFFT_32s= nmppsMalloc_32s(size);

	nm8u* srcBlur = nmppsMalloc_8u(size);

	st = ippiFFTInitAlloc_C_32fc(&spec, 8, 8, IPP_FFT_DIV_INV_BY_N, ippAlgHintNone);

	VS_CreateSlider("wantedSize", 1, 15, 255, 1, 37);

	VS_CreateImage("current Image", 1, width, height, VS_RGB24, 0);
	VS_CreateImage("current Image bw", 2, width, height, VS_RGB8, 0);
	VS_CreateImage("productFFT_fc", 3, width, height, VS_RGB8_32, 0);
	VS_CreateImage("previous Image", 4, width, height, VS_RGB8, 0);
	VS_CreateImage("Blur", 10, width, height, VS_RGB8, 0);

	nm8u* currImage = nmppsMalloc_8u(size);
	nm8u* prevImage = nmppsMalloc_8u(size);
	Ipp8u* blurImage = (Ipp8u*)ippMalloc(size);

	nm32u* currImageC = nmppsMalloc_32u(size);
	
	int wantedSize = 10;
	int wantedY = 10;
	int wantedX = 10;
	int caughtX = 10;
	int caughtY = 10;
	
	S_VS_MouseStatus MouseStatus;
	int status;
	while (status=VS_Run()) {

		if (!(status&VS_PAUSE)) {
			swap((void**)&currImage_fc, (void**)&prevImage_fc);
			swap((void**)&currImage, (void**)&prevImage);
			VS_GetData(VS_SOURCE, currImageC);
			VS_GetGrayData(VS_SOURCE, currImage);
			wantedX = caughtX;
			wantedY = caughtY;
		}

		VS_SetData(1, currImageC);
		VS_SetData(2, currImage);
		VS_SetData(4, prevImage);
		int wantedSize = VS_GetSlider(1);
		
		VS_GetMouseStatus(&MouseStatus);
		if (MouseStatus.nKey == VS_MOUSE_LBUTTON) {
			wantedY = MouseStatus.nY;
			wantedX = MouseStatus.nX;
		}
		VS_Rectangle(4, wantedX, wantedY, wantedX + wantedSize, wantedY + wantedSize, VS_RED, VS_NULL_COLOR);
		
		//for (int i = 0; i < size; i++) {
		//	blurImage[i] = 0;
		//}

		IppiSize s;
		s.width = width;
		s.height = height;

		ippiFilterBox_8u_C1R((Ipp8u*)currImage, width, blurImage, width, s, { wantedSize, wantedSize }, { 17, 17 });

		for (int i = 0; i < size; i++) {
			//currImage_fc[i].re = (float)currImage[i] - blur(currImage, width, height, wantedSize, i%width, i / width);
			currImage_fc[i].re = (float)currImage[i] - blurImage[i];
			currImage_fc[i].im = 0;
			srcBlur[i] = 128 + currImage_fc[i].re;
			currFFT_fc[i]     = {0,0};
			productIFFT_fc[i] = {0,0};
			wantedImage_fc[i] = {0,0};
			wantedFFT_fc[i]   = {0,0};
			productFFT_fc[i]  = {0,0};
		}
		VS_SetData(10, srcBlur);

		for (int i = 0; i < height * wantedSize; i += width) {
			for (int j = 0; j < wantedSize; j++) {
				wantedImage_fc[i + j].re = prevImage_fc[wantedY*width+wantedX+ i + j].re;;
			}
		}

		st = ippiFFTFwd_CToC_32fc_C1R(wantedImage_fc, width * 8, wantedFFT_fc, width * 8, spec, 0); 
		st = ippiFFTFwd_CToC_32fc_C1R(currImage_fc,   width * 8, currFFT_fc, width * 8, spec, 0); 

		for (int i = 0; i < size; i++) {
			wantedFFT_fc[i].im *= -1; 
		}
		for (int i = 0; i < size; i++) { 
			productFFT_fc[i].re = currFFT_fc[i].re * wantedFFT_fc[i].re - currFFT_fc[i].im * wantedFFT_fc[i].im;
			productFFT_fc[i].im = currFFT_fc[i].re * wantedFFT_fc[i].im + currFFT_fc[i].im * wantedFFT_fc[i].re;
		}
		st = ippiFFTInv_CToC_32fc_C1R(productFFT_fc, width * 8, productIFFT_fc, width * 8, spec, 0);
		for (int i = 0; i < size; i++) {
			productIFFT_32s[i] = (nm32s)productIFFT_fc[i].re;
		}
		VS_SetData(3, productIFFT_32s);
		
		do {
			//for (int i = 0; i < 10; i++) {
				float max = 0;
				int indx = 0;

				for (int i = 0; i < size; i++) {
					if (max < productIFFT_fc[i].re) {
						max = productIFFT_fc[i].re;
						indx = i;
					}
				}
				caughtX = indx % width;
				caughtY = (indx >> 8);
				productIFFT_fc[indx].re = 0;
				
			//}
		}while ((caughtY + wantedSize >= height) || (caughtX + wantedSize >= width));
		
		//VS_Rectangle(1, caughtX, caughtY, caughtX + wantedSize, caughtY + wantedSize, VS_GREEN, VS_NULL_COLOR);

		VS_Rectangle(3, caughtX, caughtY, caughtX + wantedSize, caughtY + wantedSize, VS_GREEN, VS_NULL_COLOR);
		VS_Rectangle(1, caughtX, caughtY, caughtX + wantedSize, caughtY + wantedSize, VS_GREEN, VS_NULL_COLOR);
		VS_Text("dx:%d dy:%d\r\n", caughtX- wantedX, caughtY- wantedY);

		VS_Draw(VS_DRAW_ALL);
	}
}