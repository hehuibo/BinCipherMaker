// BinCipherMaker.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <windows.h>
#include <string.h>
#include <shlwapi.h>

#pragma  comment(lib, "shlwapi.lib")
#pragma comment (lib,"User32.lib")

#define  FILE_PATH_SIZE		4096
#define  READ_BUFFER_SZIE	1024
#define  BINCipher_MAGIC	0x5959

#define  MINCipherSIZE		64
#define  CipherSIZE			256


struct _tag_DevFirmwareInfo
{
	unsigned int softVer;
	unsigned int DevKey;
	unsigned int CipherKey;
};

int BinCipherMaker(char *pBuffer, int len, const unsigned short BinCipherMagic)
{
	unsigned int *p = (unsigned int *)pBuffer;

	*(unsigned int*)(p) = (*(unsigned int*)(p)) ^ BinCipherMagic;

	return 0;
}




int main(int argc, char *argv[])
{
	int rflen = 0;
	int rSize = CipherSIZE;
	FILE *fpsrc, *fpdes;
	struct _tag_DevFirmwareInfo DevFirmwareInfo;
	char **lpPart = NULL;
	char srcFilePath[FILE_PATH_SIZE];
	char desFilePath[FILE_PATH_SIZE];
	char ReadBuffer[FILE_PATH_SIZE];
	unsigned short CipherMagic = BINCipher_MAGIC;

	if (argc < 2) {
		printf("Usage : %s [File]\n", argv[0]);
		return -1;
	}

	/*修改程序图标*/
	HWND hWnd = GetConsoleWindow();
	SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(NULL, _T("IDI_ICON1")));

	/*
	获取文件全路径
	*/
	int pathlen = ::GetFullPathName(argv[1],
									FILE_PATH_SIZE,
									srcFilePath,
									lpPart);

	/*获取文件版本信息*/
	if (NULL == (fpsrc = fopen(srcFilePath, "rb"))) {
		printf("File Open Error\n");	
		return -1;
	}
	fseek(fpsrc, 32, SEEK_SET);
	fread((void *)&DevFirmwareInfo, sizeof(char), sizeof(DevFirmwareInfo), fpsrc);
	fclose(fpsrc);

	//生成加密文件名称
	strcpy(desFilePath, srcFilePath);
	::PathRemoveExtension(desFilePath);/*去除文件后缀*/
	//::PathRemoveFileSpec(desFilePath);/*获得文件路径*/
	int len = strlen(desFilePath);
	sprintf(desFilePath + len, 
		   "_V%d.%d.%d.%d.dwn", (DevFirmwareInfo.softVer >> 24) & 0xFF,
							(DevFirmwareInfo.softVer >> 16) & 0xFF,
		                    (DevFirmwareInfo.softVer >> 8) & 0xFF,
		                    (DevFirmwareInfo.softVer & 0xFF));
	if (argc > 2) {
		//strcat(desFilePath, argv[2]);
		PathRenameExtension(desFilePath, argv[2]);
	}
	//PathRenameExtension(desFilePath, ".dwn");/
	//strcat(desFilePath, ".dwn"); /*生成的文件*/

	/*获得加密密钥*/
	CipherMagic = (unsigned short)(DevFirmwareInfo.CipherKey & 0xFFFF);

	if (argc > 3) {
		if (sscanf(argv[3], "%d", &rSize) < 0) {
			rSize = READ_BUFFER_SZIE;
			if (rSize < MINCipherSIZE) {
				rSize = MINCipherSIZE;
				printf("Error : argv[3] < 64\n");
			}
		}
	}

	fpsrc = fopen(srcFilePath, "rb");
	fpdes = fopen(desFilePath, "wb");
	if ((NULL == fpsrc) && (NULL == fpdes)) {
		printf("File Open Error\n");
		return -1;
	}

	while ((rflen = fread(ReadBuffer, sizeof(char), rSize, fpsrc)) > 0) {
		BinCipherMaker(ReadBuffer, rflen, CipherMagic);
		fwrite(ReadBuffer, sizeof(char), rflen, fpdes);
	}

	//::PathIsFileSpec(desFilePath);

	printf("%s , Size %d\n", ::PathFindFileName(desFilePath),CipherSIZE);
	fclose(fpsrc);
	fclose(fpdes);

    return 0;
}

