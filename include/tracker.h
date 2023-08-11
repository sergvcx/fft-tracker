#define WIDTH 256
#define HEIGHT 256
#define SIZE WIDTH*WIDTH
#define IMAGE_SIZE WIDTH*HEIGHT
#define SIZE_RING_BUFFER 1

#define STOP 666
#define STOPPED 777
#include "tringbuffer.h"
struct Message {
	unsigned command;
	unsigned reply;
};

union Image256x256_32s {
	int data2D[256][256];
	int data[256 * 256];
};
union Image256x256_8u {
	unsigned data2D[256][256/4];
	unsigned data[256*256 / 4];
};

struct TrackingObject {
	int wantedX;
	int wantedY;
	int width;
	int height;
	int caughtX;
	int caughtY;
};
typedef tHalRingBuffer<Image256x256_32s, SIZE_RING_BUFFER> RingBufferImage256x256_32s;
typedef tHalRingBuffer<Image256x256_8u,  SIZE_RING_BUFFER> RingBufferImage256x256_8u;
typedef tHalRingBuffer<TrackingObject,   SIZE_RING_BUFFER> RingBufferTracking;


