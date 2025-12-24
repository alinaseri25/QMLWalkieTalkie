#include "audiobackend.h"
#include <qdebug.h>

// ------- GLOBAL POINTER -------
static AudioBackend* g_mainWindowInstance = nullptr;

#ifdef Q_OS_ANDROID
extern "C"
    JNIEXPORT void JNICALL
    Java_org_verya_QMLWalkieTalkie_TestBridge_onMessageFromKotlin(JNIEnv* env, jclass /*clazz*/, jstring msg)
{
    if (!g_mainWindowInstance)
        return;

    QString qmsg = QJniObject(msg).toString();

    // 🧵 انتقال امن به Thread اصلی UI با Qt
    QMetaObject::invokeMethod(g_mainWindowInstance, [=]() {
        g_mainWindowInstance->showMessageBox(QStringLiteral("From Kotlin: %1").arg(qmsg));
    }, Qt::QueuedConnection);
}

extern "C"
    JNIEXPORT void JNICALL
    Java_org_verya_QMLWalkieTalkie_TestBridge_nativeOnNotificationAction(JNIEnv *, jclass /*clazz*/, jstring msg)
{
    if (!g_mainWindowInstance)
        return;

    QMetaObject::invokeMethod(g_mainWindowInstance, [=]() {
        g_mainWindowInstance->showMessageBox(QStringLiteral("Action pressed"));
    }, Qt::QueuedConnection);
}
#endif

AudioBackend::AudioBackend(QObject *parent)
    : QObject{parent}
{
    CurrentID = QDateTime::currentMSecsSinceEpoch();
    _myId = 1;
    _sendToId = 255;
    _inputDeviceIndex = 0;
    _outputDeviceIndex = 0;

    acquireMulticastLock();
    makeUDPserver();
    g_mainWindowInstance = this;


    refreshAudioDevices();
    initializeAudio();

#ifdef Q_OS_ANDROID
    QJniObject jMsg = QJniObject::fromString("Hello From C++");

    QJniObject::callStaticMethod<void>(
        cls,
        "notifyCPlusPlus",
        "(Ljava/lang/String;)V",
        jMsg.object<jstring>()
        );

    qDebug()<< QString("First USE");
#endif
}

QAbstractItemModel *AudioBackend::inputDevicesModel()
{
    return &m_inputDevices;
}

QAbstractItemModel *AudioBackend::outputDevicesModel()
{
    return &m_outputDevices;
}

void AudioBackend::showMessageBox(QString message)
{
    qDebug() << message;
}

void AudioBackend::onSettingapplied(int myId, int sendToId, int inputDeviceIndex, int outputDeviceIndex)
{
    _myId = myId;
    _sendToId = sendToId;
    _inputDeviceIndex = inputDeviceIndex;
    _outputDeviceIndex = outputDeviceIndex;
    initializeAudio();
    updateNotification(QString("QMLWalkieTalkie"),QString("settings changed"));
}

void AudioBackend::onStartSend()
{
    initializeAudio();
    m_input = m_audioInput->start();
    m_audioInput->setBufferSize(BufferSize);
    connect(m_input,&QIODevice::readyRead,this,&AudioBackend::onReadInput);
}

void AudioBackend::onStopSend()
{
    m_audioInput->stop();
    disconnect(m_input,&QIODevice::readyRead,this,&AudioBackend::onReadInput);
}

void AudioBackend::acquireMulticastLock()
{
#ifdef Q_OS_ANDROID
    QJniObject context = QNativeInterface::QAndroidApplication::context();
    if (!context.isValid())
        return;

    QJniObject wifiManager = context.callObjectMethod(
        "getSystemService",
        "(Ljava/lang/String;)Ljava/lang/Object;",
        QJniObject::fromString("wifi").object<jstring>()
        );

    // wifi lock
    g_wifiLock = wifiManager.callObjectMethod(
        "createWifiLock",
        "(ILjava/lang/String;)Landroid/net/wifi/WifiManager$WifiLock;",
        3, QJniObject::fromString("MyWifiLock").object<jstring>()
        );
    g_wifiLock.callMethod<void>("acquire");

    // multicast lock
    g_multicastLock = wifiManager.callObjectMethod(
        "createMulticastLock",
        "(Ljava/lang/String;)Landroid/net/wifi/WifiManager$MulticastLock;",
        QJniObject::fromString("MyUdpLock").object<jstring>()
        );
    g_multicastLock.callMethod<void>("acquire");

    // wake lock
    QJniObject pm = context.callObjectMethod(
        "getSystemService",
        "(Ljava/lang/String;)Ljava/lang/Object;",
        QJniObject::fromString("power").object<jstring>()
        );
    g_wakeLock = pm.callObjectMethod(
        "newWakeLock",
        "(ILjava/lang/String;)Landroid/os/PowerManager$WakeLock;",
        1, QJniObject::fromString("MyWakeLock").object<jstring>()
        );
    g_wakeLock.callMethod<void>("acquire");
#endif
}

void AudioBackend::refreshAudioDevices()
{
    QStringList inputs;
    for (const auto &dev : QMediaDevices::audioInputs())
        inputs << dev.description();

    QStringList outputs;
    for (const auto &dev : QMediaDevices::audioOutputs())
        outputs << dev.description();

    m_inputDevices.setStringList(inputs);
    m_outputDevices.setStringList(outputs);

    emit devicesChanged();
}

void AudioBackend::updateNotification(QString Tittle, QString Text)
{
#ifdef Q_OS_ANDROID
    QJniObject context = QNativeInterface::QAndroidApplication::context();
    if (!context.isValid())
        return;

    int notifyId = 2025;
    QJniObject jTitle = QJniObject::fromString(Tittle);
    QJniObject jMsg   = QJniObject::fromString(Text);

    QJniObject::callStaticMethod<void>(
        cls,
        "postNotification",
        "(Landroid/content/Context;Ljava/lang/String;Ljava/lang/String;I)V",
        context.object(),
        jTitle.object<jstring>(),
        jMsg.object<jstring>(),
        notifyId);
#endif
}

void AudioBackend::makeUDPserver()
{
    Server = new QUdpSocket(this);
    //QHostAddress groupAddress("239.255.0.1");

    if(!Server->bind(QHostAddress::AnyIPv4, PortNumber,
                      QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint))
    {
        updateNotification(QString("QMLWalkieTalkie"),QString("Error : Cannot Bind Server"));
    }
    else
    {
        updateNotification(QString("QMLWalkieTalkie"),QString("Now UDP Server ready to use"));
    }
    connect(Server,&QUdpSocket::readyRead,this,&AudioBackend::onUDPReadyRead);

    Client = new QUdpSocket(this);
}

void AudioBackend::initializeAudio()
{
    m_format.setSampleRate(16000);
    m_format.setChannelCount(1); //set channels to mono
    m_format.setSampleFormat(QAudioFormat::UInt8); //set sample size to 16 bit

    m_formatSample = m_format;

    qDebug() << QString("m_format : sample rate : %1 , channel count : %2 , audio Format : %3 ")
                    .arg(m_format.sampleRate()).arg(m_format.channelCount()).arg(m_format.sampleFormat());

    m_Inputdevice = QMediaDevices::audioInputs().at(_inputDeviceIndex);
    m_Outputdevice = QMediaDevices::audioOutputs().at(_outputDeviceIndex);

    if(!m_Inputdevice.isFormatSupported(m_format))
    {
        m_format = m_Inputdevice.preferredFormat();
        qDebug() << QString("m_Inputdevice Replaced with preferredFormat(); sample rate : %1 , channel count : %2 , audio Format : %3 ")
                        .arg(m_format.sampleRate()).arg(m_format.channelCount()).arg(m_format.sampleFormat());
    }
    createAudioInput();

    if(!m_Outputdevice.isFormatSupported(m_format))
    {
        m_format = m_Inputdevice.preferredFormat();
        qDebug() << QString("m_Outputdevice Replaced with preferredFormat(); sample rate : %1 , channel count : %2 , audio Format : %3 ")
                        .arg(m_format.sampleRate()).arg(m_format.channelCount()).arg(m_format.sampleFormat());
    }
    createAudioOutput();
}

void AudioBackend::createAudioInput()
{
    m_audioInput = new QAudioSource(m_Inputdevice, m_format, this);
}

void AudioBackend::createAudioOutput()
{
    m_audioOutput = new QAudioSink(m_Outputdevice, m_format, this);
    m_output = m_audioOutput->start();
}

void AudioBackend::onUDPReadyRead()
{
    AudioPacket pack;
    if(!Server->hasPendingDatagrams())
        return;
    int size = Server->pendingDatagramSize();
    Server->readDatagram((char *)(&pack),size);
    if(pack.SenderId == CurrentID)
        return;
    if(pack.RecipientGP == 255 || pack.RecipientGP == _myId)
    {
        if(m_output != NULL)
        {
            QAudioFormat outFmt = m_audioOutput->format();

            // فقط اگه فرمت‌ها با هم فرق دارن تبدیل کنیم
            bool needConvert = false;
            if (m_formatSample.sampleFormat() != outFmt.sampleFormat() ||
                m_formatSample.channelCount() != outFmt.channelCount() ||
                m_formatSample.sampleRate()   != outFmt.sampleRate())
            {
                needConvert = true;
            }

            QByteArray outData;

            if (!needConvert)
            {
                // مستقیماً بنویس چون فرمت یکیه
                outData = QByteArray((char*)pack.Data, BufferSize);
            }
            else
            {
                // ---- تبدیل UInt8 -> Int16 ----
                QByteArray tmp;
                tmp.resize(BufferSize * 2);
                int16_t* dst = reinterpret_cast<int16_t*>(tmp.data());
                const uint8_t* src = reinterpret_cast<const uint8_t*>(pack.Data);
                for (int i = 0; i < BufferSize; ++i)
                    dst[i] = ((int16_t)src[i] - 128) << 8;

                // ---- SampleRate fix: 16000 -> 48000 (3x upsample) ----
                QByteArray upsampled;
                upsampled.resize(tmp.size() * 3);
                int16_t* up = reinterpret_cast<int16_t*>(upsampled.data());
                const int16_t* in = reinterpret_cast<const int16_t*>(tmp.data());
                int sampleCount = BufferSize;

                for (int i = 0; i < sampleCount; ++i)
                {
                    up[3*i]   = in[i];
                    up[3*i+1] = in[i];
                    up[3*i+2] = in[i];
                }

                // ---- تبدیل از Mono به Stereo اگر لازم بود ----
                if (outFmt.channelCount() == 2)
                {
                    QByteArray stereo;
                    stereo.resize(upsampled.size() * 2);
                    int16_t* out = reinterpret_cast<int16_t*>(stereo.data());
                    const int16_t* src16 = reinterpret_cast<const int16_t*>(upsampled.constData());
                    int count = sampleCount * 3;
                    for (int i = 0; i < count; ++i)
                    {
                        out[2*i]   = src16[i];
                        out[2*i+1] = src16[i];
                    }
                    outData = stereo;
                }
                else
                {
                    outData = upsampled;
                }
            }
            m_output->write(outData);
        }
        else
        {
            qDebug() << QString("Data in : %1 \r\nm_output is null").arg(size);
        }
    }
    else
    {
        qDebug() << QString("pack.Recipient : %1 - pack.Sender : %2").arg(pack.RecipientGP).arg(pack.SenderGP);
    }
}

void AudioBackend::onReadInput()
{
    if(_myId == 0)
    {
        return;
    }
    if(m_input->bytesAvailable() > BufferSize)
    {
        QByteArray Buffer = m_input->read(BufferSize);
        AudioPacket packet;
        packet.SenderId = CurrentID;
        packet.RecipientGP = _sendToId;
        packet.SenderGP = _myId;
        memcpy(packet.Data,Buffer.data(),BufferSize);
        QByteArray Datagram((char *)(&packet),sizeof(AudioPacket));
        Client->writeDatagram(Datagram,QHostAddress::Broadcast,PortNumber);
    }
}
