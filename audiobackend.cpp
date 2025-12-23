#include "audiobackend.h"
#include <qdebug.h>

AudioBackend::AudioBackend(QObject *parent)
    : QObject{parent}
{

}

void AudioBackend::onSettingapplied(int myId, int sendToId, int inputDeviceIndex, int outputDeviceIndex)
{
    qDebug() << QString("sendToId : %1").arg(sendToId);
    emit setWindowsTittle(QString("Test - %1").arg(myId));
}
