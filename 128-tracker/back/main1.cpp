#include "dtp/dtp.h"
#include "dtp/file.h"
#include "hal/ringbuffert.h"
#include "dtp/mc12101.h"
#include "mc12101load_nm.h"
#include "nmpp.h"

const int WIDTH = 128;
const int HEIGHT = 128;
const int ECHO = 128;
const int SIZE =  WIDTH * HEIGHT;


__attribute__((section(".data.imu1"))) 	nm32fcr ringBufferLo[SIZE];
__attribute__((section(".data.imu2"))) 	nm32fcr ringBufferHi[SIZE];

#define X86_TO_NM0_BUFFER_SIZE ECHO		//0
#define X86_TO_NM1_BUFFER_SIZE SIZE*2	//1
#define NM0_TO_X86_BUFFER_SIZE ECHO		//2
#define NM1_TO_X86_BUFFER_SIZE ECHO		//3
#define NM0_TO_NM1_BUFFER_SIZE ECHO		//4
#define NM1_TO_NM0_BUFFER_SIZE SIZE*2	//5


#define FILE "../exchange.bin"
int main(){
	printf("nmc1 started\n");
	int file_desc = 0;
	do {
		file_desc = dtpOpenFile(FILE, "rb");
	} while (file_desc < 0);

	unsigned ring_addr[6];
	printf("0\n");
	dtpRecv(file_desc, ring_addr, 6);
	printf("1\n");
	
	printf("2\n");
	for (int i = 0; i < 6; i++)
		printf("%d: %08x\n", i, ring_addr[i]);

	
	// -------------------------------------------------------------

	int rbIn  = dtpOpenRingbufferDefault(ring_addr[1]);
	int rbOut = dtpOpenRingbufferDefault(ring_addr[5]);


	while (1) {
		//printf("<<< 1:\n");
		dtpRecv(rbIn, ringBufferLo, SIZE/4);
		nmppsConvert_8s32s((nm8s*)ringBufferLo, (nm32s*)ringBufferHi, SIZE);
		
		//printf(">>> 2:\n");
		dtpSend(rbOut, ringBufferHi, SIZE );
		//printf("<<< 3:\n");
		dtpRecv(rbIn, ringBufferLo, SIZE /4);
		nmppsConvert_8s32s((nm8s*)ringBufferLo, (nm32s*)ringBufferHi, SIZE);
		//printf(">>>> 4:\n");
		dtpSend(rbOut, ringBufferHi, SIZE );
	}

	dtpClose(file_desc);

    return 0;
}