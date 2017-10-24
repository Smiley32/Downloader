#include "downloader.h"
#include "ui_downloader.h"

#include <QDebug>

downloader::downloader(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::downloader)
{
    ui->setupUi(this);

    ui->nombre->setValidator(new QIntValidator(0, 10, this));
    ui->nombreSave->setValidator(new QIntValidator(0, 10, this));
    ui->chapNombre->setValidator(new QIntValidator(0, 10, this));
    ui->minNumber->setValidator(new QIntValidator(0, 1000, this));
    ui->maxNumber->setValidator(new QIntValidator(0, 1000, this));

    ui->progressBar->hide();
    ui->actual->setText("waiting...");
    pBar = ui->progressBar;
}

downloader::~downloader()
{
    delete ui;
}

void downloader::on_nombre_editingFinished()
{
    updateNumbers(ui->nombre->text().toInt(), ui->extLineEdit->text());
}

void downloader::updateNumbers(int nbZero, QString ext)
{
    QString ex1 = "";
    for(int k = 0; k < nbZero; k++) {
        ex1 = ex1 + "0";
    }
    ex1 = ex1 + "1." + ext;
    ui->zeroLabel1->setText(ex1);
    ui->imageLabel->setText(ex1);

    ex1 = "";
    for(int k = 0; k < nbZero - 1; k++) {
        ex1 = ex1 + "0";
    }
    ex1 = ex1 + "10." + ext;
    ui->zeroLabel2->setText(ex1);

    ex1 = "";
    for(int k = 0; k < nbZero - 2; k++) {
        ex1 = ex1 + "0";
    }
    ex1 = ex1 + "100." + ext;
    ui->zeroLabel3->setText(ex1);
}

void downloader::on_extLineEdit_editingFinished()
{
    updateNumbers(ui->nombre->text().toInt(), ui->extLineEdit->text());
}

void downloader::on_nombreSave_editingFinished()
{
    int nbZero = ui->nombreSave->text().toInt();
    QString ex1 = "";
    for(int k = 0; k < nbZero; k++) {
        ex1 = ex1 + "0";
    }
    ex1 = ex1 + "1." + ui->extLineEdit->text();
    ui->zeroSaveLabel1->setText(ex1);

    ex1 = "";
    for(int k = 0; k < nbZero - 1; k++) {
        ex1 = ex1 + "0";
    }
    ex1 = ex1 + "10." + ui->extLineEdit->text();
    ui->zeroSaveLabel2->setText(ex1);

    ex1 = "";
    for(int k = 0; k < nbZero - 2; k++) {
        ex1 = ex1 + "0";
    }
    ex1 = ex1 + "100." + ui->extLineEdit->text();
    ui->zeroSaveLabel3->setText(ex1);
}

void downloader::on_download_clicked()
{
    // First picture
    i = ui->minNumber->text().toInt();
    j = ui->chapMinNumber->text().toInt();
    dl();
}

void downloader::dl()
{
    if(i > ui->maxNumber->text().toInt()) {
        if(j == ui->chapMaxNumber->text().toInt()) {
            pBar->hide();
            ui->actual->setText("Done.");

            return;
        } else {
            i = ui->minNumber->text().toInt();
            j++;
        }
    }

    manager = new QNetworkAccessManager(this);

    ui->progressBar->show();

    // int i = ui->minNumber->text().toInt();
    int nbDigit = (int)log10(i);

    QString surl = ui->url->text();

    int nbZero = ui->chapNombre->text().toInt();
    for(int k = 0; k < nbZero - (int)log10(j); k++) {
        surl = surl + "0";
    }
    surl = surl + QString("%1").arg(j) + "/";

    nbZero = ui->nombre->text().toInt();
    for(int k = 0; k < nbZero - nbDigit; k++) {
        surl = surl + "0";
    }
    surl = surl + QString("%1").arg(i) + "." + ui->extLineEdit->text();
    url = QUrl(surl);

    nbZero = ui->nombreSave->text().toInt();
    QString fileName = "";
    fileName = fileName + "Chap " + QString("%1").arg(j) + " - ";

    for(int k = 0; k < nbZero - nbDigit; k++) {
        fileName = fileName + "0";
    }
    fileName = fileName + QString("%1").arg(i) + "." + ui->extLineEdit->text();

    ui->actual->setText(fileName);

    QString filePath = ui->dossier->text() + "/" + fileName;
    qDebug() << surl << endl;

    if(QFile::exists(filePath)) {
        if (QMessageBox::question(this, tr("HTTP"),
                tr("There already exists a file called %1 in "
                "the current directory. Overwrite?").arg(fileName),
                QMessageBox::Yes|QMessageBox::No, QMessageBox::No)
                == QMessageBox::No)
                return;
        QFile::remove(fileName);
    }

    file = new QFile(filePath);
    if(!file->open(QIODevice::WriteOnly)) {
        QMessageBox::information(this, tr("HTTP"),
                      tr("Unable to save the file %1: %2.")
                      .arg(fileName).arg(file->errorString()));
        delete file;
        file = 0;
        return;
    }

    httpRequestAborted = false;

    /*progressDialog->setWindowTitle(tr("HTTP"));
    progressDialog->setLabelText(tr("Downloading %1.").arg(fileName));*/

    ui->download->setEnabled(false);

    startRequest(url);
}

void downloader::httpReadyRead()
{
    if(file) {
        file->write(reply->readAll());
    }
}

void downloader::updateDownloadProgress(qint64 bytesRead, qint64 totalBytes)
{
    if(httpRequestAborted)
        return;

    pBar->setMaximum(totalBytes);
    pBar->setValue(bytesRead);
    /*progressDialog->setMaximum(totalBytes);
    progressDialog->setValue(bytesRead);*/
}

void downloader::httpDownloadFinished()
{
    if(httpRequestAborted) {
        if(file) {
            file->close();
            file->remove();
            delete file;
            file = 0;
        }
        reply->deleteLater();
        pBar->hide();
        return;
    }

    pBar->hide();
    file->flush();
    file->close();

    QVariant redirectionTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (reply->error()) {
        file->remove();
        /*QMessageBox::information(this, tr("HTTP"),
                                 tr("Download failed: %1.")
                                 .arg(reply->errorString()));*/
        ui->download->setEnabled(true);
    } else if (!redirectionTarget.isNull()) {
        QUrl newUrl = url.resolved(redirectionTarget.toUrl());
        if (QMessageBox::question(this, tr("HTTP"),
                                  tr("Redirect to %1 ?").arg(newUrl.toString()),
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            url = newUrl;
            reply->deleteLater();
            file->open(QIODevice::WriteOnly);
            file->resize(0);
            startRequest(url);
            return;
        }
    } else {
        ui->download->setEnabled(true);
    }

    reply->deleteLater();
    reply = 0;
    delete file;
    file = 0;
    manager = 0;

    // On continue les dl
    i++;
    dl();
}

void downloader::startRequest(QUrl url)
{
    reply = manager->get(QNetworkRequest(url));

    connect(reply, SIGNAL(readyRead()), this, SLOT(httpReadyRead()));

    connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(updateDownloadProgress(qint64,qint64)));

    connect(reply, SIGNAL(finished()), this, SLOT(httpDownloadFinished()));
}

void downloader::on_chapNombre_editingFinished()
{
    int nbZero = ui->chapNombre->text().toInt();
    QString ex1 = "";
    for(int k = 0; k < nbZero; k++) {
        ex1 = ex1 + "0";
    }
    ex1 = ex1 + "1";
    ui->chapZeroLabel1->setText(ex1);

    ex1 = "";
    for(int k = 0; k < nbZero - 1; k++) {
        ex1 = ex1 + "0";
    }
    ex1 = ex1 + "10";
    ui->chapZeroLabel2->setText(ex1);

    ex1 = "";
    for(int k = 0; k < nbZero - 2; k++) {
        ex1 = ex1 + "0";
    }
    ex1 = ex1 + "100";
    ui->chapZeroLabel3->setText(ex1);
}
