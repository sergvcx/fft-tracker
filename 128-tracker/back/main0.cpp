//#include "ippdefs.h"
//#include "ippi.h"
//#include "ippcore.h"
//#include "ipps.h"
//#include "ippi.h"
#include "nmpp.h"
#include "math.h"
#include "dtp/dtp.h"
#include "dtp/file.h"
#include "dtp/mc12101.h"
#include "stdio.h"
#include "string.h"
#include "hal/ringbuffert.h"
#include "dumpx.h"
#include "mc12101load_nm.h"

void swap(void** ptr0, void** ptr1) {
	void* tmp = *ptr1;
	*ptr1 = *ptr0;
	*ptr0 = tmp;
}


struct Pos{
	int x;
	int y;
} caught;

const int WIDTH = 128;
const int HEIGHT= 128;
const int ECHO= 128; 
const int SIZE= WIDTH *HEIGHT;
#define FULL_BANK 32*1024

__attribute__((section(".data.imu1"))) 		int nm1_to_nm0_buffer[FULL_BANK]; 
__attribute__((section(".data.imu2"))) 		nm32fcr ringBufferHi[SIZE];
__attribute__ ((section (".data.imu3"))) 	nm32fcr currFFT_fcr[SIZE];
__attribute__ ((section (".data.imu4"))) 	nm32fcr wantedFFT_fcr[SIZE];
__attribute__ ((section (".data.imu5"))) 	nm32fcr tmpFFT_fcr[SIZE];
__attribute__ ((section (".data.imu6"))) 	nm32fcr productFFT_fcr[SIZE];
__attribute__ ((section (".data.imu7"))) 	nm32fcr productIFFT_fcr[SIZE];
//__attribute__ ((section (".data.emi"))) 	nm32fcr buffer[128*128*8];


__attribute__((section(".data.imu0"))) nm32fcr fwdBuffer0[120];
__attribute__((section(".data.imu0"))) nm32fcr fwdBuffer1[120];
__attribute__((section(".data.imu0"))) nm32fcr fwdBuffer2[96];
__attribute__((section(".data.imu0"))) nm32fcr fwdBuffer3[64];

__attribute__((section(".data.imu0"))) nm32fcr invBuffer0[120];
__attribute__((section(".data.imu0"))) nm32fcr invBuffer1[120];
__attribute__((section(".data.imu0"))) nm32fcr invBuffer2[96];
__attribute__((section(".data.imu0"))) nm32fcr invBuffer3[64];



#define FILE "../exchange.bin"

#define X86_TO_NM0_BUFFER_SIZE ECHO		//0
#define X86_TO_NM1_BUFFER_SIZE SIZE*4	//1
#define NM0_TO_X86_BUFFER_SIZE ECHO		//2
#define NM1_TO_X86_BUFFER_SIZE ECHO		//3
#define NM0_TO_NM1_BUFFER_SIZE ECHO		//4
#define NM1_TO_NM0_BUFFER_SIZE SIZE	//5



__attribute__((section(".data.shmem0"))) HalRingBufferData<int, 2> ring_x86_to_nm0;
__attribute__((section(".data.shmem0"))) HalRingBufferData<int, 2> ring_x86_to_nm1;
__attribute__((section(".data.shmem0"))) HalRingBufferData<int, 2> ring_nm0_to_x86;
__attribute__((section(".data.shmem0"))) HalRingBufferData<int, 2> ring_nm1_to_x86;
__attribute__((section(".data.shmem0"))) HalRingBufferData<int, 2> ring_nm0_to_nm1;
__attribute__((section(".data.shmem0"))) HalRingBufferData<int, 2> ring_nm1_to_nm0;

//__attribute__((section(".data.shmem1"))) int x86_to_nm1_buffer[X86_TO_NM1_BUFFER_SIZE];
__attribute__((section(".data.shmem0"))) int x86_to_nm0_buffer[X86_TO_NM0_BUFFER_SIZE];
__attribute__((section(".data.shmem0"))) int nm0_to_x86_buffer[NM0_TO_X86_BUFFER_SIZE];
__attribute__((section(".data.shmem0"))) int nm1_to_x86_buffer[NM1_TO_X86_BUFFER_SIZE];
__attribute__((section(".data.shmem0"))) int nm0_to_nm1_buffer[NM0_TO_NM1_BUFFER_SIZE];
//__attribute__((section(".data.imu7"))) int nm1_to_nm0_buffer[FULL_BANK];


//HalRingBufferData<int, 128 * 128 * 4> ring;

void* nmppsCopy_32f_(void* src, void* dst, unsigned size) {
	//return memcpy(dst, src, size);
	//return (int*)dst+size;
	return 0;
}
/*
extern "C" void halSleep( int) {
	
}*/
int main()
{
	int file_desc = dtpOpenFile(FILE, "wb");

	ring_x86_to_nm0.init(X86_TO_NM0_BUFFER_SIZE);	ring_x86_to_nm0.data = x86_to_nm0_buffer; ring_x86_to_nm0.bufferId=0xBEEF0000;
	//ring_x86_to_nm1.init(X86_TO_NM1_BUFFER_SIZE);	ring_x86_to_nm1.data = x86_to_nm1_buffer+0x80; ring_x86_to_nm1.bufferId=0xBEEF0001;
	ring_nm0_to_x86.init(NM0_TO_X86_BUFFER_SIZE);	ring_nm0_to_x86.data = nm0_to_x86_buffer; ring_nm0_to_x86.bufferId=0xBEEF0002;
	//ring_nm1_to_x86.init(NM1_TO_X86_BUFFER_SIZE);	ring_nm1_to_x86.data = nm1_to_x86_buffer; ring_nm1_to_x86.bufferId=0xBEEF0003;
	ring_nm0_to_nm1.init(NM0_TO_NM1_BUFFER_SIZE);	ring_nm0_to_nm1.data = nm0_to_nm1_buffer; ring_nm0_to_nm1.bufferId=0xBEEF0004;
	ring_nm1_to_nm0.init(sizeof(nm1_to_nm0_buffer));ring_nm1_to_nm0.data = (int*)nm1_to_nm0_buffer+0x40000; ring_nm1_to_nm0.bufferId=0xC1C0B00F;

	//printf("input: %p, output: %p\n", &ring_x86_to_nm1, &ring_X86_output);
	//printf("host2nm1_buffer: %p, data_X86_output: %p\n", host2nm1_buffer, data_X86_output);
	//int offset = 0x40000 + 0x40000 * ncl_getProcessorNo();

	HalRingBufferData<int, 2>* ring[6];
	// write to file addr of pc-nm0 ring buffers
	ring[0] = &ring_x86_to_nm0;
	ring[1] = &ring_x86_to_nm1;
	ring[2] = &ring_nm0_to_x86;
	ring[3] = &ring_nm1_to_x86;
	ring[4] = &ring_nm0_to_nm1;
	ring[5] = &ring_nm1_to_nm0;
	
	for (int i = 0; i < 6; i++)
		printf("%d: ring:%08x data:%08x size:%8d id:%08x\n", i, ring[i], ring[i]->data, ring[i]->size, ring[i]->bufferId);


	//int ring_x86_to_nm1_ = (int)&ring_x86_to_nm1;
	//int ring_nm1_to_nm0_a = (int)&ring_nm1_to_host;
	//if (ring_input_addr < 0x80000) ring_input_addr += offset;
	//if (ring_output_addr < 0x80000) ring_output_addr += offset;
	//printf("ring_input_addr %p, ring_output_addr %p\n", ring_input_addr, ring_output_addr);
	//dtpSend(file_desc, &ring_input_addr, 1);
	//dtpSend(file_desc, &ring_output_addr, 1);
	printf("<<< ... 0\n");
	dtpSend(file_desc, ring, 6);
	printf("<<<]\n:");

	// write to file addr of nm0-nm1 ring buffers
	//ring_input_addr = (int)&ring_nm1_to_nm0;
	//ring_output_addr = (int)&ring_nm0_to_nm1;
	//if (ring_input_addr < 0x80000) ring_input_addr += offset;
	//if (ring_output_addr < 0x80000) ring_output_addr += offset;
	//printf("ring_input_addr %p, ring_output_addr %p\n", ring_input_addr, ring_output_addr);
	//dtpSend(file_desc, &ring_input_addr, 1);
	//dtpSend(file_desc, &ring_output_addr, 1);
	dtpClose(file_desc);
	
	// -------------------------------------------------------------


	//--------------pc-nm0----------------
	//int rbIn  = dtpOpenRingbufferDefault(&ring_x86_to_nm1);
	int rbIn  = dtpOpenRingbufferDefault(&ring_nm1_to_nm0);
	int rbOut = dtpOpenRingbufferDefault(&ring_nm0_to_x86);

	//int data[2] = { 0, 1 };
	//dtpSend(rb_desc_w, data, 2);
	//
	//dtpRecv(rb_desc_r, data, 2);
	//printf("recv: %d, %d\n", data[0], data[1]);
	//
	//dtpClose(rb_desc_r);
	//dtpClose(rb_desc_w);




	//--------------nm0-nm1----------------
	//rb_desc_r = dtpOpenRingbufferDefault(&ring_nm1_to_nm0);
	//rb_desc_w = dtpOpenRingbufferDefault(&ring_nm0_to_nm1);

	//data[0] = 0;
	//data[1] = 1;
	//dtpSend(rb_desc_w, data, 2);
	//
	//dtpRecv(rb_desc_r, data, 2);
	//printf("recv: %d, %d\n", data[0], data[1]);
	//
	//dtpClose(rb_desc_r);
	//dtpClose(rb_desc_w);


	/*


	ring.sizeofInt = 1;
	ring.init(128*128*4);
	ring.data= (int*)ringBufferLo;
	//ring.data= (int*)wantedImage_fcr;
	ring.sizeofInt = 1;
	
	int dr, dw;
	int drb = dtpOpenRingbuffer(&ring, nmppsCopy_32f_, nmppsCopy_32f_);


#ifdef __NM__
	int desc = dtpOpenPloadFile("../handshake.bin");

	printf("desc=%d \n", desc);
	dr = desc;
	dw = desc;
#endif

	//dr = dtpOpenFile("../curr-pc2nm.bin", "rb");
	//dw = dtpOpenFile("../caught-nm2pc.bin", "wb");

	int ringAddr = (int)&ring;
	dtpSend(dw, &ringAddr, 1); // send ring buffer address
	printf("----\n");
	*/

	int counter = 0;

	int dim = 128;
	int size = dim * dim;
	int width = dim;
	int height = dim;


	//DtpAsync task;
	//task.buf = currImage_fcr;
	//task.nwords = size * 2;
	//task.type = DTP_TASK_1D;

	//#define LOG2DIM 7



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
	
	
	nm32fcr* currImage_fcr   = productIFFT_fcr;
	nm32fcr* wantedImage_fcr = ringBufferHi;


	while (1){
		
		//dtpRecv(dr, currImage_fcr, size * 2);
		//printf("--- first recv ---- \n");

		//printf("head =%d tail=%d \n",ring.head, ring.tail);
		//printf("<<< ... 1:\n");
		dtpRecv(rbIn, tmpFFT_fcr, size );
		//printf("<<<]\n:");

		nmppsConvert_32s32fcr((nm32s*)tmpFFT_fcr,currImage_fcr, size);

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
		dtpRecv(rbIn, tmpFFT_fcr, size );
		//printf("<<<]\n:");
		nmppsConvert_32s32fcr((nm32s*)tmpFFT_fcr, wantedImage_fcr, size);
		//printf("--- second recv ----\n");
		
		//printf("head =%d tail=%d \n", ring.head, ring.tail);
		
		
		//dtpSend(dw, &caught, sizeof32(caught));
		//printf("--- send done----\n");
	//}
	//{
		// ----------forward fft ----------------
		
		//specFwd->dstStep =  dim*2;
		for (int i = 0; i < dim; i++) {
			nmppsFFT128Fwd_32fcr(currImage_fcr + i * dim, 1, tmpFFT_fcr + i, dim, &specFwd);
		}
		
		for (int i = 0; i < dim; i++) {
			nmppsFFT128Fwd_32fcr(tmpFFT_fcr  + i * dim, 1, currFFT_fcr + i,dim, &specFwd);
		}


		

		//---------------------------------
		//dtpRecv(rbIn, wantedImage_fcr, size * 2);

		//printf("wanted:\n");
		//dump_32f("%.0f ", (float*)wantedImage_fcr, 8, 16, dim * 2, 1);


		for (int i = 0; i < dim; i++) {
			nmppsFFT128Fwd_32fcr(wantedImage_fcr + i * dim, 1,  tmpFFT_fcr + i, dim, &specFwd);
		}
		for (int i = 0; i < dim; i++) {
			nmppsFFT128Fwd_32fcr(tmpFFT_fcr + i * dim, 1,  wantedFFT_fcr + i, dim,  &specFwd);
		}
		

		//printf("currFFT_fcr:\n");
		//dump_32f("%.1f ", (float*)currFFT_fcr, 8, 16, dim * 2, 1);
		//
		//
		//printf("wantedFFT_fcr:\n");
		//dump_32f("%.1f ", (float*)wantedFFT_fcr, 8, 16, dim * 2, 1);

		//---------- mul -------- 
		
		nmppsConjMul_32fcr(currFFT_fcr, wantedFFT_fcr, productFFT_fcr, dim*dim);
		//for (int i = 0; i < size; i++) {
		//	wantedFFT_fcr[i].im *= -1;
		//}
		//
		//for (int i = 0; i < size; i++) {
		//	productFFT_fcr[i].re = currFFT_fcr[i].re * wantedFFT_fcr[i].re - currFFT_fcr[i].im * wantedFFT_fcr[i].im;
		//	productFFT_fcr[i].im = currFFT_fcr[i].re * wantedFFT_fcr[i].im + currFFT_fcr[i].im * wantedFFT_fcr[i].re;
		//}
	
		
		//printf("head =%d tail=%d \n", ring.head, ring.tail);

		//for (int i = 0; i < 128; i++) {
		//	printf("%d %f %f\n", i, wantedImage_fcr[i].re, wantedImage_fcr[i].im);
		//}
		

		
		//----------- inverse fft-------------- 
		
		for (int i = 0; i < dim; i++) {
			nmppsFFT128Inv_32fcr(productFFT_fcr + i * dim, 1,  tmpFFT_fcr + i, dim, &specInv);
		}
		for (int i = 0; i < dim; i++) {
			nmppsFFT128Inv_32fcr(tmpFFT_fcr + i * dim, 1, productIFFT_fcr + i, dim, &specInv);
		}
		

	
	
		float max = 0;
	
		for (int i = 0; i < dim -  wantedSize ; i++) {
			for (int j = 0; j < dim -  wantedSize; j++) {
				if (max < productIFFT_fcr[i*dim + j].re) {
					max = productIFFT_fcr[i*dim + j].re;
					caught.x = j;
					caught.y = i;
				}
			}
		}
		//printf("productFFT_fcr:\n");
		//dump_32f("%.1f ", (float*)productFFT_fcr, 8, 16, dim * 2, 1);
		//
		//
		//printf("productIFFT_fcr:\n");
		//dump_32f("%.1f ", (float*)productIFFT_fcr, 8, 16, dim * 2, 1);


		//swap((void**)&currImage_fcr, (void**)&wantedImage_fcr);
		
		dtpSend(rbOut, &caught, sizeof32(caught));
	}
}
