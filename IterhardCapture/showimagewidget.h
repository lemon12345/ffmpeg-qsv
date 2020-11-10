#ifndef SHOWIMAGEWIDGET_H
#define SHOWIMAGEWIDGET_H

#include <QWidget>

namespace Ui {
class showImageWidget;
}

class showImageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit showImageWidget(QWidget *parent = 0);
    ~showImageWidget();
public slots:
    void loadImage(QImage image);
private:
    Ui::showImageWidget *ui;
};

#endif // SHOWIMAGEWIDGET_H
