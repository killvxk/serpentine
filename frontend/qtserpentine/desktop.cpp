#include "desktop.h"
#include "ui_desktop.h"

#include <QtNetwork/QNetworkReply>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>

Desktop::Desktop(QString clientName, QString apiAddress, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Desktop)
{
    ui->setupUi(this);
    this->clientName = clientName;
    this->apiAddress = apiAddress;
    this->networkManager = new QNetworkAccessManager();
    this->label = new QLabel(this);
    this->label->setAlignment(Qt::AlignCenter);
    this->setCentralWidget(this->label);
    this->run();
}

void Desktop::run() {
    connect(this->networkManager, &QNetworkAccessManager::finished, this, [=](QNetworkReply *reply) {
        if (reply->error()) {
            qDebug() << reply->errorString();
            return;
        }
        QString fileBase64 = QJsonDocument::fromJson(reply->readAll()).object().value("file").toString();
        QByteArray file = QByteArray::fromBase64(fileBase64.toUtf8());
        QPixmap pixmap;
        if (pixmap.loadFromData(file, "JPG")) {
            pixmap = pixmap.scaled(this->label->size(), Qt::KeepAspectRatio);
            this->label->setPixmap(pixmap);
        } else {
            qDebug() << "Failed to load returned Desktop screenshot as JPEG";
        }
    });

    QTimer *timer = new QTimer(this);
    timer->setSingleShot(false);
    connect(timer, &QTimer::timeout, this, &Desktop::fetchScreenshot);
    timer->start(1000);

}

void Desktop::fetchScreenshot() {
    QNetworkRequest *request = new QNetworkRequest();
    request->setUrl(QUrl(this->apiAddress + "/desktop/" + this->clientName));
    this->networkManager->get(*request);
    delete request;
}

Desktop::~Desktop()
{
    delete ui;
}
