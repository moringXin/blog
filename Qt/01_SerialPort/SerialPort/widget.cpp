#include "widget.h"
#include "ui_widget.h"
#include <QDebug>
#include <QMessageBox>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    resize(800, 600);
    setWindowTitle("串口调试");

    ui->comboBox_2->setCurrentIndex(1);
    ui->comboBox_3->setCurrentIndex(3);
    ui->comboBox_4->setCurrentIndex(2);
    ui->comboBox_5->setCurrentIndex(0);

    serial = new QSerialPort(this);

    timer = new QTimer(this);
    timer->start(500);

    //每500ms回调，执行槽函数，扫描有没有串口
    connect(timer, &QTimer::timeout, this, &Widget::TimerEvent);

    //收到数据，跳转到数据处理函数中
    connect(serial, QSerialPort::readyRead, this, &Widget::serialPort_readyRead);

    ui->checkBox->setCheckState(Qt::Checked);

    receiveByte = 0;
    sendByte = 0;
}

Widget::~Widget()
{
    delete ui;
}

void Widget::TimerEvent()
{
    QStringList newPortStringList;
    newPortStringList.clear();
    foreach(const QSerialPortInfo &info,QSerialPortInfo::availablePorts())
        newPortStringList += info.portName();

    if(newPortStringList.size() != portStringList.size())
    {
        portStringList = newPortStringList;
        ui->comboBox->clear();
        ui->comboBox->addItems(newPortStringList);
    }
}

void Widget::on_pushButton_clicked()
{
    if(ui->pushButton->text() == QString("打开串口"))
    {
        serial->setPortName(ui->comboBox->currentText());
        serial->setBaudRate(ui->comboBox_2->currentText().toInt());
        switch(ui->comboBox_3->currentText().toInt())
        {
        case 5:serial->setDataBits(QSerialPort::Data5);break;
        case 6:serial->setDataBits(QSerialPort::Data6);break;
        case 7:serial->setDataBits(QSerialPort::Data7);break;
        case 8:serial->setDataBits(QSerialPort::Data8);break;
        default:serial->setDataBits(QSerialPort::UnknownDataBits);break;
        }
        switch(ui->comboBox_4->currentIndex())
        {
        case 0:serial->setParity(QSerialPort::EvenParity);break;
        case 1:serial->setParity(QSerialPort::OddParity);break;
        case 2:serial->setParity(QSerialPort::NoParity);break;
        default:serial->setParity(QSerialPort::UnknownParity);break;
        }
        switch(ui->comboBox_5->currentIndex())
        {
        case 0:serial->setStopBits(QSerialPort::OneStop);break;
        case 1:serial->setStopBits(QSerialPort::OneAndHalfStop);break;
        case 2:serial->setStopBits(QSerialPort::TwoStop);break;
        default:serial->setStopBits(QSerialPort::UnknownStopBits);break;
        }
        serial->setFlowControl(QSerialPort::NoFlowControl);

        if(!serial->open(QIODevice::ReadWrite))
        {
            QMessageBox::information(this, "错误提示", "无法打开串口", QMessageBox::Ok);
            return;
        }

        ui->comboBox->setEnabled(false);
        ui->comboBox_2->setEnabled(false);
        ui->comboBox_3->setEnabled(false);
        ui->comboBox_4->setEnabled(false);
        ui->comboBox_5->setEnabled(false);
        ui->pushButton->setText("关闭串口");
    }
    else
    {
        serial->close();
        ui->comboBox->setEnabled(true);
        ui->comboBox_2->setEnabled(true);
        ui->comboBox_3->setEnabled(true);
        ui->comboBox_4->setEnabled(true);
        ui->comboBox_5->setEnabled(true);
        ui->pushButton->setText("打开串口");
    }
}

void Widget::serialPort_readyRead()
{
    if(ui->checkBox_3->checkState() == Qt::Checked)
    {
        return;
    }

    QString lastText;//前一轮的历史数据
    int length;

    lastText = ui->textEdit->toPlainText();

    receiveText = serial->readAll();
    receiveByte += receiveText.length();
    ui->label_9->setText(QString::number(receiveByte));

    if(ui->checkBox_2->checkState() == Qt::Checked)//Hex
    {
        receiveText = receiveText.toLatin1().toHex();
        length = receiveText.length();
        for(int i = 0; i < length/2; i++)
        {
            receiveText.insert(2+3*i, ' ');
        }
    }
    else//ASCII
    {
        receiveText = receiveText.toLatin1();
    }

    lastText = lastText.append(receiveText);
    ui->textEdit->setText(lastText);
}

void Widget::on_checkBox_clicked()
{
    ui->checkBox->setCheckState(Qt::Checked);
    ui->checkBox_2->setCheckState(Qt::Unchecked);
    ui->checkBox_3->setCheckState(Qt::Unchecked);
}

void Widget::on_checkBox_2_clicked()
{
    ui->checkBox->setCheckState(Qt::Unchecked);
    ui->checkBox_2->setCheckState(Qt::Checked);
    ui->checkBox_3->setCheckState(Qt::Unchecked);
}

void Widget::on_checkBox_3_clicked()
{
    ui->checkBox->setCheckState(Qt::Unchecked);
    ui->checkBox_2->setCheckState(Qt::Unchecked);
    ui->checkBox_3->setCheckState(Qt::Checked);
}

void Widget::on_pushButton_2_clicked()
{
    sendText = ui->textEdit_2->toPlainText();

    QByteArray bytearray;
    bytearray = sendText.toLatin1();
    serial->write(bytearray);

    sendByte += sendText.length();
    ui->label_11->setText(QString::number(sendByte));
}

void Widget::on_pushButton_3_clicked()
{
    ui->textEdit->clear();
}

void Widget::on_pushButton_4_clicked()
{
    ui->textEdit_2->clear();
}
