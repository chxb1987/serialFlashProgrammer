#ifndef SERIAL_FLASH_H_
#define SERIAL_FLASH_H_
#include <qobject.h>
#include <QtWidgets/QWidget>

/************************************��������************************************/
extern bool g_bVerbose;
extern bool g_bQuiet;
extern bool g_bOverwrite;
extern bool g_bUpload;
extern bool g_bClear;
extern bool g_bBinary;
extern bool g_bWaitOnExit;
extern bool g_bReset;
extern bool g_bSwitchMode;
extern bool g_baudLock;       //������������־
extern bool cpu1;            //CPU1���߱�־
extern bool cpu2;            //CPU2���߱�־
/**************************************�豸��*********************************/
extern bool g_bf2802x;
extern bool g_bf2803x;
extern bool g_bf2805x;
extern bool g_bf2806x;
extern bool g_bf2833x;
extern bool g_bf2837xD;
extern bool g_bf2837xS;
extern bool g_bf2807x;
extern bool g_bf28004x;


/********************************���й���***************************************/
extern uint32_t gu32_branchAddress;   //��ڵ�ַ



extern wchar_t* g_pszAppFile;
extern wchar_t* g_pszAppFile2;
extern wchar_t* g_pszKernelFile;
extern wchar_t* g_pszKernelFile2;
extern wchar_t* g_pszComPort;
extern wchar_t* g_pszBaudRate;
extern wchar_t* g_pszDeviceName;
/*******************************��������*****************************************/
#define DFU_CPU1					    0x0100
#define DFU_CPU2					    0x0200
#define ERASE_CPU1					0x0300
#define ERASE_CPU2					0x0400
#define VERIFY_CPU1					0x0500
#define VERIFY_CPU2					0x0600
#define LIVE_DFU_CPU1                 0x0700
#define CPU1_UNLOCK_Z1				0x000A
#define CPU1_UNLOCK_Z2				0x000B
#define CPU2_UNLOCK_Z1				0x000C
#define CPU2_UNLOCK_Z2				0x000D
#define RUN_CPU1					    0x000E
#define RESET_CPU1					0x000F
#define RUN_CPU1_BOOT_CPU2			    0x0004
#define RESET_CPU1_BOOT_CPU2          0x0007
#define RUN_CPU2					    0x0010
#define RESET_CPU2					0x0020
/*******************************���ش�����*****************************************/
#define NO_ERROR					    0x1000
#define BLANK_ERROR					0x2000
#define VERIFY_ERROR				    0x3000
#define PROGRAM_ERROR				    0x4000
#define COMMAND_ERROR				    0x5000
#define UNLOCK_ERROR				    0x6000
/************************************��������************************************/
extern int openCom();
extern int closeCom();


/********************************�������߳�ʹ��**********************************/
class SerialFlash : public QObject
{
	Q_OBJECT
public:
	SerialFlash();
	~SerialFlash();
	int function_DFU_CPU1();//����CPU1����
	int function_RUN_CPU1();
	int function_DFU_CPU2();
	int function_RUN_CPU2();
	int function_ERASE_CPU1();
	int function_RESET_CPU1_BOOT_CPU2();
	int function_RUN_CPU1_BOOT_CPU2();

	void emitWorkScheduleSignal(int);//�������ؽ����ź�
	void printfMessage(QString);//��ӡ��Ϣ����

public slots:
	void dowork(int);

signals:
	//��������źţ�������
	void workFinishSignal(int);
	//��Ϣ����źţ�������Ϣ
	void workMessageSignal(QString);
	//���������źţ����ؽ���
	void workScheduleSignal(int);
};

extern SerialFlash* pSerialFlash;

#endif // !SERIAL_FLASH_H_

