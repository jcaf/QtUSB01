#include "widget.h"//definition of all qtserial
#include "ui_widget.h"

#include <QtSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QMessageBox>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    usbCDC_is_available = false;
    usbCDC_port_name = "";
    usbCDC = new QSerialPort;

    //+- Solo para depurar
    qDebug()<< "number of available ports "<< QSerialPortInfo::availablePorts().length();
    foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts())
    {
        qDebug()<< "Has vendor ID" << serialPortInfo.hasVendorIdentifier();
        if (serialPortInfo.hasVendorIdentifier()){
            qDebug()<< "Vendor id"<< serialPortInfo.vendorIdentifier();
        }

        qDebug()<< "Has product ID" << serialPortInfo.hasProductIdentifier();
        if (serialPortInfo.hasProductIdentifier())
        {
            qDebug()<< "Product ID" << serialPortInfo.productIdentifier();
        }
    }
    //+-
    foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts())
    {
        if (serialPortInfo.hasProductIdentifier() && serialPortInfo.hasVendorIdentifier())
        {
            if ( (serialPortInfo.productIdentifier() == usbCDC_product_id) && (serialPortInfo.vendorIdentifier() == usbCDC_vendor_id) )
            {
                usbCDC_is_available = true;
                usbCDC_port_name  =serialPortInfo.portName();
                qDebug()<< usbCDC_port_name;
            }
        }
    }
    //+-
    if (usbCDC_is_available)
    {
        usbCDC->setPortName(usbCDC_port_name);
        //usbCDC->open(QSerialPort::WriteOnly);
        usbCDC->open(QSerialPort::ReadWrite);
        //usbCDC->open(QSerialPort::ReadOnly);//ok x lecturas
        usbCDC->setBaudRate(QSerialPort::Baud38400);
        usbCDC->setDataBits(QSerialPort::Data8);
        usbCDC->setParity(QSerialPort::NoParity);
        usbCDC->setStopBits(QSerialPort::OneStop);
        usbCDC->setFlowControl(QSerialPort::NoFlowControl);
        //
        QObject::connect(usbCDC, SIGNAL(readyRead()), this, SLOT(readSerial()));
    }
    else
    {
        QMessageBox::warning(this, "Port error", "Tarjeta de control no encontrado");
    }

}

Widget::~Widget()
{
    delete ui;

    if (usbCDC->isOpen())
    {
        usbCDC->close();
    }
}



/*
* Data payload: @dataASCII1,dataASCII2,..dataASCIIN\r\n
* el final de linea es compuesto por \r\n, pero solo uso \r para detectar el final como token de cierre
*/

#define TOKEN_BEGIN '@'
#define TOKEN_SEPARATOR ','
#define TOKEN_END '\r'
#define RXDATA_NUM_DATOS 3 //3 datos se envian
#define RXDATA_NUMMAX_CHARS_EN1DATO 20  //Numero max. de caracters en 1 dato esperado
#define RXDATA_NUM_TOKENS RXDATA_NUM_DATOS//Numero de tokens = # de datos que se envian

#if (RXDATA_NUM_DATOS == 0)
    #error "RXDATA_PAYLOAD_MAXSIZE = 0"
#endif

QString str_acc = "";
float meters = 0;
float volts = 0;
float current = 0;

void Widget::readSerial()
{
    static int counter = 0;
    qDebug()<< "counter:" << counter++;

//    a.append(usbCDC->readAll());
//    QString temp = QString::fromStdString(a.toStdString());
//    if (counter == 3)
//    {
//        counter = 0;
//        a.clear();
//    }
//    qDebug()<< temp;

    QByteArray serialBuff = usbCDC->readAll();
    QString str_payload = QString::fromStdString(serialBuff.toStdString());
    str_acc += str_payload;
//    qDebug()<< str_acc;

    char c;
    char v[RXDATA_NUM_DATOS][RXDATA_NUMMAX_CHARS_EN1DATO];
    int sm0;
    int fil;
    int col;
    int counter_tokens = 0;
    char kc;

//    @120.1, 45, 555 \n
//    qDebug()<<"antes de buscar: " << QString::fromStdString(Cstr) << Qt::endl;
//    qDebug()<< "longitud"<< length << Qt::endl;

    sm0 = 0;
    fil = 0;
    col = 0;
    counter_tokens = 0;
    std::string Cstr = str_acc.toStdString();
    int length = Cstr.length();

    for (int i=0; i< length; i++)
    {
        c =  Cstr[i];
        //qDebug()<< i  << Qt::endl;
        if (sm0 == 0)
        {
            if ( c == TOKEN_BEGIN)
            {
                fil = 0;
                col = 0;

                #if (RXDATA_NUM_DATOS  == 1)
                    kc = TOKEN_END;
                #else
                    kc = TOKEN_SEPARATOR;
                #endif

                counter_tokens = 0;
                sm0++;
            }
        }
        else if (sm0 == 1)
        {
            if (c == kc)
            {
                v[fil][col] = '\0';

                #if (RXDATA_NUM_DATOS  > 1)
                    col = 0x0;
                    if (++fil == RXDATA_NUM_TOKENS-1)
                    {
                        kc = TOKEN_END; //'\n'
                    }
                #endif

                if (++counter_tokens == RXDATA_NUM_TOKENS)
                {
                    str_acc = "";
                    sm0 = 0;
                    break;
                }
            }
            else
            {
                v[fil][col++] = c;
            }
        }
    }
    if (counter_tokens == RXDATA_NUM_TOKENS)
    {
        counter_tokens = 0x00;

        //specific application
        meters = atof(&v[0][0]);
        volts = atof(&v[1][0]);
        current = atof(&v[2][0]);
        qDebug() << meters;
        qDebug() << volts;
        qDebug() << current;
    //                    qDebug() << &v[0][0];
    //                    qDebug() << &v[1][0];
    //                    qDebug() << &v[2][0];
    }
}

