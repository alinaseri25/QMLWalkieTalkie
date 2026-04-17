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
    qWarning() << QString("app version : %1").arg(VERSION_STRING);
    CurrentID = QDateTime::currentMSecsSinceEpoch();
    _myId = 1;
    _sendToId = 255;
    _inputDeviceIndex = 0;
    _outputDeviceIndex = 0;
    _bufferSize = 1024;

    latestVersion = VERSION_CODE;

    makeUDPserver();
    g_mainWindowInstance = this;


    refreshAudioDevices();
    initializeAudio();

    processPacketsTimer.setInterval(5);
    connect(&processPacketsTimer,&QTimer::timeout,this,&AudioBackend::onProcessPacketsTimerTimeout);
    processPacketsTimer.start();

#ifdef Q_OS_ANDROID
    QJniObject jMsg = QJniObject::fromString("Hello From C++");

    QJniObject::callStaticMethod<void>(
        cls,
        "notifyCPlusPlus",
        "(Ljava/lang/String;)V",
        jMsg.object<jstring>()
        );

    QJniObject context = QNativeInterface::QAndroidApplication::context();
    if (!context.isValid())
        return;

    QJniObject::callStaticMethod<void>(
        "org/verya/QMLWalkieTalkie/MainActivity",
        "manageScreenAndWakeLock",
        "(Landroid/content/Context;ZZ)V",
        context.object(),
        (jboolean)true,  // screenAlwaysOn
        (jboolean)true    // wakeLock
        );

    QJniObject::callStaticMethod<void>(
        "org/verya/QMLWalkieTalkie/MainActivity",
        "setDimTimeoutFromQt",
        "(J)V",
        10000
        );

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

void AudioBackend::onSettingapplied(int myId, int sendToId, int inputDeviceIndex, int outputDeviceIndex, int outputBufferSize)
{
    _myId = myId;
    _sendToId = sendToId;
    _inputDeviceIndex = inputDeviceIndex;
    _outputDeviceIndex = outputDeviceIndex;
    _bufferSize = outputBufferSize;
    initializeAudio();
    updateNotification(QString("QMLWalkieTalkie"),QString("settings changed"));
}

void AudioBackend::onStartSend()
{
#ifdef Q_OS_ANDROID
    // فعال‌سازی Echo Cancellation هنگام شروع ارسال
    QJniObject activity = QJniObject::callStaticObjectMethod(
        "org/qtproject/qt/android/QtNative",
        "activity",
        "()Landroid/app/Activity;"
        );

    if (activity.isValid()) {
        QJniObject::callStaticMethod<void>(
            "org/verya/QMLWalkieTalkie/MainActivity",
            "enableEchoCancellation",
            "(Landroid/content/Context;)V",
            activity.object()
            );
    }
#endif
    initializeAudio();
    if(!m_audioInput)
    {
        emit debugMessage(false,QString("We cannot connect to audioInput device..."));
        return;
    }
    m_input = m_audioInput->start();
    //m_audioInput->setBufferSize(BufferSize);
    connect(m_input,&QIODevice::readyRead,this,&AudioBackend::onReadInput);
}

void AudioBackend::onStopSend()
{
    if(!m_audioInput)
    {
        return;
    }
    m_audioInput->stop();
    disconnect(m_input,&QIODevice::readyRead,this,&AudioBackend::onReadInput);
#ifdef Q_OS_ANDROID
    // غیرفعال‌سازی Echo Cancellation هنگام توقف ارسال
    QJniObject::callStaticMethod<void>(
        "org/verya/QMLWalkieTalkie/MainActivity",
        "disableEchoCancellation",
        "()V"
        );
#endif
}

void AudioBackend::onSendMessage(QString _msg)
{
    if(_myId == 0)
    {
        emit debugMessage(false,QString("You're not premitted to send message\r\nplease change your ID"));
        return;
    }

    QByteArray _buffer,payload;
    QJsonObject header;
    header["type"] = TextMessage;
    header["senderID"] = QString::number(CurrentID);
    header["senderGP"] = QString::number(_myId);
    header["recipientID"] = QString::number(255);
    header["recipientGP"] = QString::number(_sendToId);
    header["ver"] = QString::number(VERSION_CODE);
    header["msg"] = _msg;
    QJsonDocument doc(header);
    DataHeader bufferheader;
    bufferheader.jsonSize = doc.toJson(QJsonDocument::Compact).size();
    bufferheader.payloadSize = payload.size();
    _buffer.append((char *)&bufferheader,sizeof(DataHeader));
    _buffer.append(doc.toJson(QJsonDocument::Compact));
    _buffer.append(payload);
    Client->writeDatagram(_buffer,multicastAddress,PortNumber);
    emit newTextMessage(QString("<p style=\"color: green;\">%1</p>").arg(_msg),QString("me"));
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

void AudioBackend::updateNotification(QString Tittle, QString Text, bool alert)
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
        "(Landroid/content/Context;Ljava/lang/String;Ljava/lang/String;IZ)V",
        context.object(),
        jTitle.object<jstring>(),
        jMsg.object<jstring>(),
        notifyId,
        alert);
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
        multicastAddress = QHostAddress("239.99.55.11");
        Server->joinMulticastGroup(multicastAddress);
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

    if(m_Inputdevice != QMediaDevices::audioInputs().at(_inputDeviceIndex))
    {
        m_Inputdevice = QMediaDevices::audioInputs().at(_inputDeviceIndex);

        if(!m_Inputdevice.isFormatSupported(m_format))
        {
            m_format = m_Inputdevice.preferredFormat();
            qDebug() << QString("m_Inputdevice Replaced with preferredFormat(); sample rate : %1 , channel count : %2 , audio Format : %3 ")
                            .arg(m_format.sampleRate()).arg(m_format.channelCount()).arg(m_format.sampleFormat());
            emit debugMessage(true,QString("m_Inputdevice Replaced with preferredFormat(); sample rate : %1 , channel count : %2 , audio Format : %3 ")
                                        .arg(m_format.sampleRate()).arg(m_format.channelCount()).arg(m_format.sampleFormat()));
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
            emit debugMessage(true,QString("m_Outputdevice Replaced with preferredFormat(); sample rate : %1 , channel count : %2 , audio Format : %3 ")
                                        .arg(m_format.sampleRate()).arg(m_format.channelCount()).arg(m_format.sampleFormat()));
        }
        createAudioOutput();
    }
}

void AudioBackend::createAudioInput()
{
    if (m_audioInput != nullptr) {
        m_audioInput->stop();
        m_audioInput->deleteLater();
        m_audioInput = nullptr;
    }

    m_audioInput = new QAudioSource(m_Inputdevice, m_format, this);
    if(!m_audioInput)
    {
        emit debugMessage(false,QString("Cannot use this AudioInput Device"));
        return;
    }

    connect(m_audioInput, &QAudioSource::stateChanged,
        this, [this](QAudio::State state)
        {
            if (state == QAudio::StoppedState)
            {
                QAudio::Error err = m_audioInput->error();

                if (err != QAudio::NoError)
                {
                    qWarning() << "Audio input error:" << err;

                    handleInputError(err);
                }
            }
        });
}

void AudioBackend::createAudioOutput()
{
    if (m_audioOutput != nullptr) {
        m_audioOutput->stop();
        m_audioOutput->deleteLater();
        m_audioOutput = nullptr;
    }

    m_audioOutput = new QAudioSink(m_Outputdevice, m_format, this);
    if(!m_audioOutput)
    {
        emit debugMessage(false,QString("Cannot use this AudioOutput Device"));
        return;
    }
    m_output = m_audioOutput->start();

    connect(m_audioOutput, &QAudioSink::stateChanged,
        this, [this](QAudio::State state)
        {
            if (state == QAudio::StoppedState)
            {
                QAudio::Error err = m_audioOutput->error();

                if (err != QAudio::NoError)
                {
                    qWarning() << "Audio output error:" << err;

                    handleOutputError(err);
                }
            }
        });
}

void AudioBackend::processBuffer()
{
    //qDebug() << "Data entered...";
    while(true)
    {
        // اگر حتی هدر کامل نداریم
        if(buffer.size() < sizeof(DataHeader))
            return;

        DataHeader *header;
        while(buffer.size() >= sizeof(DataHeader))
        {
            header = (DataHeader *)buffer.data();
            if(header->startBytes != startByte)
            {
                buffer.remove(0,1);
                continue;
            }
            break;
        }

        int packetSize = sizeof(DataHeader) + header->jsonSize + header->payloadSize;

        // اگر پکت کامل هنوز نرسیده
        if(buffer.size() < packetSize)
            return;

        // استخراج پکت
        QByteArray packet = buffer.left(packetSize);
        packets.append(packet);

        // حذف از بافر
        buffer.remove(0, packetSize);
    }
}

void AudioBackend::onUDPReadyRead()
{
    if(buffer.size() >= maxBufferSize)
    {
        buffer.clear();
        qWarning() << "Buffer overflow, cleared";
    }
    while(Server->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(Server->pendingDatagramSize());

        QHostAddress sender;
        quint16 senderPort;

        Server->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

        // اضافه کردن به بافر تجمعی
        buffer.append(datagram);
    }
    processBuffer();
}

void AudioBackend::onReadInput()
{
    if(_myId == 0)
    {
        emit debugMessage(false,QString("You're not premitted to send message\r\nplease change your ID"));
        return;
    }
    if(m_input->bytesAvailable() >= _bufferSize)
    {
        QByteArray _buffer,payload = m_input->read(_bufferSize);
        QJsonObject header;
        header["type"] = AudioMessage;
        header["senderID"] = QString::number(CurrentID);
        header["senderGP"] = QString::number(_myId);
        header["recipientID"] = QString::number(255);
        header["recipientGP"] = QString::number(_sendToId);
        header["SF"] = QString::number(m_audioOutput->format().sampleFormat());
        header["SR"] = QString::number(m_audioOutput->format().sampleRate());
        header["CC"] = QString::number(m_audioOutput->format().channelCount());
        header["ver"] = QString::number(VERSION_CODE);
        QJsonDocument doc(header);
        DataHeader bufferheader;
        bufferheader.jsonSize = doc.toJson(QJsonDocument::Compact).size();
        bufferheader.payloadSize = payload.size();
        _buffer.append((char *)&bufferheader,sizeof(DataHeader));
        _buffer.append(doc.toJson(QJsonDocument::Compact));
        _buffer.append(payload);
        Client->writeDatagram(_buffer,multicastAddress,PortNumber);
    }
}

void AudioBackend::onProcessPacketsTimerTimeout()
{
    if(packets.size() > 0)
    {
        DataHeader *bufferheader;
        //qDebug() << "packet data size : " << packets.first().size();
        bufferheader = (DataHeader*)packets.first().data();
        if(bufferheader->jsonSize > MAX_JSON_SIZE || bufferheader->payloadSize > MAX_PAYLOAD_SIZE)
        {
            packets.removeFirst();
            return;
        }
        QByteArray jsonData = packets.first().mid(sizeof(DataHeader) ,bufferheader->jsonSize);
        QByteArray payload = packets.first().mid((sizeof(DataHeader) + bufferheader->jsonSize ), bufferheader->payloadSize);
        packets.removeFirst();
        // qDebug() << "jsonData : " << jsonData;
        // qDebug() << QString("payloadSize : %1").arg(payload.size());

        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(jsonData, &err);

        if (err.error != QJsonParseError::NoError)
        {
            qWarning() << "json data : " << jsonData << "JSON parse error:" << err.errorString();
            return;
        }

        QJsonObject obj = doc.object();

        if(obj["senderID"].toString("0").toULongLong() == CurrentID)
        {
            return;
        }

        if(obj["ver"].toString("0").toULongLong() > latestVersion)
        {
            latestVersion = obj["ver"].toString("0").toULongLong();
            emit newVersionFound(latestVersion);
        }

        if(obj["recipientGP"].toString("0").toULongLong() == 255 || obj["recipientGP"].toString("0").toULongLong() == _myId)
        {
            if(obj["type"].toInt(0) == AudioMessage)
            {
                if(m_output != NULL)
                {
                    QAudioFormat outFmt = m_audioOutput->format();
                    bool needConvert = false;
                    if (obj["SF"].toString("0").toInt() != outFmt.sampleFormat() ||
                        obj["CC"].toString("0").toInt() != outFmt.channelCount() ||
                        obj["SR"].toString("0").toInt() != outFmt.sampleRate())
                    {
                        needConvert = true;
                    }
                    if (!needConvert)
                    {
                        //qDebug() << QString("Json Data : %1").arg(doc.toJson());
                        // مستقیماً بنویس چون فرمت یکیه
                    }
                    else
                    {
                        qDebug() << QString("Current sound not supported!");
                        emit debugMessage(false,QString("Current sound not supported!"));
                        return;
                    }

                    m_output->write(payload);
                }
                else
                {
                    qDebug() << QString("Data in : %1 \r\nm_output is null").arg(packets.first().size());
                    emit debugMessage(false,QString("Data in : %1 \r\nm_output is null").arg(packets.first().size()));
                }
            }
            else if(obj["type"].toInt(0) == TextMessage)
            {
                emit newTextMessage(QString("<p style=\"color: white;\">%1</p>").arg(obj["msg"].toString("")),obj["senderID"].toString("0"));
                updateNotification(QString("QMLWalkieTalkie"),QString("from : %1\r\ncontent: %2").arg(obj["senderID"].toString("0")).arg(obj["msg"].toString("")),true);
            }
        }
        else
        {
            qDebug() << QString("pack.Recipient : %1 - pack.Sender : %2").arg(obj["recipientGP"].toString("0").toULongLong()).arg(obj["senderGP"].toString("0").toULongLong());
            emit debugMessage(false,QString("pack.Recipient : %1 - pack.Sender : %2").arg(obj["recipientGP"].toString("0").toULongLong()).arg(obj["senderGP"].toString("0").toULongLong()));
        }
    }
}

void AudioBackend::handleInputError(QAudio::Error err)
{
    switch (err) {
    case QAudio::NoError:
        return;

    case QAudio::OpenError:
    case QAudio::IOError:
    case QAudio::UnderrunError:
    case QAudio::FatalError:
        qWarning() << "HandleInputError: audio input error" << err
                   << "- reinitializing audio input...";
        initializeAudio();
        break;
    }
}

void AudioBackend::handleOutputError(QAudio::Error err)
{
    switch (err) {
    case QAudio::NoError:
        return;

    case QAudio::OpenError:
    case QAudio::IOError:
    case QAudio::UnderrunError:
    case QAudio::FatalError:
        qWarning() << "HandleOutputError: audio output error" << err
                   << "- reinitializing audio output...";
        initializeAudio();
        break;
    }
}
