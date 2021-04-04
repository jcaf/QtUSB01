#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class QSerialPort;

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();


private slots:
    void readSerial();

private:
    Ui::Widget *ui;

    QSerialPort *usbCDC;
    static const quint16 usbCDC_vendor_id = 9025;//0x2341
    static const quint16 usbCDC_product_id = 67;//0x0047
    QString usbCDC_port_name;
    bool usbCDC_is_available;

};
#endif // WIDGET_H
