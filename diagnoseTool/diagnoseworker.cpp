#include "diagnoseworker.h"
#include <QEventLoop>
#include <QNetworkInterface>
#include <QNetworkProxy>
#include <QHostInfo>
#include <QDateTime>
#include <QDir>
#include "wxconfig.h"
#include "wintool.h"
#define _BASE_(fmt, args...) SendMessage(QString().sprintf(QString::fromUtf8(fmt).toAscii().constData(), ##args))
#define WARN(fmt, args...) _BASE_("<font color=DeepPink>[警告]</font>　" fmt, ##args)
#define ERR(fmt, args...) _BASE_("<font color=red>[错误]</font>　" fmt, ##args)
#define NORM(fmt, args...) _BASE_("[正常]　" fmt, ##args)
#define INFO(fmt, args...) _BASE_("[信息]　" fmt, ##args)
#define STR2CHAR(str) str.toUtf8().constData()
inline uint GetTimestamp(){
    return QDateTime::currentDateTime().toTime_t();
}
inline int GetTimeCost(uint last){
    return GetTimestamp() - last;
}
inline bool isValidWeixinQrcode(const QString& str){
    static QRegExp exp("1[0-9]\\d{16}");
    return exp.exactMatch(str);
}
DiagnoseWorker::DiagnoseWorker()
{
    m_timer_id = 0;
    m_reply = NULL;
    m_ip_exp.setPattern(("^([01]?\\d\\d?|2[0-4]\\d|25[0-5])\\.([01]?\\d\\d?|2[0-4]\\d|25[0-5])\\."
                         "([01]?\\d\\d?|2[0-4]\\d|25[0-5])\\.([01]?\\d\\d?|2[0-4]\\d|25[0-5])"));
}

DiagnoseWorker::~DiagnoseWorker()
{
    if(m_timer_id != 0)
        killTimer(m_timer_id);
}

void DiagnoseWorker::StartDiagnosis()
{
    DiagshowFileVersion();
    DiagnoseConfig();
    DiagnoseInputDevice();
    DiagnoseMachineInfo();
    DiagnoseProxy();
}

bool DiagnoseWorker::DiagnoseConfig()
{
    INFO("开始检查配置……");
    INFO("当前系统时间:%s", STR2CHAR(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:dd")));

    WxConfig& _config = WxConfig::g();
    if(_config.PROXY_IP.isEmpty()){
        WARN("代理ＩＰ为空");
    } else if(!m_ip_exp.exactMatch(_config.PROXY_IP)){
        ERR("代理ＩＰ：%s 不是有效的ＩＰ地址", STR2CHAR(_config.PROXY_IP));
    } else{
        NORM("代理ＩＰ:%s 格式检查正常", STR2CHAR(_config.PROXY_IP));
    }
    if(_config.PROXY_PORT < 1 || _config.PROXY_PORT > 65535){
        ERR("代理ＩＰ端口:%d 不是有效端口", _config.PROXY_PORT);
    }else{
        NORM("代理ＩＰ端口:%d 格式检查正常", _config.PROXY_PORT);
    }

    if(_config.PROXY_USER.isEmpty() ^ _config.PROXY_PWD.isEmpty()){
        WARN("代理用户名:%s 代理密码有一个为空", STR2CHAR(_config.PROXY_USER));
    }

    return true;
}

bool DiagnoseWorker::DiagnoseMachineInfo()
{
    INFO("开始检查网络配置……");
//    isConnectedToNetwork();
    bool _network_connect = false;
    QList<QNetworkInterface> ifaces(QNetworkInterface::allInterfaces());
    if(ifaces.isEmpty()){
        ERR("没有网卡，或网卡驱动没有安装");
        goto NEXT_STEP;
    }
    for (int i = 0; i < ifaces.count(); ++i){
        const QNetworkInterface& iface = ifaces.at(i);
        if(iface.flags().testFlag(QNetworkInterface::IsLoopBack)){
            continue;
        }

        if(!iface.flags().testFlag(QNetworkInterface::IsUp)){
            if(iface.humanReadableName().length() < 8)
                INFO("网卡:[%s]被禁用", STR2CHAR(iface.humanReadableName()));
            continue;
        }
//        bool _is_running = iface.flags().testFlag(QNetworkInterface::IsRunning);
//        INFO("网卡:%s running state:%s", STR2CHAR(iface.humanReadableName()), _is_running?"RUNNING":"NO-RUNNING");
        QList<QNetworkAddressEntry> _entrylist(iface.addressEntries());
        for (int j=0; j<_entrylist.count(); j++){
            QNetworkAddressEntry& _entry(_entrylist[j]);
            if(_entry.ip().isNull())continue;
            if(m_ip_exp.exactMatch(_entry.ip().toString())){
                NORM("网卡:[%s] 已连接，检测通过", STR2CHAR(iface.humanReadableName()));
                _network_connect = true;
                break;
            }
        }
    }

NEXT_STEP:
    if(!_network_connect){
        ERR("没有检测到网卡连接，请检查你的网线，或网卡是否正常");
    }
    return _network_connect;
}

bool DiagnoseWorker::DiagnoseProxy()
{
    m_step = STEP_PROXY_CHECK;
    WxConfig& _config = WxConfig::g();
    if(_config.PROXY_IP.isEmpty() || _config.PROXY_PORT < 1 || _config.PROXY_PORT > 65535){
        WARN("网络代理未配置！跳过代理检查");
        GotoNextStep(true);
        return true;
    }
    QString _proxy_url(QString("http://%1:%2/").arg(_config.PROXY_IP).arg(_config.PROXY_PORT));
    m_qnm.setProxy(QNetworkProxy(QNetworkProxy::NoProxy));
    CheckServiceByUrl(_proxy_url);
    INFO("开始检查[%s:%d]%s", STR2CHAR(_config.PROXY_IP), _config.PROXY_PORT, GetStepName());
    return true;
}

bool DiagnoseWorker::DiagnoseWeixinService()
{
    m_step = STEP_SERVICE_CHECK;
    if(!WxConfig::g().PROXY_IP.isEmpty() && WxConfig::g().PROXY_PORT > 0){
        QNetworkProxy _proxy(QNetworkProxy::HttpProxy, WxConfig::g().PROXY_IP, WxConfig::g().PROXY_PORT,
                             WxConfig::g().PROXY_USER, WxConfig::g().PROXY_PWD);
        m_qnm.setProxy(_proxy);
    }
    QString _proxy_url(WxConfig::g().TEST_URL);
    CheckServiceByUrl(_proxy_url);
    INFO("开始检查%s", GetStepName());
    return true;
}

bool DiagnoseWorker::DiagnoseInputDevice()
{
    INFO("检查扫码设备……");
    INFO("请扫描微信二维码，若要跳过本步骤，请点击下方“跳过”按钮\n");
    m_step = STEP_INPUTDEVICE_CHECK;
    m_timer_id = startTimer(300);
    emit DiagInputDeviceStarted();
    QEventLoop a;
    connect(this, SIGNAL(DiagnoseInputDeviceFinished(bool)), &a, SLOT(quit()));
    a.exec();
    m_step = STEP_NONE;
    StopTimer();
    return true;
}

void DiagnoseWorker::SendMessage(const QString &msg)
{
    emit DiagnosisInfo(msg);
}

const char *DiagnoseWorker::GetStepName()
{
    if(m_step == STEP_PROXY_CHECK){
        return "HTTP代理";
    }
    if(m_step == STEP_SERVICE_CHECK){
        return "微信支付";
    }
    return "未知状态";
}

void DiagnoseWorker::GotoNextStep(bool current_step_succ)
{
    StopTimer();
    if(m_reply){
        m_reply->deleteLater();
        m_reply = NULL;
    }
    if(m_step == STEP_NONE){
        return;
    }
    if(m_step == STEP_PROXY_CHECK){
        if(current_step_succ)
            DiagnoseWeixinService();
        else
            emit DiagnoseFinished();
    }else {
        emit DiagnoseFinished();
    }
}

void DiagnoseWorker::StopTimer()
{
    if(m_timer_id > 0){
        killTimer(m_timer_id);
        m_timer_id = 0;
    }
}

void DiagnoseWorker::CheckServiceByUrl(const QString &service_url)
{
    QUrl _url(service_url);
//    INFO("url:%s", STR2CHAR(service_url));
    QNetworkRequest _request(_url);
    m_reply = m_qnm.get(_request);
    connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(onNetworkError(QNetworkReply::NetworkError)));
    connect(m_reply, SIGNAL(sslErrors(QList<QSslError>)), SLOT(onNetworkSslErrors(QList<QSslError>)));
    connect(m_reply, SIGNAL(finished()), SLOT(onNetworkFinished()));
    m_begin_count_time = GetTimestamp();
    m_timer_id = startTimer(300);
}

void DiagnoseWorker::CheckInputDevice()
{
    QString _input_str = m_input;
    if(_input_str.size() > 0){
        if(isValidWeixinQrcode(_input_str)){
            NORM("已扫描到微信二维码，扫码设备正常");
            emit DiagnoseInputDeviceFinished(true);
        }else{
            WARN("扫描到二维码:%s，但不是微信支付二维码，请重新扫描", STR2CHAR(_input_str));
        }
        return;
    }
}


void DiagnoseWorker::onNetworkFinished()
{
    if(m_reply->error() == QNetworkReply::NoError){
        NORM("连接%s成功，检测通过.耗时:%d秒", GetStepName(), GetTimeCost(m_begin_count_time));
        GotoNextStep(true);
    }else if(m_reply->error() == QNetworkReply::UnknownContentError){
        QVariant _code = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        if(!_code.isNull() && _code.toInt() == 400){
            NORM("连接%s成功，检测通过.耗时:%d秒", GetStepName(), GetTimeCost(m_begin_count_time));
            GotoNextStep(true);
        }else{
            ERR("连接%s失败，检测失败.耗时:%d秒", GetStepName(), GetTimeCost(m_begin_count_time));
            ERR("可能是代理配置问题，导致网络不通，请检查代理配置");
            GotoNextStep(false);
        }
    }else{
        GotoNextStep(false);
    }
}

void DiagnoseWorker::onNetworkError(QNetworkReply::NetworkError code)
{
    if(code == QNetworkReply::NoError){
        NORM("%s状态正常", GetStepName());
        return;
    }
    if(code <= QNetworkReply::UnknownNetworkError ){
        ERR("连接%s失败。网络状态码：%d 错误信息：%s", GetStepName(), code, STR2CHAR(m_reply->errorString()));
        return;
    }
    if(code <= QNetworkReply::UnknownProxyError){
        ERR("连接%s时，代理返回失败。网络状态码：%d 错误信息：%s", GetStepName(), code, STR2CHAR(m_reply->errorString()));
        return;
    }
    if(code < QNetworkReply::UnknownContentError){
        ERR("%s返回失败。网络状态码：%d 错误信息：%s", GetStepName(), code, STR2CHAR(m_reply->errorString()));
        return;
    }
    if(code == QNetworkReply::UnknownContentError){
        QVariant _code = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        if(!_code.isNull() && _code.toInt() == 400){
            return;
        }
        INFO("%s返回失败。网络状态码：%d 错误信息：%s", GetStepName(), code, STR2CHAR(m_reply->errorString()));
        return;
    }
    if(code <= QNetworkReply::ProtocolFailure){
        ERR("连接%s返回协议错误。网络状态码：%d 错误信息：%s", GetStepName(), code, STR2CHAR(m_reply->errorString()));
        return;
    }
    ERR("连接%s时发生错误. 网络状态码：%d 错误信息：%s", GetStepName(), code, STR2CHAR(m_reply->errorString()));
}

void DiagnoseWorker::onNetworkSslErrors(const QList<QSslError> &errors)
{
    ERR("HTTPS连接出错");
    GotoNextStep(false);
}

void DiagnoseWorker::onInputBarcode(QString barcode)
{
    m_input = barcode;
}


bool DiagnoseWorker::DiagshowFileVersion()
{
    INFO("检查文件版本号……");
    QDir _yum(WxConfig::g().WXPAY_WORKDIR);
    QStringList _name_filter;
    _name_filter << "*.exe" << "wxpay*.dll";
    int _max_len = 0;
    const int TAB_WIDTH = 8;
    foreach(QString name, _yum.entryList(_name_filter)){
        if(name.length() > _max_len)
            _max_len = name.length();
    }
    _max_len = (_max_len/TAB_WIDTH + 1)*TAB_WIDTH;
    foreach(QString file, _yum.entryList(_name_filter)){
        int _tab_count = (_max_len - file.length() + TAB_WIDTH - 1) / TAB_WIDTH;
        QString _fmt_name = file.leftJustified(file.length() + _tab_count, '\t');
        QString _full_path = _yum.filePath(file);
        INFO("%s版本号:%s", STR2CHAR(_fmt_name), STR2CHAR(WinTool::GetFileVersion(_full_path)));
    }
    return true;
}

void DiagnoseWorker::timerEvent(QTimerEvent *e)
{
    if(m_step == STEP_NONE){
        return;
    }
    if(m_step == STEP_INPUTDEVICE_CHECK){
        CheckInputDevice();
        return;
    }
    int _time_cost = GetTimeCost(m_begin_count_time);
    if(_time_cost < NET_TIMEOUT){
        emit DiagnosisInfo(".");
    }else{
        ERR("检测%s超时，当前耗时:%s秒", GetStepName(), STR2CHAR(QString::number(_time_cost)));
        ERR("可能是代理配置问题，导致网络不通，请检查代理配置");
        GotoNextStep(false);
    }
}
