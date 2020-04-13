﻿#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    pm3=new PM3Process;
    mifare=new Mifare;
    connect(pm3,&PM3Process::readyRead,this,&MainWindow::refresh);
    connect(ui->Raw_CMDEdit,&QLineEdit::editingFinished,this,&MainWindow::sendMSG);
    uiInit();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ******************** basic functions ********************

void MainWindow::on_PM3_refreshPortButton_clicked()
{
    ui->PM3_portBox->clear();
    ui->PM3_portBox->addItem("");
    foreach(QString port,pm3->findPort())
    {
        ui->PM3_portBox->addItem(port);
    }
}

void MainWindow::on_PM3_connectButton_clicked()
{
    QString port=ui->PM3_portBox->currentText();
    if(port=="")
        QMessageBox::information(NULL, "Info", "Plz choose a port first", QMessageBox::Ok);
    else
        qDebug()<<pm3->start(ui->PM3_pathEdit->text(),port);
}

void MainWindow::on_PM3_disconnectButton_clicked()
{
    pm3->kill();
}

// *********************************************************

// ******************** raw command ********************

void MainWindow::on_Raw_sendCMDButton_clicked()
{
    if(ui->Raw_CMDHistoryWidget->count()==0 || ui->Raw_CMDHistoryWidget->item(ui->Raw_CMDHistoryWidget->count()-1)->text()!=ui->Raw_CMDEdit->text())
        ui->Raw_CMDHistoryWidget->addItem(ui->Raw_CMDEdit->text());
    qDebug()<<(ui->Raw_CMDEdit->text().toLocal8Bit());
    pm3->write((ui->Raw_CMDEdit->text()+"\r\n").toLocal8Bit());
    pm3->waitForBytesWritten(3000);
}

void MainWindow::on_Raw_clearOutputButton_clicked()
{
    ui->Raw_outputEdit->clear();
}

void MainWindow::on_Raw_moreFuncCheckBox_stateChanged(int arg1)
{
    if(ui->Raw_moreFuncCheckBox->isChecked())
    {
        ui->Raw_CMDTreeWidget->setVisible(true);
        ui->Raw_CMDTreeLabel->setVisible(true);
        ui->Raw_CMDHistoryWidget->setVisible(true);
        ui->Raw_CMDHistoryLabel->setVisible(true);
        ui->Raw_clearHistoryButton->setVisible(true);
    }
    else
    {
        ui->Raw_CMDTreeWidget->setVisible(false);
        ui->Raw_CMDTreeLabel->setVisible(false);
        ui->Raw_CMDHistoryWidget->setVisible(false);
        ui->Raw_CMDHistoryLabel->setVisible(false);
        ui->Raw_clearHistoryButton->setVisible(false);
    }
}

void MainWindow::on_Raw_clearHistoryButton_clicked()
{
    ui->Raw_CMDHistoryWidget->clear();
}

void MainWindow::on_Raw_CMDHistoryWidget_itemDoubleClicked(QListWidgetItem *item)
{
    ui->Raw_CMDEdit->setText(item->text());
    ui->Raw_CMDEdit->setFocus();
}

// *****************************************************

// ******************** mifare ********************

void MainWindow::on_MF_Attack_chkButton_clicked()
{
    pm3->setRequiringOutput(true);
    ui->Raw_CMDEdit->setText("hf mf chk *1 ?");
    on_Raw_sendCMDButton_clicked();
    while(pm3->waitForReadyRead(5000))
        ;
    QString result=pm3->getRequiredOutput();
     pm3->setRequiringOutput(false);
    result=result.mid(result.indexOf("|---|----------------|----------------|"));
    QStringList keys=result.split("\r\n");
    for(int i=0;i<16;i++)
    {
        ui->MF_keyWidget->setItem(i,1,new QTableWidgetItem(keys[i+3].mid(7,12).trimmed().toUpper()));
        ui->MF_keyWidget->setItem(i,2,new QTableWidgetItem(keys[i+3].mid(24,12).trimmed().toUpper()));
    }
    qDebug()<<"***********\n"<<keys<<"***********\n";
}

void MainWindow::on_MF_Attack_nestedButton_clicked()
{
    pm3->setRequiringOutput(true);
    ui->Raw_CMDEdit->setText("hf mf nested 1 *");
    on_Raw_sendCMDButton_clicked();
    while(pm3->waitForReadyRead(5000))
        ;
    QString result=pm3->getRequiredOutput();
     pm3->setRequiringOutput(false);
    result=result.mid(result.indexOf("|---|----------------|---|----------------|---|"));
    QStringList keys=result.split("\r\n");
    for(int i=0;i<16;i++)
    {
        if(keys[i+3].at(23)=='1')
            ui->MF_keyWidget->setItem(i,1,new QTableWidgetItem(keys[i+3].mid(7,12).trimmed().toUpper()));
        if(keys[i+3].at(44)=='1')
            ui->MF_keyWidget->setItem(i,2,new QTableWidgetItem(keys[i+3].mid(28,12).trimmed().toUpper()));
    }
    qDebug()<<"***********\n"<<keys<<"***********\n";
}

void MainWindow::on_MF_Attack_hardnestedButton_clicked()
{

}

void MainWindow::on_MF_Attack_sniffButton_clicked()
{
    ui->Raw_CMDEdit->setText("hf mf sniff");
    on_Raw_sendCMDButton_clicked();
    ui->funcTab->setCurrentIndex(1);
}

void MainWindow::on_MF_Attack_listButton_clicked()
{
    ui->Raw_CMDEdit->setText("hf list mf");
    on_Raw_sendCMDButton_clicked();
    ui->funcTab->setCurrentIndex(1);
}

// ************************************************


// ******************** other ********************

void MainWindow::refresh()
{
    QString btay=pm3->readLine();
    while(btay!="")
    {
        qDebug()<<btay;
        ui->Raw_outputEdit->insertPlainText(btay);
        btay=pm3->readLine();
    }
    ui->Raw_outputEdit->moveCursor(QTextCursor::End);
}

void MainWindow::sendMSG()
{
    if(ui->Raw_CMDEdit->hasFocus())
        on_Raw_sendCMDButton_clicked();
}

void MainWindow::uiInit()
{
    connectStatusBar=new QLabel(this);
    programStatusBar=new QLabel(this);
    PM3VersionBar=new QLabel(this);
    setStatusBar(connectStatusBar,"Not Connected");
    setStatusBar(programStatusBar,"Idle");
    setStatusBar(PM3VersionBar,"");
    ui->statusbar->addPermanentWidget(PM3VersionBar,1);
    ui->statusbar->addPermanentWidget(connectStatusBar,1);
    ui->statusbar->addPermanentWidget(programStatusBar,1);

    ui->MF_dataWidget->setColumnCount(3);
    ui->MF_dataWidget->setRowCount(64);
    ui->MF_dataWidget->setHorizontalHeaderItem(0,new QTableWidgetItem("Sector"));
    ui->MF_dataWidget->setHorizontalHeaderItem(1,new QTableWidgetItem("Block"));
    ui->MF_dataWidget->setHorizontalHeaderItem(2,new QTableWidgetItem("Data"));
    for(int i=0;i<64;i++)
        ui->MF_dataWidget->setItem(i,1,new QTableWidgetItem(QString::number(i)));
    for(int i=0;i<16;i++)
        ui->MF_dataWidget->setItem(i*4,0,new QTableWidgetItem(QString::number(i)));
    ui->MF_dataWidget->verticalHeader()->setVisible(false);
    ui->MF_dataWidget->setColumnWidth(0,50);
    ui->MF_dataWidget->setColumnWidth(1,40);
    ui->MF_dataWidget->setColumnWidth(2,400);

    ui->MF_keyWidget->setColumnCount(3);
    ui->MF_keyWidget->setRowCount(16);
    ui->MF_keyWidget->setHorizontalHeaderItem(0,new QTableWidgetItem("Sector"));
    ui->MF_keyWidget->setHorizontalHeaderItem(1,new QTableWidgetItem("KeyA"));
    ui->MF_keyWidget->setHorizontalHeaderItem(2,new QTableWidgetItem("KeyB"));
    for(int i=0;i<16;i++)
        ui->MF_keyWidget->setItem(i,0,new QTableWidgetItem(QString::number(i)));
    ui->MF_keyWidget->verticalHeader()->setVisible(false);
    ui->MF_keyWidget->setColumnWidth(0,50);
    ui->MF_keyWidget->setColumnWidth(1,200);
    ui->MF_keyWidget->setColumnWidth(2,200);

    on_Raw_moreFuncCheckBox_stateChanged(0);
    on_PM3_refreshPortButton_clicked();
}

void MainWindow::setStatusBar(QLabel* target,const QString & text)
{
    if(target==PM3VersionBar)
        target->setText("Version:"+text);
    else if(target==connectStatusBar)
        target->setText("Connecton State:"+text);
    else if(target==programStatusBar)
        target->setText("Program State:"+text);
}

// ***********************************************






