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
    //initializeAudio();
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

    // QJniObject activity = QNativeInterface::QAndroidApplication::context();
    // activity.callMethod<void>("requestPermissions",
    //                           "([Ljava/lang/String;I)V",
    //                           QJniObject::fromString("android.permission.RECORD_AUDIO").object<jstring>(),
    //                           100);
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
    m_format.setSampleFormat(QAudioFormat::Int16); //set sample size to 16 bit

    m_formatSample = m_format;

    qDebug() << QString("m_format : sample rate : %1 , channel count : %2 , audio Format : %3 ")
                    .arg(m_format.sampleRate()).arg(m_format.channelCount()).arg(m_format.sampleFormat());

    if(m_Inputdevice != QMediaDevices::audioInputs().at(_inputDeviceIndex))
    {
        m_Inputdevice = QMediaDevices::audioInputs().at(_inputDeviceIndex);


        //m_Inputdevice.supportedSampleFormats()

        if(!m_Inputdevice.isFormatSupported(m_format))
        {
            m_format = m_Inputdevice.preferredFormat();
            qDebug() << QString("m_Inputdevice Replaced with preferredFormat(); sample rate : %1 , channel count : %2 , audio Format : %3 ")
                            .arg(m_format.sampleRate()).arg(m_format.channelCount()).arg(m_format.sampleFormat());
        }
        createAudioInput();
    }

    if(m_Outputdevice != QMediaDevices::audioOutputs().at(_outputDeviceIndex))
    {
        m_Outputdevice = QMediaDevices::audioOutputs().at(_outputDeviceIndex);
        if(!m_Outputdevice.isFormatSupported(m_format))
        {
            m_format = m_Inputdevice.preferredFormat();
            qDebug() << QString("m_Outputdevice Replaced with preferredFormat(); sample rate : %1 , channel count : %2 , audio Format : %3 ")
                            .arg(m_format.sampleRate()).arg(m_format.channelCount()).arg(m_format.sampleFormat());
        }
        createAudioOutput();
    }
}

void AudioBackend::createAudioInput()
{
    m_audioInput = new QAudioSource(m_Inputdevice, m_format, this);
}

void AudioBackend::createAudioOutput()
{
    m_audioOutput = new QAudioSink(m_Outputdevice, m_format, this);
    m_audioOutput->setBufferSize(BufferSize);
    m_output = m_audioOutput->start();
}

void AudioBackend::onUDPReadyRead()
{
    //AudioPacket pack;
    if(!Server->hasPendingDatagrams())
        return;
    int size = Server->pendingDatagramSize();

    //Server->readDatagram((char *)(&pack),size);
    QByteArray buffer;
    sizeHeader *bufferheader;
    buffer.resize(size);
    Server->readDatagram(buffer.data(),size);
    //qDebug() << "RCV Buffer Size : " << buffer.size();
    bufferheader = (sizeHeader*)buffer.data();

    QByteArray jsonData = buffer.mid(sizeof(sizeHeader) ,bufferheader->jsonSize);
    QByteArray payload = buffer.mid((sizeof(sizeHeader) + bufferheader->jsonSize ), bufferheader->bufferSize);

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &err);

    if (err.error != QJsonParseError::NoError)
    {
        qWarning() << "json data : " << jsonData << "JSON parse error:" << err.errorString();
        return;
    }

    QJsonObject obj = doc.object();

    // if(pack.SenderId == CurrentID)
    //     return;
    if(obj["senderID"].toString() == QString::number(CurrentID))
    {
        return;
    }

    //if(pack.RecipientGP == 255 || pack.RecipientGP == _myId)
    if(obj["recipientGP"].toString() == QString("255") || obj["recipientGP"].toString() == QString::number(_myId))
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
                //qDebug() << QString("Json Data : %1").arg(doc.toJson());
                // مستقیماً بنویس چون فرمت یکیه
                outData = payload;//QByteArray((char*)pack.Data, BufferSize);
            }
            else
            {
                qDebug() << "Current sound not supported!";
                return;
            }
            m_output->write(outData);
            if (m_audioOutput->error() != QAudio::NoError) {
                qWarning() << "Audio Error:" << m_audioOutput->error();
            }
            if(m_audioOutput->state() != QAudio::ActiveState)
            {
                m_audioOutput->start();
            }
        }
        else
        {
            qDebug() << QString("Data in : %1 \r\nm_output is null").arg(size);
        }
    }
    else
    {
        //qDebug() << QString("pack.Recipient : %1 - pack.Sender : %2").arg(pack.RecipientGP).arg(pack.SenderGP);
        qDebug() << QString("pack.Recipient : %1 - pack.Sender : %2").arg(obj["recipientGP"].toString()).arg(obj["senderGP"].toString());
    }
}

void AudioBackend::onReadInput()
{
    if(_myId == 0)
    {
        return;
    }
    // if(m_input->bytesAvailable() > BufferSize)
    // {
    //     QByteArray Buffer = m_input->read(BufferSize);
    //     AudioPacket packet;
    //     packet.SenderId = CurrentID;
    //     packet.RecipientGP = _sendToId;
    //     packet.SenderGP = _myId;
    //     memcpy(packet.Data,Buffer.data(),BufferSize);
    //     QByteArray Datagram((char *)(&packet),sizeof(AudioPacket));
    //     Client->writeDatagram(Datagram,QHostAddress::Broadcast,PortNumber);
    // }
    if(m_input->bytesAvailable() > BufferSize)
    {
        QByteArray Buffer;// = m_input->readAll();
        QJsonObject header;
        header["senderID"] = QString::number(CurrentID);
        header["senderGP"] = QString::number(_myId);
        header["recipientID"] = QString("0");
        header["recipientGP"] = QString::number(_sendToId);
        QJsonDocument doc(header);
        sizeHeader bufferheader;
        bufferheader.jsonSize = doc.toJson(QJsonDocument::Compact).size();
        bufferheader.bufferSize = m_input->bytesAvailable();//(sizeof(sizeHeader) + bufferheader.jsonSize + m_input->bytesAvailable());
        Buffer.append((char *)&bufferheader,sizeof(sizeHeader));
        int test = Buffer.size();
        Buffer.append(doc.toJson(QJsonDocument::Compact));
        Buffer.append(m_input->read(bufferheader.bufferSize));
        Client->writeDatagram(Buffer,QHostAddress::Broadcast,PortNumber);
    }
}
