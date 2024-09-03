#include "clientfetcher.h"

#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QTimer>

ClientFetcher::ClientFetcher(QString apiAddress, QTableWidget *table, QObject *parent) : QObject(parent)
{
    this->apiAddress = apiAddress;
    this->table = table;
    this->networkManager = new QNetworkAccessManager();
    this->run();
}

void ClientFetcher::run() {
    connect(this->networkManager, &QNetworkAccessManager::finished, this, [=](QNetworkReply *reply) {
        if (reply->error()) {
            qDebug() << reply->errorString();
            return;
        }
        QString replyString = reply->readAll();
        QJsonArray replyArray = QJsonDocument::fromJson(replyString.toUtf8()).array();
        this->table->setRowCount(0);
        foreach (const QJsonValue &value, replyArray) {
            this->table->setRowCount(this->table->rowCount() + 1);
            QJsonObject object = value.toObject();
            this->table->setItem(this->table->rowCount() - 1, 0, new QTableWidgetItem(object["name"].toString()));
            this->table->setItem(this->table->rowCount() - 1, 1, new QTableWidgetItem(object["address"].toString()));
            this->table->setItem(this->table->rowCount() - 1, 2, new QTableWidgetItem(object["computerName"].toString()));
            this->table->setItem(this->table->rowCount() - 1, 3, new QTableWidgetItem(object["stubName"].toString()));
            this->table->setItem(this->table->rowCount() - 1, 4, new QTableWidgetItem(object["activeWindowTitle"].toString()));
        }
    });

    QTimer *timer = new QTimer(this);
    timer->setSingleShot(false);
    connect(timer, &QTimer::timeout, this, &ClientFetcher::fetchClients);
    timer->start(3000);
}

void ClientFetcher::fetchClients() {
    QNetworkRequest *request = new QNetworkRequest();
    request->setUrl(QUrl(this->apiAddress + "/client"));
    this->networkManager->get(*request);
    delete request;
}
