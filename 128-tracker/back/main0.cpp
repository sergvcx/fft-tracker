//#include "ippdefs.h"
//#include "ippi.h"
//#include "ippcore.h"
//#include "ipps.h"
//#include "ippi.h"
#include "nmtype.h"
#include "nmpp.h"
#include "nmblas.h"
#include "math.h"
#include "dtp/dtp.h"
#include "dtp/file.h"
#include "dtp/mc12101.h"
#include "stdio.h"
#include "string.h"
#include "hal/ringbuffert.h"
#include "dumpx.h"
#include "mc12101load_nm.h"
#include "tracker.h"
void swap(void** ptr0, void** ptr1) {
	void* tmp = *ptr1;
	*ptr1 = *ptr0;
	*ptr0 = tmp;
}



NmppPoint caught;

//const int WIDTH = 128;
//const int HEIGHT= 128;
//const int ECHO= 128; 
//const int SIZE= WIDTH *HEIGHT;
//#define FULL_BANK 32*1024

//__attribute__((section(".data.imu1"))) 		int nm1_to_nm0_buffer[FULL_BANK]; 
//__attribute__((section(".data.imu2"))) 		nm32fcr ringBufferHi[SIZE];
//__attribute__ ((section (".data.imu3"))) 	nm32fcr currFFT_fcr[SIZE];
//__attribute__ ((section (".data.imu4"))) 	nm32fcr wantedFFT_fcr[SIZE];
//__attribute__ ((section (".data.imu5"))) 	nm32fcr tmpFFT_fcr[SIZE];
//__attribute__ ((section (".data.imu6"))) 	nm32fcr productFFT_fcr[SIZE];
//__attribute__ ((section (".data.imu7"))) 	nm32fcr productIFFT_fcr[SIZE];
//__attribute__ ((section (".data.emi"))) 	nm32fcr buffer[128*128*8];

//__attribute__((section(".data.imu1"))) 	int nm1_to_nm0_buffer[FULL_BANK];
//__attribute__((section(".data.imu2"))) 	nm32fcr ringBufferHi[SIZE];
__attribute__((section(".data.imu3"))) 	nm32fcr tmpFFT_fcr[DIM*DIM];
__attribute__((section(".data.imu4"))) 	nm32fcr FFT0_fcr[DIM*DIM];
__attribute__((section(".data.imu5"))) 	nm32fcr FFT1_fcr[DIM*DIM];
//__attribute__((section(".data.imu6"))) 	nm32fcr productFFT_fcr[SIZE];
//__attribute__((section(".data.imu7"))) 	nm32fcr productIFFT_fcr[SIZE];



__attribute__((section(".data.imu0"))) nm32fcr fwdBuffer0[120];
__attribute__((section(".data.imu1"))) nm32fcr fwdBuffer1[120];
__attribute__((section(".data.imu2"))) nm32fcr fwdBuffer2[96];
__attribute__((section(".data.imu0"))) nm32fcr fwdBuffer3[64];

__attribute__((section(".data.imu0"))) nm32fcr invBuffer0[120];
__attribute__((section(".data.imu1"))) nm32fcr invBuffer1[120];
__attribute__((section(".data.imu2"))) nm32fcr invBuffer2[96];
__attribute__((section(".data.imu0"))) nm32fcr invBuffer3[64];



#define FILE "../exchange.bin"

//#define X86_TO_NM0_BUFFER_SIZE ECHO		//0
//#define X86_TO_NM1_BUFFER_SIZE SIZE*4	//1
//#define NM0_TO_X86_BUFFER_SIZE ECHO		//2
//#define NM1_TO_X86_BUFFER_SIZE ECHO		//3
//#define NM0_TO_NM1_BUFFER_SIZE ECHO		//4
//#define NM1_TO_NM0_BUFFER_SIZE SIZE	//5
//
//#define DIM 128
//
//__attribute__((section(".data.shmem0"))) HalRingBufferData<int, 2> ring_x86_to_nm0;
//__attribute__((section(".data.shmem0"))) HalRingBufferData<int, 2> ring_x86_to_nm1;
//__attribute__((section(".data.shmem0"))) HalRingBufferData<int, 2> ring_nm0_to_x86;
//__attribute__((section(".data.shmem0"))) HalRingBufferData<int, 2> ring_nm1_to_x86;
//__attribute__((section(".data.shmem0"))) HalRingBufferData<int, 2> ring_nm0_to_nm1;
//__attribute__((section(".data.shmem0"))) HalRingBufferData<int, 2> ring_nm1_to_nm0;


__attribute__((section(".data.shmem0"))) HalRingBufferData<int, 2> ring_x86_to_nm1_cmd;
__attribute__((section(".data.shmem0"))) HalRingBufferData<int, 2> ring_x86_to_nm1_img;
__attribute__((section(".data.shmem0"))) HalRingBufferData<int, 2> ring_nm1_to_x86_out;
__attribute__((section(".data.shmem0"))) HalRingBufferData<int, 2> ring_nm1_to_nm0_cmd;
__attribute__((section(".data.shmem0"))) HalRingBufferData<int, 2> ring_nm1_to_nm0_diff;
__attribute__((section(".data.shmem0"))) HalRingBufferData<int, 2> ring_nm0_to_nm1_corr;

__attribute__((section(".data.shmem0"))) int data_x86_to_nm1_cmd[16 * 16 ]; //sizeof32(Cmd_x86_to_nm1)
__attribute__((section(".data.emi")))    int data_x86_to_nm1_img[16 * 256 * 256 / 4];
__attribute__((section(".data.emi")))    int data_nm1_to_x86_out[2 * DIM*DIM * 2]; // declared on nm1 
__attribute__((section(".data.shmem0"))) int data_nm1_to_nm0_cmd[16 * sizeof(Cmd_nm1_to_nm0)];
__attribute__((section(".data.imu6")))   int data_nm1_to_nm0_diff[2 * DIM*DIM];
__attribute__((section(".data.imu7")))	 int data_nm0_to_nm1_corr[2 * DIM*DIM];



//__attribute__((section(".data.shmem1"))) int x86_to_nm1_buffer[X86_TO_NM1_BUFFER_SIZE];
//__attribute__((section(".data.shmem0"))) int x86_to_nm0_buffer[X86_TO_NM0_BUFFER_SIZE];
//__attribute__((section(".data.shmem0"))) int nm0_to_x86_buffer[NM0_TO_X86_BUFFER_SIZE];
//__attribute__((section(".data.shmem0"))) int nm1_to_x86_buffer[NM1_TO_X86_BUFFER_SIZE];
//__attribute__((section(".data.shmem0"))) int nm0_to_nm1_buffer[NM0_TO_NM1_BUFFER_SIZE];

//__attribute__((section(".data.shmem0"))) int data_cnd_nm1_x86[4*DIM*DIM];


//__attribute__((section(".data.emi"))) int img_m1_to_nm0_buffer[FULL_BANK];






//HalRingBufferData<int, 128 * 128 * 4> ring;

void* nmppsCopy_32f_(void* src, void* dst, unsigned size) {
	//return memcpy(dst, src, size);
	//return (int*)dst+size;
	return 0;
}
/*
extern "C" void halSleep( int) {
	
}*/


static void* memCopyPop(const void *src, void *dst, unsigned int size32) {
	if (size32 & 1)
		memcpy(dst, src, size32 * sizeof(int));
	else
		nmppsCopy_32f((nm32f*)src, (nm32f*)dst, size32);
	return 0;
}
static void* memCopyPush(const void *src, void *dst, unsigned int size32) {
	//memcpy(dst, src, size32 * sizeof(int));
	if (((int)src & 1 )||( (int)dst & 1 )||( size32 & 1))
		printf("error\n");
	nmppsCopy_32f((nm32f*)src, (nm32f*)dst, size32);
	return 0;
}

extern "C" void DisableInterrupts_IMR_Low(int);

#define DISABLE_SYS_TIMER()  DisableInterrupts_IMR_Low(1 << 10); *((int*)(0x40000800)) = 0;
//? *((int*)(0x40000802)) = 1;

int* toGlobal0(void* addr) {
	if ((unsigned)addr < 0x40000)
		return (int*)addr + 0x40000;
	return (int*)addr;
}
int* toLocal0(void* addr) {
	if ((unsigned)addr >= 0x40000 && (unsigned)addr< 0x80000)
		return (int*)addr - 0x40000;
	return (int*)addr;
}

int main()
{

	DISABLE_SYS_TIMER()
	

	int file_desc = dtpOpenFile(FILE, "wb");


	ring_x86_to_nm1_cmd.init(sizeof32(data_x86_to_nm1_cmd));
	ring_x86_to_nm1_img.init(sizeof32(data_x86_to_nm1_img));
	ring_nm1_to_x86_out.init(sizeof32(data_nm1_to_x86_out));
	ring_nm1_to_nm0_cmd.init(sizeof32(data_nm1_to_nm0_cmd));
	ring_nm1_to_nm0_diff.init(sizeof32(data_nm1_to_nm0_diff));
	ring_nm0_to_nm1_corr.init(sizeof32(data_nm0_to_nm1_corr));

	ring_x86_to_nm1_cmd.data= toGlobal0(data_x86_to_nm1_cmd);
	ring_x86_to_nm1_img.data= toGlobal0(data_x86_to_nm1_img);
	ring_nm1_to_x86_out.data= toGlobal0(data_nm1_to_x86_out);
	ring_nm1_to_nm0_cmd.data= toGlobal0(data_nm1_to_nm0_cmd);
	ring_nm1_to_nm0_diff.data=toGlobal0(data_nm1_to_nm0_diff);
	ring_nm0_to_nm1_corr.data=toGlobal0(data_nm0_to_nm1_corr);

	ring_x86_to_nm1_cmd.bufferId  = 0x8601cdcd;
	ring_x86_to_nm1_img.bufferId  = 0x8601b00f;
	ring_nm1_to_x86_out.bufferId  = 0x0186b00f;
	ring_nm1_to_nm0_cmd.bufferId  = 0x0100cdcd;
	ring_nm1_to_nm0_diff.bufferId = 0x0100deef;
	ring_nm0_to_nm1_corr.bufferId = 0x0001beef;

	HalRingBufferData<int, 2>* ring[6];
	// write to file addr of pc-nm0 ring buffers
	ring[0] = &ring_x86_to_nm1_cmd;
	ring[1] = &ring_x86_to_nm1_img;
	ring[2] = &ring_nm1_to_x86_out;
	ring[3] = &ring_nm1_to_nm0_cmd;
	ring[4] = &ring_nm1_to_nm0_diff;
	ring[5] = &ring_nm0_to_nm1_corr;

	dtpSend(file_desc, ring, 6);
	dtpClose(file_desc);


	for (int i = 0; i < 6; i++)
		printf("%d: ring:%08x data:%08x size:%8d id:%08x\n", i, (int)ring[i], (int)ring[i]->data, ring[i]->size, ring[i]->bufferId);


	//--------------pc-nm0----------------

	int rbCmd     = dtpOpenRingbuffer(&ring_nm1_to_nm0_cmd,  memCopyPush, memCopyPop);
	//int rbDiff    = dtpOpenRingbuffer(&ring_nm1_to_nm0_diff, memCopyPush, memCopyPop);
	//int rbCorr    = dtpOpenRingbuffer(&ring_nm0_to_nm1_corr, memCopyPush, memCopyPop);
	int rbTo86    = dtpOpenRingbuffer(&ring_nm1_to_x86_out, memCopyPush, memCopyPop);


	//---------- fwd spec ----------
// специальная структура прямого БПФ, которая должна быть инициализирована с помощью nmppsFFT128FwdInit_32fcr
	NmppsFFTSpec_32fcr specFwd;
	specFwd.Buffs[0] = fwdBuffer0;
	specFwd.Buffs[1] = fwdBuffer1;
	specFwd.Buffs[2] = fwdBuffer2;
	specFwd.Buffs[3] = fwdBuffer3;
	nmppsFFT128FwdInit_32fcr(&specFwd); // инициализация БПФ

	// специальная структура обратного БПФ, которая должна быть инициализирована с помощью nmppsFFT128InvInit_32fcr
	NmppsFFTSpec_32fcr specInv;
	specInv.Buffs[0] = invBuffer0;
	specInv.Buffs[1] = invBuffer1;
	specInv.Buffs[2] = invBuffer2;
	specInv.Buffs[3] = invBuffer3;
	nmppsFFT128InvInit_32fcr(&specInv); // инициализация БПФ


	
	
	//clock_t t0, t1;


	
	Cmd_nm1_to_nm0 cmd;
	printf("Waiting handshake ... \n");
	dtpRecv(rbCmd, &cmd, sizeof32(cmd));
	if (cmd.command == 0x6407600D)
		printf("Handshake ok. Working ... \n");
	else {
		printf("Handshake error %d\n",cmd.command);
		return -1;
	}
	

	
	while (1){
		
		//t1 = clock();
		
		dtpRecv(rbCmd,&cmd, sizeof32(cmd));
		printf("in: %d 0x%x\n", cmd.counter, cmd.command);
		if (cmd.command == DO_FFT0) {
			while (ring_nm1_to_nm0_diff.isEmpty())
				printf("ring_nm1_to_nm0_diff: head:%d tail:%d\n", ring_nm1_to_nm0_diff.head, ring_nm1_to_nm0_diff.tail);

			nm32s* in = toLocal0(ring_nm1_to_nm0_diff.ptrTail());
			nmppsConvert_32s32fcr(in, FFT0_fcr, DIM*DIM);
			ring_nm1_to_nm0_diff.tail += DIM * DIM;
			for (int i = 0; i < DIM; i++) {
				nmppsFFT128Fwd_32fcr(FFT0_fcr + i * DIM, 1, tmpFFT_fcr + i, DIM, &specFwd);
			}
			for (int i = 0; i < DIM; i++) {
				nmppsFFT128Fwd_32fcr(tmpFFT_fcr + i * DIM, 1, FFT0_fcr + i, DIM, &specFwd);
			}
		}
		else if (cmd.command == DO_FFT1) {
			while (ring_nm1_to_nm0_diff.isEmpty())
				printf("ring_nm1_to_nm0_diff: head:%d tail:%d\n", ring_nm1_to_nm0_diff.head, ring_nm1_to_nm0_diff.tail);

			nm32s* in = toLocal0(ring_nm1_to_nm0_diff.ptrTail());
			nmppsConvert_32s32fcr(in, FFT1_fcr, DIM*DIM );
			ring_nm1_to_nm0_diff.tail+=DIM*DIM;
			for (int i = 0; i < DIM; i++) {
				nmppsFFT128Fwd_32fcr(FFT1_fcr + i * DIM, 1, tmpFFT_fcr + i, DIM, &specFwd);
			}
			
			for (int i = 0; i < DIM; i++) {
				nmppsFFT128Fwd_32fcr(tmpFFT_fcr + i * DIM, 1, FFT1_fcr + i, DIM, &specFwd);
			}
		}
		else if (cmd.command == DO_CORR) {

			nmppsConjMul_32fcr(FFT0_fcr, FFT1_fcr, tmpFFT_fcr, DIM*DIM);
			//----------- inverse fft-------------- 
			for (int i = 0; i < DIM; i++) {
				nmppsFFT128Inv_32fcr(tmpFFT_fcr + i * DIM, 1, FFT1_fcr + i, DIM, &specInv);
			}
			for (int i = 0; i < DIM; i++) {
				nmppsFFT128Inv_32fcr(FFT1_fcr + i * DIM, 1, tmpFFT_fcr + i, DIM, &specInv);
			}
			
			//float max = 0;
			int idx = nmblas_isamax(DIM*DIM * 2, (const float*)tmpFFT_fcr, 1);
			caught.y = idx >> 8;
			caught.x = (idx % 256) >> 1;
			
			dtpSend(rbTo86, &caught, sizeof32(caught));
			printf("out:\n");
		}
		else {
			printf("%d command:%d\n", cmd.counter, cmd.command);
		}

		//t0 = clock();
	}
}
