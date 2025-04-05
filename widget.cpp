#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    ui->pbSend->setEnabled(false);
    ui->pbDisconnect->setEnabled(false);
    ui->leProtocol->setReadOnly(true);
    ui->lePort->setReadOnly(true);
    ui->pteMessage->setReadOnly(true);

    //socket_.connected();
    /*QObject::connect(&socket_, &QAbstractSocket::connected, this, &Widget::doConnected);
    QObject::connect(&socket_, &QAbstractSocket::disconnected, this, &Widget::doDisconnected);*/
    QObject::connect(&socket_, &QIODevice::readyRead, this, &Widget::doReadyRead);
    QObject::connect(&socket_, &QAbstractSocket::stateChanged, this, &Widget::doStateChanged);
    QObject::connect(ui->cbSSL, &QCheckBox::checkStateChanged, this, &Widget::on_cbSSL_stateChanged);
    QObject::connect(ui->leHost,&QLineEdit::textChanged, this, &Widget::on_leHost_textChanged);

}

Widget::~Widget()
{
    delete ui;
}
/*
void Widget::doConnected(){
    QString msg = "connected\r\n";
    ui->pteMessage->insertPlainText(msg);
}

void Widget::doDisconnected(){
    QString msg = "disconnected\r\n";
    ui->pteMessage->insertPlainText(msg);
}*/

void Widget::doReadyRead(){
    QString msg = socket_.readAll();
    ui->pteMessage->insertPlainText("\r\n"+msg);
}

void Widget::on_cbSSL_stateChanged(int state){
    if(state==Qt::Checked){
        ui->lePort->setText("443");
        ui->leProtocol->setText("HTTPS");
    }else{
        ui->lePort->setText("80");
        ui->leProtocol->setText("HTTP");
    }
}

void Widget::doStateChanged(QAbstractSocket::SocketState state){

    bool isConnected = (state == QAbstractSocket::ConnectedState);
    ui->pbSend->setEnabled(isConnected);
    ui->pbDisconnect->setEnabled(isConnected);
    ui->pbConnect->setEnabled(!isConnected);

    QString stateStr;
    switch(state){
        case QAbstractSocket::UnconnectedState: stateStr="Unconnected";break;
        case QAbstractSocket::HostLookupState: stateStr="HostLookup";break;
        case QAbstractSocket::ConnectingState: stateStr="Connecting";break;
        case QAbstractSocket::ConnectedState: stateStr="Connected";break;
        case QAbstractSocket::BoundState: stateStr="Bound";break;
        case QAbstractSocket::ClosingState: stateStr="Closing";break;
        case QAbstractSocket::ListeningState: stateStr="Listening";break;
    }
    ui->pteMessage->appendPlainText("Socket state changed: "+ stateStr+"\r\n");
}

void Widget::on_leHost_textChanged(const QString &text){
    QString httpRequest = QString("GET / HTTP/1.1\r\nHost: %1\r\n\r\n").arg(text);
    ui->pteSend->setPlainText(httpRequest);
}

void Widget::on_pbConnect_clicked()//tcp connection(syn)
{
    //socket_.connectToHost(ui->leHost->text(),ui->lePort->text().toUShort());
    //socket_.connectToHostEncrypted(ui->leHost->text(),ui->lePort->text().toUShort());
    QString host = ui->leHost->text();
    quint16 port = ui->lePort->text().toUShort();

    if(ui->cbSSL->isChecked()){
        if(QSslSocket::supportsSsl())
            socket_.connectToHostEncrypted(host, port);
        else
            ui->pteMessage->appendPlainText("CAN'T USE SSL");
    }else{
        socket_.connectToHost(host,port);
    }
}


void Widget::on_pbDisconnect_clicked()//tcp disconnection(fin)
{
    socket_.disconnectFromHost();
    doStateChanged(socket_.state());
}


void Widget::on_pbSend_clicked()
{
    socket_.write(ui->pteSend->toPlainText().toUtf8());//korean byte exchange
}


void Widget::on_pbClear_clicked()
{
    ui->pteMessage->clear();
}

