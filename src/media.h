#pragma once

#include <time.h>
#include "base/dialog.h"

// 音视频 通道
class CMediaChannel : public CAVChannelBase
{
public:
    // ======= 需要实现的功能接口
    virtual BVCU_Result OnSetName(const char* name); // 收到配置通道名称请求
    virtual BVCU_Result OnOpenRequest();   // 收到打开请求，回复是否同意，0：同意
    virtual void OnOpen();   // 建立通道连接成功通知
    virtual void OnClose();  // 通道连接关闭通知
    virtual void OnPLI();    // 收到生成关键帧请求
    virtual void OnRecvAudio(long long iPTS, const void* pkt, int len);   // 收到平台发来的音频数据。编码信息同ReplySDP()。
    virtual BVCU_Result OnPTZCtrl(const BVCU_PUCFG_PTZControl* ptzCtrl);   // 收到平台发来的云台控制命令。
    virtual BVCU_PUCFG_PTZAttr* OnGetPTZParam() { return &m_ptzAttr; } // 收到平台发来的云台查询命令。

protected:
    time_t m_replytime;   // 回复请求时间。秒。time(); // 用于控制回复延迟（模拟设备回的慢）
    int m_interval;   // 上报数据时间间隔，毫秒。// 用于模拟收到音视频输入数据
    int m_lasttime;   // 上次上报时间。毫秒。GetTickCount();
    int m_lastAdjtime;   // 上次调整码率时间。毫秒。GetTickCount();
    int m_lastPlitime;   // 上次产生关键帧时间。毫秒。GetTickCount();
    long long m_pts;  // 上次时间戳。
    int m_audioPackLen; // 每个音频帧数据大小
    FILE* m_audioFile;// 音频输入文件
    FILE* m_videoFile;// 音频输入文件
    SAVCodec_ID m_videoCodec;

    BVCU_PUCFG_PTZAttr  m_ptzAttr;// 云台测试

    char* ReadVideo(char* buf, int* len); // 从文件中读取h264数据，模拟收到编码器数据。
    char* ReadAudio(char* buf, int* len); // 从文件中读取g726数据，模拟收到编码器数据。
    void Reply();     // 回复请求，用写死的SDP信息，所以不轻易修改自带的音视频文件，除非您知道要修改什么。
public:
    CMediaChannel();
    virtual ~CMediaChannel() {}
    void SendData();  // 模拟收到音视频编码后数据，发送给平台。
};
