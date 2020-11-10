#ifndef RVIDEODECODE_H
#define RVIDEODECODE_H

#include <QObject>
#include <QThread>
class rvideodecode : public QObject
{
    Q_OBJECT
public:
    explicit rvideodecode(QObject *parent = 0);
    void decodecing();
signals:
   // void sigShowFrame(QImage img);
public slots:
    void slot_start();
private:
    QThread* pthread;
};

#endif // RVIDEODECODE_H
