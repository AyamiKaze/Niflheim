/*
用于解包BH的pac文件
made by Darkness-TX
2016.12.01

添加新版NeXAS支持
upload by AyamiKaze
2020.03.18
*/
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <direct.h>
#include <Windows.h>
//#include <zlib.h>
#include"zstd.h"
#include"zdict.h"
#include"zstd_errors.h"
#include <locale.h>
#include "BH_huffman_dec.h"
#pragma comment(lib, "libzstd.lib")
typedef unsigned char  unit8;
typedef unsigned short unit16;
typedef unsigned int   unit32;

unit32 FileNum = 0;//总文件数，初始计数为0

struct header
{
	unit8 magic[4];//PAC\x7F
	unit32 num;//文件数
	unit32 mode;//7
}pac_header;

struct index
{
	unit8 name[64];//文件名
	unit32 Offset;//文件偏移
	unit32 FileSize;//解压大小
	unit32 ComSize;//未解压大小
}Index[7000];

void ReadIndex(char *fname)
{
	FILE *src, *dst;
	unit32 ComSize = 0, UncomSize = 0, i = 0;
	unit8 *cdata, *udata, dstname[200];
	src = fopen(fname, "rb");
	sprintf(dstname, "%s_INDEX", fname);
	fread(pac_header.magic, 4, 1, src);
	if (strncmp(pac_header.magic, "PAC\x7F", 4) != 0)
	{
		printf("文件头不是PAC\\x7F!\n要继续解包请按任意键，不解包请关闭程序。\n");
		system("pause");
	}
	fread(&pac_header.num, 4, 1, src);
	fread(&pac_header.mode, 4, 1, src);
	printf("%s filenum:%d mode:%d\n\n", fname, pac_header.num, pac_header.mode);
	if (pac_header.mode != 7)
	{
		printf("不是模式7！\n");
		system("pause");
		exit(0);
	}
	else
	{
		fseek(src, -4, SEEK_END);
		fread(&ComSize, 4, 1, src);
		fseek(src, -4 - ComSize, SEEK_END);
		cdata = malloc(ComSize);
		fread(cdata, ComSize, 1, src);
		for (i = 0; i < ComSize; i++)
			cdata[i] = ~cdata[i];
		UncomSize = 76 * pac_header.num;
		udata = malloc(UncomSize);
		huffman_uncompress(udata, &UncomSize, cdata, ComSize);
		dst = fopen(dstname, "wb");
		fwrite(udata, UncomSize, 1, dst);
		free(cdata);
		fclose(dst);
		fclose(src);
		for (i = 0; i < pac_header.num; i++)
			memcpy(&Index[i], &udata[i * 76], 76);
		free(udata);
	}
}

void UnpackFile(char *fname)
{
	FILE *src, *dst;
	WCHAR dstname[200];
	unit8 *cdata, *udata, dirname[200];
	unit32 i;
	src = fopen(fname, "rb");
	sprintf(dirname, "%s_unpack", fname);
	_mkdir(dirname);
	_chdir(dirname);
	for (i = 0; i < pac_header.num; i++)
	{
		MultiByteToWideChar(932, 0, Index[i].name, -1, dstname, 64);
		dst = _wfopen(dstname, L"wb");
		cdata = malloc(Index[i].ComSize);
		udata = malloc(Index[i].FileSize);
		fseek(src, Index[i].Offset, SEEK_SET);
		fread(cdata, Index[i].ComSize, 1, src);
		if (Index[i].ComSize != Index[i].FileSize)
		{
			ZSTD_decompress(udata, Index[i].FileSize, cdata, Index[i].ComSize);
			fwrite(udata, Index[i].FileSize, 1, dst);
			wprintf(L"%ls offset:0x%X filesize:0x%X comsize:0x%X\n", dstname, Index[i].Offset, Index[i].FileSize, Index[i].ComSize);
			free(udata);
		}
		else
		{
			fwrite(cdata, Index[i].FileSize, 1, dst);
			wprintf(L"%ls offset:0x%X filesize:0x%X\n", dstname, Index[i].Offset, Index[i].FileSize);
			free(cdata);
		}
		fclose(dst);
		FileNum += 1;
	}
	fclose(src);
}

int main(int argc, char *argv[])
{
	char* InputFileName = argv[1];
	setlocale(LC_ALL, "chs");
	printf("project：Niflheim-BALDR HEART\n用于解包BH的pac文件。\n将pac文件拖到程序上。\nby Darkness-TX 2016.12.01\n\n添加新版NeXAS封包支持\nby AyamiKaze 2020.03.18\n\n\n");
	ReadIndex(InputFileName);
	UnpackFile(InputFileName);
	printf("已完成，总文件数%d\n", FileNum);
	system("pause");
	return 0;
}
