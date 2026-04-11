#ifndef AUDIOBACKEND_H
#define AUDIOBACKEND_H

#include <QObject>
#include <QDebug>
#include <QStringListModel>
#include <QtMultimedia>
#ifdef Q_OS_ANDROID
    #include <QJniObject>
#endif
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonParseError>
#include <QTimer>

#include <../../QtLibraries/packetStructs.hpp>
#include <version.h>

#define BufferSize 3000
#define PortNumber 1255
#define maxBufferSize 100000

class AudioBackend : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QAbstractItemModel* inputDevicesModel
                   READ inputDevicesModel
                       NOTIFY devicesChanged)

    Q_PROPERTY(QAbstractItemModel* outputDevicesModel
                   READ outputDevicesModel
                       NOTIFY devicesChanged)

public:
    explicit AudioBackend(QObject *parent = nullptr);

    QAbstractItemModel* inputDevicesModel();
    QAbstractItemModel* outputDevicesModel();
    void showMessageBox(QString message);

signals:
    void setWindowsTittle(QString Tittle);
    void devicesChanged();
    void newVersionFound(uint32_t versionCode);
    void debugMessage(bool _state,const QString &_message);
    void newTextMessage(QString _msg);

public slots:
    void onSettingapplied(int myId,int sendToId,int inputDeviceIndex,int outputDeviceIndex,int outputBufferSize);
    void onStartSend();
    void onStopSend();
    void onSendMessage(QString _msg);

private:
    void acquireMulticastLock(void);
    void refreshAudioDevices(void);
    void updateNotification(QString Tittle,QString Text);
    void makeUDPserver(void);
    void initializeAudio(void);
    void createAudioInput(void);
    void createAudioOutput(void);
    void processBuffer(void);

    QByteArray buffer;
    QList<QByteArray> packets;
    QTimer processPacketsTimer;
    uint32_t latestVersion = 0;

    const char* cls = "org/verya/QMLWalkieTalkie/TestBridge";
    QStringListModel m_inputDevices;
    QStringListModel m_outputDevices;

    QUdpSocket *Server = nullptr;
    QUdpSocket *Client = nullptr;
    QHostAddress multicastAddress;

    int _myId,_sendToId,_inputDeviceIndex,_outputDeviceIndex,_bufferSize;

    QAudioFormat m_format;
    QAudioDevice m_Inputdevice;
    QAudioDevice m_Outputdevice;

    QAudioSource *m_audioInput = nullptr;
    QAudioSink *m_audioOutput = nullptr;

    QIODevice *m_input = nullptr;
    QIODevice *m_output = nullptr;

    uint64_t CurrentID;

#ifdef Q_OS_ANDROID
    QJniObject g_wifiLock;
    QJniObject g_multicastLock;
    QJniObject g_wakeLock;
#endif

private slots:
    void onUDPReadyRead(void);
    void onReadInput(void);
    void onProcessPacketsTimerTimeout(void);
    void handleInputError(QAudio::Error err);
    void handleOutputError(QAudio::Error err);
};

#endif // AUDIOBACKEND_H
