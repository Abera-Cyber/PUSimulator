#pragma once
#include "BVCSP.h"

#define MAX_FILE_TRANSFER_COUNT 64
#define MODULE_FILE_TRANSFER_TIMEOUT (2*60*1000)

typedef  void* BVCU_File_HTransfer;
class CFileTransfer;
typedef struct _BVCU_File_TransferParam BVCU_File_TransferParam;
class CFileTransManager
{
public:
    // �ϲ�ʵ���ļ���/�ر�/��д ����ؽӿ�
    virtual FILE* bv_fsopen(const char* pathName, const char* mode, int bWrite) = 0;
    virtual FILE* bv_fclose(FILE* _File) = 0;
    virtual int bv_fread(void* _DstBuf, int _ElementSize, int _Count, FILE* _File) = 0;
    virtual int bv_fwrite(const void* _Str, int _Size, int _Count, FILE* _File) = 0;
    virtual int bv_fseek(FILE* _File, long _Offset, int _Origin) = 0;
    virtual long bv_ftell(FILE* _File) = 0;

    /* �յ��ļ���������ص�
    hTransfer���ļ�����ľ����
    pParam: ÿ���¼���Ӧ�Ĳ������������Ͳο������¼����˵�������pParam��NULL����ʾ�޲�����
    ���أ�BVCU_RESULT_S_OK��ʾӦ�ó�����ܸô�����������ֵ��ʾ�ܾ���ͬ������ʱע����дpParam����Ҫ��д��ֵ��
    */
    virtual BVCU_Result OnFileRequest(BVCU_File_HTransfer hTransfer, BVCU_File_TransferParam* pParam) = 0;

    void SetBandwidthLimit(int iBandwidthLimit) { m_iBandwidthLimit = iBandwidthLimit; } // �������ơ���λkbps��0��ʾ�����ƣ����������ļ�����Ĵ���
public:
    CFileTransManager();
    ~CFileTransManager();
    CFileTransfer** GetFileTransferList() { return m_fileList; }
    int IsFileTransferInList(CFileTransfer* pFileTransfer);
    CFileTransfer* AddFileTransfer();
    int RemoveFileTransfer(CFileTransfer* pFileTransfer);
    CFileTransfer* FindFileTransfer(BVCSP_HDialog hdialog);
    int GetFileTransferCount();

    int HandleEvent();

    int m_iSendDataCount; // ȫ�����١�1024pms ; (800*m_iSendDataCount) Bps
public:
    // BVCSP Event
    BVCU_Result OnDialogCmd_BVCSP(BVCSP_HDialog hDialog, int iEventCode, BVCSP_DialogParam* pParam);
    static void OnDialogEvent_BVCSP(BVCSP_HDialog hDialog, int iEventCode, BVCSP_Event_DialogCmd* pParam);
    static BVCU_Result OnAfterRecv_BVCSP(BVCSP_HDialog hDialog, BVCSP_Packet* pPacket);
private:
    CFileTransfer* m_fileList[MAX_FILE_TRANSFER_COUNT];
    int m_iBandwidthLimit;// �������ơ���λkbps��0��ʾ�����ƣ����������ļ�����Ĵ���
};

enum BVFile_Dialog_Status
{
    BVFILE_DIALOG_STATUS_NONE = 0, // û��״̬
    BVFILE_DIALOG_STATUS_INVITING, // �ڽ���������
    BVFILE_DIALOG_STATUS_TRANSFER, // �ڴ�����
    BVFILE_DIALOG_STATUS_SUCCEEDED,// ����ɹ�
    BVFILE_DIALOG_STATUS_FAILED,   // ����ʧ��
};
// �ļ�����������á��յ���������ͬ��ʱ��szLocalFileName��OnEvent��Ҫ��д��
typedef struct _BVCU_File_TransferParam {
    int iSize; // ���ṹ��Ĵ�С��������Ӧ��ʼ��Ϊsizeof(BVCU_File_TransferParam)
    void* pUserData;          // �û��Զ������ݡ�BVCU_File_GlobalParam.OnFileRequest�п�����д��
    char  szTargetID[BVCU_MAX_ID_LEN + 1]; // �ļ�����Ŀ�����ID��PU/NRU��"NRU_"ʱ���ڲ����ڻص�ǰ��д����NRU ID��
    char* pRemoteFilePathName;// Զ��·��+�ļ�����
    char* pLocalFilePathName; // ����·��+�ļ�����BVCU_File_GlobalParam.OnFileRequest��Ҫ��д��
    unsigned int iFileStartOffset;     // �ļ���ʼ����ƫ�ơ�0�����´��䡣-1(0xffffffff)�������Զ�����������
    int iTimeOut;             // ���ӳ�ʱ�䡣��λ ���롣
    int bUpload;              // 0-���أ�1-�ϴ���

    /* �ļ��������¼��ص������� BVCU_File_GlobalParam.OnFileRequest��Ҫ��д��
    hTransfer:��������
    pUserData�����ṹ���е�pUserData��
    iEventCode: �¼��룬�μ�BVCU_EVENT_DIALOG_*��Open/Close��
    iResult: �¼���Ӧ�Ľ���롣
    */
    void(*OnEvent)(BVCU_File_HTransfer hTransfer, void* pUserData, int iEventCode, BVCU_Result iResult);

}BVCU_File_TransferParam;

// �ļ�������Ϣ
typedef struct _BVCU_File_TransferInfo {
    // �������
    BVCU_File_TransferParam stParam;

    // Transfer��ʼʱ�̣���1970-01-01 00:00:00 +0000 (UTC)��ʼ��΢����
    long long iCreateTime;

    // Transfer����ʱ�䣬��λ΢��
    long long iOnlineTime;

    // �Ѿ�������ֽ���������BVCU_File_TransferParam.iFileStartOffset����iTotalBytesһ��������ǰ������ȡ�
    unsigned int iTransferBytes;
    // ���ֽ���
    unsigned int iTotalBytes;

    int iSpeedKBpsLongTerm;// ��ʱ�䴫�����ʣ���λ KBytes/second
    int iSpeedKBpsShortTerm;// ��ʱ�䴫�����ʣ���λ KBytes/second

}BVCU_File_TransferInfo;

class CFileTransfer
{
protected:
    BVCU_File_TransferInfo m_fileInfo;

    BVCSP_HDialog m_cspDialog;
    BVFile_Dialog_Status m_iStatus;
    FILE* m_fFile;
    unsigned int m_iFileSize;
    int   m_iLastDataTime; // ���ɹ�����/��������ʱ�䡣�������޷�����/�������ݣ��ر�ͨ����
    int   m_iCompleTime; // ��ɴ���ʱ�䣬�����ж�ͨ����ʱ����������������������ʡ�
    char  m_localFilePathName[BVCU_MAX_FILE_NAME_LEN + 1];
    unsigned long m_iHandle;
    CFileTransManager* m_pSession;
    unsigned short m_bClosing;
    unsigned short m_bCallOpen;
public:
    CFileTransfer();
    ~CFileTransfer() { Init(NULL); }
    void Init(CFileTransManager* pSession);
    CFileTransManager* GetSession() { return m_pSession; }
    int bClosing() { return m_bClosing; }
    int SetInfo(BVCU_File_TransferParam* pParam); // ����
    int SetInfo_RecvReq(BVCSP_DialogParam* pParam); // ����
    BVCU_File_TransferInfo* GetInfo() { return &m_fileInfo; }
    BVCU_File_TransferParam* GetParam() { return &m_fileInfo.stParam; }
    BVCU_File_TransferInfo* GetInfoNow(int bNetworkThread);
    int  GetFileSize() { return m_iFileSize; }

    void SetHandle(unsigned long iHandle) { m_iHandle = iHandle; }
    unsigned long GetHandle() { return m_iHandle; }

    void SetCSPDialog(BVCSP_HDialog hDialog) { m_cspDialog = hDialog; }
    BVCSP_HDialog GetCSPDialog() { return m_cspDialog; }

    int HandleEvent(int iTickCount); // true:keep��false:destroy

    BVCU_Result OnRecvFrame(BVCSP_Packet* pPacket);
    void OnEvent(int iEvent, BVCU_Result iResult, int bOnbvcspEvent = 0);

    BVCU_Result UpdateLocalFile(BVCSP_DialogParam* pParam);  // ����
    int BuildFileData();
};

