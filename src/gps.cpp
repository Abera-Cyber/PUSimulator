#include <string>
#include "gps.h"
#include "config.h"


CGPSChannel::CGPSChannel()
{
    memset(&m_position, 0x00, sizeof(m_position));
    m_position.bAntennaState = 1;
    m_position.bOrientationState = 1;
    m_position.iStarCount = 3;
    m_position.iSatelliteSignal = BVCU_PUCFG_SATELLITE_GPS | BVCU_PUCFG_SATELLITE_BDS;

    // �������ļ��м������á�
    PUConfig puconfig;
    LoadConfig(&puconfig);
    if (0 >= puconfig.interval || puconfig.interval <= 10*60)
        puconfig.interval = 5;
    m_lat = puconfig.lat;
    m_lng = puconfig.lng;
    SetName(puconfig.gpsName);
    m_lasttime = time(NULL) - puconfig.interval;
    m_position.iLatitude = m_lat + (rand() % 999999) * sin((float)rand()) + 100000;
    m_position.iLongitude = m_lng + (rand() % 999999) * sin((float)rand()) + 100000;
    m_chagedu = abs(m_lng - m_position.iLongitude) + abs(m_lat - m_position.iLatitude);

    // GPS ����
    memset(&m_param, 0x00, sizeof(m_param));
    m_param.bEnable = 1;
    strncpy_s(m_param.szName, sizeof(m_param.szName), GetName(), _TRUNCATE);
    m_param.iSupportSatelliteSignal = BVCU_PUCFG_SATELLITE_GPS | BVCU_PUCFG_SATELLITE_BDS;
    m_param.iSetupSatelliteSignal = BVCU_PUCFG_SATELLITE_GPS | BVCU_PUCFG_SATELLITE_BDS;
    m_param.iReportInterval = puconfig.interval;
    m_param.iReportDistance = 1;
}

bool CGPSChannel::ReadGPSData()
{   // ģ����豸�ж�ȡGPSλ������
    time_t now = time(NULL);  // ʱ��Ӧ��Ҳ�Ǵ�GPS�豸�ж�ȡ
    tm* ptm = (tm*)gmtime(&now);  // ������˵����������ʱ�䶼��UTCʱ�䡣
    m_position.stTime.iYear = ptm->tm_year + 1900;
    m_position.stTime.iMonth = ptm->tm_mon + 1;
    m_position.stTime.iDay = ptm->tm_mday;
    m_position.stTime.iHour = ptm->tm_hour;
    m_position.stTime.iMinute = ptm->tm_min;
    m_position.stTime.iSecond = ptm->tm_sec;
    if (m_position.iLongitude > m_lng)
    {
        if (m_position.iLatitude > m_lat)
        {
            m_position.iLongitude += 20000;
            if (m_position.iLongitude > m_lng + m_chagedu)
            {
                m_position.iLongitude = m_lng + m_chagedu;
                m_position.iLatitude = m_lat;
                m_position.iAngle = 45000;
            }
            else
            {
                m_position.iLatitude -= 20000;
                m_position.iAngle = 135000;
            }
        }
        else
        {
            m_position.iLongitude -= 20000;
            m_position.iLatitude -= 20000;
            m_position.iAngle = 225000;
        }
    }
    else if (m_position.iLongitude == m_lng)
    {
        if (m_position.iLatitude > m_lat)
        {
            m_position.iLongitude += 20000;
            m_position.iLatitude -= 20000;
            m_position.iAngle = 135000;
        }
        else
        {
            m_position.iLongitude -= 20000;
            m_position.iLatitude += 20000;
            m_position.iAngle = 315000;
        }
    }
    else
    {
        if (m_position.iLatitude >= m_lat)
        {
            m_position.iLongitude += 20000;
            m_position.iLatitude += 20000;
            m_position.iAngle = 45000;
        }
        else
        {
            m_position.iLongitude -= 20000;
            if (m_position.iLongitude < m_lng - m_chagedu)
            {
                m_position.iLongitude = m_lng - m_chagedu;
                m_position.iLatitude = m_lat;
                m_position.iAngle = 225000;
            }
            m_position.iLatitude += 20000;
            m_position.iAngle = 315000;
        }
    }
    return true;
}
void CGPSChannel::UpdateData()
{
    if (!BOpen())
        return;
    // ========================  ��ʱ���豸�л�ȡ����λ�ã����ϱ��� ������ģ��λ��
    time_t now = time(NULL);
    int dely = now - m_lasttime;
    if (dely >= m_param.iReportInterval)
    {
        m_lasttime = now;
        ReadGPSData();
        WriteData(&m_position);
        printf("send GPS Data, lat: %d  lng: %d\n", m_position.iLatitude, m_position.iLongitude);
    }
}

BVCU_Result CGPSChannel::OnSetName(const char* name)
{
    PUConfig puconfig;
    LoadConfig(&puconfig);
    strncpy_s(puconfig.gpsName, sizeof(puconfig.gpsName), name, _TRUNCATE);
    SetConfig(&puconfig);
    SetName(name);
    strncpy_s(m_param.szName, sizeof(m_param.szName), GetName(), _TRUNCATE);
    return BVCU_RESULT_S_OK;
}
BVCU_Result CGPSChannel::OnOpenRequest()
{
    // ���������GPS�豸�����������õ��ϱ�����ϱ�λ��
    printf("================  recv open gps request \n");
    return BVCU_RESULT_S_OK;
}
void CGPSChannel::OnOpen()
{
    // ͨ���Ѿ������ɹ������Կ�ʼ�ϱ������ˡ�
    printf("================  open gps success \n");
}
void CGPSChannel::OnClose()
{
    // ����Ӧ�ÿ��Թر�����GPS�豸�ˡ�
    printf("================  gps closed \n");
    return ;
}
void CGPSChannel::OnPLI()
{
    // GPS�յ�PLI��������Ϊ���³�Ա��ͨ������û���յ�����(��Ϊ�豸ֻ�ϴ�һ·���������Ĵ����󱻷�����������)��
    // ��ʱӦ�����������µĶ�λ��Ϣ���ͳ�ȥ
    printf("================  gps pli \n");
    UpdateData();
}
const BVCU_PUCFG_GPSData* CGPSChannel::OnGetGPSData()
{   // ���豸�ж�ȡGPSλ�ò�����
    ReadGPSData();
    return &m_position;
}
const BVCU_PUCFG_GPSParam* CGPSChannel::OnGetGPSParam()
{   // ��ѯGPS����
    return &m_param;
}
BVCU_Result CGPSChannel::OnSetGPSParam(const BVCU_PUCFG_GPSParam* pParam)
{
    // �����ϱ���� �Ȳ���
    m_param.iReportInterval = pParam->iReportInterval;
    if (0 >= m_param.iReportInterval || m_param.iReportInterval <= 10 * 60)
        m_param.iReportInterval = 5;
    PUConfig puconfig;
    LoadConfig(&puconfig);
    puconfig.interval = pParam->iReportInterval;
    SetConfig(&puconfig);
    return BVCU_RESULT_S_OK;
}
