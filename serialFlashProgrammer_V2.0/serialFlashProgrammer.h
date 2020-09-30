#pragma once

#include <QtWidgets/QWidget>
#include <qprocess.h>
#include <qthread.h>
#include <qtimer.h>
#include "./flashProgramer/include/serialFlash.h"
#include "ui_serialFlashProgrammer.h"



#define PRG_FILE_NAME      "program.txt"
#define PRG_FILE1_NAME     "program1.txt"
#define APP_ERTER_ADDRESS  "84000"         //APP������ڵ�ַ  
#define MAX_BLACK_NUMBER   100             //����APP���ݿ���������

//��Ϣ����
#define MSG_TEXT           0x01            //�ı�
#define MSG_CMD            0x02            //����
#define MSG_ERROR          0x04            //����
#define MSG_WARNING        0x08            //����
#define MSG_WITH_TIME      0x10            //��ʱ����Ϣ

//��Ϣ�ı�
#define WELCOME_TEXT  "*******************  C2000������������  *******************"

class serialFlashProgrammer : public QWidget
{
    Q_OBJECT
        //appFileInfo�ṹ��
        typedef struct APPFILEINFO {
        QString   appFileDir;      //·��
        QString   appFileName;     //�ļ�����
        quint32   appEnterAddress; //��ڵ�ַ
        quint32   allBlockNumber;  //��������
        quint32   allBlockSize;    //���ܴ�С
        float     estimatedTime;   //Ԥ��������ʱ
    }appFileInfo;
public:
    serialFlashProgrammer(QWidget *parent = Q_NULLPTR);
    ~serialFlashProgrammer();
    void programToSci8(QString path,QString programName);
    void appFileMessage(QString name, appFileInfo* info);
    void outputMessage(QString str,qint16 type = MSG_TEXT | MSG_WITH_TIME);
    void scanComPort();
    bool nativeEvent(const QByteArray& eventType, void* message, long* result);
private slots:
    void on_btn_selectProgramFile_clicked();//ѡ������ļ���ť������
    void on_btn_selectProgramFile1_clicked();//ѡ������ļ�1��ť������
    void on_btn_portSearch_clicked();//�˿�������ť������
    void on_btn_openPart_clicked();//�򿪴��ڰ�ť������
    void vn_cbox_device_activated(QString);//�豸����ѡ���ı��
    void vn_btn_execute_clicked();//ִ�а�ť������
    void vn_lineEdit_enterAddr_editingFinished();//��ַ�������������
    void vn_checkBox_bandStatus_stateChange(int);//�����ʿ�����ѡ���
    void vn_btn_cleanMessage_clicked();//��������Ϣ��ť
    void vn_btn_viewProgram_clicked();//�鿴������Ϣ��ť
private slots:
    void workThreadStart();//�߳������ۺ���
    void workThreadFinish();//�߳̽����ۺ���
    void workFinish(int);//��������źŲ�
    void workMessage(QString);//������Ϣ�źŲ�
    void workSchedule(int);//������ɽ����źŲ�
    void timeoutTimerUpdate();//��ʱ��ʱ�����
signals:
    //���̷߳����źţ��ź�Я��������
    void sendFunctionNumToThread(int code);

private:
    void closeEvent(QCloseEvent*);//���ڹر�

private:
    Ui::serialFlashProgrammerClass ui;
    QProcess hex2000;

private:

    bool m_functionStatus;//����ִ��״̬ 

    wchar_t m_appName[20];//Ӧ�ó�������
    appFileInfo m_appCpu1FileInfo;//CPU1�����ļ���Ϣ
    wchar_t m_appName1[20];//Ӧ�ó�������
    appFileInfo m_appCpu2FileInfo;//CPU2�����ļ���Ϣ
    uint32_t m_branchAddress;//Ӧ�ó�����ڵ�ַ

    int m_workProgress;//����ִ�н���

    wchar_t m_comPort[20];//���ں�(���ͼ��ݵײ�����)
    wchar_t m_baudRate[20];//������
    bool m_autoBand;//�Զ�������
    bool m_comStatus;//�˿�״̬

    QThread workerThread;//���������߳�
    SerialFlash serialFlash;//����serialFlash����
    QTimer timeoutTimer;//��ʱ��ʱ��
    uint m_timeCount;//��ʱʱ��
};
