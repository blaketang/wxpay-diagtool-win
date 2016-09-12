/******************************************************************************
  * Copyright (C) 2016, Tecent Co.Ltd. 
  * All Rights Reserved. 
  * Created:    2016-01-07 by blaketang
  * Last Modify:2016-08-10
  * @By blaketang
  * Description:
  * 
 ******************************************************************************/

#ifndef DIAGNOSEWORKER_H
#define DIAGNOSEWORKER_H

#include <QNetworkAccessManager>
#include <QObject>
#include <QNetworkReply>
#include <QRegExp>
const int NET_TIMEOUT = 10;
class DiagnoseWorker:public QObject
{
    Q_OBJECT
    enum STEP{
        STEP_NONE,
        STEP_INPUTDEVICE_CHECK,
        STEP_PROXY_CHECK,
        STEP_SERVICE_CHECK
    };

public:
    DiagnoseWorker();
    ~DiagnoseWorker();
public:
    void StartDiagnosis();
signals:
    void DiagnosisInfo(QString msg);
    void DiagInputDeviceStarted();
    void DiagnoseInputDeviceFinished(bool succ);
    void DiagnoseFinished();
private slots:
    void onNetworkFinished();
    void onNetworkError(QNetworkReply::NetworkError code );
    void onNetworkSslErrors(const QList<QSslError> & errors);
    void onInputBarcode(QString barcode);
private:
    bool DiagshowFileVersion();
    bool DiagnoseConfig();
    bool DiagnoseMachineInfo();
    bool DiagnoseProxy();
    bool DiagnoseWeixinService();
    bool DiagnoseInputDevice();
    void SendMessage(const QString& msg);
    const char* GetStepName();
    void GotoNextStep(bool);
    void StopTimer();
    void CheckServiceByUrl(const QString& url);
    void CheckInputDevice();
private:
    QNetworkAccessManager m_qnm;
    QRegExp m_ip_exp;
    QNetworkReply* m_reply;
    uint m_begin_count_time;
    STEP m_step;
    int m_timer_id;
    QString m_input;
    // QObject interface
protected:
    virtual void timerEvent(QTimerEvent *);
};

#endif // DIAGNOSEWORKER_H
