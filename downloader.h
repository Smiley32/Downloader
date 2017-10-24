#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QMainWindow>
#include <math.h>
#include <QUrl>
#include <QProgressDialog>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

namespace Ui {
class downloader;
}

class downloader : public QMainWindow
{
    Q_OBJECT

public:
    explicit downloader(QWidget *parent = 0);
    ~downloader();
    void startRequest(QUrl url);

private slots:
    void on_nombre_editingFinished();

    void on_extLineEdit_editingFinished();

    void on_nombreSave_editingFinished();

    void on_download_clicked();

    void httpReadyRead();

    void httpDownloadFinished();

    void updateDownloadProgress(qint64, qint64);

    void on_chapNombre_editingFinished();

private:
    Ui::downloader *ui;
    void updateNumbers(int nbZero, QString ext);
    void dl();

    QUrl url;
    QNetworkAccessManager *manager;
    QNetworkReply *reply;
    /*QProgressDialog *progressDialog;*/
    QProgressBar *pBar;
    QFile *file;
    bool httpRequestAborted;
    qint64 fileSize;
    int i, j;
};

#endif // DOWNLOADER_H
