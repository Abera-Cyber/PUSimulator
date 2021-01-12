#pragma once
#include "BVCSP.h"

class CChannelBase
{
public:
    // ======= ��Ҫʵ�ֵĹ��ܽӿ�
    virtual BVCU_Result OnSetName(const char* name) = 0; // �յ�����ͨ����������
    virtual void OnOpen() = 0;   // ����ͨ�����ӳɹ�֪ͨ
    virtual void OnClose() = 0;  // ͨ�����ӹر�֪ͨ

protected:
    int m_index; // Ӳ����������0��ʼ��ţ�������sessionע��ʱ���䣨��ʱÿ��ͨ��ע��˳���ܱ仯����
    int m_channelIndexBase;  // ͨ������ʼֵ��
    int m_supportMediaDir;   // ֧�ֵ�ý�巽��
    char m_name[64];         // ͨ�����ơ�

    BVCSP_HDialog m_hDialog; // BVCSP��������
    int m_iOpenMediaDir;     // ��ǰ��ý�巽��
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
    void SetHialog(BVCSP_HDialog hDialog, int mediaDir) { m_hDialog = hDialog; m_iOpenMediaDir = mediaDir; }
};

// ����Ƶͨ��
class CAVChannelBase : public CChannelBase
{
public:
    CAVChannelBase();
    virtual ~CAVChannelBase() {}
};

// GPS ͨ��
class CGPSChannelBase : public CChannelBase
{
public:
    // ======= ��Ҫʵ�ֵĹ��ܽӿ�
    virtual BVCU_Result OnOpenRequest() = 0;   // �յ������󣬻ظ��Ƿ�ͬ�⣬0��ͬ��
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
    CTSPChannelBase() : CChannelBase(BVCU_SUBDEV_INDEXMAJOR_MIN_TSP) {};
    virtual ~CTSPChannelBase() {}
};
