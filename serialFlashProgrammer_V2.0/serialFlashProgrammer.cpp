/************************************************************************
* �ļ����ƣ�
* �ļ�˵����
* ����˵����
*           1.�̶�APP������ڵ�ַ��0x84000
*     ���ߣ�
* ���ʱ�䣺
* �޸ļ�¼��
*************************************************************************/
#include "serialFlashProgrammer.h"
#include "./flashProgramer/include/serialFlash.h"
#include <qdebug.h>
#include <qfiledialog.h>
#include <QtSerialPort/qserialport.h>
#include <QtSerialPort/qserialportinfo.h>
#include <qdatetime.h>
#include <qtoolbutton.h>
#include <qmessagebox.h>
#include <qevent.h>
#include <qmenu.h>
#include <qaction.h>
#include <iostream>
#include <qtoolbar.h>
#include <windows.h>

//�����������
#if _MSC_VER >= 1600	
#pragma execution_character_set("utf-8")
#endif

//���ӱ������Ҽ��˵�������
#pragma comment(lib, "User32.lib")
#define IDM_ABOUTBOX 0x0010

/************************************************************************
* �������ƣ�serialFlashProgrammer
* ����˵�������캯��
* ����˵����
* ���������
* ���ز�����
*************************************************************************/
serialFlashProgrammer::serialFlashProgrammer(QWidget* parent)
	: QWidget(parent)
{
	//�������
	ui.setupUi(this);
	//������ʼ��

	//��ڵ�ַ������ʼ��
	ui.enterAddrLineEdit->setText(APP_ERTER_ADDRESS);
	ui.enterAddrLineEdit->setEnabled(false);
	//���ؽ�����
	ui.progressBar->setMinimum(0);
	ui.progressBar->setMaximum(100);
	ui.progressBar->setValue(0);
	ui.progressBar->hide();
	//����������
	ui.cbox_core->addItem("F021");
	//�豸������
	ui.cbox_device->addItem("F2837xD");
	ui.cbox_device->addItem("F2837xS");
	ui.cbox_device->addItem("F2807x");
	ui.cbox_device->addItem("F28004x");
	//F021�ںˣ�F2827xD����������������
	ui.cbox_function->clear();
	ui.cbox_function->addItem("����CPU1", QVariant(DFU_CPU1));
	ui.cbox_function->addItem("����CPU2", QVariant(DFU_CPU2));
	ui.cbox_function->addItem("����CPU1", QVariant(ERASE_CPU1));
	ui.cbox_function->addItem("����CPU2", QVariant(ERASE_CPU2));
	ui.cbox_function->addItem("��֤CPU1", QVariant(VERIFY_CPU1));
	ui.cbox_function->addItem("��֤CPU2", QVariant(VERIFY_CPU2));
	ui.cbox_function->addItem("����CPU1����1", QVariant(CPU1_UNLOCK_Z1));
	ui.cbox_function->addItem("����CPU1����2", QVariant(CPU1_UNLOCK_Z2));
	ui.cbox_function->addItem("����CPU2����1", QVariant(CPU2_UNLOCK_Z1));
	ui.cbox_function->addItem("����CPU2����2", QVariant(CPU2_UNLOCK_Z2));
	ui.cbox_function->addItem("����CPU1", QVariant(RUN_CPU1));
	ui.cbox_function->addItem("����CPU1", QVariant(RESET_CPU1));
	ui.cbox_function->addItem("����CPU1����CPU2", QVariant(RUN_CPU1_BOOT_CPU2));
	ui.cbox_function->addItem("����CPU1����CPU2", QVariant(RESET_CPU1_BOOT_CPU2));
	ui.cbox_function->addItem("����CPU2", QVariant(RUN_CPU2));
	ui.cbox_function->addItem("����CPU2", QVariant(RESET_CPU2));
	//�����ӭ��Ϣ
	outputMessage(WELCOME_TEXT, MSG_TEXT);
	//ɨ��˿���Ϣ��ʾ��������
	scanComPort();
	//���ò�����
	ui.cbox_baud->clear();
	ui.cbox_baud->addItem("1200");
	ui.cbox_baud->addItem("2400");
	ui.cbox_baud->addItem("4800");
	ui.cbox_baud->addItem("9600");
	ui.cbox_baud->addItem("19200");
	ui.cbox_baud->addItem("38400");
	ui.cbox_baud->addItem("57600");
	ui.cbox_baud->addItem("115200");
	//��ѡ��Ĭ��ѡ��
	m_autoBand = true;
	ui.bandSelectCheckBox->setChecked(true);
	//���õ�ַ����������
	QRegExp regExp("[a-fA-F0-9]{6}");
	ui.enterAddrLineEdit->setValidator(new QRegExpValidator(regExp, this));
    //��ʼ����ʱʱ�䣨10s��
	m_timeCount = 10000;
	//���ӳ�ʱ��ʱ���ź�
	connect(&timeoutTimer, SIGNAL(timeout()), this, SLOT(timeoutTimerUpdate()));
	//�̴߳���
	serialFlash.moveToThread(&workerThread);//�����ع����Ƶ������߳���
	//��������ź�
	connect(this, SIGNAL(sendFunctionNumToThread(int)), &serialFlash, SLOT(dowork(int)));//����
	connect(&serialFlash, SIGNAL(workFinishSignal(int)), this, SLOT(workFinish(int)));//�������
	connect(&serialFlash, SIGNAL(workMessageSignal(QString)), this, SLOT(workMessage(QString)));//��Ϣ���
	connect(&serialFlash, SIGNAL(workScheduleSignal(int)), this, SLOT(workSchedule(int)));//����
	connect(&workerThread, SIGNAL(started()), this, SLOT(workThreadStart()));//��ʼ
	connect(&workerThread, SIGNAL(finished()), this, SLOT(workThreadFinish()));//����
	//�����߳�
	workerThread.start();
	//���ô��ڱ������Ҽ��˵�
	HMENU sysMenu = GetSystemMenu((HWND)winId(), FALSE);
	if (sysMenu != NULL)
	{
		QString str = "About(&A)";
		AppendMenuA(sysMenu, MF_SEPARATOR, 0, 0);
		AppendMenuA(sysMenu, MF_STRING, IDM_ABOUTBOX, str.toStdString().c_str());		
	}
	//����C����
	pSerialFlash = &serialFlash;
}
/************************************************************************
* �������ƣ�~serialFlashProgrammer
* ����˵������������
* ����˵����
* ���������
* ���ز�����
*************************************************************************/
serialFlashProgrammer::~serialFlashProgrammer()
{

}
/************************************************************************
* �������ƣ�
* ����˵����
* ����˵����
* ���������
* ���ز�����
*************************************************************************/
bool serialFlashProgrammer::nativeEvent(const QByteArray& eventType, void* message, long* result)
{
	Q_UNUSED(eventType);
    MSG* msg = reinterpret_cast<MSG*>(message);
	if (msg->message == WM_SYSCOMMAND)
	{
		if ((msg->wParam & 0xfff0) == IDM_ABOUTBOX)
		{
			*result = 0;
			//���Զ�����ڶԻ���
			QMessageBox::about(this, "����", "����:liyu\r\nʱ��:2020.9.3");
			return (true);
		}
	}
	return (false);
}
/************************************************************************
* �������ƣ�on_btn_selectProgramFile_clicked()
* ����˵����ѡ��CPU1����ť->�����ź�->�ۺ���
* ����˵������ʾ�ļ�·����lineEdit��,��ת���ļ���SCI8��ʽ
* ���������
* ���ز�����
*************************************************************************/
void serialFlashProgrammer::on_btn_selectProgramFile_clicked()
{

	QString digTile = "ѡ������ļ�";//����
	QString curPath = QDir::currentPath();//��ȡϵͳ��ǰĿ¼
	QString filter = "�����ļ�(*.out)";//�ı�������
	//���ļ��Ի���
	QString aFileName = QFileDialog::getOpenFileName(this, digTile, curPath, filter);
	//���ص��ļ�����Ϊ��
	if (!aFileName.isEmpty())
	{
		//·����ʾ��text����
		ui.lineEdit->setText(aFileName);
		//����·��������
		int first = aFileName.lastIndexOf("/");
		QString name = aFileName.right(aFileName.length() - first - 1);
		QString dir = aFileName.left(aFileName.length() - first);
		m_appCpu1FileInfo.appFileDir = dir;
		m_appCpu1FileInfo.appFileName = name;
		//ת���ļ�
		programToSci8(aFileName, PRG_FILE_NAME);
		//����ļ���Ϣ
		appFileMessage(PRG_FILE_NAME, &m_appCpu1FileInfo);
		//����ת�����ļ���
		memset(m_appName, 0, sizeof(m_appName));
		QString(PRG_FILE_NAME).toWCharArray(m_appName);

	}
}
/************************************************************************
* �������ƣ�on_btn_selectProgramFile1_clicked()
* ����˵����ѡ��CPU2����ť->�����ź�->�ۺ���
* ����˵������ʾ�ļ�·����lineEdit��,��ת���ļ���SCI8��ʽ
* ���������
* ���ز�����
*************************************************************************/
void serialFlashProgrammer::on_btn_selectProgramFile1_clicked()
{
	QString digTile = "ѡ������ļ�";//����
	QString curPath = QDir::currentPath();//��ȡϵͳ��ǰĿ¼
	QString filter = "�����ļ�(*.out)";//�ı�������
	//���ļ��Ի���
	QString aFileName = QFileDialog::getOpenFileName(this, digTile, curPath, filter);
	//���ص��ļ�����Ϊ��
	if (!aFileName.isEmpty())
	{
		//·����ʾ��text����
		ui.lineEditPrg1FilePath->setText(aFileName);
		//ת���ļ�
		programToSci8(aFileName, PRG_FILE1_NAME);
		//����ļ���Ϣ
		appFileMessage(PRG_FILE1_NAME, &m_appCpu2FileInfo);
		//�����ļ���
		memset(m_appName1, 0, sizeof(m_appName1));
		QString(PRG_FILE1_NAME).toWCharArray(m_appName1);
	}
}
/************************************************************************
* �������ƣ�on_btn_portSearch_clicked()
* ����˵�����˿�������ť->�����ź�->�ۺ���
* ����˵����
* ���������
* ���ز�����
*************************************************************************/
void serialFlashProgrammer::on_btn_portSearch_clicked()
{
	//ɨ��ȫ���˿���Ϣ����ʾ����������
	scanComPort();
}
/************************************************************************
* �������ƣ�on_btn_openPart_clicked()
* ����˵�����򿪴��ڰ�ť->�����ź�->�ۺ���
* ����˵����
* ���������
* ���ز�����
*************************************************************************/
void serialFlashProgrammer::on_btn_openPart_clicked()
{
	//�жϰ���״̬
	if (ui.btn_openPart->text() == "�򿪴���")
	{
		//���˿�ѡ����������û���豸��
		if (ui.cbox_port->count() == 0)
		{
			outputMessage(QString("����ʧ�ܣ�δѡ����ȷ����"),MSG_ERROR |MSG_WITH_TIME);
			return;
		}
		//���ö˿ں�
		memset(m_comPort, 0, sizeof(m_comPort));
		ui.cbox_port->currentText().toWCharArray(m_comPort);
		g_pszComPort = m_comPort;
		//���ò�����
		memset(m_baudRate, 0, sizeof(m_baudRate));
		//�жϲ����ʼ���־
		if (m_autoBand == true)
		{
			ui.cbox_baud->currentText().toWCharArray(m_baudRate);
		}
		else
		{
			//�̶�������
			QString("115200").toWCharArray(m_baudRate);
		}
		g_pszBaudRate = m_baudRate;
		//�����˿�
		int ret = openCom();

		//�ж��Ƿ����ɹ�
		if (ret == 0)
		{
			outputMessage(QString("���ڿ���ʧ��"), MSG_ERROR | MSG_WITH_TIME);
		}
		else
		{
			outputMessage(QString("���ڿ����ɹ�"), MSG_TEXT | MSG_WITH_TIME);
			ui.btn_openPart->setText("�رմ���");
			//��ֹ�˿ڼ�������ѡ���Լ�ɨ��
			ui.cbox_port->setEnabled(false);
			ui.cbox_baud->setEnabled(false);
			ui.btn_portSearch->setEnabled(false);
			m_comStatus = true;//�ı�״̬
			if (m_autoBand == false)
			{
				g_baudLock = true;//�رղ����ʼ��
			}
			
		}
	}
    //�رմ���
	else if (ui.btn_openPart->text() == "�رմ���")
	{
		//�رն˿�
		int rtn = closeCom();
	    //�ж��Ƿ�رճɹ�
		if (rtn == 0)
		{
			outputMessage(QString("���ڹر�ʧ��"), MSG_ERROR | MSG_WITH_TIME);
		}
		else
		{
			outputMessage(QString("���ڹرճɹ�"));
			ui.btn_openPart->setText("�򿪴���"); 
			//�����˿ڼ�������ѡ���Լ�ɨ��
			ui.cbox_port->setEnabled(true);
			ui.cbox_baud->setEnabled(true);
			ui.btn_portSearch->setEnabled(true);
			m_comStatus = false;//�ı�״̬
		}
	}
}
/************************************************************************
* �������ƣ�vn_cbox_device_activated
* ����˵�����豸ѡ��ı�ۺ���
* ����˵����
* ���������
* ���ز�����
*************************************************************************/
void serialFlashProgrammer::vn_cbox_device_activated(QString str)
{
	//���豸��������
	if (str == "F2837xD")
	{
		//F021�ںˣ�F2827xD����������������
		ui.cbox_function->clear();
		ui.cbox_function->addItem("����CPU1", QVariant(DFU_CPU1));
		ui.cbox_function->addItem("����CPU2", QVariant(DFU_CPU2));
		ui.cbox_function->addItem("����CPU1", QVariant(ERASE_CPU1));
		ui.cbox_function->addItem("����CPU2", QVariant(ERASE_CPU2));
		ui.cbox_function->addItem("��֤CPU1", QVariant(VERIFY_CPU1));
		ui.cbox_function->addItem("��֤CPU2", QVariant(VERIFY_CPU2));
		ui.cbox_function->addItem("����CPU1����1", QVariant(CPU1_UNLOCK_Z1));
		ui.cbox_function->addItem("����CPU1����2", QVariant(CPU1_UNLOCK_Z2));
		ui.cbox_function->addItem("����CPU2����1", QVariant(CPU2_UNLOCK_Z1));
		ui.cbox_function->addItem("����CPU2����2", QVariant(CPU2_UNLOCK_Z2));
		ui.cbox_function->addItem("����CPU1", QVariant(RUN_CPU1));
		ui.cbox_function->addItem("����CPU1", QVariant(RESET_CPU1));
		ui.cbox_function->addItem("����CPU1����CPU2", QVariant(RUN_CPU1_BOOT_CPU2));
		ui.cbox_function->addItem("����CPU1����CPU2", QVariant(RESET_CPU1_BOOT_CPU2));
		ui.cbox_function->addItem("����CPU2", QVariant(RUN_CPU2));
		ui.cbox_function->addItem("����CPU2", QVariant(RESET_CPU2));
	}
	else
	{
		//F021�ںˣ�F2807x��F2837xS��F28004x�豸��������������
		ui.cbox_function->clear();
		ui.cbox_function->addItem("����CPU", QVariant(DFU_CPU1));
		ui.cbox_function->addItem("����CPU", QVariant(ERASE_CPU1));
		ui.cbox_function->addItem("��֤CPU", QVariant(VERIFY_CPU1));
		ui.cbox_function->addItem("����CPU����1", QVariant(CPU1_UNLOCK_Z1));
		ui.cbox_function->addItem("����CPU����2", QVariant(CPU1_UNLOCK_Z2));
		ui.cbox_function->addItem("����CPU", QVariant(RUN_CPU1));
		ui.cbox_function->addItem("��λCPU", QVariant(RESET_CPU1));
	}

}
/************************************************************************
* �������ƣ�vn_lineEdit_enterAddr_editingFinished
* ����˵������ڵ�ַ�����->���ػ�س�->��
* ����˵����
* ���������
* ���ز�����
*************************************************************************/
void serialFlashProgrammer::vn_lineEdit_enterAddr_editingFinished()
{
	////��ȡ�ı�
	//QString str = ui.enterAddrLineEdit->text();
	//qDebug() << str << endl;
	////ת����32λ����
	//qDebug() << str.toUInt(nullptr,16) << endl;
}
/************************************************************************
* �������ƣ�timeoutTimerUpdate
* ����˵������ʱ��ʱ����ʱ�źŲ�
* ����˵����
* ���������
* ���ز�����
*************************************************************************/
void serialFlashProgrammer::timeoutTimerUpdate()
{
	//�رն�ʱ��
	timeoutTimer.stop();
	//��Ϣ����ʾ�ѳ�ʱ���Ƿ�ǿ�ƹر��߳�
	QMessageBox::StandardButton rtn;
	QMessageBox::StandardButtons buttons = QMessageBox::Yes | QMessageBox::No;
	QMessageBox::StandardButton defbutton = QMessageBox::Yes;
	QString txtLable = "ִ�й��ܳ�ʱ���Ƿ�ǿ�������߳�";
	rtn = QMessageBox::warning(this, "����", txtLable, buttons, defbutton);//����
	//ȷ��ǿ�������߳�
	if (rtn == QMessageBox::Yes)
	{
		outputMessage(QString("���ڳ��Թر��߳�"));
		workerThread.terminate();
		outputMessage(QString("�ȴ��̹߳ر�"));
		workerThread.wait();
		outputMessage(QString("������"));
		workerThread.start();
		//�������ִ�б�־
		m_functionStatus = false;
		//����ִ�а�ť
		ui.btn_execute->setText("ִ��");
		ui.btn_execute->setEnabled(true);
		//���ùرմ��ڰ�ť
		ui.btn_openPart->setEnabled(true);
		//���ؽ�����
		ui.progressBar->hide();
	}
	else
	{
		//���¿�����ʱ��
		timeoutTimer.start(m_timeCount);
	}
}
/************************************************************************
* �������ƣ�vn_checkBox_bandStatus_stateChange
* ����˵����������״̬ѡ���->״̬�ı�->��
* ����˵����
* ���������
* ���ز�����
*************************************************************************/
void serialFlashProgrammer::vn_checkBox_bandStatus_stateChange(int state)
{
	//ѡ��
	if (state == Qt::Checked)
	{
		//������ѡ���ʹ��
		ui.cbox_baud->setEnabled(true);
		m_autoBand = true;
	}
	//δѡ��
	else if(state == Qt::Unchecked)
	{
		ui.cbox_baud->setCurrentIndex(ui.cbox_baud->findText("115200"));
		//������ѡ���ʧ��
		ui.cbox_baud->setEnabled(false);
		m_autoBand = false;
	}
}
/************************************************************************
* �������ƣ�vn_btn_cleanMessage_clicked
* ����˵���������Ϣ��ť->����->��
* ����˵����
* ���������
* ���ز�����
*************************************************************************/
void serialFlashProgrammer::vn_btn_cleanMessage_clicked()
{

}
/************************************************************************
* �������ƣ�vn_btn_execute_clicked()
* ����˵����ִ�а�ť->����->��
* ����˵����
* ���������
* ���ز�����
*************************************************************************/
void serialFlashProgrammer::vn_btn_execute_clicked()
{
	//��鰴ť״̬
	if (ui.btn_execute->text() == "ִ��")
	{
		//��鴮���Ƿ��
		if (m_comStatus == false)
		{
			outputMessage(QString("����δ�򿪣�����δִ��"), MSG_ERROR | MSG_WITH_TIME);
			return;
		}
		//����豸
		if (ui.cbox_device->currentText() == "F2837xD")
		{
			cpu1 = true;
			g_bf2837xD = true;
		}
		
		//��������߳��Ƿ�������
		if (workerThread.isRunning() == false)
		{
			outputMessage(QString("�����߳�δ���У����������"), MSG_ERROR | MSG_WITH_TIME);
			return;
		}
		//��ȡ������
		int command = ui.cbox_function->currentData().toInt();
		//�����ļ���Ϣ
		if (command == DFU_CPU1)
		{
			g_pszAppFile = m_appName;
		}
		if (command == DFU_CPU2)
		{
			g_pszAppFile2 = m_appName1;
		}
		//���ó�����ڵ�ַ
		if (command == RUN_CPU1)
		{
			//��ȡ�ı�
			//QString str = ui.enterAddrLineEdit->text();
			QString str = APP_ERTER_ADDRESS;
			//��16����ת����32λ����
			m_branchAddress = str.toUInt(nullptr, 16);
			gu32_branchAddress = m_branchAddress;
		}
		//�����Ӧ�������ź�
		emit sendFunctionNumToThread(ui.cbox_function->currentData().toInt());
		//ִ�а�ť�޸�Ϊǿ�ƹرհ�ť
		ui.btn_execute->setText("ǿ�ƹر�");
		//���ùرմ��ڰ�ť
		ui.btn_openPart->setEnabled(false);
		//��ʾ������
		ui.progressBar->show();
		//������ʱ��ʱ��
		outputMessage(QString("��ʱ��ʱ������"));
		timeoutTimer.start(m_timeCount);
		//���빦��ִ��״̬
		m_functionStatus = true;
	}
	else if (ui.btn_execute->text() == "ǿ�ƹر�")
	{
		ui.btn_execute->setEnabled(false);
		outputMessage(QString("���ڳ��Թر��߳�"));
		workerThread.terminate();
		outputMessage(QString("�ȴ��̹߳ر�"));
		workerThread.wait();
		outputMessage(QString("������"));
		workerThread.start();
		//�������ִ�б�־
		m_functionStatus = false;
		//����ִ�а�ť
		ui.btn_execute->setText("ִ��");
		ui.btn_execute->setEnabled(true);
		//���ùرմ��ڰ�ť
		ui.btn_openPart->setEnabled(true);
		//���ؽ�����
		ui.progressBar->hide();
        //�رճ�ʱ��ʱ��
		timeoutTimer.stop();
	}
}
/************************************************************************
* �������ƣ�
* ����˵�����ļ�ת������
* ����˵�����Ѵ����ļ������ļ�ת����UTF8��ʽ�ļ�(�ļ���Ĭ��Ϊprogram.txt)
*           hex2000 -boot -a -sci8 <.out�ļ���> -o <����ļ���>
* ���������
*           QString   path          �ļ���(����·��)
*           QString   programName   ����ļ���(����׺)
* ���ز�����
*************************************************************************/
void serialFlashProgrammer::programToSci8(QString path,QString programName)
{
	//��������
	QString program = "hex2000.exe";
	//��������
	QStringList arguments;
	arguments << "-boot" << "-a" << "-sci8" << path << "-o" << programName;
	//���г���
	hex2000.start(program, arguments);
	//�ȴ����
	hex2000.waitForFinished();
	//�����Ϣ
	outputMessage(QString("�ļ�ת���ɹ�"));
}
/************************************************************************
* �������ƣ�
* ����˵����ת������У�鼰��Ϣ���
* ����˵����
* ���������
*           QString       name      ת����ɵ��ļ�����
*           appFileInfo*  fileInfo  ת��ǰ�ļ���Ϣָ��
* ���ز�����
*************************************************************************/
void serialFlashProgrammer::appFileMessage(QString name,appFileInfo* info)
{
	/*��ȡת���ļ���Ϣ(�����ļ��ṹ)*/
	//���ַ����С
	unsigned int blackNumber = 0;
	unsigned int allBlackSize = 0;
	unsigned int blackSize = 0;
	unsigned int blackAddr = 0;
    //���ļ�
	QFile programFile(name);
	if (!programFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		std::cout << programFile.errorString().toStdString() << endl;
	}
	//�����ı���
	QTextStream in(&programFile);
	/*��ȡ��ڵ�ַ*/
	in.seek(57);//��ڵ�ַƫ����
	unsigned int array[4];
	QString str;
	bool ok;
	int i;
	for ( i = 0; i < 4; i++)
	{
		in >> str;
		array[i] = str.toUInt(&ok, 16);
	}
	array[0] = array[0] << 16;
	array[1] = array[1] << 24;
	array[3] = array[3] << 8;
	unsigned int addr = array[0] + array[1] + array[2] + array[3];
	qDebug() << hex << showbase << addr << endl;

	//��ȡ���п���Ϣ
	while (blackNumber < MAX_BLACK_NUMBER)
	{
		//��ȡ���С��Ϣ
		for (i = 0; i < 2; i++)
		{
			in >> str;
			array[i] = str.toUInt(&ok, 16);
		}
		array[1] = array[1] << 8;
		blackSize = array[0] + array[1];
		//������СΪ0��ֱ���˳�
		if (blackSize == 0) { break; }
		allBlackSize += blackSize;
		//��ȡ���ַ
		for (i = 0; i < 4; i++)
		{
			in >> str;
			array[i] = str.toUInt(&ok, 16);
		}
		array[0] = array[0] << 16;
		array[1] = array[1] << 24;
		array[3] = array[3] << 8;
		blackAddr = array[0] + array[1] + array[2] + array[3];
		//���ݵ�ַ��booltloader2����γ�ͻ
		if (blackAddr < 0x84000 && blackAddr > 0x80000)
		{
			outputMessage("Ӧ�ó���洢�����bootloader�洢����ͻ���޷���������", MSG_ERROR | MSG_WITH_TIME);
		}
		//���ݿ���RAM��
		if (blackAddr < 0x80000)
		{
			outputMessage("Ӧ�ó���������RAM����RAM���ֲ��ᱻ���أ�����Ӧ�ó���", MSG_ERROR | MSG_WITH_TIME);
		}
		//����Ϣ��λ��
		qDebug() << in.pos();
		//ƫ�Ƶ���һ����
		blackNumber++;
		//TODO:�˴��޷�ͨ������λ��ƫ�Ƽ��ٶ�ȡ��Ϣ����Ϊλ��ƫ�����޷����㡣
		for (i = 0; i < (blackSize * 2); i++)
		{
			in >> str;
		}
		//��Ϣ���
		qDebug() << "black addr:" << hex << showbase << blackAddr << endl;
		qDebug() << "blackSize:" << blackSize << endl;
	} 
	//�ж��Ƿ񵽴���������
	if (blackNumber > MAX_BLACK_NUMBER)
	{
		outputMessage(QString("������Ϣ�п����ݳ����ɶ����ֵ����Ϣ��ȡʧ��"), MSG_ERROR|MSG_WITH_TIME);
	}
	//��������
	info->appEnterAddress = addr;
	info->allBlockNumber = blackNumber;
	info->allBlockSize = allBlackSize;
	info->estimatedTime = (float)allBlackSize * 100 / 115200;
	//�����ʾ��Ϣ
	outputMessage(QString("******************������Ϣ*****************"),MSG_TEXT);
	outputMessage(QString("����·����") + info->appFileDir, MSG_TEXT);
	outputMessage(QString("�������ƣ�") + info->appFileName, MSG_TEXT);
	outputMessage(QString("_c_int00:0x%1").arg(addr, 4, 16, QLatin1Char('0')), MSG_TEXT);
	outputMessage(QString("������:%1").arg(blackNumber), MSG_TEXT);
	outputMessage(QString("�����ܴ�С:%1").arg(allBlackSize), MSG_TEXT);
	outputMessage(QString("Ԥ��������ʱ:%1s").arg(info->estimatedTime,3), MSG_TEXT);
	outputMessage(QString("*******************************************"), MSG_TEXT);
	programFile.close();
}
/************************************************************************
* �������ƣ�
* ����˵����ɨ��COM��
* ����˵����
* ���������
* ���ز�����
*************************************************************************/
void serialFlashProgrammer::scanComPort()
{
	outputMessage(QString("ɨ��˿��豸"),MSG_CMD | MSG_WITH_TIME);
	ui.cbox_port->clear();
	qint16 count = 0;
	//�����˿���Ϣ
	foreach(const QSerialPortInfo & info, QSerialPortInfo::availablePorts())
	{
		count++;
		outputMessage(QString("�豸%1:").arg(count), MSG_TEXT);
		outputMessage(QString("�˿�:") + info.portName(), MSG_TEXT);
		outputMessage(QString("�豸����:") + info.description(), MSG_TEXT);
		ui.cbox_port->addItem(info.portName());
	}
	//δɨ�赽�˿�
	if (count == 0)
	{
		outputMessage(QString("δɨ�赽�κδ����豸"),MSG_WARNING | MSG_WITH_TIME);
	}
}
/************************************************************************
* �������ƣ�
* ����˵������Ϣ���
* ����˵����
* ���������
*           qint16    type    ��Ϣ����
*           QString   str     �����Ϣ
* ���ز�����
*************************************************************************/
void serialFlashProgrammer::outputMessage(QString str,qint16 type)
{
	QDateTime current_date_time = QDateTime::currentDateTime();
	QString current_date = current_date_time.toString("yyyy.MM.dd hh:mm:ss") + "    ";
	QString color_end = "</font>";
	QString color_start;
	bool showTime = false;

	if ((type & MSG_WITH_TIME) > 0)
	{
		showTime = true;
		type = type & (~MSG_WITH_TIME);
	}
	else
	{
		showTime = false;
		current_date.clear();
	}
	switch (type)
	{
	    case MSG_TEXT:
			color_start = "<font color=\"#000000\">";break;
	    case MSG_CMD:
			color_start = "<font color=\"#000000\">";str = ">>" + str; break;
		case MSG_WARNING:
			color_start = "<font color=\"#0000FF\">";str = "WARNING:" + str; break;

		case MSG_ERROR:
			color_start = "<font color=\"#FF0000\">";str = "ERROR:" + str; break;
	}
	std::cout << current_date.toStdString() << str.toStdString() << std::endl;
	ui.textBrowser->append(current_date + color_start + str + color_end);
}
/************************************************************************
* �������ƣ�
* ����˵���������߳������źŲۺ���
* ����˵����
* ���������
* ���ز�����
*************************************************************************/
void serialFlashProgrammer::workThreadStart()
{
	outputMessage(QString("�����߳������ɹ�"));
}
/************************************************************************
* �������ƣ�
* ����˵���������߳�����źŲۺ���
* ����˵����
* ���������
* ���ز�����
*************************************************************************/
void serialFlashProgrammer::workThreadFinish()
{
	outputMessage(QString("�����߳��˳��ɹ�"));
}
/************************************************************************
* �������ƣ�
* ����˵���������̹߳�������źŲۺ���
* ����˵����
* ���������
* ���ز�����
*************************************************************************/
void serialFlashProgrammer::workFinish(int num)
{
    //�����Ϣ
	outputMessage(QString("�������"));
	//�������ִ�б�־
	m_functionStatus = false;
	//�رճ�ʱ��ʱ��
	timeoutTimer.stop();
	//����ִ�а�ť
	ui.btn_execute->setText("ִ��");
	ui.btn_execute->setEnabled(true);
	//���ùرմ��ڰ�ť
	ui.btn_openPart->setEnabled(true);
	//���ؽ�����
	ui.progressBar->hide();
}
/************************************************************************
* �������ƣ�
* ����˵����������Ϣ�źŲۺ���
* ����˵����
* ���������
* ���ز�����
*************************************************************************/
void serialFlashProgrammer::workMessage(QString str)
{
	outputMessage(str);
}
/************************************************************************
* �������ƣ�
* ����˵���������źŲۺ���
* ����˵����
* ���������
* ���ز�����
*************************************************************************/
void serialFlashProgrammer::workSchedule(int num)
{
	//���ý�����
	ui.progressBar->setValue(num);
	m_workProgress = num;
	std::cout << "ִ�н���:" << num << std::endl;
}
/************************************************************************
* �������ƣ�
* ����˵���������˳��¼�
* ����˵����
* ���������
* ���ز�����
*************************************************************************/
void serialFlashProgrammer::closeEvent(QCloseEvent* ev)
{
    //��鹤���߳��Ƿ�������
    if (workerThread.isRunning() == true)
    {
		//����Ƿ���ִ�й���
		if (m_functionStatus == true)
		{
			QMessageBox::StandardButton rtn;
			QMessageBox::StandardButtons buttons = QMessageBox::Ignore | QMessageBox::Abort;
			QMessageBox::StandardButton defbutton = QMessageBox::NoButton;
			QString txtLable = "����ִ�й��ܣ��رճ�����ܻᵼ������ʧ�ܣ�ǿ�ƹرհ�Ignore����,��ֹ��Abort����";
			rtn = QMessageBox::warning(this, "����", txtLable, buttons, defbutton);//����
			//���ԣ��˳�
			if (rtn == QMessageBox::Ignore)
			{
				//��Ϊǿ��ʹ�߳��˳�
				workerThread.terminate();
				//workerThread.exit();
				workerThread.wait();
				ev->accept();
			}
			//��ֹ�˳�
			else
			{
				ev->ignore();
			}
		}
		else
		{
			//δ��ִ�й��ܣ����߳������У������߳������˳�
			workerThread.exit();
			workerThread.wait();
		}      
    }
    else
    {
        ev->accept();
    }
}

