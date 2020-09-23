//###########################################################################
//�ļ����ƣ�f021_DownloadIimage.cpp
//�ļ�˵����Ϊf021�豸���س����ܡ�
//         �˹����������豸����ͨ�ź����ء� ����F021����������������������ͬ��
//         Ӧ�ó����͸�Ӧ�ó�����ִ���ں˵ķ�ʽ�� ������������£�������������
//         ����һ���ֽڣ��豸���Ը���ͬ�ֽڡ�
//###########################################################################

#include "../include/f021_DownloadImage.h"
#include "../include/f021_DownloadKernel.h"
#include "../include/f021_SendMessage.h"
#include "../include/serialFlash.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <stdbool.h>


#pragma once
#include <conio.h>
#include <windows.h>
#include <dos.h>

//�����������
#if _MSC_VER >= 1600	
#pragma execution_character_set("utf-8")
#endif

//*****************************************************************************
//
// ������Ϣ���
//
//*****************************************************************************
#define VERBOSEPRINT(...) if(g_bVerbose) { _tprintf(__VA_ARGS__); }
#define QUIETPRINT(...) if(!g_bQuiet) { _tprintf(__VA_ARGS__); }

//*****************************************************************************
//
// �ⲿȫ�ֱ���
//
//*****************************************************************************
extern bool g_bVerbose;
extern bool g_bQuiet;
extern bool g_bOverwrite;
extern bool g_bUpload;
extern bool g_bClear;
extern bool g_bBinary;
extern bool g_bWaitOnExit;
extern bool g_bReset;
extern bool g_bSwitchMode;
extern bool g_bDualCore;
extern wchar_t *g_pszAppFile;
extern wchar_t *g_pszAppFile2;
extern wchar_t *g_pszKernelFile;
extern wchar_t *g_pszKernelFile2;
extern wchar_t *g_pszComPort;
extern wchar_t *g_pszBaudRate;
extern wchar_t *g_pszDeviceName;

/**************************ȫ�ֱ����ж˿ھ���Լ��˿ں�************************/
extern HANDLE file;
extern DCB port;

//*****************************************************************************
//�ⲿ��������
//*****************************************************************************
extern void clearBuffer(void);
extern void autobaudLock(void);
extern void loadProgram(FILE *fh);
extern int f021_SendFunctionMessage(uint8_t message);
/*********************************��������************************************/
int f021_DownloadImage(wchar_t* applicationFile);

//*****************************************************************************
//�������ƣ�loadProgram_checksum
//����˵�������������ص�ͨ�����ݵľ����ʶ���豸�ϣ�Ҫ���صĳ����Լ���ò����йص�
//         ��������ͨ��ȫ�ֱ����������в������ơ�
//���������FILE  *fh    �����ļ�
//���ز�����
//         ������˳ɹ����򷵻�1��
//         ˫�˳ɹ�����2��
//         ʧ��ʱ����-1��
//
//*****************************************************************************
#define checksum_enable 1
#define g_bBlockSize 0x80 //���Ϳ����ݴ�С
#include <assert.h>
void loadProgram_checksum(FILE *fh)
{
	unsigned char sendData[20];
	unsigned int fileStatus;
	unsigned int rcvData = 0;
	DWORD dwRead;
	unsigned int checksum = 0;
	char ack = 0x2D;
	assert(g_bBlockSize % 4 == 0);  //��ΪECCʹ��64λ��4���ֵı�������4 == 0
	DWORD dwWritten;
	//�Ƴ��ļ����޹���Ϣ
	getc(fh);
	getc(fh);
	getc(fh);

	float bitRate = 0;
	DWORD millis = GetTickCount();
	//ǰ22���ֽ�Ϊ��ʼ������
	for (int i = 0; i < 22; i++)
	{
		fileStatus = fscanf_s(fh, "%x", &sendData[0]);
		//�����ַ�
		WriteFile(file, &sendData[0], 1, &dwWritten, NULL);
		bitRate += 8;
		checksum += sendData[0];
	}
	//�����豸У��͵ĵ��ֽ�
	dwRead = 0;
	while (dwRead == 0)
	{
		ReadFile(file, &sendData[0], 1, &dwRead, NULL);
	}
	//����ACK�ź�
	WriteFile(file, &ack, 1, &dwWritten, NULL);

	//�����豸У��͵ĸ��ֽ�
	dwRead = 0;
	while (dwRead == 0)
	{
		ReadFile(file, &sendData[1], 1, &dwRead, NULL);
	}
	//����ACK�ź�
	WriteFile(file, &ack, 1, &dwWritten, NULL);

	rcvData = (sendData[1] << 8) + sendData[0];
	//У��
	if (checksum != rcvData)
	{
		VERBOSEPRINT(_T("\nChecksum does not match... Please press Ctrl-C to abort."));
		while (1){}
	}
	unsigned int blockCount = 0;
	unsigned int blockSize;
	unsigned long destAddr;
	bool blockEffective;
	while (fileStatus == 1)
	{
		//��ȡ��һ�����ݿ��С���֣���hex2000ת�������ļ�
		fileStatus = fscanf_s(fh, "%x", &sendData[0]); //LSB
		fileStatus = fscanf_s(fh, "%x", &sendData[1]); //MSB
		blockSize = (sendData[1] << 8) | sendData[0];
		//������СΪ0,���ļ�����
		if (blockSize == 0x0000) 
		{
			//���Ϳ��СLSB
			WriteFile(file, &sendData[0], 1, &dwWritten, NULL);
			QUIETPRINT(_T("\n%lx"), sendData[0]);
			checksum += sendData[0];
			bitRate += 8;
			//���Ϳ��СMSB
			WriteFile(file, &sendData[1], 1, &dwWritten, NULL);
			QUIETPRINT(_T("\n%lx"), sendData[1]);
			checksum += sendData[1];
			bitRate += 8;
			break;
		}
		//��ȡ��һ��Ŀ���ַ��4���ֽڣ�32λ��
		fileStatus = fscanf_s(fh, "%x", &sendData[2]); //MSW[23:16]
		fileStatus = fscanf_s(fh, "%x", &sendData[3]); //MSW[31:24]
		fileStatus = fscanf_s(fh, "%x", &sendData[4]); //LSW[7:0]
		fileStatus = fscanf_s(fh, "%x", &sendData[5]); //LSW[15:8]
		destAddr = (sendData[3] << 24) | (sendData[2] << 16) |
			      (sendData[5] << 8) | (sendData[4]);
		//У�����ݵ�ַ��������ݵ�ַ��RAM�����򲻻��������ݿ�
		if (destAddr < 0x80000)
		{ 
			//�Ƴ����Ե����ݿ� 
			for (int j = 0; j < blockSize; j++)
			{
				fileStatus = fscanf_s(fh, "%x", &sendData[0]);
				fileStatus = fscanf_s(fh, "%x", &sendData[0]);
			}
			blockCount++;
			pSerialFlash->printfMessage(QString("��%1�����ݿ鱻����:").arg(blockCount));
			pSerialFlash->printfMessage(QString("address:0x%1 size:%2").arg(destAddr, 5, 16, QLatin1Char('0')).arg(blockSize));
		}
		else
		{ 
			//���Ϳ��СLSB
			WriteFile(file, &sendData[0], 1, &dwWritten, NULL);
			QUIETPRINT(_T("\n%lx"), sendData[0]);
			checksum += sendData[0];
			bitRate += 8;
			//���Ϳ��СMSB
			WriteFile(file, &sendData[1], 1, &dwWritten, NULL);
			QUIETPRINT(_T("\n%lx"), sendData[1]);
			checksum += sendData[1];
			bitRate += 8;

			//����Ŀ���ַMSW[23:16]
			WriteFile(file, &sendData[2], 1, &dwWritten, NULL);
			QUIETPRINT(_T("\n%lx"), sendData[2]);
			checksum += sendData[2];
			bitRate += 8;

			//����Ŀ���ַMSW[31:24]
			WriteFile(file, &sendData[3], 1, &dwWritten, NULL);
			QUIETPRINT(_T("\n%lx"), sendData[3]);
			checksum += sendData[3];
			bitRate += 8;

			//����Ŀ���ַLSW[7:0]
			WriteFile(file, &sendData[4], 1, &dwWritten, NULL);
			QUIETPRINT(_T("\n%lx"), sendData[4]);
			checksum += sendData[4];
			bitRate += 8;

			//����Ŀ���ַLSW[15:8]
			WriteFile(file, &sendData[5], 1, &dwWritten, NULL);
			QUIETPRINT(_T("\n%lx"), sendData[5]);
			checksum += sendData[5];
			bitRate += 8;
			/*���淢����Ϣ*/
			blockCount++;
			pSerialFlash->printfMessage(QString("�������ص�%1�����ݿ�:").arg(blockCount));
			pSerialFlash->printfMessage(QString("address:0x%1 size:%2").arg(destAddr, 5, 16, QLatin1Char('0')).arg(blockSize));

			//���Ϳ�����
			for (int j = 0; j < blockSize; j++)
			{
				if (((j % g_bBlockSize == 0) && (j > 0)) || ((blockSize < g_bBlockSize) && (j == blockSize)))
				{
					//����У���LSB
					dwRead = 0;
					while (dwRead == 0)
					{
						ReadFile(file, &sendData[0], 1, &dwRead, NULL);
					}
					//����ACK�ź�
					WriteFile(file, &ack, 1, &dwWritten, NULL);
					//����У���MSB
					dwRead = 0;
					while (dwRead == 0)
					{
						ReadFile(file, &sendData[1], 1, &dwRead, NULL);
					}
					//����ACK�ź�
					WriteFile(file, &ack, 1, &dwWritten, NULL);

					rcvData = sendData[0] | (sendData[1] << 8);
					//У��
					if ((checksum & 0xFFFF) != rcvData)
					{
						VERBOSEPRINT(_T("\nChecksum does not match... Please press Ctrl-C to abort."));
						while (1) {}
					}
				}


				//����������LSB
				fileStatus = fscanf_s(fh, "%x", &sendData[0]);
				WriteFile(file, &sendData[0], 1, &dwWritten, NULL);
				QUIETPRINT(_T("\n%lx"), sendData[0]);
				checksum += sendData[0];
				bitRate += 8;

				//����������MSB
				fileStatus = fscanf_s(fh, "%x", &sendData[0]);
				WriteFile(file, &sendData[0], 1, &dwWritten, NULL);
				QUIETPRINT(_T("\n%lx"), sendData[0]);
				checksum += sendData[0];
				bitRate += 8;
			}
			//����У���LSB

			dwRead = 0;
			while (dwRead == 0)
			{
				ReadFile(file, &sendData[0], 1, &dwRead, NULL);
			}
			//����ACK�ź�
			WriteFile(file, &ack, 1, &dwWritten, NULL);
			//����У���MSB
			dwRead = 0;
			while (dwRead == 0)
			{
				ReadFile(file, &sendData[1], 1, &dwRead, NULL);
			}
			//����ACK�ź�
			WriteFile(file, &ack, 1, &dwWritten, NULL);

			rcvData = sendData[0] | (sendData[1] << 8);
			//У��
			if ((checksum & 0xFFFF) != rcvData)
			{
				VERBOSEPRINT(_T("\nChecksum does not match... Please press Ctrl-C to abort."));
				while (1) {}
			}
		}

	}
	//���㷢�͵������ٶ�
	millis = GetTickCount() - millis;
	bitRate = bitRate / millis * 1000;
	QUIETPRINT(_T("\nBit rate /s of transfer was: %f"), bitRate);
	rcvData = 0;
	//������Ϣ������
    pSerialFlash->printfMessage(QString("��������ʱ��:%1s").arg((float)millis/1000,3));
    pSerialFlash->printfMessage(QString("�����ٶ�:%1 bit/s").arg(bitRate));
}
//*****************************************************************************
//�������ƣ�loadProgram_checksum
//����˵�������������ص�ͨ�����ݵľ����ʶ���豸�ϣ�Ҫ���صĳ����Լ���ò����йص�
//         ��������ͨ��ȫ�ֱ����������в������ơ�
//���������FILE  *fh    �����ļ�
//���ز�����
//         ������˳ɹ����򷵻�1��
//         ˫�˳ɹ�����2��
//         ʧ��ʱ����-1��
//
//*****************************************************************************
//#define checksum_enable 1
//#define g_bBlockSize 0x80 //���Ϳ����ݴ�С
//#include <assert.h>
void loadProgram_checksum1(FILE* fh)
{
	unsigned char sendData[8];
	unsigned int fileStatus;
	unsigned int rcvData = 0;
	DWORD dwRead;
	unsigned int checksum = 0;
	char ack = 0x2D;
	assert(g_bBlockSize % 4 == 0);  //��ΪECCʹ��64λ��4���ֵı�������4 == 0
	DWORD dwWritten;
	//�Ƴ��ļ����޹���Ϣ
	getc(fh);
	getc(fh);
	getc(fh);

	float bitRate = 0;
	DWORD millis = GetTickCount();
	//ǰ22���ֽ�Ϊ��ʼ������
	for (int i = 0; i < 22; i++)
	{
		fileStatus = fscanf_s(fh, "%x", &sendData[0]);
		//�����ַ�
		WriteFile(file, &sendData[0], 1, &dwWritten, NULL);
		bitRate += 8;
		checksum += sendData[0];
	}
	//�����豸У��͵ĵ��ֽ�
	dwRead = 0;
	while (dwRead == 0)
	{
		ReadFile(file, &sendData[0], 1, &dwRead, NULL);
	}
	//����ACK�ź�
	WriteFile(file, &ack, 1, &dwWritten, NULL);

	//�����豸У��͵ĸ��ֽ�
	dwRead = 0;
	while (dwRead == 0)
	{
		ReadFile(file, &sendData[1], 1, &dwRead, NULL);
	}
	//����ACK�ź�
	WriteFile(file, &ack, 1, &dwWritten, NULL);

	rcvData = (sendData[1] << 8) + sendData[0];
	//У��
	if (checksum != rcvData)
	{
		VERBOSEPRINT(_T("\nChecksum does not match... Please press Ctrl-C to abort."));
		while (1) {}
	}
	unsigned int blockCount = 0;
	while (fileStatus == 1)
	{
		unsigned int blockSize;
		//��ȡ��һ�����ݿ��С���֣���hex2000ת�������ļ�
		fileStatus = fscanf_s(fh, "%x", &sendData[0]); //LSB
		fileStatus = fscanf_s(fh, "%x", &sendData[1]); //MSB
		blockSize = (sendData[1] << 8) | sendData[0];

		//���Ϳ��СLSB
		WriteFile(file, &sendData[0], 1, &dwWritten, NULL);
		QUIETPRINT(_T("\n%lx"), sendData[0]);
		checksum += sendData[0];
		bitRate += 8;

		//���Ϳ��СMSB
		WriteFile(file, &sendData[1], 1, &dwWritten, NULL);
		QUIETPRINT(_T("\n%lx"), sendData[1]);
		checksum += sendData[1];
		bitRate += 8;
		//������СΪ0,���ļ�����
		if (blockSize == 0x0000)
		{
			break;
		}

		//��hex2000�ı��ļ��ж�ȡ��һ��Ŀ���ַ��4���ֽڣ�32λ��
		fileStatus = fscanf_s(fh, "%x", &sendData[0]); //MSW[23:16]
		fileStatus = fscanf_s(fh, "%x", &sendData[1]); //MSW[31:24]
		fileStatus = fscanf_s(fh, "%x", &sendData[2]); //LSW[7:0]
		fileStatus = fscanf_s(fh, "%x", &sendData[3]); //LSW[15:8]
		unsigned long destAddr = (sendData[1] << 24) | (sendData[0] << 16) |
			(sendData[3] << 8) | (sendData[2]);

		//����Ŀ���ַMSW[23:16]
		WriteFile(file, &sendData[0], 1, &dwWritten, NULL);
		QUIETPRINT(_T("\n%lx"), sendData[0]);
		checksum += sendData[0];
		bitRate += 8;

		//����Ŀ���ַMSW[31:24]
		WriteFile(file, &sendData[1], 1, &dwWritten, NULL);
		QUIETPRINT(_T("\n%lx"), sendData[1]);
		checksum += sendData[1];
		bitRate += 8;

		//����Ŀ���ַLSW[7:0]
		WriteFile(file, &sendData[2], 1, &dwWritten, NULL);
		QUIETPRINT(_T("\n%lx"), sendData[2]);
		checksum += sendData[2];
		bitRate += 8;

		//����Ŀ���ַLSW[15:8]
		WriteFile(file, &sendData[3], 1, &dwWritten, NULL);
		QUIETPRINT(_T("\n%lx"), sendData[3]);
		checksum += sendData[3];
		bitRate += 8;

		/*���淢����Ϣ*/
		blockCount++;
		pSerialFlash->printfMessage(QString("�������ص�%1�����ݿ�:").arg(blockCount));
		pSerialFlash->printfMessage(QString("address:0x%1 size:%2").arg(destAddr, 5, 16, QLatin1Char('0')).arg(blockSize));

		//���Ϳ�����
		for (int j = 0; j < blockSize; j++)
		{
			if (((j % g_bBlockSize == 0) && (j > 0)) || ((blockSize < g_bBlockSize) && (j == blockSize)))
			{
				//����У���LSB
				dwRead = 0;
				while (dwRead == 0)
				{
					ReadFile(file, &sendData[0], 1, &dwRead, NULL);
				}
				//����ACK�ź�
				WriteFile(file, &ack, 1, &dwWritten, NULL);
				//����У���MSB
				dwRead = 0;
				while (dwRead == 0)
				{
					ReadFile(file, &sendData[1], 1, &dwRead, NULL);
				}
				//����ACK�ź�
				WriteFile(file, &ack, 1, &dwWritten, NULL);

				rcvData = sendData[0] | (sendData[1] << 8);
				//У��
				if ((checksum & 0xFFFF) != rcvData)
				{
					VERBOSEPRINT(_T("\nChecksum does not match... Please press Ctrl-C to abort."));
					while (1) {}
				}
			}

			//����������LSB
			fileStatus = fscanf_s(fh, "%x", &sendData[0]);
			WriteFile(file, &sendData[0], 1, &dwWritten, NULL);
			QUIETPRINT(_T("\n%lx"), sendData[0]);
			checksum += sendData[0];
			bitRate += 8;

			//����������MSB
			fileStatus = fscanf_s(fh, "%x", &sendData[0]);
			WriteFile(file, &sendData[0], 1, &dwWritten, NULL);
			QUIETPRINT(_T("\n%lx"), sendData[0]);
			checksum += sendData[0];
			bitRate += 8;
		}
		//����У���LSB
		dwRead = 0;
		while (dwRead == 0)
		{
			ReadFile(file, &sendData[0], 1, &dwRead, NULL);
		}
		//����ACK�ź�
		WriteFile(file, &ack, 1, &dwWritten, NULL);
		//����У���MSB
		dwRead = 0;
		while (dwRead == 0)
		{
			ReadFile(file, &sendData[1], 1, &dwRead, NULL);
		}
		//����ACK�ź�
		WriteFile(file, &ack, 1, &dwWritten, NULL);

		rcvData = sendData[0] | (sendData[1] << 8);
		//У��
		if ((checksum & 0xFFFF) != rcvData)
		{
			VERBOSEPRINT(_T("\nChecksum does not match... Please press Ctrl-C to abort."));
			while (1) {}
		}
	}
	//���㷢�͵������ٶ�
	millis = GetTickCount() - millis;
	bitRate = bitRate / millis * 1000;
	QUIETPRINT(_T("\nBit rate /s of transfer was: %f"), bitRate);
	rcvData = 0;
	//������Ϣ������
	pSerialFlash->printfMessage(QString("��������ʱ��:%1s").arg((float)millis / 1000, 3));
	pSerialFlash->printfMessage(QString("�����ٶ�:%1 bit/s").arg(bitRate));
}
///******************************************************************************
// * �������ƣ�f021_DownloadImage
// * ����˵�������س����ļ�
// * ���������
// *           wchar_t* applicationFile  Ӧ�ó����ļ�·��
// * ���ز�����
// *           1�ɹ�    -1ʧ��
// ******************************************************************************/
int f021_DownloadImage(wchar_t* applicationFile)
{
	FILE *Afh;
	unsigned int rcvData = 0;
	unsigned int rcvDataH = 0;
	unsigned int txCount = 0;

	DWORD dwLen = 1;
	//��ʾӦ�ó������Ʋ���ʾ��ʼ����
	QUIETPRINT(_T("Downloading %s to device...\n"), applicationFile);

	//��Ӧ�ó����ļ�
	Afh = _tfopen(applicationFile, L"rb");
	//�ļ���ʧ��
	if (!Afh)
	{
		QUIETPRINT(_T("Unable to open Application file %s. Does it exist?\n"), applicationFile);
		return(-1);
	}
	//���س����ļ�
#if checksum_enable
	loadProgram_checksum(Afh);
	//loadProgram_checksum1(Afh);
#else
	loadProgram(Afh);
#endif
	//�����Ϣ
	VERBOSEPRINT(_T("\nApplication load successful!"));

	VERBOSEPRINT(_T("\nDone waiting for application to download and boot... "));
	//������ڻ���
	clearBuffer();
	return(1);
}