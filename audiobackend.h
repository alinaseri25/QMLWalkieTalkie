#ifndef AUDIOBACKEND_H
#define AUDIOBACKEND_H

#include <QObject>
#include <QDebug>
#include <QStringListModel>
#include <QtMultimedia>
#ifdef Q_OS_ANDROID
    #include <QJniObject>
#endif

#define BufferSize 1024
#define PortNumber 1255

typedef struct
{
    uint64_t SenderId;
    uint32_t SenderGP;
    uint32_t RecipientGP;
    uint8_t Data[BufferSize];
}AudioPacket;

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

public slots:
    void onSettingapplied(int myId,int sendToId,int inputDeviceIndex,int outputDeviceIndex);
    void onStartSend();
    void onStopSend();

private:
    void acquireMulticastLock(void);
    void refreshAudioDevices(void);
    void updateNotification(QString Tittle,QString Text);
    void makeUDPserver(void);
    void initializeAudio(void);
    void createAudioInput(void);
    void createAudioOutput(void);

    const char* cls = "org/verya/QMLWalkieTalkie/TestBridge";
    QStringListModel m_inputDevices;
    QStringListModel m_outputDevices;

    QUdpSocket *Server = nullptr;
    QUdpSocket *Client = nullptr;

    int _myId,_sendToId,_inputDeviceIndex,_outputDeviceIndex;

    QAudioFormat m_format,m_formatSample;
    QAudioDevice m_Inputdevice;
    QAudioDevice m_Outputdevice;

    QAudioSource *m_audioInput;
    QAudioSink *m_audioOutput;

    QIODevice *m_input;
    QIODevice *m_output;

    uint64_t CurrentID;

#ifdef Q_OS_ANDROID
    QJniObject g_wifiLock;
    QJniObject g_multicastLock;
    QJniObject g_wakeLock;
#endif

private slots:
    void onUDPReadyRead(void);
    void onReadInput(void);
};

#endif // AUDIOBACKEND_H
