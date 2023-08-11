#include "dtp/dtp.h"
#include "dtp/file.h"
#include "hal/ringbuffert.h"
#include "dtp/mc12101.h"
#include "mc12101load_nm.h"
#include "nmpp.h"
#include "string.h"
#include "dumpx.h"

const int WIDTH = 128;
const int HEIGHT = 128;
const int ECHO = 128;
const int SIZE =  WIDTH * HEIGHT;


__attribute__((section(".data.imu1"))) 	nm32fcr ringBufferLo[SIZE];
__attribute__((section(".data.imu2"))) 	nm32fcr ringBufferHi[SIZE];

//#define X86_TO_NM0_BUFFER_SIZE ECHO		//0
#define X86_TO_NM1_BUFFER_SIZE SIZE*2	//1
//#define NM0_TO_X86_BUFFER_SIZE ECHO		//2
//#define NM1_TO_X86_BUFFER_SIZE ECHO		//3
//#define NM0_TO_NM1_BUFFER_SIZE ECHO		//4
//#define NM1_TO_NM0_BUFFER_SIZE SIZE*2	//5

#define FULL_BANK 32*1024 // 128kB
//__attribute__((section(".data.shmem1"))) int x86_to_nm1_buffer[X86_TO_NM1_BUFFER_SIZE];
//__attribute__((section(".data.shmem0"))) int x86_to_nm0_buffer[X86_TO_NM0_BUFFER_SIZE];
//__attribute__((section(".data.shmem0"))) int nm0_to_x86_buffer[NM0_TO_X86_BUFFER_SIZE];
//__attribute__((section(".data.shmem0"))) int nm1_to_x86_buffer[NM1_TO_X86_BUFFER_SIZE];
//__attribute__((section(".data.shmem0"))) int nm0_to_nm1_buffer[NM0_TO_NM1_BUFFER_SIZE];
//__attribute__((section(".data.shmem0"))) int nm1_to_nm0_buffer[NM1_TO_NM0_BUFFER_SIZE];

__attribute__((section(".data.imu3"))) int x86_to_nm1_buffer[FULL_BANK];

// ok
// memcp pop  
// opy push 

static void* memCopyPop(const void *src, void *dst, unsigned int size32) {
	 memcpy(dst, src, size32 * sizeof(int));
	//if ((int)src & 1 || (int)dst & 1 || size32 & 1)
	//	printf("error\n");
	//nmppsCopy_32s((nm32s*)src,(nm32s*) dst, size32);
	//printf("dma start \n");
	//halDmaStart(src, dst, size32);
	//printf("dma end \n");
	////halSleep(1000);
	//
	//while (!halDmaIsCompleted());
	//printf("dma completed \n");
	//dump_32f("%s ", dst, 128, 16,16,0);
	return 0;
}
static void* memCopyPush(const void *src, void *dst, unsigned int size32) {
	//memcpy(dst, src, size32 * sizeof(int));
	if ((int)src & 1 || (int)dst & 1 || size32 & 1)
		printf("error\n");
	nmppsCopy_32s((nm32s*)src,(nm32s*) dst, size32);
	return 0;
}



#define FILE "../exchange.bin"
int main(){
	
	
	printf("nmc1 started\n");
	int file_desc = 0;
	do {
		file_desc = dtpOpenFile(FILE, "rb");
	} while (file_desc < 0);
	
	HalRingBufferData<int, 2>* ringBufferData[6];
	dtpRecv(file_desc, ringBufferData, 6);
	dtpClose(file_desc);

	HalRingBufferData<int, 2>* ring_x86_to_nm0=ringBufferData[0];
	HalRingBufferData<int, 2>* ring_x86_to_nm1=ringBufferData[1];
	HalRingBufferData<int, 2>* ring_nm0_to_x86=ringBufferData[2];
	HalRingBufferData<int, 2>* ring_nm1_to_x86=ringBufferData[3];
	HalRingBufferData<int, 2>* ring_nm0_to_nm1=ringBufferData[4];
	HalRingBufferData<int, 2>* ring_nm1_to_nm0=ringBufferData[5];

	ring_x86_to_nm1->init(sizeof(x86_to_nm1_buffer));
	ring_x86_to_nm1->data=x86_to_nm1_buffer;
	ring_x86_to_nm1->bufferId = 0x8601B00F;

	printf("2\n");
	for (int i = 0; i < 6; i++)
		printf("%d: ring:%08x data:%08x id:%08x\n", i, ringBufferData[i], ringBufferData[i]->data, ringBufferData[i]->bufferId);

	//return 1;
	//halDmaInit();
	//halEnbExtInt();


	
	// -------------------------------------------------------------
	int rbIn=dtpOpenRingbuffer(ring_x86_to_nm1, memCopyPush, memCopyPop);
	//int rbIn=dtpOpenRingbuffer(ring_addr[1], nmppsCopy_32s, nmppsCopy_32s);

	//int rbIn  = dtpOpenRingbufferDefault(ring_addr[1]);
	int rbOut = dtpOpenRingbufferDefault(ring_nm1_to_nm0);


	while (1) {
		printf("<<< 1:\n");
		dtpRecv(rbIn, ringBufferLo, SIZE/4);
		nmppsConvert_8s32s((nm8s*)ringBufferLo, (nm32s*)ringBufferHi, SIZE);
		
		printf(">>> 2:\n");
		dtpSend(rbOut, ringBufferHi, SIZE );
		printf("<<< 3:\n");
		dtpRecv(rbIn, ringBufferLo, SIZE /4);
		nmppsConvert_8s32s((nm8s*)ringBufferLo, (nm32s*)ringBufferHi, SIZE);
		printf(">>>> 4:\n");
		dtpSend(rbOut, ringBufferHi, SIZE );
	}

	dtpClose(file_desc);

    return 0;
}