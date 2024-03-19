#define  _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <Windows.h> //GUI 지원기능
#include <math.h>
#include <conio.h>
#include < stdlib.h>

/////////////////////
// 전역 변수부
/////////////////////
HWND hwnd;//윈도우 화면(외부,물리적)
HDC hdc; // 윈도우 화면 (내부,논리적)

FILE* rfp, * wfp;
char fileName[200];
int inH, inW, outH, outW;
int maskheight, maskwidth;
// 메모리 확보

unsigned char** inImage = NULL, ** outImage = NULL, ** mask1 = NULL;
int** temp = NULL;
/////////////////////
// 함수 선언부
/////////////////////
void loadImage(); int getIntValue(); void printImage(); void saveImage();
void mallocInputMemory(); void mallocOutputMemory(); void freeInputMemory();
void freeOutputMemory(); void freeMaskMemory(); void mallocTempMemory(); void freeTempMemory();
int compare(const void*, const void*); void freeDoubleMemory(double**, int );
double** mallocDoubleMemory(int , int );

void equalImage(); void addImage(); void printMenu(); void divImage();
void opImage(); void blackImage128(); void blackImageAvg();
void andImage(); void xorImage(); void orImage(); void zoomOut();
void zoomIn(); void zoomIn2(); void rotate(); void rotate2();
void moveImage(); void mirrorRL(); void mirrorUD(); void zoomOutAvg(); void histoStretch();
void endIn(); void histoEqual(); void emboss(); void blur(); void sharpning(); void sharpning2();
void zoomOutMid(); void rotate3(); void zoomIn3(); void edgever(); void edgehor(); void blur99();

void main() {

	hwnd = GetForegroundWindow();
	hdc = GetWindowDC(NULL);
	char inKey = 0;
	while (inKey != '9') {
		printMenu();
		inKey = _getch();
		system("cls");

		switch (inKey)
		{
		case '0':loadImage(); break;
		case'1': saveImage(); break;
		case '9':break;
		case 'a':case 'A': equalImage(); break;
		case 'b':case 'B':addImage(); break;
		case 'c':case 'C': divImage(); break;
		case 'd':case 'D':opImage(); break;
		case 'e':case 'E':blackImage128(); break;
		case'f':case'F':blackImageAvg(); break;
		case'g':case'G':andImage(); break;
		case'h':case'H':xorImage(); break;
		case'5':orImage(); break;
		case 'i':case'I': zoomOut(); break;
		case 'j':case'J':zoomIn(); break;
		case'k':case'K':zoomIn2(); break;
		case'l':case'L':rotate(); break;
		case'm':case'M':rotate2(); break;
		case'n':case'N':moveImage(); break;
		case'o':case'O':mirrorRL(); break;
		case'p':case'P':mirrorUD(); break;
		case'q':case'Q':zoomOutAvg(); break;
		case'r':case'R':histoStretch(); break;
		case's':case'S':endIn(); break;
		case't':case'T':histoEqual(); break;
		case'u':case'U':zoomOutMid(); break;
		case'v':case'V':emboss(); break;
		case'w':case'W':blur(); break;
		case'x':case'X':sharpning(); break;
		case'y':case'Y':rotate3(); break;
		case'z':case'Z': zoomIn3(); break;
		case'2':edgever(); break;
		case'3':edgehor(); break;
		case'4':blur99(); break;
		case'6':sharpning2(); break;



		}
	}

	freeInputMemory();
	freeOutputMemory();
	freeMaskMemory();
}

/////////////////////
// 함수 정의부
/////////////////////
/// 공통 함수
//////////////
void printMenu() {
	puts("\n ## GrayScale Image Processing (Beta4) ##\n");
	puts("0. 열기 1. 저장 9. 종료");
	puts("\n A.동일 B. 밝게 C. 어둡게 D. 반전 E. 흑백128 F. 흑백(평균값) ");
	puts("\n G.AND H.XOR 5.OR I.축소 J.zoomIn(포워딩) K. zoomIn(백워딩) L. rotate M.rotate(중앙,백워딩");
	puts("\n N.이동 O.좌우미러링 P.상하 미러링 Q. 축소(평균값) R. 히스토그램 스트레치");
	puts("\n S. 엔드인 T.평활화 U.축소(중앙값) V. 엠보싱 W. 블러링");
	puts("\n X. 샤프닝 Y. 회전(확대) Z. 양선형 보간 2. 수직 엣지 검출 3. 수평 엣지 검출 4.99블러링 6. 샤프닝2");
}
void printImage() {
	system("cls");
	for (int i = 0; i < outH; i++) {
		for (int k = 0; k < outW; k++) {
			int px = outImage[i][k];
			SetPixel(hdc, k + 1000, i + 350, RGB(px, px, px));
		}
	}
}

void loadImage() {
	char fullName[200] = "D:/RAW/Etc_Raw(squre)/";
	char tmpName[50];
	printf("파일명-->"); //flower512, LENA256
	scanf("%s", tmpName);
	strcat(fullName, tmpName);
	strcat(fullName, ".raw");
	strcpy(fileName, fullName);

	//중요! 
	rfp = fopen(fileName, "rb");
	if (rfp == NULL) {
		printf("파일이 열리지 않았습니다.\n");
		return 0;
	}
	fseek(rfp, 0L, SEEK_END);//파일의 끝으로 이동
	long long fsize = ftell(rfp);//나 어디쯤이지? 262,155-->512
	fclose(rfp);
	freeInputMemory();
	//높이, 폭 구하기
	inH = inW = (int)sqrt(fsize);

	//메모리 할당
	mallocInputMemory();

	//파일--> 메모리
	rfp = fopen(fileName, "rb");
	for (int i = 0; i < inH; i++)
		fread(inImage[i], sizeof(unsigned char), inW, rfp);
	fclose(rfp);

	rfp = fopen("D:/RAW/Etc_Raw(squre)/circle512.raw", "rb");
	fseek(rfp, 0L, SEEK_END);
	long long maskfsize = ftell(rfp);
	fclose(rfp);
	maskheight = maskwidth = (int)sqrt(maskfsize);

	//마스크 메모리 할당
	mask1 = (unsigned char**)malloc(sizeof(unsigned char*) * maskheight);
	for (int i = 0; i < maskheight; i++)
		mask1[i] = (unsigned char*)malloc(sizeof(unsigned char) * maskwidth);

	rfp = fopen("D:/RAW/Etc_Raw(squre)/circle512.raw", "rb");
	for (int i = 0; i < maskheight; i++)
		fread(mask1[i], sizeof(unsigned char), maskwidth, rfp);
	fclose(rfp);

	equalImage();
}
int getIntValue() {
	int retValue;
	printf("정수값-->"); scanf("%d", &retValue);
	return retValue;
}
void saveImage() {
	if (outImage == NULL)
		return;
	char fullName[200] = "D:/RAW/Etc_Raw(squre)/";
	char tmpName[50];
	printf("파일명-->"); // out01, out02......
	scanf("%s", tmpName);
	strcat(fullName, tmpName);
	strcat(fullName, ".raw");
	strcpy(fileName, fullName);

	wfp = fopen(fileName, "wb");
	// 메모리 --> 파일 (한행씩)
	for (int i = 0; i < outH; i++)
		fwrite(outImage[i], sizeof(unsigned char), outW, wfp);
	fclose(wfp);
	// MessageBox(hwnd, L"저장 완료", L"저장 창", NULL);
	printf("저장 완료");
}

void mallocInputMemory() {
	inImage = (unsigned char**)malloc(sizeof(unsigned char*) * inH);
	for (int i = 0; i < inH; i++)
		inImage[i] = (unsigned char*)malloc(sizeof(unsigned char) * inH);
}

void mallocOutputMemory() {
	outImage = (unsigned char**)malloc(sizeof(unsigned char*) * outH);
	for (int i = 0; i < outH; i++)
		outImage[i] = (unsigned char*)malloc(sizeof(unsigned char) * outW);
}

void mallocTempMemory() {
	temp = (int**)malloc((sizeof(int*) * outH));
	for (int i = 0; i < outH; i++)
		temp[i] = (int*)malloc(sizeof(int) * outW);
}

double** mallocDoubleMemory(int h, int w) {
	double** retMemory;
	retMemory = (double**)malloc(sizeof(double*) * h);
	for (int i = 0; i < h; i++)
		retMemory[i] = (double*)malloc(sizeof(double) * w);
	return retMemory;
}

void freeInputMemory() {
	if (inImage == NULL)
		return;
	for (int i = 0; i < inH; i++)
		free(inImage[i]);
	free(inImage);
	inImage = NULL;
}

void freeOutputMemory() {
	if (outImage == NULL)
		return;
	for (int i = 0; i < outH; i++)
		free(outImage[i]);
	free(outImage);
	outImage = NULL;
}
void freeTempMemory() {
	for (int i = 0; i < outH; i++)
		free(mask1[i]);
	free(mask1);

}

void freeMaskMemory() {
	for (int i = 0; i < maskheight; i++)
		free(mask1[i]);
	free(mask1);
}

void freeDoubleMemory(double** memory, int h) {
	if (memory == NULL)
		return;
	for (int i = 0; i < h; i++)
		free(memory[i]);
	free(memory);
	memory = NULL;
}
//////////////////
//  영상처리 함수
////////////////////
void equalImage() { // 동일영상 알고리즘
	// 입력 배열 --> 출력 배열
	freeOutputMemory();
	outH = inH;
	outW = inW;
	mallocOutputMemory();
	for (int i = 0; i < inH; i++) {
		for (int k = 0; k < inW; k++) {
			outImage[i][k] = inImage[i][k];
		}
	}
	printImage();
}
void addImage() { // 더하기 알고리즘
	// 입력 배열 --> 출력 배열
	freeOutputMemory();
	inH = outH;
	inW = outW;
	mallocOutputMemory();
	int val = getIntValue();
	for (int i = 0; i < outH; i++) {
		for (int k = 0; k < outW; k++) {
			if (inImage[i][k] + val < 255)
				outImage[i][k] = inImage[i][k] + val;
			else
				outImage[i][k] = 255;
		}
	}
	printImage();
}

void divImage() {
	freeOutputMemory();
	inH = outH;
	inW = outW;
	mallocOutputMemory();
	int val = getIntValue();
	for (int i = 0; i < outH; i++) {
		for (int k = 0; k < outW; k++) {
			if (inImage[i][k] - val > 0) {
				outImage[i][k] = inImage[i][k] - val;
			}
			else outImage[i][k] = 0;
		}
	}
	printImage();

}

void opImage() {
	freeOutputMemory();
	inH = outH;
	inW = outW;
	mallocOutputMemory();
	for (int i = 0; i < outH; i++) {
		for (int k = 0; k < outW; k++) {
			outImage[i][k] = 255 - inImage[i][k];
		}
	}
	printImage();
}

void blackImage128() {
	freeOutputMemory();
	inH = outH;
	inW = outW;
	mallocOutputMemory();
	for (int i = 0; i < outH; i++) {
		for (int k = 0; k < outW; k++) {
			if (inImage[i][k] > 128) {
				outImage[i][k] = 255;
			}
			else
				outImage[i][k] = 0;

		}
	}
	printImage();
}

void blackImageAvg() {
	int avg = 0;
	int hap = 0;
	freeOutputMemory();
	inH = outH;
	inW = outW;
	mallocOutputMemory();
	for (int i = 0; i < inH; i++) {
		for (int k = 0; k < inW; k++) {
			hap += inImage[i][k];
			avg = hap / (inH * inW);
		}
	}
	for (int i = 0; i < outH; i++) {
		for (int k = 0; k < outW; k++) {
			if (inImage[i][k] > avg)
				outImage[i][k] = 255;
			else
				outImage[i][k] = 0;
		}
	}
	printImage();
}

void andImage() {
	freeOutputMemory();
	inH = outH;
	inW = outW;
	mallocOutputMemory();
	for (int i = 0; i < outH; i++) {
		for (int k = 0; k < outW; k++) {
			outImage[i][k] = inImage[i][k] & mask1[i][k];
		}
	}
	printImage();

}

void xorImage() {
	freeOutputMemory();
	inH = outH;
	inW = outW;
	printf("비교할");
	int compare;
	compare = getIntValue();
	mallocOutputMemory();
	for (int i = 0; i < outH; i++) {
		for (int k = 0; k < outW; k++) {
			outImage[i][k] = inImage[i][k] ^ compare;
		}
	}
	printImage();

}

void orImage() {
	freeOutputMemory();
	inH = outH;
	inW = outW;
	mallocOutputMemory();
	for (int i = 0; i < outH; i++) {
		for (int k = 0; k < outW; k++) {
			outImage[i][k] = inImage[i][k] | mask1[i][k];
		}
	}
	printImage();
}

void zoomOut() { //축소 알고리즘
	int scale = getIntValue();
	//메모리 해제
	freeOutputMemory();
	outH = (int)(inH / scale);
	outW = (int)(inW / scale);
	//메모리 할당
	mallocOutputMemory();
	for (int i = 0; i < inH; i++) {
		for (int k = 0; k < inW; k++) {
			outImage[(int)(i / scale)][(int)(k / scale)] = inImage[i][k];
		}
	}
	printImage();
}

void zoomIn() { //확대 알고리즘
	int scale = getIntValue();
	//메모리 해제
	freeOutputMemory();
	//(중요!) 출력 이미지의 크기를 결정 -->알고리즘에 의존
	outH = (int)(inH * scale);
	outW = (int)(inW * scale);
	//메모리 할당
	mallocOutputMemory();
	for (int i = 0; i < inH; i++) {
		for (int k = 0; k < inW; k++) {
			outImage[(int)(i * scale)][(int)(k * scale)] = inImage[i][k];
		}
	}
	printImage();

}

void zoomIn2() { //확대 알고리즘
	int scale = getIntValue();
	//메모리 해제
	freeOutputMemory();
	//(중요!) 출력 이미지의 크기를 결정 -->알고리즘에 의존
	outH = (int)(inH * scale);
	outW = (int)(inW * scale);
	//메모리 할당
	mallocOutputMemory();
	for (int i = 0; i < outH; i++) {
		for (int k = 0; k < outW; k++) {
			outImage[(int)(i)][(int)(k)] = inImage[i / scale][k / scale];
		}
	}
	printImage();

}

void rotate() { // 회전 알고리즘
	// 입력 배열 --> 출력 배열
	freeOutputMemory();
	outH = inH;
	outW = inW;
	mallocOutputMemory();
	int degree = getIntValue();
	double radian = -degree * 3.141592 / 180.0;
	//xd = cos*(xs)-sin*(ys)
	//yd = sin*(sx) + cos*(ys)


	for (int i = 0; i < inH; i++) {
		for (int k = 0; k < inW; k++) {
			int xs = i;
			int ys = k;
			int xd = (int)(cos(radian) * xs + sin(radian) * ys);
			int yd = (int)(-sin(radian) * xs + cos(radian) * ys);

			if ((0 <= xd && xd < outH) && (0 <= yd && yd < outW))
				outImage[xd][yd] = inImage[xs][ys];
		}
	}
	printImage();
}

void rotate2() { // 회전 알고리즘
	// 입력 배열 --> 출력 배열
	freeOutputMemory();
	outH = inH;
	outW = inW;
	mallocOutputMemory();
	int degree = getIntValue();
	double radian = -degree * 3.141592 / 180.0;

	int cx = inH / 2;
	int cy = inW / 2;

	for (int i = 0; i < outH; i++) {
		for (int k = 0; k < outW; k++) {
			int xd = i;
			int yd = k;

			int xs = (int)(cos(radian) * (xd - cx) + sin(radian) * (yd - cy)) + cx;
			int ys = (int)(-sin(radian) * (xd - cx) + cos(radian) * (yd - cy)) + cy;
			
			if ((0 <= xs && xs < outH) && (0 <= ys && ys < outW))
				outImage[xd][yd] = inImage[xs][ys];
		}
	}
	printImage();
}

void moveImage() { // 동일영상 알고리즘
	// 입력 배열 --> 출력 배열
	freeOutputMemory();
	int xmove, ymove;
	printf("x증가");
	xmove = getIntValue();
	printf("y증가");
	ymove = getIntValue();
	outH = inH;
	outW = inW;
	mallocOutputMemory();


	for (int i = 0; i < outH; i++) {
		for (int k = 0; k < outW; k++) {

			if ((0 <= i - xmove && i - xmove < outH) && (0 <= k - ymove && k - ymove < outW))
				outImage[i][k] = inImage[i - xmove][k - ymove];
		}
	}
	printImage();
}

void mirrorRL() {
	freeOutputMemory();
	outH = inH;
	outW = inW;
	mallocOutputMemory();
	for (int i = 0; i < outH; i++) {
		for (int k = 0; k < outW; k++) {

			outImage[i][k] = inImage[i][(inW - 1) - k];

		}
	}
	printImage();

}

void mirrorUD() {
	freeOutputMemory();
	outH = inH;
	outW = inW;
	mallocOutputMemory();
	for (int i = 0; i < outH; i++) {
		for (int k = 0; k < outW; k++) {

			outImage[i][k] = inImage[(inH - 1) - i][k];
		}
	}
	printImage();

}

void zoomOutAvg() {
	int scale = getIntValue();
	//메모리 해제
	freeOutputMemory();
	outH = (int)(inH / scale);
	outW = (int)(inW / scale);
	//메모리 할당
	mallocOutputMemory();
	mallocTempMemory();
	for (int i = 0; i < outH; i++) {
		for (int k = 0; k < outW; k++) {
			temp[i][k] = 0;
		}
	}
	for (int i = 0; i < inH; i++) {
		for (int k = 0; k < inW; k++) {
			temp[(int)(i / scale)][(int)(k / scale)] += (int)inImage[i][k];
		}
	}
	for (int i = 0; i < outH; i++) {
		for (int k = 0; k < outW; k++) {
			outImage[i][k] = (unsigned char)(temp[i][k] / (scale * scale));
		}
	}
	freeTempMemory();
	printImage();

}

void histoStretch() {//히스토그램 스르래칭 알고리즘

	// 입력 배열 --> 출력 배열
	freeOutputMemory();
	outH = inH;
	outW = inW;
	mallocOutputMemory();

	//진짜 영상처리 알고리즘
	//new = (old - low) / (high - low) * 255.0
	int high=inImage[0][0], low = inImage[0][0];
	for (int i = 0; i < inH; i++) {
		for (int k = 0; k < inW; k++) {
			if (inImage[i][k] < low)
				low = inImage[i][k];
			if (inImage[i][k] > high)
				high = inImage[i][k];
		}
	}
	int old, new;
	for (int i = 0; i < inH; i++) {
		for (int k = 0; k < inW; k++) {
			old = inImage[i][k];
			new = (int)((double)(old - low) / (double)(high -low) * 255.0);

			if (new > 255)
				new = 255;
			if (new < 0)
				new = 0;
			outImage[i][k] = new;
		}
	}
	printImage();


}

void endIn() {//엔드인 알고리즘

	// 입력 배열 --> 출력 배열
	freeOutputMemory();
	outH = inH;
	outW = inW;
	mallocOutputMemory();

	//진짜 영상처리 알고리즘
	//new = (old - low) / (high - low) * 255.0
	int high = inImage[0][0], low = inImage[0][0];
	for (int i = 0; i < inH; i++) {
		for (int k = 0; k < inW; k++) {
			if (inImage[i][k] < low)
				low = inImage[i][k];
			if (inImage[i][k] > high)
				high = inImage[i][k];
		}
	}
	high -= 50;
	low += 50;
	int old, new;
	for (int i = 0; i < inH; i++) {
		for (int k = 0; k < inW; k++) {
			old = inImage[i][k];
			new = (int)((double)(old - low) / (double)(high - low) * 255.0);

			if (new > 255)
				new = 255;
			if (new < 0)
				new = 0;
			outImage[i][k] = new;
		}
	}
	printImage();


}

void histoEqual() { // 히스토 그램 평활화 알고리즘
	// 입력 배열 --> 출력 배열
	freeOutputMemory();
	outH = inH;
	outW = inW;
	mallocOutputMemory();

	//1단계 : 빈도수 세기(=히스토그램) histo[256]
	int histo[256] = { 0, };
	for (int i = 0; i < inH; i++) {
		for (int k = 0; k < inW; k++) {
			histo[inImage[i][k]]++;
		}
	}
	//2단계 : 누적 히스토그램 생성
	int sumHisto[256] = { 0, };
	sumHisto[0] = histo[0];
	for (int i = 1; i < 256; i++) {
		sumHisto[i] = sumHisto[i - 1] + histo[i];
	}

	//3단계 : 정규화된 히스토그램 생성 normalHisto = sumHisto *(1.0/inH*inW)*255.0;
	double normalHisto[256] = { 1.0, };
	for (int i = 0; i < 256; i++) {
		normalHisto[i] = sumHisto[i] * (1.0 / (inH * inW)) * 255.0;
	}

	//4단계 : inImage를 정규화된 이미지로 치환

	for (int i = 0; i < inH; i++) {
		for (int k = 0; k < inW; k++) {
			outImage[i][k] = (unsigned char)normalHisto[inImage[i][k]];
		}
	}
	printImage();
}

int compare(const void* a, const void* b) {
	int num1 = *(int*)a;    // void 포인터를 int 포인터로 변환한 뒤 역참조하여 값을 가져옴
	int num2 = *(int*)b;    // void 포인터를 int 포인터로 변환한 뒤 역참조하여 값을 가져옴

	if (num1 < num2)    // a가 b보다 작을 때는
		return -1;      // -1 반환

	if (num1 > num2)    // a가 b보다 클 때는
		return 1;       // 1 반환

	return 0;    // a와 b가 같을 때는 0 반환
}

void zoomOutMid() {
	int scale = getIntValue();
	//메모리 해제
	freeOutputMemory();
	outH = (int)(inH / scale);
	outW = (int)(inW / scale);
	//메모리 할당

	mallocOutputMemory();
	int* temp;
	temp = (int*)malloc((sizeof(int)) * (scale*scale));//inImage 크기만큼의 일차원 배열 만들기
	int num = 0;
	for (int i = 0; i < outH; i++) {
		for (int k = 0; k < outW; k++) {
			for (int j = 0; j < scale ; j++) {
				for (int m = 0; m < scale; m++) {
					temp[num] = inImage[scale * i + j][scale  * k + m];
					num++;
				}
			}
			num = 0;
			qsort(temp, sizeof(temp) / sizeof(temp[0]), sizeof(temp[0]), compare);
			outImage[i][k] = temp[(scale * scale) / 2];
		}
	}
	
	free(temp);
	printImage();

}

void emboss() { // 화소영역 처리 : 엠보싱 알고리즘
	// 메모리 해제
	freeOutputMemory();
	// (중요!) 출력 이미지의 크기를 결정 ---> 알고리즘에 의존
	outH = inH;
	outW = inW;
	// 메모리 할당	
	mallocOutputMemory();

	////////////////
	// 화소 영역 처리
	/////////////////
	double mask[3][3] = { {-1.0, 0.0, 0.0}, // 엠보싱 마스크
						  { 0.0, 0.0, 0.0},
						  { 0.0, 0.0, 1.0} };
	// 임시 메모리 할당(실수형)
	double** tmpInImage = mallocDoubleMemory(inH + 2, inW + 2);
	double** tmpOutImage = mallocDoubleMemory(outH, outW);

	// 임시 입력 메모리를 초기화(127) : 필요시 평균값
	for (int i = 0; i < inH + 2; i++)
		for (int k = 0; k < inW + 2; k++)
			tmpInImage[i][k] = 127;

	// 입력 이미지 --> 임시 입력 이미지
	for (int i = 0; i < inH; i++)
		for (int k = 0; k < inW; k++)
			tmpInImage[i + 1][k + 1] = inImage[i][k];

	// *** 회선 연산 ***
	double S;
	for (int i = 0; i < inH; i++) {
		for (int k = 0; k < inW; k++) {
			// 마스크(3x3) 와 한점을 중심으로한 3x3을 곱하기
			S = 0.0; // 마스크 9개와 입력값 9개를 각각 곱해서 합한 값.

			for (int m = 0; m < 3; m++)
				for (int n = 0; n < 3; n++)
					S += tmpInImage[i + m][k + n] * mask[m][n];

			tmpOutImage[i][k] = S;
		}
	}
	// 후처리 (마스크 값의 합계에 따라서...)
	for (int i = 0; i < outH; i++)
		for (int k = 0; k < outW; k++)
			tmpOutImage[i][k] += 127.0;

	// 임시 출력 영상--> 출력 영상. 
	for (int i = 0; i < outH; i++) {
		for (int k = 0; k < outW; k++) {
			if (tmpOutImage[i][k] < 0.0)
				outImage[i][k] = 0;
			else if (tmpOutImage[i][k] > 255.0)
				outImage[i][k] = 255;
			else
				outImage[i][k] = (unsigned char)tmpOutImage[i][k];
		}
	}
	freeDoubleMemory(tmpInImage, inH + 2);
	freeDoubleMemory(tmpOutImage, outH);
	printImage();
}

void blur() { // 화소영역 처리 : 블러링 알고리즘
	// 메모리 해제
	freeOutputMemory();
	// (중요!) 출력 이미지의 크기를 결정 ---> 알고리즘에 의존
	outH = inH;
	outW = inW;
	// 메모리 할당	
	mallocOutputMemory();

	////////////////
	// 화소 영역 처리
	/////////////////
	double mask[3][3] = { {1.0/9, 1.0 / 9, 1.0 / 9}, // 블러링 마스크
						  { 1.0 / 9, 1.0 / 9, 1.0 / 9},
						  { 1.0 / 9, 1.0 / 9, 1.0/9} };
	// 임시 메모리 할당(실수형)
	double** tmpInImage = mallocDoubleMemory(inH + 2, inW + 2);
	double** tmpOutImage = mallocDoubleMemory(outH, outW);

	// 임시 입력 메모리를 초기화(127) : 필요시 평균값
	for (int i = 0; i < inH + 2; i++)
		for (int k = 0; k < inW + 2; k++)
			tmpInImage[i][k] = 127;

	// 입력 이미지 --> 임시 입력 이미지
	for (int i = 0; i < inH; i++)
		for (int k = 0; k < inW; k++)
			tmpInImage[i + 1][k + 1] = inImage[i][k];

	// *** 회선 연산 ***
	double S;
	for (int i = 0; i < inH; i++) {
		for (int k = 0; k < inW; k++) {
			// 마스크(3x3) 와 한점을 중심으로한 3x3을 곱하기
			S = 0.0; // 마스크 9개와 입력값 9개를 각각 곱해서 합한 값.

			for (int m = 0; m < 3; m++)
				for (int n = 0; n < 3; n++)
					S += tmpInImage[i + m][k + n] * mask[m][n];

			tmpOutImage[i][k] = S;
		}
	}
	//// 후처리 (마스크 값의 합계에 따라서...)
	//for (int i = 0; i < outH; i++)
	//	for (int k = 0; k < outW; k++)
	//		tmpOutImage[i][k] += 127.0;

	// 임시 출력 영상--> 출력 영상. 
	for (int i = 0; i < outH; i++) {
		for (int k = 0; k < outW; k++) {
			if (tmpOutImage[i][k] < 0.0)
				outImage[i][k] = 0;
			else if (tmpOutImage[i][k] > 255.0)
				outImage[i][k] = 255;
			else
				outImage[i][k] = (unsigned char)tmpOutImage[i][k];
		}
	}
	freeDoubleMemory(tmpInImage, inH + 2);
	freeDoubleMemory(tmpOutImage, outH);
	printImage();
}

void sharpning() { // 화소영역 처리 : 샤프닝 알고리즘
	// 메모리 해제
	freeOutputMemory();
	// (중요!) 출력 이미지의 크기를 결정 ---> 알고리즘에 의존
	outH = inH;
	outW = inW;
	// 메모리 할당	
	mallocOutputMemory();

	////////////////
	// 화소 영역 처리
	/////////////////
	double mask[3][3] = { {-1.0, -1.0, -1.0}, // 샤프닝 마스크
						  {-1.0, 9.0 , -1.0},
						  {-1.0, -1.0, -1.0} };
	// 임시 메모리 할당(실수형)
	double** tmpInImage = mallocDoubleMemory(inH + 2, inW + 2);
	double** tmpOutImage = mallocDoubleMemory(outH, outW);

	// 임시 입력 메모리를 초기화(127) : 필요시 평균값
	for (int i = 0; i < inH + 2; i++)
		for (int k = 0; k < inW + 2; k++)
			tmpInImage[i][k] = 127;

	// 입력 이미지 --> 임시 입력 이미지
	for (int i = 0; i < inH; i++)
		for (int k = 0; k < inW; k++)
			tmpInImage[i + 1][k + 1] = inImage[i][k];

	// *** 회선 연산 ***
	double S;
	for (int i = 0; i < inH; i++) {
		for (int k = 0; k < inW; k++) {
			// 마스크(3x3) 와 한점을 중심으로한 3x3을 곱하기
			S = 0.0; // 마스크 9개와 입력값 9개를 각각 곱해서 합한 값.

			for (int m = 0; m < 3; m++)
				for (int n = 0; n < 3; n++)
					S += tmpInImage[i + m][k + n] * mask[m][n];

			tmpOutImage[i][k] = S;
		}
	}
	//// 후처리 (마스크 값의 합계에 따라서...)
	//for (int i = 0; i < outH; i++)
	//	for (int k = 0; k < outW; k++)
	//		tmpOutImage[i][k] += 127.0;

	// 임시 출력 영상--> 출력 영상. 
	for (int i = 0; i < outH; i++) {
		for (int k = 0; k < outW; k++) {
			if (tmpOutImage[i][k] < 0.0)
				outImage[i][k] = 0;
			else if (tmpOutImage[i][k] > 255.0)
				outImage[i][k] = 255;
			else
				outImage[i][k] = (unsigned char)tmpOutImage[i][k];
		}
	}
	freeDoubleMemory(tmpInImage, inH + 2);
	freeDoubleMemory(tmpOutImage, outH);
	printImage();
}

void sharpning2() {
	// 메모리 해제
	freeOutputMemory();
	// (중요!) 출력 이미지의 크기를 결정 ---> 알고리즘에 의존
	outH = inH;
	outW = inW;
	// 메모리 할당	
	mallocOutputMemory();

	////////////////
	// 화소 영역 처리
	/////////////////
	double mask[3][3] = { {0, -1.0, 0}, // 샤프닝 마스크2
						  {-1.0, 5.0 ,-1.0},
						  {0, -1.0 , 0} };
	// 임시 메모리 할당(실수형)
	double** tmpInImage = mallocDoubleMemory(inH + 2, inW + 2);
	double** tmpOutImage = mallocDoubleMemory(outH, outW);

	// 임시 입력 메모리를 초기화(127) : 필요시 평균값
	for (int i = 0; i < inH + 2; i++)
		for (int k = 0; k < inW + 2; k++)
			tmpInImage[i][k] = 127;

	// 입력 이미지 --> 임시 입력 이미지
	for (int i = 0; i < inH; i++)
		for (int k = 0; k < inW; k++)
			tmpInImage[i + 1][k + 1] = inImage[i][k];

	// *** 회선 연산 ***
	double S;
	for (int i = 0; i < inH; i++) {
		for (int k = 0; k < inW; k++) {
			// 마스크(3x3) 와 한점을 중심으로한 3x3을 곱하기
			S = 0.0; // 마스크 9개와 입력값 9개를 각각 곱해서 합한 값.

			for (int m = 0; m < 3; m++)
				for (int n = 0; n < 3; n++)
					S += tmpInImage[i + m][k + n] * mask[m][n];

			tmpOutImage[i][k] = S;
		}
	}

	// 임시 출력 영상--> 출력 영상. 
	for (int i = 0; i < outH; i++) {
		for (int k = 0; k < outW; k++) {
			if (tmpOutImage[i][k] < 0.0)
				outImage[i][k] = 0;
			else if (tmpOutImage[i][k] > 255.0)
				outImage[i][k] = 255;
			else
				outImage[i][k] = (unsigned char)tmpOutImage[i][k];
		}
	}
	freeDoubleMemory(tmpInImage, inH + 2);
	freeDoubleMemory(tmpOutImage, outH);
	printImage();
}
void rotate3() { // 회전 알고리즘
	// 입력 배열 --> 출력 배열
	freeOutputMemory();
	int degree = getIntValue();
	double radian = -degree * 3.141592 / 180.0;
	double radian90 = 90 * 3.141592 / 180.0;
	outH =(int)(inW*cos(radian90 + radian)+ (inH *(cos(radian)))); //outImage 배열의 크기를 돌린 각도에 맞게 증가
	outW = (int)(inH*cos(radian90 + radian) +(inW *(cos(radian))));
	
	mallocOutputMemory();
	
	//xd = cos*(xs)-sin*(ys)
	//yd = sin*(sx) + cos*(ys)

	int cx = inH / 2;
	int cy = inW / 2;
	for (int i = 0; i < outH; i++) {
		for (int k = 0; k < outW; k++) {
			int xd = i;
			int yd = k;

			int xs = (int)(cos(radian) * (xd - outH / 2) + sin(radian) * (yd - outW / 2));
			int ys = (int)(-sin(radian) * (xd - outH / 2) + cos(radian) * (yd - outW / 2));
			xs += inH/2;
			ys += inW/2;
			if ((0 <= xs && xs < inH) && (0 <= ys && ys < inW))
				outImage[xd][yd] = inImage[xs][ys];
		}
	}
	printImage();
}

void zoomIn3() {
	// 입력 배열 --> 출력 배열
	freeOutputMemory();
	int scale = getIntValue();
	outH = (inH*scale);
	outW = (inW*scale);
	mallocOutputMemory();

	for (int i = 0; i < inH; i++) {
		for (int k = 0; k < inW; k++) {
			outImage[i * scale][k * scale] = inImage[i][k];
		}
	}

	int i_h, i_w;
	double r_H, r_W, s_H, s_W;
	double C1, C2, C3, C4;

	for (int i = 0; i < outH; i++) {
		for (int k = 0; k < outW; k++) {
			r_H = i / scale;								// 현재 위치를 scale 크기의 블록 중 몇 번째 쯤인지 연산
			r_W = k / scale;

			i_h = (int)floor(r_H);							// 블록의 가로세로 칸을 저장
			i_w = (int)floor(r_W);

			s_H = r_H - i_h;								// 현재 블록 내에서의 위치를 저장 
			s_W = r_W - i_w;

			if (i_h < 0 | i_h >= (inH - 1) | i_w < 0 | i_w >= (inW - 1)) {
				outImage[i][k] = 255;						// 현재 칸이 보간을 진행할 수 없다면 흰색으로 지정
				// 주로 왼쪽과 아래쪽이 그런 경향이 있음
			}
			else {
				C1 = (double)inImage[i_h][i_w];				// 좌 상단점 지정
				C2 = (double)inImage[i_h][i_w + 1];			// 우 상단점 지정
				C3 = (double)inImage[i_h + 1][i_w + 1];		// 우 하단점 지정
				C4 = (double)inImage[i_h + 1][i_w];			// 좌 하단점 지정
				outImage[i][k] = (unsigned char)(C1 * (1 - s_H) * (1 - s_W)
					+ C2 * s_W * (1 - s_H) + C3 * s_W * s_H + C4 * (1 - s_W) * s_H);
				// 각 점들과의 위치 비율을 반영해서 점의 색 지정 
			}

		}
	}
	
	printImage();
}

void edgever() {// 수직 엣지 검출
	// 메모리 해제
	freeOutputMemory();
	// (중요!) 출력 이미지의 크기를 결정 ---> 알고리즘에 의존
	outH = inH;
	outW = inW;
	// 메모리 할당	
	mallocOutputMemory();

	////////////////
	// 화소 영역 처리
	/////////////////
	double mask[3][3] = { {0.0, 0.0, 0.0}, // 엣지 검출 마스크
						  { -1.0, 1.0, 0.0},
						  { 0.0, 0.0, 0.0} };
	// 임시 메모리 할당(실수형)
	double** tmpInImage = mallocDoubleMemory(inH + 2, inW + 2);
	double** tmpOutImage = mallocDoubleMemory(outH, outW);

	// 임시 입력 메모리를 초기화(127) : 필요시 평균값
	for (int i = 0; i < inH + 2; i++)
		for (int k = 0; k < inW + 2; k++)
			tmpInImage[i][k] = 127;

	// 입력 이미지 --> 임시 입력 이미지
	for (int i = 0; i < inH; i++)
		for (int k = 0; k < inW; k++)
			tmpInImage[i + 1][k + 1] = inImage[i][k];

	// *** 회선 연산 ***
	double S;
	for (int i = 0; i < inH; i++) {
		for (int k = 0; k < inW; k++) {
			// 마스크(3x3) 와 한점을 중심으로한 3x3을 곱하기
			S = 0.0; // 마스크 9개와 입력값 9개를 각각 곱해서 합한 값.

			for (int m = 0; m < 3; m++)
				for (int n = 0; n < 3; n++)
					S += tmpInImage[i + m][k + n] * mask[m][n];

			tmpOutImage[i][k] = S;
		}
	}
	// 후처리 (마스크 값의 합계에 따라서...)
	for (int i = 0; i < outH; i++)
		for (int k = 0; k < outW; k++)
			tmpOutImage[i][k] += 50.0;

	// 임시 출력 영상--> 출력 영상. 
	for (int i = 0; i < outH; i++) {
		for (int k = 0; k < outW; k++) {
			if (tmpOutImage[i][k] < 0.0)
				outImage[i][k] = 0;
			else if (tmpOutImage[i][k] > 255.0)
				outImage[i][k] = 255;
			else
				outImage[i][k] = (unsigned char)tmpOutImage[i][k];
		}
	}
	freeDoubleMemory(tmpInImage, inH + 2);
	freeDoubleMemory(tmpOutImage, outH);
	printImage();

}

void edgehor() { //수평 엣지 검출
	// 메모리 해제
	freeOutputMemory();
	// (중요!) 출력 이미지의 크기를 결정 ---> 알고리즘에 의존
	outH = inH;
	outW = inW;
	// 메모리 할당	
	mallocOutputMemory();

	////////////////
	// 화소 영역 처리
	/////////////////
	double mask[3][3] = { {0.0, -1.0, 0.0}, // 엠보싱 마스크
						  { 0.0, 1.0, 0.0},
						  { 0.0, 0.0, 0.0} };
	// 임시 메모리 할당(실수형)
	double** tmpInImage = mallocDoubleMemory(inH + 2, inW + 2);
	double** tmpOutImage = mallocDoubleMemory(outH, outW);

	// 임시 입력 메모리를 초기화(127) : 필요시 평균값
	for (int i = 0; i < inH + 2; i++)
		for (int k = 0; k < inW + 2; k++)
			tmpInImage[i][k] = 127;

	// 입력 이미지 --> 임시 입력 이미지
	for (int i = 0; i < inH; i++)
		for (int k = 0; k < inW; k++)
			tmpInImage[i + 1][k + 1] = inImage[i][k];

	// *** 회선 연산 ***
	double S;
	for (int i = 0; i < inH; i++) {
		for (int k = 0; k < inW; k++) {
			// 마스크(3x3) 와 한점을 중심으로한 3x3을 곱하기
			S = 0.0; // 마스크 9개와 입력값 9개를 각각 곱해서 합한 값.

			for (int m = 0; m < 3; m++)
				for (int n = 0; n < 3; n++)
					S += tmpInImage[i + m][k + n] * mask[m][n];

			tmpOutImage[i][k] = S;
		}
	}
	// 후처리 (마스크 값의 합계에 따라서...)
	for (int i = 0; i < outH; i++)
		for (int k = 0; k < outW; k++)
			tmpOutImage[i][k] += 50.0;

	// 임시 출력 영상--> 출력 영상. 
	for (int i = 0; i < outH; i++) {
		for (int k = 0; k < outW; k++) {
			if (tmpOutImage[i][k] < 0.0)
				outImage[i][k] = 0;
			else if (tmpOutImage[i][k] > 255.0)
				outImage[i][k] = 255;
			else
				outImage[i][k] = (unsigned char)tmpOutImage[i][k];
		}
	}
	freeDoubleMemory(tmpInImage, inH + 2);
	freeDoubleMemory(tmpOutImage, outH);
	printImage();


}

void blur99() { // 화소영역 처리 : 블러링 알고리즘
	// 메모리 해제
	freeOutputMemory();
	// (중요!) 출력 이미지의 크기를 결정 ---> 알고리즘에 의존
	outH = inH;
	outW = inW;
	// 메모리 할당	
	mallocOutputMemory();

	////////////////
	// 화소 영역 처리
	/////////////////
	double mask[9][9] = { {1.0 / 81, 1.0 / 81, 1.0 / 81,1.0 / 81,1.0 / 81,1.0 / 81,1.0 / 81,1.0 / 81,1.0 / 81}, // 블러링 마스크
						  { 1.0 / 81,1.0 / 81,1.0 / 81,1.0 / 81,1.0 / 81,1.0 / 81,1.0 / 81,1.0 / 81,1.0 / 81},
						  { 1.0 / 81,1.0 / 81,1.0 / 81,1.0 / 81,1.0 / 81,1.0 / 81,1.0 / 81,1.0 / 81,1.0 / 81},
							{1.0 / 81,1.0 / 81,1.0 / 81,1.0 / 81,1.0 / 81,1.0 / 81,1.0 / 81,1.0 / 81,1.0 / 81},
							{1.0 / 81, 1.0 / 81, 1.0 / 81,1.0 / 81,1.0 / 81,1.0 / 81,1.0 / 81,1.0 / 81,1.0 / 81},
							{ 1.0 / 81, 1.0 / 81, 1.0 / 81,1.0 / 81,1.0 / 81,1.0 / 81,1.0 / 81,1.0 / 81,1.0 / 81 },
							{ 1.0 / 81, 1.0 / 81, 1.0 / 81,1.0 / 81,1.0 / 81,1.0 / 81,1.0 / 81,1.0 / 81,1.0 / 81 },
							{ 1.0 / 81, 1.0 / 81, 1.0 / 81,1.0 / 81,1.0 / 81,1.0 / 81,1.0 / 81,1.0 / 81,1.0 / 81 },
							{ 1.0 / 81, 1.0 / 81, 1.0 / 81,1.0 / 81,1.0 / 81,1.0 / 81,1.0 / 81,1.0 / 81,1.0 / 81 } };
	// 임시 메모리 할당(실수형)
	double** tmpInImage = mallocDoubleMemory(inH + 8, inW + 8);
	double** tmpOutImage = mallocDoubleMemory(outH, outW);

	// 임시 입력 메모리를 초기화(127) : 필요시 평균값
	for (int i = 0; i < inH + 8; i++)
		for (int k = 0; k < inW + 8; k++)
			tmpInImage[i][k] = 127;

	// 입력 이미지 --> 임시 입력 이미지
	for (int i = 0; i < inH; i++)
		for (int k = 0; k < inW; k++)
			tmpInImage[i + 3][k + 3] = inImage[i][k];

	// *** 회선 연산 ***
	double S;
	for (int i = 0; i < inH; i++) {
		for (int k = 0; k < inW; k++) {
			// 마스크(3x3) 와 한점을 중심으로한 9x3을 곱하기
			S = 0.0; // 마스크 81개와 입력값 81개를 각각 곱해서 합한 값.

			for (int m = 0; m < 9; m++)
				for (int n = 0; n < 9; n++)
					S += tmpInImage[i + m][k + n] * mask[m][n];

			tmpOutImage[i][k] = S;
		}
	}
	//// 후처리 (마스크 값의 합계에 따라서...)
	//for (int i = 0; i < outH; i++)
	//	for (int k = 0; k < outW; k++)
	//		tmpOutImage[i][k] += 127.0;

	// 임시 출력 영상--> 출력 영상. 
	for (int i = 0; i < outH; i++) {
		for (int k = 0; k < outW; k++) {
			if (tmpOutImage[i][k] < 0.0)
				outImage[i][k] = 0;
			else if (tmpOutImage[i][k] > 255.0)
				outImage[i][k] = 255;
			else
				outImage[i][k] = (unsigned char)tmpOutImage[i][k];
		}
	}
	freeDoubleMemory(tmpInImage, inH + 8);
	freeDoubleMemory(tmpOutImage, outH);
	printImage();
}