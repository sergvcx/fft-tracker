
#define VS_RGB1 1
#define VS_RGB4 2
#define VS_RGB8 3
#define VS_RGB16 4
#define VS_RGB24 5
#define VS_RGB32 6
#define VS_RGB8_8 7
#define VS_RGB8_16 8
#define VS_RGB8_32 9
#define VS_RGB32F 10
#define VS_RGB32FC 11
#include <stdio.h>
void	vsSaveImage(char* filename, void* data, int width, int height, int type) {
	
	int* data32 = (int*)data;
	FILE* f = fopen(filename,"wb");
	int size= width * height;
	int size32=0;
	switch (type) {
	case VS_RGB1 :
		size32 = size/32; break;
	case VS_RGB4:
		size32 = size/8; break;
	case VS_RGB8_8:
	case VS_RGB8:
		size32 = size/4; break;
	case VS_RGB16 :
	case VS_RGB8_16:
		size32 = size/2; break;
	case VS_RGB24 :
		size32 = size*3/4; break;
	case VS_RGB32:
	case VS_RGB8_32:
	case VS_RGB32F:
		size32 = size; break;
	case VS_RGB32FC:
		size32 = size*2; break;
	}
	if (f) {
		int id=0x00006407;
		fwrite(&id, 1, 1, f);
		fwrite(&type, 1, 1, f);
		fwrite(&width, 1, 1, f);
		fwrite(&height, 1, 1, f);
		//fwrite(data, 1, size32, f);

		int block32 = 4*1024;
		if (1) while (size32) {
			size_t rsize;
			if (size32 < block32) {
				rsize = fwrite(data32,  sizeof(int),size32, f);
				size32 -= size32;
				
			}
			else {
				rsize = fwrite(data32,  sizeof(int), block32, f);
				size32 -= block32;
				data32 += block32;
			}
		}
		fclose(f);
	}
}

int	vsReadImage(const char* filename, void* data, int* width, int* height, int* type) {
	FILE* f = fopen(filename, "rb");

	if (f) {

		fseek(f, 0, SEEK_END);
		int filesize = ftell(f);
		fseek(f, 0, SEEK_SET);
		int id;// = 0x00006407;
		fread(&id, sizeof(int), 1, f);
		fread(type, sizeof(int), 1, f);
		fread(width, sizeof(int), 1, f);
		fread(height, sizeof(int), 1, f);
		
		int count = *width * *height;
		
		int datasize32=0; // data size
		switch (*type) {
		case VS_RGB1:
			datasize32 = count / 32; break;
		case VS_RGB4:
			datasize32 = count / 8; break;
		case VS_RGB8_8:
		case VS_RGB8:
			datasize32 = count / 4; break;
		case VS_RGB16:
		case VS_RGB8_16:
			datasize32 = count / 2; break;
		case VS_RGB24:
			datasize32 = count * 3 / 4; break;
		case VS_RGB32:
		case VS_RGB8_32:
		case VS_RGB32F:
			datasize32 = count; break;
		case VS_RGB32FC:
			datasize32 = count * 2; break;
		default:
			printf("error: incorrect image type %d\n",*type);
			fclose(f);
			return 2;
		}
		if ((datasize32 + 4) * sizeof(int) > filesize) {
			fclose(f);
			return -1;
		}

		fread(data, sizeof(int), datasize32 , f);
		fclose(f);
		return 0;
	}
	return 1;
}

