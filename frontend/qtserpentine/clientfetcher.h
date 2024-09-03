#ifndef CLIENTFETCHER_H
#define CLIENTFETCHER_H

#include <QObject>
#include <QTableWidget>
#include <QtNetwork/QNetworkAccessManager>

class ClientFetcher : public QObject
{
    Q_OBJECT
public:
    explicit ClientFetcher(QString apiAddress, QTableWidget *table, QObject *parent = nullptr);

public slots:
    void fetchClients();

private:
    QTableWidget *table;
    QString apiAddress;
    QNetworkAccessManager *networkManager;
    void run();
};

#endif // CLIENTFETCHER_H
