/******************************************************************************
*    RC Module
*    NeuroMatrix(r) NM6406 Software
*
*    Image Processing Library 
*    Software design:    S.Mushkaev
*
*    Contents:           Sobel operator
*
*
******************************************************************************/

#ifndef  __SOBEL_H_INCLUDED__
#define  __SOBEL_H_INCLUDED__

void sobel( const unsigned char *source, unsigned char *result,int width,int height);
void sobelCmplx(const unsigned char *source, nm32fcr* result, int width, int height);
#endif
