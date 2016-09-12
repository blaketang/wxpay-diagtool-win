#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QKeyEvent>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->skipBtn->setVisible(false);
    connect(&m_diagworker, SIGNAL(DiagnosisInfo(QString)), SLOT(onDiagInfoReceived(QString)));
    connect(&m_diagworker, SIGNAL(DiagnoseFinished()), SLOT(onDiagFinished()));
    connect(ui->startBtn, SIGNAL(clicked(bool)), SLOT(onStartDiagBtnClicked(bool)));
    connect(ui->exitBtn, SIGNAL(clicked(bool)), qApp, SLOT(quit()));
    connect(&m_diagworker, SIGNAL(DiagInputDeviceStarted()), SLOT(onDiagShowSkipBtn()));
    connect(&m_diagworker, SIGNAL(DiagnoseInputDeviceFinished(bool)), SLOT(onDiagHideSkipBtn()));
    connect(ui->skipBtn, SIGNAL(clicked(bool)), &m_diagworker, SIGNAL(DiagnoseInputDeviceFinished(bool)));
    connect(ui->skipBtn, SIGNAL(clicked(bool)), SLOT(onDiagSkipBtnClicked(bool)));
    QTextCursor _cursor(ui->textEdit->textCursor());
    QTextBlockFormat _blockfmt(_cursor.blockFormat());
    _blockfmt.setLineHeight(26, QTextBlockFormat::FixedHeight);
    _cursor.setBlockFormat(_blockfmt);
    ui->textEdit->setTextCursor(_cursor);
    ui->textEdit->setReadOnly(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onDiagInfoReceived(QString msg)
{
    if(msg.length() == 1){
        ui->textEdit->moveCursor(QTextCursor::End);
        ui->textEdit->insertPlainText(msg);
    }else{
        ui->textEdit->append(msg);
    }
}

void MainWindow::onDiagFinished()
{
    ui->textEdit->append(QString::fromUtf8("检查完成"));
    ui->startBtn->setEnabled(true);
}

void MainWindow::onDiagShowSkipBtn()
{
    ui->skipBtn->setVisible(true);
    connect(this, SIGNAL(barcodeReceived(QString)), &m_diagworker, SLOT(onInputBarcode(QString)));
}

void MainWindow::onDiagHideSkipBtn()
{
    ui->skipBtn->setVisible(false);
    disconnect(this, SIGNAL(barcodeReceived(QString)), &m_diagworker, SLOT(onInputBarcode(QString)));
}

void MainWindow::onStartDiagBtnClicked(bool)
{
    ui->textEdit->clear();
    ui->startBtn->setEnabled(false);
    m_diagworker.StartDiagnosis();
}

void MainWindow::onDiagSkipBtnClicked(bool)
{
    onDiagHideSkipBtn();
    onDiagInfoReceived(QString::fromUtf8("[信息]　跳过二维码检查"));
}


void MainWindow::keyPressEvent(QKeyEvent *e)
{
    //only work when scan device
    if(e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)//enter
    {
        if(m_barcode.size() > 0)
        emit barcodeReceived(m_barcode);
        m_barcode.clear();
    }
    else
    {
        m_barcode.append(e->text());
    }
    e->accept();
}
