#include "showimagewidget.h"
#include "ui_showimagewidget.h"

showImageWidget::showImageWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::showImageWidget)
{
    ui->setupUi(this);
}

showImageWidget::~showImageWidget()
{
    delete ui;
}

void showImageWidget::loadImage(QImage image)
{
    ui->label->setPixmap(QPixmap::fromImage(image));
}
