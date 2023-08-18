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
#include "hal/cache.h"
#include "dumpx.h"
#include "mc12101load_nm.h"
#include "tracker.h"
#include "vsimg.h"
#include "nmassert.h"
void swap(void** ptr0, void** ptr1) {
	void* tmp = *ptr1;
	*ptr1 = *ptr0;
	*ptr0 = tmp;
}

extern "C" void halSleep(int) {};


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



__attribute__((section(".data.imu0"))) nm32fcr fwdBuffer0[100+120];
__attribute__((section(".data.imu1"))) nm32fcr fwdBuffer1[100+120];
__attribute__((section(".data.imu2"))) nm32fcr fwdBuffer2[100+96];
__attribute__((section(".data.imu0"))) nm32fcr fwdBuffer3[100+64];
__attribute__((section(".data.imu0"))) nm32fcr invBuffer0[100+120];
__attribute__((section(".data.imu1"))) nm32fcr invBuffer1[100+120];
__attribute__((section(".data.imu2"))) nm32fcr invBuffer2[100+96];
__attribute__((section(".data.imu0"))) nm32fcr invBuffer3[100+64];



#define FILE "../exchange.bin"

__attribute__((section(".data.shmem0"))) HalRingBufferData<int, 2> ring_x86_to_nm1_cmd;
__attribute__((section(".data.shmem0"))) HalRingBufferData<int, 2> ring_x86_to_nm1_img;
__attribute__((section(".data.shmem0"))) HalRingBufferData<int, 2> ring_nm1_to_x86_out;
__attribute__((section(".data.shmem0"))) HalRingBufferData<int, 2> ring_nm1_to_nm0_cmd;
__attribute__((section(".data.shmem0"))) HalRingBufferData<int, 2> ring_nm1_to_nm0_diff;
__attribute__((section(".data.shmem0"))) HalRingBufferData<int, 2> ring_nm0_to_nm1_corr;

__attribute__((section(".data.shmem0")))  int data_x86_to_nm1_cmd[16 * 16 ]; //sizeof32(Cmd_x86_to_nm1)
__attribute__((section(".data.emi.bss"))) int data_x86_to_nm1_img[128 * 512 * 512 / 4];
__attribute__((section(".data.emi")))     int data_nm1_to_x86_out[2 * DIM*DIM * 2]; // declared on nm1 
__attribute__((section(".data.shmem0")))  int data_nm1_to_nm0_cmd[1024]; //sizeof(Cmd_nm1_to_nm0)];
__attribute__((section(".data.imu6")))    int data_nm1_to_nm0_diff[2 * DIM*DIM];
__attribute__((section(".data.imu7")))	  int data_nm0_to_nm1_corr[2 * DIM*DIM];



void* nmppsCopy_32f_(void* src, void* dst, unsigned size) {
	//return memcpy(dst, src, size);
	//return (int*)dst+size;
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
#define VS_SAVE_IMAGE 
//vsSaveImage
#define USE_SEMIHOSTING 1


#define PRINT(...) printf(__VA_ARGS__)
#define PRINTRT(...) 
//printf(__VA_ARGS__)
//#define PRINT0				printf
#define PRINT1(a) 				printf(a) 
#define PRINT2(a,b) 			printf(a,b) 
#define PRINT3(a,b,c) 			printf(a,b,c) 
#define PRINT4(a,b,c,d) 		printf(a,b,c,d) 
#define PRINT5(a,b,c,d,e) 		printf(a,b,c,d,e) 
#define PRINT6(a,b,c,d,f,g) 	printf(a,b,c,d,f,g) 


//printf 

//printf

//#define  printf 
__attribute__((section(".text.nmpp")))
int main()
{
	PRINT("Starting nm0 ... \n");
	
	DISABLE_SYS_TIMER()
	
	halInstrCacheEnable(); 


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

	if (USE_SEMIHOSTING) {
		int file_desc = dtpOpenFile(FILE, "wb");
		dtpSend(file_desc, ring, 6);
		dtpClose(file_desc);
	}

	memset(ring_x86_to_nm1_img.data, 0, ring_x86_to_nm1_img.size);
	memset(ring_nm1_to_x86_out.data, 0, ring_nm1_to_x86_out.size);
	memset(ring_nm1_to_x86_out.data, 0, ring_nm1_to_x86_out.size);

	for (int i = 0; i < 6; i++)
		PRINT ("%d: ring:%08x data:%08x size:%8d id:%08x\n", i, (int)ring[i], (int)ring[i]->data, ring[i]->size, ring[i]->bufferId);


	//--------------pc-nm0----------------

	//int rbCmd     = dtpOpenRingbuffer(&ring_nm1_to_nm0_cmd,  memCopyPush, memCopyPop);
	int rbCmd     = dtpOpenRingbufferDefault(&ring_nm1_to_nm0_cmd);
	//int rbDiff    = dtpOpenRingbuffer(&ring_nm1_to_nm0_diff, memCopyPush, memCopyPop);
	//int rbCorr    = dtpOpenRingbuffer(&ring_nm0_to_nm1_corr, memCopyPush, memCopyPop);
	//int rbTo86    = dtpOpenRingbuffer(&ring_nm1_to_x86_out, memCopyPush, memCopyPop);
	int rbTo86    = dtpOpenRingbufferDefault(&ring_nm1_to_x86_out);


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
	PRINT("Waiting handshake ... \n");
	dtpRecv(rbCmd, &cmd, sizeof32(cmd));
	if (cmd.command == 0x6407600D)
		PRINT("Handshake with nm1 - ok \n");
	else {
		//PRINT0("Handshake with nm1 - error %d\n",cmd.command);
		PRINT("Handshake with nm1 - error \n");
		return -1;
	}
	long long sz= ring_x86_to_nm1_img.size;
	//dtpSend(rbTo86, &ring_x86_to_nm1_img.size, 1);
	dtpSend(rbTo86, &sz, 2);
	
	PRINT("NM0 - running ....\n");
	
	while (1){
		
		//t1 = clock();
		
		dtpRecv(rbCmd,&cmd, sizeof32(Cmd_nm1_to_nm0));
		PRINTRT("in: %d 0x%x\n", cmd.counter, cmd.command);
		if (cmd.command == DO_FFT0) {
			PRINTRT("out: DO_FFT0\n");
			while (ring_nm1_to_nm0_diff.isEmpty())
				PRINT("ring_nm1_to_nm0_diff: head:%d tail:%d\n", ring_nm1_to_nm0_diff.head, ring_nm1_to_nm0_diff.tail);

			nm32s* in = toLocal0(ring_nm1_to_nm0_diff.ptrTail());
			//printf("---32s-\n");
			//dump_32s("%d ", (int*)in, 16, 16, 128, 1);
			

			//VS_SAVE_IMAGE("0_inFFT0_32s.vsimg", in, DIM, DIM, VS_RGB8_32);
			VS_SAVE_IMAGE("0_inFFT0_32s.vsimg", in, DIM, DIM, VS_RGB8_32);
			nmppsConvert_32s32fcr(in, FFT0_fcr, DIM*DIM);
			VS_SAVE_IMAGE("0_inFFT0.vsimg", FFT0_fcr, DIM, DIM, VS_RGB32FC);
			//printf("---32s-\n");
			//dump_32s("%d ", (int*)in, 16, 16, 128, 1);
			//printf("--32f--\n");
			//dump_32f("%.0f ", (float*)FFT0_fcr, 16, 32, 256, 1);


			ring_nm1_to_nm0_diff.tail += DIM * DIM;

			//dump_32f("%f ", (nm32f*)FFT0_fcr, 16, 16, DIM * 2, 0);
			//printf("---\n");
			for (int i = 0; i < DIM; i++) {
				nmppsFFT128Fwd_32fcr(FFT0_fcr + i * DIM, 1, tmpFFT_fcr + i, DIM, &specFwd);
			}
			//dump_32f("%f ", (nm32f*)tmpFFT_fcr, 16, 16, DIM * 2, 0);
			//printf("---\n");
			for (int i = 0; i < DIM; i++) {

				nmppsFFT128Fwd_32fcr(tmpFFT_fcr + i * DIM, 1, FFT0_fcr + i, DIM, &specFwd);
			}
			//dump_32f("%f ", (nm32f*)FFT0_fcr, 16, 16, DIM * 2,0);
			VS_SAVE_IMAGE("0_outFFT0.vsimg", FFT0_fcr, DIM , DIM, VS_RGB32FC);

		}
		else if (cmd.command == DO_FFT1) {
			PRINTRT("out: DO_FFT1\n");
			//while (ring_nm1_to_nm0_diff.isEmpty())
			//	PRINT0("ring_nm1_to_nm0_diff: head:%d tail:%d\n", ring_nm1_to_nm0_diff.head, ring_nm1_to_nm0_diff.tail);

			nm32s* in = toLocal0(ring_nm1_to_nm0_diff.ptrTail());
			//printf("--32s--\n");
			//dump_32s("%d ", (int*)in, 16, 16, 128, 1);
			//printf("--32f--\n");
			VS_SAVE_IMAGE("0_inFFT1_32s.vsimg", in, DIM, DIM, VS_RGB8_32);
			nmppsConvert_32s32fcr(in, FFT1_fcr, DIM*DIM );
			VS_SAVE_IMAGE("0_inFFT1_32fcr.vsimg", FFT1_fcr, DIM, DIM, VS_RGB32FC);

			//dump_32f("%.0f ",(float*) FFT1_fcr, 16, 32, 256, 1);
			ring_nm1_to_nm0_diff.tail+=DIM*DIM;
			for (int i = 0; i < DIM; i++) {
				nmppsFFT128Fwd_32fcr(FFT1_fcr + i * DIM, 1, tmpFFT_fcr + i, DIM, &specFwd);
			}
			
			for (int i = 0; i < DIM; i++) {
				nmppsFFT128Fwd_32fcr(tmpFFT_fcr + i * DIM, 1, FFT1_fcr + i, DIM, &specFwd);
			}
			VS_SAVE_IMAGE("0_outFFT1.vsimg", FFT1_fcr, DIM, DIM, VS_RGB32FC);
			
		}
		else if (cmd.command == DO_CORR) {
			nmppsConjMul_32fcr( FFT1_fcr, FFT0_fcr, tmpFFT_fcr, DIM*DIM);
			VS_SAVE_IMAGE("0_ProdFFT.vsimg", tmpFFT_fcr, DIM, DIM, VS_RGB32FC);
			//dump_32f("%f ", (nm32f*)tmpFFT_fcr, 16, 16, DIM * 2, 0);
			//----------- inverse fft-------------- 
			for (int i = 0; i < DIM; i++) {
				nmppsFFT128Inv_32fcr(tmpFFT_fcr + i * DIM, 1, FFT1_fcr + i, DIM, &specInv);
			}
			for (int i = 0; i < DIM; i++) {
				nmppsFFT128Inv_32fcr(FFT1_fcr + i * DIM, 1, tmpFFT_fcr + i, DIM, &specInv);
			}
			NmppPoint rcaught;
			float max = 0;
			nm32fcr rmax;
			float* temp32f = (float*)tmpFFT_fcr;
			//for (int i = 0; i < DIM; i++) {
			//	for (int j = 0; j < DIM; j++) {
			//		//if (max < productIFFT_fc[i*dim + j].re) {
			//		//	max = productIFFT_fc[i*dim + j].re;
			//		//float abs= 
			//		float re = tmpFFT_fcr[i*DIM + j].re;
			//		float im = tmpFFT_fcr[i*DIM + j].im;
			//		//float abs = re * re + im * im;
			//
			//		//productAbs32f[i*dim + j] = abs;
			//		//temp32f[i*DIM + j] = re;
			//
			//		if (max < re) {
			//			max = re;
			//			rcaught.y = i;
			//			rcaught.x = j;
			//			rmax = { im,re };
			//		}
			//		if (max < im) {
			//			max = im;
			//			rcaught.y = i;
			//			rcaught.x = j;
			//			rmax = { im,re };
			//		}
			//
			//
			//	}
			//}
			////dump_32f("%.3f ", (nm32f*)tmpFFT_fcr, 14, 14, DIM * 2, 0);
			//printf("---\n");
			//dump_32f("%.2e ", (nm32f*)tmpFFT_fcr, 14, 14, DIM * 2, 0);

			VS_SAVE_IMAGE("0_IFFT.vsimg", tmpFFT_fcr, DIM, DIM, VS_RGB32FC);
			//vsSaveImage("0_IFFT.vsimg", tmpFFT_fcr, DIM, DIM, VS_RGB32FC);
			//vsSaveImage("nmIFFT.vsimg", tmpFFT_fcr, DIM, DIM, VS_RGB32FC);
			//float max = 0;
			
			int idx = nmblas_isamax(DIM*DIM * 2, (const float*)tmpFFT_fcr, 1);
			caught.y = idx >> 8;
			caught.x = (idx % 256) >> 1;
			nm32fcr maxx = tmpFFT_fcr[idx>>1];// ->re;
			//if (rcaught.y != caught.y || rcaught.x != caught.x) {
			//	printf("ERROR %d %d %d %d\n", rcaught.x, rcaught.y, caught.x, caught.y);
			//	//NMASSERT_MSG(0,"erroorrr");
			//}
			
			//dtpSend(rbTo86, &rcaught, sizeof32(rcaught));
			
			
			dtpSend(rbTo86, &caught, sizeof32(caught));
			
			
			//dtpSend(rbTo86, &maxx, sizeof32(maxx));// bug
			//dtpSend(rbTo86, &rmax, sizeof32(rmax));// bug
			
		}
		else {
			PRINT("%d command:%d\n", cmd.counter, cmd.command);
		}

		//t0 = clock();
	}
	vsReadImage(0, 0, 0, 0, 0);
	vsWriteImage(0, 0, 0, 0, 0);
}


/*
extern "C" void halSleep( int) {

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
}*/
