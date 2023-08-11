#include "ippdefs.h"
#include "ippi.h"
#include "ippcore.h"
#include "ipps.h"
#include "nmpp.h"
#include "math.h"
#include "VShell.h"

#define IMG_WIDTH			 256
#define IMG_HEIGHT           256

int blur(nm8u *img, int width, int height, int size,  int x, int y) {
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


int main()
{
	if (!VS_Init())
		return 0;
	//if (!VS_Bind("d:\\GIT\\fft-tracker\\Samples\\MAKS2015_256x256.avi"))
	if (!VS_Bind("d:\\video\\films\\256x256\\2xFormula21.avi"))
		return 0;
	int width = VS_GetWidth(VS_SOURCE);
	int height = VS_GetHeight(VS_SOURCE);

	int size = width * height;

	VS_CreateSlider("Slider1", 1, 15, 255, 1, 220);
	VS_CreateSlider("Slider2", 2, 15, 255, 1, 220);

	IppiFFTSpec_C_32fc *spec;
	IppStatus st;

	Ipp32fc *srcf = (Ipp32fc *)ippMalloc(size * sizeof(Ipp32fc));
	Ipp32fc *dstfInv = (Ipp32fc *)ippMalloc(size * sizeof(Ipp32fc));
	Ipp32fc *dstf = (Ipp32fc *)ippMalloc(size * sizeof(Ipp32fc));

	Ipp32fc *srcFind = (Ipp32fc *)ippMalloc(size * sizeof(Ipp32fc));
	Ipp32fc *dstFind = (Ipp32fc *)ippMalloc(size * sizeof(Ipp32fc));
	Ipp32fc *Cor = (Ipp32fc *)ippMalloc(size * sizeof(Ipp32fc));

	st = ippiFFTInitAlloc_C_32fc(&spec, 8, 8, IPP_FFT_DIV_INV_BY_N, ippAlgHintNone);

	VS_CreateImage("Source Image", 1, width, height, VS_RGB24, 0);
	VS_CreateImage("Source Image_Find", 2, width, height, VS_RGB8, 0);
	VS_CreateImage("Cor", 3, width, height, VS_RGB8_32, 0);
	VS_CreateImage("Blur", 10, width, height, VS_RGB8, 0);
	//VS_CreateImage("SrcFind", 4, width, height, VS_RGB8_32, 0);

	nm8u* srcImage = nmppsMalloc_8u(size);
	nm8u* srcBlur = nmppsMalloc_8u(size);
	nm32u* srcImageC = nmppsMalloc_32u(size);
	nm8u* srcData = nmppsMalloc_8u(size);
	nm8u* Sub = nmppsMalloc_8u(2 * size);
	nm32s* CorU = nmppsMalloc_32s(size);

	int NowY = 10;
	int NowX = 10;
	//nmppmCopyua_8s((nm8s *)(srcImage), width, NowY * height + NowX, (nm8s *)srcData, WidthFind, HeightFind, WidthFind);
	S_VS_MouseStatus MouseStatus;
	while (VS_Run()) {
		VS_GetData(VS_SOURCE, srcImageC);
		VS_GetGrayData(VS_SOURCE, srcImage);

		int WidthFind = VS_GetSlider(1);
		int HeightFind = WidthFind;// VS_GetSlider(2);

		//bool a = sobel(srcImage, srcImageOrig, width, height);
		//bool b = sobel(srcData, srcDataSob, WidthFind, HeightFind);

		//if(MouseStatus.nKey == VS_MOUSE_LBUTTON) {
		//nmppmCopyua_8s((nm8s *)(srcImage), width, 100 * height + 100, (nm8s *)srcData, WidthFind, HeightFind, WidthFind);
		//VS_SetData(1, srcImage);
		//VS_SetData(2, srcData);

		float brightF = 0;
		float brightS = 0;

		for (int i = 0; i < width * height; i++) {
			brightS += srcImage[i];
		}
		brightS = brightS / (width * height);

		for (int i = 0; i < WidthFind * HeightFind; i++) {
			brightF += srcData[i];
		}

		brightF = brightF / (WidthFind * HeightFind);

		for (int i = 0; i < size; i++) {
			srcf[i].re = (float)srcImage[i] - brightS;

			srcBlur[i] =128+ srcImage[i]-blur(srcImage,width,height, WidthFind, i%width, i/width);

			Sub[i] = srcImage[i];
			srcf[i].im = 0;
			dstf[i].re = 0;
			dstf[i].im = 0;
			dstfInv[i].re = dstfInv[i].im = 0;
			srcFind[i].re = 0; //1 - Ðàñøèðèòü èçîáðàæåíèå Y äî ðàçìåðà (N1,N2), äîïîëíèâ åãî íóëÿìè
			srcFind[i].im = 0;
			dstFind[i].re = 0;
			dstFind[i].im = 0;
			Cor[i].re = 0;
			Cor[i].im = 0;
		}
		VS_SetData(10, srcBlur);

		int k = 0;
		float sum = 0;
		for (int i = 0; i < height * WidthFind; i += width) {
			for (int j = 0; j < WidthFind; j++) {
				srcFind[i + j].re = (float)srcData[k] - brightF;
				//srcFind[i + j].re -= brightF;
				k++;
			}
		}

		st = ippiFFTFwd_CToC_32fc_C1R(srcf, width * 8, dstf, width * 8, spec, 0); // FFT îò èñõîäíîãî èçîáðàæåíèÿ

		st = ippiFFTFwd_CToC_32fc_C1R(srcFind, width * 8, dstFind, width * 8, spec, 0); // FFT îò ôðàãìåíòà

		for (int i = 0; i < size; i++) {
			dstFind[i].im *= -1; // äëÿ êîìïëåêñíî-ñîïðÿæåíîé ìàòðèöû (Conj)
								 //sum += srcFind[i].re;
		}
		for (int i = 0; i < size; i++) { // êîððåëÿöèÿ - FFT(èñõîäíîãî) * Conj(FFT(ôðàãìåòà))
			Cor[i].re = dstf[i].re * dstFind[i].re - dstf[i].im * dstFind[i].im;
			Cor[i].im = dstf[i].re * dstFind[i].im + dstf[i].im * dstFind[i].re;
		}
		st = ippiFFTInv_CToC_32fc_C1R(Cor, width * 8, dstfInv, width * 8, spec, 0);
		for (int i = 0; i < size; i++) {
			CorU[i] = (nm32s)dstfInv[i].re;
			//CorU[i] = (nm32s)srcf[i].re;
			//CorF[i] = (nm32s)srcFind[i].re;
		}
		VS_SetData(1, srcImageC);
		//VS_SetData(2, srcData);
		VS_SetData(3, CorU);
		//VS_SetData(4, CorF);

		int indx, x, y;
		//for(int a = 0; a < 50; a++) {
		float max = 0;
		indx = 0;
		for (int i = 0; i < size; i++) {
			if (max < dstfInv[i].re) {
				max = dstfInv[i].re;
				indx = i;
			}
		}

		//dstfInv[indx].re = 0;
		y = (indx >> 8);
		x = indx % width;
		nmppsSet_8u( 0, Sub,
			size);
		for (int i = 0; i < HeightFind; i++) {
			for (int j = 0; j < WidthFind; j++) {
				Sub[(i + y) * width + j + x] = ABS(srcImage[(i + y) * width + j + x]-srcData[i * WidthFind + j]);
			}
		}

		VS_SetData(2, Sub);
		//NowX = x;
		//NowY = y;
		VS_GetMouseStatus(&MouseStatus);
		if (MouseStatus.nKey == VS_MOUSE_LBUTTON) {
			NowY = MouseStatus.nY;
			NowX = MouseStatus.nX;
		}
		VS_Rectangle(1, NowX, NowY, NowX + WidthFind, NowY + HeightFind, VS_RED, VS_NULL_COLOR);
		VS_Rectangle(3, NowX, NowY, NowX + WidthFind, NowY + HeightFind, VS_RED, VS_NULL_COLOR);
		nmppmCopyua_8s((nm8s *)(srcImage), width, NowY * height + NowX, (nm8s *)srcData, WidthFind, HeightFind, WidthFind);
		VS_Text("dx:%d dy:%d\r\n", NowX - x, NowY - y);
		//nmppmCopyua_8s((nm8s *)(srcImage), width, y * height + x, (nm8s *)srcData, WidthFind, HeightFind, WidthFind);
		VS_Rectangle(1, x, y, x + WidthFind, y + HeightFind, VS_BLUE, VS_NULL_COLOR);
		//}
		VS_Draw(VS_DRAW_ALL);
		//}
	}
}