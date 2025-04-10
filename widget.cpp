#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{

    //ReadOnly
    ui->setupUi(this);
    ui->pbSend->setEnabled(false);
    ui->pbDisconnect->setEnabled(false);
    ui->leProtocol->setReadOnly(true);
    ui->lePort->setReadOnly(true);
    ui->pteMessage->setReadOnly(true);
    ui->pteSend->setReadOnly(true);

    //socket_.connected();
    /*QObject::connect(&socket_, &QAbstractSocket::connected, this, &Widget::doConnected);
    QObject::connect(&socket_, &QAbstractSocket::disconnected, this, &Widget::doDisconnected);*/
    QObject::connect(&socket_, &QIODevice::readyRead, this, &Widget::doReadyRead);
    QObject::connect(&socket_, &QAbstractSocket::stateChanged, this, &Widget::doStateChanged);
    QObject::connect(ui->cbSSL, &QCheckBox::checkStateChanged, this, &Widget::on_cbSSL_stateChanged);
    QObject::connect(ui->leHost,&QLineEdit::textChanged, this, &Widget::on_leHost_textChanged);

    QFile file("save_data.txt");
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);

        int x, y;
        QString line;

        // 좌표
        in >> x >> y;
        this->move(x, y);
        in.readLine();
        ui->leHost->setText(in.readLine());
        line = in.readLine();
        ui->pteSend->setPlainText(line.replace("\\n", "\n"));
        line = in.readLine();
        ui->pteMessage->setPlainText(line.replace("\\n", "\n"));

        file.close();
    }
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


//SSL 선택에 따른 프로토콜/포트 설정
//socket must be not null->need to handle the error!!
void Widget::on_cbSSL_stateChanged(int state){
    if(state==Qt::Checked){
        ui->lePort->setText("443");
        ui->leProtocol->setText("HTTPS");
    }else{
        ui->lePort->setText("80");
        ui->leProtocol->setText("HTTP");
    }
}


//소켓 상태(stateChanged)에 따른 UI 버튼 활성화
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


//HOST 입력 시 자동 HTTP Request 생성
void Widget::on_leHost_textChanged(const QString &text){
    QString httpRequest = QString("GET / HTTP/1.1\r\nHost: %1\r\n\r\n").arg(text);
    ui->pteSend->setPlainText(httpRequest);
}


// TCP / SSL 통신 연결
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

//save state
void Widget::closeEvent(QCloseEvent *event)
{
    QFile file("save_data.txt");
    if(file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);

        // 화면 좌표 저장
        QPoint pos = this->pos();
        out << pos.x() << " " << pos.y() << "\n";

        out << ui->leHost->text() << "\n";
        out << ui->pteSend->toPlainText().replace("\n", "\\n") << "\n";
        out << ui->pteMessage->toPlainText().replace("\n", "\\n") << "\n";

        file.close();
    }

    QWidget::closeEvent(event);
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


//화면 메시지 초기화
void Widget::on_pbClear_clicked()
{
    ui->pteMessage->clear();
}

