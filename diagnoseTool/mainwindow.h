/******************************************************************************
  * Copyright (C) 2016, Tecent Co.Ltd. 
  * All Rights Reserved. 
  * Created:    2016-01-07 by blaketang
  * Last Modify:2016-08-10
  * @By blaketang
  * Description:
  * 
 ******************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "diagnoseworker.h"
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
signals:
    void barcodeReceived(QString barcode);
private slots:
    void onDiagInfoReceived(QString msg);
    void onDiagFinished();
    void onDiagShowSkipBtn();
    void onDiagHideSkipBtn();
    void onStartDiagBtnClicked(bool);
    void onDiagSkipBtnClicked(bool);
private:
    Ui::MainWindow *ui;
    DiagnoseWorker m_diagworker;
    QString m_barcode;

    // QWidget interface
protected:
    virtual void keyPressEvent(QKeyEvent *);
};

#endif // MAINWINDOW_H
