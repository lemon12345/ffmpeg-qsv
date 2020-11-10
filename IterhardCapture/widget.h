#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "rvideodecode.h"
#include "showimagewidget.h"
namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
    void start();
	void showVideo();
signals:
    void sigstart();
private:
    Ui::Widget *ui;
    showImageWidget * imageWidget;
    rvideodecode* rcodec;
};

#endif // WIDGET_H
