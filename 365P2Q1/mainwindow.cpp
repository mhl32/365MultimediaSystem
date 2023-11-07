#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<iostream>
#include<string>
#include<fstream>
#include<QImage>
#include<QLabel>
#include<QPainter>
//#include<sndfile.hh>
#include<QFileDialog>
#include<QMessageBox>
using namespace std;
//#include<stdio.h>
//#include<sndfile.h>
//#include<fftw3.h>
#include <QTextImageFormat>
#include <QImageReader>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    QString filename = QFileDialog::getOpenFileName(
        this,
        tr("Open File"),
        "C://",
        "Tiff file (*.tif *.tiff)"
        );

    // Clear the old image
    ui->label_3->setPixmap(QPixmap());
    ui->label_4->setPixmap(QPixmap());
    if(!filename.isEmpty()){
        /*
        QImageReader imageReader(filename);
        QImage image = imageReader.read();
        if(!image.isNull()){
            if(image.depth() == 24){
                QPixmap pixmap(filename);
                ui->label_3->setPixmap(pixmap);
                ui->label_3->setScaledContents(true);
            }
        }*/
        QFile file(filename);
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray headerBytes = file.read(8); // Read the first 8 bytes
            file.close();
            // Check if it's a valid TIFF file
            if (headerBytes.size() == 8 &&
                (
                    (headerBytes[0] == 'I' && headerBytes[1] == 'I' &&
                    headerBytes[2] == 0x2A && headerBytes[3] == 0x00)
                    ||
                    (headerBytes[0] == 'M' && headerBytes[1] == 'M' &&
                    headerBytes[2] == 0x00 && headerBytes[3] == 0x2A)
                                            )){
                QImage image(filename);
                qDebug() << "Image Depth:" << image.depth();
                qDebug() << "Image Depth:" << image.format();

                if (image.depth() != 32 || image.format() != QImage::Format_RGB32 || image.isGrayscale()) {
                    QMessageBox::warning(this, tr("Invalid TIFF File"), tr("The TIFF file is not 24-bit RGB color."));
                } else {
                    QPixmap pixmap(filename);
                    colorImage = image;
                    ui->label_3->setPixmap(pixmap);
                    ui->label_3->setScaledContents(true);
                    //convert image to grayscaleImage
                    QImage gImage(image.size(), QImage::Format_Grayscale8);
                    qDebug() << "Image height:" << image.height();
                    qDebug() << "Image width:" << image.width();
                    for (int x = 0; x < image.width(); x++){
                        for (int y = 0; y < image.height(); y++){
                            QColor pixelColor = image.pixelColor(x,y);
                            int grayValue = qGray(pixelColor.red(), pixelColor.green(), pixelColor.blue());
                            gImage.setPixelColor(x, y, QColor(grayValue, grayValue, grayValue));
                        }
                    }
                    grayImage = gImage;
                    QPixmap pixmap2 = QPixmap::fromImage(gImage);
                    ui->label_4->setPixmap(pixmap2);
                    ui->label_4->setScaledContents(true);
                    currentStep = 1;

                }
            } else {
                QMessageBox::warning(this, tr("Invalid File"), tr("Invalid .tiff file!"));
            }
        } else {
            QMessageBox::critical(this, tr("File Error"), tr("Error opening the .tiff file!"));
        }
    }
}

/*
    string tiffFile = filename.toStdString();
    ifstream file(tiffFile, ios::binary);

    if (file.is_open()) {
        // Read the .tiff header
        uint16_t byteOrder;
        file.read(reinterpret_cast<char*>(&byteOrder), sizeof(byteOrder));
        // Read the next two bytes (magic number field)
        uint16_t magicNumber;
        file.read(reinterpret_cast<char*>(&magicNumber), sizeof(magicNumber));

        qDebug() << "Byte Order: " << byteOrder;
        qDebug() << "Magic Number: " << magicNumber;

        // Check byte order and magic number
        if ((byteOrder == 0x4949 || byteOrder == 0x4D4D) && magicNumber == 0x002A) {
            QPixmap pixmap(filename);
            ui->label_3->setPixmap(pixmap);
            ui->label_3->setScaledContents(true);

        } else {
            QMessageBox::warning(this, tr("Invalid File"), tr("Invalid .tiff file!"));
        }

        // Close the file
        file.close();
    } else {
        QMessageBox::critical(this, tr("File Error"), tr("Error opening the .tiff file!"));
    }
*/
    //QMessageBox::information(this, tr("File Name"), filename);
    //"All files (*.*);;Wav file (*.wav);;Tif file (*.tif)"




void MainWindow::on_pushButton_2_clicked()
{
    QApplication::quit();
}


void MainWindow::on_pushButton_3_clicked()
{
    if(!colorImage.isNull() && !grayImage.isNull()){
        if (currentStep == 1){
            qDebug() << "step2" ;
            if(!fiftyColor.isNull() && !fiftyGray.isNull()){
                ui->label_3->setPixmap(QPixmap());
                QPixmap pixmap1 = QPixmap::fromImage(fiftyColor);
                ui->label_3->setPixmap(pixmap1);
                ui->label_3->setScaledContents(true);

                ui->label_4->setPixmap(QPixmap());
                QPixmap pixmap2 = QPixmap::fromImage(fiftyGray);
                ui->label_4->setPixmap(pixmap2);
                ui->label_4->setScaledContents(true);
            }
            else{
                fiftyColor = colorImage;
                for (int x = 0; x < fiftyColor.width(); x++){
                    for (int y = 0; y < fiftyColor.height(); y++){
                        QColor pixelColor = fiftyColor.pixelColor(x, y);
                        int newRed = int(pixelColor.red()*0.5);
                        int newGreen = int(pixelColor.green()*0.5);
                        int newBlue = int(pixelColor.blue()*0.5);
                        QColor newColor(newRed, newGreen, newBlue);
                        fiftyColor.setPixelColor(x, y, newColor);
                    }
                }
                ui->label_3->setPixmap(QPixmap());
                QPixmap pixmap1 = QPixmap::fromImage(fiftyColor);
                ui->label_3->setPixmap(pixmap1);
                ui->label_3->setScaledContents(true);


                //convert image to grayscaleImage
                QImage fiftyGray(fiftyColor.size(), QImage::Format_Grayscale8);
                for (int x = 0; x < fiftyColor.width(); x++){
                    for (int y = 0; y < fiftyColor.height(); y++){
                        QColor pixelColor = fiftyColor.pixelColor(x,y);
                        int grayValue = qGray(pixelColor.red(), pixelColor.green(), pixelColor.blue());
                        fiftyGray.setPixelColor(x, y, QColor(grayValue, grayValue, grayValue));
                    }
                }


                ui->label_4->setPixmap(QPixmap());
                QPixmap pixmap2 = QPixmap::fromImage(fiftyGray);
                ui->label_4->setPixmap(pixmap2);
                ui->label_4->setScaledContents(true);
            }
            currentStep += 1;
        }
        else if(currentStep == 2){
            qDebug() << "step3" ;

            int ditherMatrix[2][2] = {
                { 0, 2 },
                { 3, 1 }
            };

            /*
            int ditherMatrix[4][4] = {
                {0, 8, 2, 10},
                {12, 4, 14, 6},
                {3, 11, 1, 9},
                {15, 7, 13, 5}
            };
*/
            QImage ditherGray(grayImage.size(), QImage::Format_Mono);
            for (int x = 0; x < grayImage.width(); x++){
                for (int y = 0; y < grayImage.height(); y++){
                    int grayValue = qGray(grayImage.pixel(x, y));
                    int mapValue = grayValue * 5 / 256;
                    int ditherValue = ditherMatrix[y % 2][x % 2];
                    if (mapValue + ditherValue > 1){
                        ditherGray.setPixel(x, y, 1);
                    } else {
                        ditherGray.setPixel(x, y, 0);
                    }
                }
            }

            ui->label_3->setPixmap(QPixmap());
            QPixmap pixmap1 = QPixmap::fromImage(grayImage);
            ui->label_3->setPixmap(pixmap1);
            ui->label_3->setScaledContents(true);

            ui->label_4->setPixmap(QPixmap());
            QPixmap pixmap2 = QPixmap::fromImage(ditherGray);
            ui->label_4->setPixmap(pixmap2);
            ui->label_4->setScaledContents(true);


            currentStep += 1;
        }
        else if(currentStep == 3){
            qDebug() << "step4" ;
            int countRed[256] = {0};
            int countGreen[256] = {0};
            int countBlue[256] = {0};
            QImage autoImage = colorImage;
            for (int x = 0; x < autoImage.width(); x++){
                for (int y = 0; y < autoImage.height(); y++){
                    QColor pixel = autoImage.pixelColor(x, y);
                    countRed[pixel.red()]++;
                    countGreen[pixel.green()]++;
                    countBlue[pixel.blue()]++;
                }
            }
            int cdfRed[256];
            int cdfGreen[256];
            int cdfBlue[256];
            cdfRed[0] = countRed[0];
            cdfGreen[0] = countGreen[0];
            cdfBlue[0] = countBlue[0];
            for (int i = 1; i < 256; i++) {
                cdfRed[i] = cdfRed[i - 1] + countRed[i];
                cdfGreen[i] = cdfGreen[i - 1] + countGreen[i];
                cdfBlue[i] = cdfBlue[i - 1] + countBlue[i];
            }
            int cdfMinRed = cdfRed[0];
            int cdfMinGreen = cdfGreen[0];
            int cdfMinBlue = cdfBlue[0];
            int cdfMaxRed = cdfRed[255];
            int cdfMaxGreen = cdfGreen[255];
            int cdfMaxBlue = cdfBlue[255];

            for (int y = 0; y < autoImage.height(); y++) {
                for (int x = 0; x < autoImage.width(); x++) {
                    QColor pixelColor = autoImage.pixelColor(x, y);
                    int newRed = (cdfRed[pixelColor.red()] - cdfMinRed) * 255 / (cdfMaxRed - cdfMinRed);
                    int newGreen = (cdfGreen[pixelColor.green()] - cdfMinGreen) * 255 / (cdfMaxGreen - cdfMinGreen);
                    int newBlue = (cdfBlue[pixelColor.blue()] - cdfMinBlue) * 255 / (cdfMaxBlue - cdfMinBlue);

                    pixelColor.setRgb(newRed, newGreen, newBlue);
                    autoImage.setPixelColor(x, y, pixelColor);
                }
            }


            /*
            for (int x = 0; x < autoImage.width(); x++){
                for (int y = 0; y < autoImage.height(); y++){
                    QColor pixel = autoImage.pixelColor(x, y);
                    int newRed = (int)((pixel.red() - minVal) * 255.0 / (maxVal - minVal));
                    int newGreen = (int)((pixel.green() - minVal) * 255.0 / (maxVal - minVal));
                    int newBlue = (int)((pixel.blue() - minVal) * 255.0 / (maxVal - minVal));

                    newRed = qBound(0, newRed, 255);
                    newGreen = qBound(0, newGreen, 255);
                    newBlue = qBound(0, newBlue, 255);

                    //pixel.setRgb(newGray);
                    //pixel.setRed(grayValue);
                    //pixel.setRed(grayValue);
                    //pixel.setRed(grayValue);
                    pixel.setRgb(newRed, newGreen, newBlue);
                    autoImage.setPixelColor(x, y, pixel);
                }
            }*/

            ui->label_3->setPixmap(QPixmap());
            QPixmap pixmap1 = QPixmap::fromImage(colorImage);
            ui->label_3->setPixmap(pixmap1);
            ui->label_3->setScaledContents(true);

            ui->label_4->setPixmap(QPixmap());
            QPixmap pixmap2 = QPixmap::fromImage(autoImage);
            ui->label_4->setPixmap(pixmap2);
            ui->label_4->setScaledContents(true);

            currentStep += 1;
        }
        else{
            qDebug() << "step1" ;
            ui->label_3->setPixmap(QPixmap());
            ui->label_4->setPixmap(QPixmap());


            ui->label_3->setPixmap(QPixmap::fromImage(colorImage));
            ui->label_3->setScaledContents(true);


            ui->label_4->setPixmap(QPixmap::fromImage(grayImage));
            ui->label_4->setScaledContents(true);

            currentStep = 1;
        }

    } else{
        QMessageBox::warning(this, tr("No Image"), tr("Please open an image to process"));
    }
}

