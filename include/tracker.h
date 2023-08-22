//#define WIDTH 256
//#define HEIGHT 256
//#define SIZE WIDTH*WIDTH
//#define IMAGE_SIZE WIDTH*HEIGHT
//#define SIZE_RING_BUFFER 1
//
//#define STOP 666
//#define STOPPED 777
//#include "tringbuffer.h"
//#include "nmtype.h"
//
//struct Message {
//	unsigned command;
//	unsigned reply;
//};
//
//union Image256x256_32s {
//	int data2D[256][256];
//	int data[256 * 256];
//};
//union Image256x256_8u {
//	unsigned data2D[256][256/4];
//	unsigned data[256*256 / 4];
//};
//
//struct TrackingObject {
//	int wantedX;
//	int wantedY;
//	int width;
//	int height;
//	int caughtX;
//	int caughtY;
//};

#define DO_FFT0 0xFFD0
#define DO_FFT1 0xFFD1
#define DO_CORR 0xC077
#define DO_BLUR 0xB100
#define DO_FFT  0x0FFD
#define DIM 128


struct Cmd_nm1_to_nm0{
	unsigned counter;
	unsigned command;
} ;

#ifndef __NM__
#define NmppSize IppiSize
#define NmppRect IppiRect
#endif


struct Cmd_x86_to_nm1{
	unsigned counter;
	unsigned command;
	unsigned frmIndex;
	unsigned frmAddress;
	NmppSize frmSize;
	NmppRect frmRoi;
	void info() {
		printf("cnt:%d cmd:%d indx:%d addr:%08x W:%d H:%d w:%d h:%d x:%d y:%d\n", counter, command, frmIndex, frmAddress, frmSize.width, frmSize.height, frmRoi.width, frmRoi.height,frmRoi.x, frmRoi.y);
	}
} ;

//typedef tHalRingBuffer<Image256x256_32s, SIZE_RING_BUFFER> RingBufferImage256x256_32s;
//typedef tHalRingBuffer<Image256x256_8u,  SIZE_RING_BUFFER> RingBufferImage256x256_8u;
//typedef tHalRingBuffer<TrackingObject,   SIZE_RING_BUFFER> RingBufferTracking;


