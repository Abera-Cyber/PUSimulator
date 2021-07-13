#include <string>
#include "media.h"
#include "config.h"

#ifdef _MSC_VER
#include <Windows.h>
#endif

CMediaChannel::CMediaChannel()
    : CAVChannelBase(true, true, true, true)
{
    m_interval = 40;
    PUConfig puconfig;
    LoadConfig(&puconfig);
    SetName(puconfig.mediaName);
    m_lasttime = 0;
    m_lastAdjtime = 0;
    m_lastPlitime = 0;
    m_pts = 0;
    m_audioFile = 0;
    m_videoFile = 0;
    m_audioPackLen = 320;
}

static char* findhead(char* data, int len) {
    if (len < 4)
        return 0;
    for (int i = 0; i <= len - 3; i++) {
        if (data[i] == 0 && data[i + 1] == 0 && data[i + 2] == 1)
        {
            if (i != 0 && data[i - 1] == 0)
                return &data[i - 1];
            return &data[i];
        }
    }
    return 0;
}
char* CMediaChannel::ReadVideo(char* buf, int* len)
{
    int buflen = *len - 160;
    *len = 0;
    if (m_videoFile == 0)
        return 0;
    char* pRead = buf;
    char* pStart = 0;
    char* pEnd = 0;
    int readlen = 0;
    while (readlen < buflen)
    {
        int onelen = fread(pRead, 1, 160, m_videoFile);
        readlen += onelen;
        if (onelen < 160)
        { // ���ܶ����ļ���β�ˣ���ͷ����
            fseek(m_videoFile, 0, SEEK_SET);
            if (pStart != 0)
                pEnd = pRead + onelen;
            break;
        }
        if (pRead != buf)
        {  // ��ֹhandͷ���жϣ������Ҳ�����
            pRead -= 3;
            onelen += 3;
        }
        // ��00 00 00 01 ͷ
        if (pStart == 0)
        {
            pStart = findhead(pRead, onelen);
            if (pStart != 0)
            {  // �����ƣ�׼����end
                onelen = onelen - (pStart - pRead) - 3;
                pRead = pStart + 3;
            }
        }
        if ( pStart != 0 && pEnd == 0 && readlen > 160) // �Ѿ��ҵ���ʼ��û���ҵ���β��һ��֡����С��160����ֹsps\pps�������͡�
        {
            pEnd = findhead(pRead, onelen);
            if (pEnd != 0)
            {  // ����ˣ�
                int moreLen = onelen - (pEnd - pRead);
                fseek(m_videoFile, -moreLen, SEEK_CUR);
                break;
            }
        }
        pRead += onelen;
    }
    readlen = 0;
    if (pEnd != 0)
        readlen = pEnd - pStart;
    if (readlen > 0)
        *len = readlen;
    return pStart;
}
char* CMediaChannel::ReadAudio(char* buf, int* len)
{
    int buflen = *len;
    *len = 0;
    if (buflen < m_audioPackLen || m_audioFile == 0)
        return 0;
    int readlen = fread(buf, 1, m_audioPackLen, m_audioFile);
    if (readlen < m_audioPackLen)
    { // ���ܶ����ļ���β�ˣ���ͷ����
        fseek(m_audioFile, 0, SEEK_SET);
        readlen = fread(buf, 1, m_audioPackLen, m_audioFile);
    }
    if (readlen == m_audioPackLen)
        *len = m_audioPackLen;
    return buf;
}
void CMediaChannel::Reply()
{
    static char videoEx[64] = { 0x00, 0x00, 0x00, 0x01, 0x67, 0x42, 0x80, 0x1E, 0xDA, 0x05, 0x02, 0x11, 0x00, 0x00, 0x00, 0x01, 0x68, 0xCE, 0x3C, 0x80 };
    static int  videoExLen = 20;
    if (m_bOpening)
    {
        BVCSP_VideoCodec videoSdp;
        BVCSP_AudioCodec audioSdp;
        memset(&videoSdp, 0x00, sizeof(videoSdp));
        memset(&audioSdp, 0x00, sizeof(audioSdp));
        videoSdp.codec = SAVCODEC_ID_H264;
        videoSdp.pExtraData = videoEx;
        videoSdp.iExtraDataSize = videoExLen;
        if (m_audioPackLen == 160)
            audioSdp.codec = SAVCODEC_ID_G726;
        else
            audioSdp.codec = SAVCODEC_ID_G711A;
        audioSdp.iBitrate = 32000;
        audioSdp.iChannelCount = 1;
        audioSdp.iSampleRate = 8000;
        audioSdp.eSampleFormat = SAV_SAMPLE_FMT_S16;

        ReplySDP(BVCU_RESULT_S_OK, &videoSdp, &audioSdp);
    }
}
void CMediaChannel::SendData()
{
    Reply();
    if (!BOpen() || m_lasttime == 0)
        return;
    // ========================  ��ʱ���豸�л�ȡ����λ�ã����ϱ��� ������ģ��λ��
    int now = GetTickCount();
    int dely = now - m_lasttime;
    if (dely < 0) {
        dely = m_interval;
        m_lasttime = now - m_interval;
    }
    while (dely >= m_interval)
    {
        m_lasttime += m_interval;
        dely -= m_interval;

        m_pts = m_pts + m_interval * 1000;
        static char g_packetbuf[1 * 1024 * 1024]; // ����������һ��֡�Ĵ�С��Ҫ���� 1M����Ϊ�Դ�������Ƶ�ļ���֡���ݶ���С��
        if (m_audioFile != 0)
        {  // ��Ƶ�����ˣ�������Ƶ���ݡ�
            int len = sizeof(g_packetbuf);
            char* pdata = ReadAudio(g_packetbuf, &len);
            if (len > 0)
                WriteAudio(m_pts, pdata, len);
            //printf("send audio Data, %d\n", len);
        }
        if (m_videoFile != 0)
        {  // ��Ƶ�����ˣ�������Ƶ���ݡ�
            int len = sizeof(g_packetbuf);
            char* pdata = ReadVideo(g_packetbuf, &len);
            if (len > 0)
                WriteVideo(m_pts, pdata, len);
            //printf("send audio Data, %d\n", len);
        }
    }
    // =======================  ��ȡ���緢��ͳ�����ݣ����ݲ²����ʣ���̬�����������ʣ������Ǽٵģ���
    dely = now - m_lastAdjtime;
    if (dely >= 2 * 1000 || m_lastAdjtime == 0)
    {
        m_lastAdjtime = now;
        BVCSP_DialogInfo dlgInfo;
        memset(&dlgInfo, 0x00, sizeof(dlgInfo));
        BVCU_Result result = BVCSP_GetDialogInfo(m_hDialog, &dlgInfo);
        if (BVCU_Result_SUCCEEDED(result))
        {
            printf("================  guess bandwidth = %d kbps\n", dlgInfo.iGuessBandwidthSend);
        }
    }
}

BVCU_Result CMediaChannel::OnSetName(const char* name)
{
    printf("================  media set name. %s \n", name);
    PUConfig puconfig;
    LoadConfig(&puconfig);
    strncpy_s(puconfig.mediaName, sizeof(puconfig.mediaName), name, _TRUNCATE);
    SetConfig(&puconfig);
    SetName(name);
    return BVCU_RESULT_S_OK;
}
BVCU_Result CMediaChannel::OnOpenRequest()
{
    // �������������Ƶ�豸���첽�򿪳ɹ��󣬵���ReplySDP()�ӿڻظ�����
    // ����Ƶͨ�����ܻ����յ���������Ϊ�������޸���Ҫ��ý�����͡�����Ҫע�ⲻҪ�ظ��򿪡�
    printf("================  recv open media request \n");
    if (BNeedVideoIn())
    {   // ������Ƶ������Ƶ�����豸
        printf("================  open video media now\n");
        if (m_videoFile == 0)
            m_videoFile = fopen(VIDEO_FILE_PATH_NAME, "rb");
    }
    else if (m_videoFile)
    {   // ����Ҫ��Ƶ������Ѿ��򿪣���ر�
        fclose(m_videoFile);
        m_videoFile = 0;
    }
    if (BNeedAudioIn())
    {   // ������Ƶ������Ƶ�����豸
        PUConfig puconfig;
        LoadConfig(&puconfig);
        printf("================  open audio media now\n");
        if (m_audioFile == 0)
            m_audioFile = fopen(puconfig.audioFile, "rb");
        if (strstr(puconfig.audioFile, "g726") != nullptr)
            m_audioPackLen = 160;
        else
            m_audioPackLen = 320; // g711a
    }
    else if (m_audioFile)
    {   // ����Ҫ��Ƶ������Ѿ��򿪣���ر�
        fclose(m_audioFile);
        m_audioFile = 0;
    }
    if (BNeedAudioOut())
    {   // ������Ƶ���������Ƶ����豸
        printf("================  open audio out now\n");
    }
    return BVCU_RESULT_S_OK;
}
void CMediaChannel::OnOpen()
{
    // ͨ���Ѿ������ɹ������Կ�ʼ�ϱ������ˡ�
    printf("================  open media success \n");
    m_lasttime = GetTickCount();
    m_pts = time(0) * 1000000;
}
void CMediaChannel::OnClose()
{
    // ����Ӧ�ÿ��Թر���������Ƶ�����豸�ˡ�
    printf("================  media closed \n");
    m_lasttime = 0;
    m_lastAdjtime = 0;
    m_lastPlitime = 0;
    if (m_audioFile)
    {
        fclose(m_audioFile);
        m_audioFile = 0;
    }
    if (m_videoFile)
    {
        fclose(m_videoFile);
        m_videoFile = 0;
    }
}
void CMediaChannel::OnPLI()
{
    // ����Ӧ��֪ͨ������Ƶ���������ɹؼ�֡��
    printf("================  media pli \n");
    int now = GetTickCount();
    int dely = now - m_lastPlitime;
    if (dely >= 2 * 1000 || m_lastPlitime == 0)
    {
        m_lastPlitime = now;
        if (m_videoFile)
        {   // ������ת���ļ���ʼλ�ã�����Ϊ �ļ���ʼλ���ǹؼ�֡��
            fseek(m_videoFile, 0, SEEK_SET);
        }
    }
}
void CMediaChannel::OnRecvAudio(long long iPTS, const void* pkt, int len)
{
    printf("================  media recv audio. len: %d \n", len);
}

BVCU_Result CMediaChannel::OnPTZCtrl(const BVCU_PUCFG_PTZControl* ptzCtrl)
{
    printf("================  media recv ptz control. command:%d %s\n", ptzCtrl->iPTZCommand, ptzCtrl->bStop?"stop":"start");
    return BVCU_RESULT_S_OK;
}
