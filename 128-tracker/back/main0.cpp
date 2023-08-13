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

void* toGlobal0(void* addr) {
	if ((unsigned)addr < 0x40000)
		return (int*)addr + 0x40000;
	return (int*)addr;
}
void* toLocal0(void* addr) {
	if ((unsigned)addr >= 0x40000 && addr< 0x80000)
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

//	ring_x86_to_nm0.init(X86_TO_NM0_BUFFER_SIZE);	ring_x86_to_nm0.data = x86_to_nm0_buffer; ring_x86_to_nm0.bufferId=0xBEEF0000;
//	//ring_x86_to_nm1.init(X86_TO_NM1_BUFFER_SIZE);	ring_x86_to_nm1.data = x86_to_nm1_buffer+0x80; ring_x86_to_nm1.bufferId=0xBEEF0001;
//	ring_nm0_to_x86.init(NM0_TO_X86_BUFFER_SIZE);	ring_nm0_to_x86.data = nm0_to_x86_buffer; ring_nm0_to_x86.bufferId=0xBEEF0002;
//	//ring_nm1_to_x86.init(NM1_TO_X86_BUFFER_SIZE);	ring_nm1_to_x86.data = nm1_to_x86_buffer; ring_nm1_to_x86.bufferId=0xBEEF0003;
//	ring_nm0_to_nm1.init(NM0_TO_NM1_BUFFER_SIZE);	ring_nm0_to_nm1.data = nm0_to_nm1_buffer; ring_nm0_to_nm1.bufferId=0xBEEF0004;
//	ring_nm1_to_nm0.init(sizeof(nm1_to_nm0_buffer));ring_nm1_to_nm0.data = (int*)nm1_to_nm0_buffer+0x40000; ring_nm1_to_nm0.bufferId=0xC1C0B00F;

	//printf("input: %p, output: %p\n", &ring_x86_to_nm1, &ring_X86_output);
	//printf("host2nm1_buffer: %p, data_X86_output: %p\n", host2nm1_buffer, data_X86_output);
	//int offset = 0x40000 + 0x40000 * ncl_getProcessorNo();

	
	
	for (int i = 0; i < 6; i++)
		printf("%d: ring:%08x data:%08x size:%8d id:%08x\n", i, ring[i], ring[i]->data, ring[i]->size, ring[i]->bufferId);


	//int ring_x86_to_nm1_ = (int)&ring_x86_to_nm1;
	//int ring_nm1_to_nm0_a = (int)&ring_nm1_to_host;
	//if (ring_input_addr < 0x80000) ring_input_addr += offset;
	//if (ring_output_addr < 0x80000) ring_output_addr += offset;
	//printf("ring_input_addr %p, ring_output_addr %p\n", ring_input_addr, ring_output_addr);
	//dtpSend(file_desc, &ring_input_addr, 1);
	//dtpSend(file_desc, &ring_output_addr, 1);
	

	
	// -------------------------------------------------------------


	//--------------pc-nm0----------------

	int rbCmd     = dtpOpenRingbuffer(&ring_nm1_to_nm0_cmd,  memCopyPush, memCopyPop);
	//int rbDiff    = dtpOpenRingbuffer(&ring_nm1_to_nm0_diff, memCopyPush, memCopyPop);
	int rbCorr    = dtpOpenRingbuffer(&ring_nm0_to_nm1_corr, memCopyPush, memCopyPop);
	int rbTo86    = dtpOpenRingbuffer(&ring_nm1_to_x86_out, memCopyPush, memCopyPop);

	


	int counter = 0;

	//int dim = 128;
	//int size = dim * dim;
	//int width = dim;
	//int height = dim;



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

	//---------------------


	int wantedSize = 10;
	//int wantedY = 10;
	//int wantedX = 10;
	caught.x = 10;
	caught.y = 11;
	
	
	//nm32fcr* currImage_fcr   = productIFFT_fcr;
	//nm32fcr* wantedImage_fcr = ringBufferHi;
	clock_t t0, t1;


	
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
		
		//dtpRecv(dr, currImage_fcr, size * 2);
		//printf("--- first recv ---- \n");

		//printf("head =%d tail=%d \n",ring.head, ring.tail);
		//printf("<<< ... 1:\n");
		t1 = clock();
		//printf("time= %ld\n", t1 - t0);

		
		dtpRecv(rbCmd,&cmd, sizeof32(cmd));
		printf("in: %d 0x%x\n", cmd.counter, cmd.command);
		//printf("<<<]\n:");
		//dtpRecv(rbIn, productFFT_fcr, size);
		if (cmd.command == DO_FFT0) {
			while (ring_nm1_to_nm0_diff.isEmpty())
				printf("ring_nm1_to_nm0_diff: head:%d tail:%d\n", ring_nm1_to_nm0_diff.head, ring_nm1_to_nm0_diff.tail);

			nm32s* in = toLocal0(ring_nm1_to_nm0_diff.ptrTail());
			//printf("tailptr: %x\n", in);
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
			
			float max = 0;
			int idx = nmblas_isamax(DIM*DIM * 2, (const float*)tmpFFT_fcr, 1);
			caught.y = idx >> 8;
			caught.x = (idx % 256) >> 1;
			
			dtpSend(rbTo86, &caught, sizeof32(caught));
			printf("out:\n");
		}
		else {
			//printf("%d command:%d\n", cmd.counter, cmd.command);
		}


		//printf("head =%d tail=%d \n", ring.head, ring.tail);
		//dtpAsyncRecv(dr, &task);
		//printf("%d %f\n",counter,  currImage_fcr[3].re);
		//counter++;
		//while ( dtpAsyncStatus(dr, &task)!=DTP_ST_DONE);
		//for (int i = 0; i < 128; i++) {
		//	printf("%d %f %f\n",i, currImage_fcr[i].re, currImage_fcr[i].im);
		//}
		//printf("curr:\n");
		//dump_32f("%.0f ", (float*)currImage_fcr, 8, 16, dim*2, 1);

		//dtpRecv(dr, wantedImage_fcr, size * 2);
		//dtpSend(dw, &caught, sizeof32(caught));
		//printf("<<< ... 2:\n");
		
		//printf("<<<]\n:");
		//nmppsConvert_32s32fcr((nm32s*)productFFT_fcr, wantedImage_fcr, size);
		//printf("--- second recv ----\n");
		
		//printf("head =%d tail=%d \n", ring.head, ring.tail);
		
		
		//dtpSend(dw, &caught, sizeof32(caught));
		//printf("--- send done----\n");
	//}
	//{
	

		

		//---------------------------------
		//dtpRecv(rbIn, wantedImage_fcr, size * 2);

		//printf("wanted:\n");
		//dump_32f("%.0f ", (float*)wantedImage_fcr, 8, 16, dim * 2, 1);


		//for (int i = 0; i < dim; i++) {
		//	nmppsFFT128Fwd_32fcr(wantedImage_fcr + i * dim, 1,  tmpFFT_fcr + i, dim, &specFwd);
		//}
		//for (int i = 0; i < dim; i++) {
		//	nmppsFFT128Fwd_32fcr(tmpFFT_fcr + i * dim, 1,  wantedFFT_fcr + i, dim,  &specFwd);
		//}
		

		//printf("currFFT_fcr:\n");
		//dump_32f("%.1f ", (float*)currFFT_fcr, 8, 16, dim * 2, 1);
		//
		//
		//printf("wantedFFT_fcr:\n");
		//dump_32f("%.1f ", (float*)wantedFFT_fcr, 8, 16, dim * 2, 1);

		//---------- mul -------- 
		
	

		
	
		
		//float max = 0;
		//
		//int idx=nmblas_isamax(size*2,(const float*)productIFFT_fcr,1);
		//
		//caught.y = idx>>8;
		//caught.x = (idx%256)>>1;

		/*
		for (int i = 0; i < dim -  wantedSize ; i++) {
			for (int j = 0; j < dim -  wantedSize; j++) {
				if (max < productIFFT_fcr[i*dim + j].re) {
					max = productIFFT_fcr[i*dim + j].re;
					caught.x = j;
					caught.y = i;
				}
			}
		}
		*/
		//printf("productFFT_fcr:\n");
		//dump_32f("%.1f ", (float*)productFFT_fcr, 8, 16, dim * 2, 1);
		//
		//
		//printf("productIFFT_fcr:\n");
		//dump_32f("%.1f ", (float*)productIFFT_fcr, 8, 16, dim * 2, 1);


		//swap((void**)&currImage_fcr, (void**)&wantedImage_fcr);
		t0 = clock();
		//dtpSend(rbOut, &caught, sizeof32(caught));
	}
}
