#ifndef DESKTOP_H
#define DESKTOP_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QLabel>

namespace Ui {
class Desktop;
}

class Desktop : public QMainWindow
{
    Q_OBJECT

public:
    explicit Desktop(QString clientName, QString apiAddress, QWidget *parent = nullptr);
    ~Desktop();

public slots:
    void fetchScreenshot();

private:
    Ui::Desktop *ui;
    QString clientName;
    QString apiAddress;
    QNetworkAccessManager *networkManager;
    QLabel *label;
    void run();
};

#endif // DESKTOP_H
