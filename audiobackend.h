#ifndef AUDIOBACKEND_H
#define AUDIOBACKEND_H

#include <QObject>
#include <QDebug>

class AudioBackend : public QObject
{
    Q_OBJECT
public:
    explicit AudioBackend(QObject *parent = nullptr);

signals:
    void setWindowsTittle(QString Tittle);

public slots:
    void onSettingapplied(int myId,int sendToId,int inputDeviceIndex,int outputDeviceIndex);
};

#endif // AUDIOBACKEND_H
