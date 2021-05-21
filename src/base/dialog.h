#pragma once
#include "BVCSP.h"

class CChannelBase
{
public:
    // ======= ��Ҫʵ�ֵĹ��ܽӿ�
    virtual BVCU_Result OnSetName(const char* name) = 0; // �յ�����ͨ����������
    virtual BVCU_Result OnOpenRequest() = 0;   // �յ������󣬻ظ��Ƿ�ͬ�⣬0��ͬ��
    virtual void OnOpen() = 0;   // ����ͨ�����ӳɹ�֪ͨ
    virtual void OnClose() = 0;  // ͨ�����ӹر�֪ͨ
    virtual void OnPLI() = 0;    // �յ����ɹؼ�֡����

protected:
    int m_index; // Ӳ����������0��ʼ��ţ�������sessionע��ʱ���䣨��ʱÿ��ͨ��ע��˳���ܱ仯����
    int m_channelIndexBase;  // ͨ������ʼֵ��
    int m_supportMediaDir;   // ֧�ֵ�ý�巽��
    char m_name[64];         // ͨ�����ơ�

    BVCSP_HDialog m_hDialog; // BVCSP��������
    int m_openMediaDir;      // ��ǰ��ý�巽��
    bool m_bOpening;         // �Ƿ����ڴ��У��ȴ��ظ���
public:
    CChannelBase(int IndexBase);
    virtual ~CChannelBase();
    void SetIndex(int index) { m_index = index; }  // ������Ӳ����
    void SetName(const char* name);
    bool BOpen() { return m_hDialog != 0; }

    int GetIndex() { return m_index; } // ��ȡ��Ӳ���ţ���ͬӲ����0��ʼ��š�
    int GetChannelIndex() { return m_channelIndexBase + m_index; } // ��ȡͨ���š�BVCU_SUBDEV_INDEXMAJOR_*
    const char* GetName() { return m_name; }
    int GetSupportMediaDir() { return m_supportMediaDir; }

    // ��Ҫ���ã��ײ㽻���ӿ�
    BVCSP_HDialog GetHDialog() { return m_hDialog; }
    int  GetOpenDir() { return m_openMediaDir; }
    void SetHialog(BVCSP_HDialog hDialog, int mediaDir) { m_hDialog = hDialog; m_openMediaDir = mediaDir; }
    void SetBOpening(bool bOpening) { m_bOpening = bOpening; }
    BVCU_Result OnRecvPacket(const BVCSP_Packet* packet);
    typedef void (*bvcsp_OnDialogEvent)(BVCSP_HDialog hDialog, int iEventCode, BVCSP_Event_DialogCmd* pParam);
    static bvcsp_OnDialogEvent g_bvcsp_onevent;
};

// ����Ƶͨ��
class CAVChannelBase : public CChannelBase
{
public:
    // ======= ��Ҫʵ�ֵĹ��ܽӿ�
    virtual void OnRecvAudio(long long iPTS, const void* pkt, int len) = 0;   // �յ�ƽ̨��������Ƶ���ݡ�������ϢͬReplySDP()��
    virtual BVCU_Result OnPTZCtrl(const BVCU_PUCFG_PTZControl* ptzCtrl) = 0;   // �յ�ƽ̨��������̨�������
    //virtual const BVCU_PUCFG_EncoderChannel* OnGetEncoder() = 0;  // �յ�ƽ̨��ѯ������������
    //virtual BVCU_Result OnSetEncoder(const BVCU_PUCFG_EncoderChannel* param) = 0;  // �յ�ƽ̨���ñ�����������
public:
    CAVChannelBase(bool bVideoIn, bool bAudioIn, bool bAudioOut, bool ptz);// �Ƿ�֧�֣���Ƶ�ɼ�����Ƶ�ɼ�����Ƶ���ţ���̨
    virtual ~CAVChannelBase() {}
    // �ظ� ������, ��֧�ֵĿ���Ϊ�ա��յ����������Ҫ����ReplySDP�ظ�����
    BVCU_Result ReplySDP(BVCU_Result result, const BVCSP_VideoCodec* video, const BVCSP_AudioCodec* audio); // �Ƿ�ɹ�����ƵSDP����ƵSDP
    BVCU_Result WriteVideo(long long iPTS, const char* pkt, int len);
    BVCU_Result WriteAudio(long long iPTS, const char* pkt, int len);
    // ��ѯ
    bool BSupportVideoIn() { return (m_supportMediaDir & BVCU_MEDIADIR_VIDEOSEND) != 0; }
    bool BSupportAudioIn() { return (m_supportMediaDir & BVCU_MEDIADIR_AUDIOSEND) != 0; }
    bool BSupportAudioOut() { return (m_supportMediaDir & BVCU_MEDIADIR_AUDIORECV) != 0; }
    bool BSupportPTZ() { return m_bptz; }
    bool BNeedVideoIn() { return (m_openMediaDir & BVCU_MEDIADIR_VIDEOSEND) != 0; }
    bool BNeedAudioIn() { return (m_openMediaDir & BVCU_MEDIADIR_AUDIOSEND) != 0; }
    bool BNeedAudioOut() { return (m_openMediaDir & BVCU_MEDIADIR_AUDIORECV) != 0; }

protected:
    bool m_bptz; // �Ƿ�֧����̨��
};

// GPS ͨ��
class CGPSChannelBase : public CChannelBase
{
public:
    // ======= ��Ҫʵ�ֵĹ��ܽӿ�
    virtual const BVCU_PUCFG_GPSData* OnGetGPSData() = 0;   // �յ���ѯ��λ
    virtual const BVCU_PUCFG_GPSParam* OnGetGPSParam() = 0; // �յ���ѯ����
    virtual BVCU_Result OnSetGPSParam(const BVCU_PUCFG_GPSParam* pParam) = 0; // �յ��޸�����
public:
    CGPSChannelBase();
    virtual ~CGPSChannelBase() {}
    // ���� GPS����
    BVCU_Result WriteData(const BVCU_PUCFG_GPSData* pGPSData);
};

// ���� ͨ��
class CTSPChannelBase : public CChannelBase
{
public:
    // ======= ��Ҫʵ�ֵĹ��ܽӿ�
    virtual void OnRecvData(const void* pkt, int len) = 0;   // �յ�ƽ̨�������ڵ����ݡ�
    virtual const BVCU_PUCFG_SerialPort* OnGetTSPParam() = 0; // �յ���ѯ����
    virtual BVCU_Result OnSetTSPParam(const BVCU_PUCFG_SerialPort* pParam) = 0; // �յ��޸�����
    virtual void OnPLI() {}
public:
    CTSPChannelBase();
    virtual ~CTSPChannelBase() {}
    // ���� �������� ��ƽ̨
    BVCU_Result WriteData(const char* pkt, int len);
};
