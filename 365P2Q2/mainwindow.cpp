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
#include<QPainterPath>
#include<QtEndian>
#include<stack>
#include <queue>

struct wavHeader {
    uint8_t         ChunkID[4];        // RIFF Header
    uint32_t        ChunkSize;      // RIFF ChunkSize
    uint8_t         Format[4];        // WAVE Header
    uint8_t         SubChunk1ID[4];         // fmt Header
    uint32_t        Subchunk1Size;  // Size of the fmt chunk
    uint16_t        AudioFormat;    // Audio format 1=PCM,6=mulaw,7=alaw,257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM
    uint16_t        NumChannels;      // Number of channels 1=Mono 2=Stereo 3>= Multi
    uint32_t        SampleRate;  // Sampling Frequency in Hz
    uint32_t        ByteRate;    // bytes per second
    uint16_t        BlockAlign;     // 2=16-bit mono, 4=16-bit stereo
    uint16_t        BitsPerSample;  // Number of bits per sample
    uint8_t         Subchunk2ID[4]; // "data"  string
    uint32_t        Subchunk2Size;  // Sampled data length
};

struct HuffmanNode {
    int16_t sample;
    int frequency;
    HuffmanNode* left;
    HuffmanNode* right;
};

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

// drawWaveform version1
void drawWaveform(QLabel* label, const vector<vector<int16_t>>& audioData, int width, int height, int numChannels) {
    int channelHeight = height / numChannels;

    QPixmap waveformPixmap(width, height);
    waveformPixmap.fill(Qt::black);
    QPainter painter(&waveformPixmap);
    painter.setPen(Qt::green);  // Use green color for all channels

    QPainterPath waveformPath;

    // Find the maximum absolute value in all channels
    int maxAmplitude = 0;
    for (int ch = 0; ch < numChannels; ++ch) {
        const vector<int16_t>& audioChannelData = audioData[ch];

        // Calculate the maximum amplitude for this channel
        for (int i = 0; i < audioChannelData.size(); ++i) {
            int amplitude = abs(audioChannelData[i]);
            if (amplitude > maxAmplitude) {
                maxAmplitude = amplitude;
            }
        }
    }
    //qDebug() << "Debug: Max Amplitude = " << maxAmplitude;
    // Calculate the scaleFactor to fit the waveform within the height
    float scaleFactor = maxAmplitude > 0 ? (0.9 * channelHeight) / maxAmplitude : 1.0;
    //qDebug() << "Debug: Scale Factor = " << scaleFactor;
    for (int ch = 0; ch < numChannels; ++ch) {
        const vector<int16_t>& audioChannelData = audioData[ch];

        waveformPath.moveTo(0, ch * channelHeight + channelHeight / 2);

        // Loop through audio samples in this channel
        for (int i = 0; i < audioChannelData.size(); ++i) {
            int x = i * width / audioChannelData.size();
            int y = ch * channelHeight + channelHeight / 2 - audioChannelData[i] * scaleFactor;
            //qDebug() << "Sample[" << ch << "][" << i << "] = " << audioChannelData[i];
            waveformPath.lineTo(x, y);
        }
    }

    // Draw the waveform path
    painter.drawPath(waveformPath);

    // Scale the waveform image to fit the label
    QPixmap scaledPixmap = waveformPixmap.scaled(label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    label->setPixmap(scaledPixmap);
}

/*
HuffmanNode* buildHuffmanTree(const map<int16_t, int>& sampleCount) {
    QVector<HuffmanNode*> nodes;
    enum { INTERNAL_NODE = -1 };
    for (const auto& pair : sampleCount) {
        HuffmanNode* node = new HuffmanNode;
        node->sample = pair.first;
        node->frequency = pair.second;
        node->left = nullptr;
        node->right = nullptr;
        nodes.append(node);
    }

    if (nodes.size() == 1) {
        HuffmanNode* newNode = new HuffmanNode;
        newNode->sample = INTERNAL_NODE;
        newNode->frequency = nodes.first()->frequency;
        newNode->left = nodes.takeLast();
        newNode->right = nullptr;
        nodes.append(newNode);
    }

    while (nodes.size() > 1) {
        // Sort nodes by frequency
        sort(nodes.begin(), nodes.end(), [](const HuffmanNode* a, const HuffmanNode* b) {
            return a->frequency < b->frequency;
        });
        HuffmanNode* left = nodes.takeLast();
        HuffmanNode* right = nodes.takeLast();
        // Create a new node by combining the two nodes with the lowest frequencies
        HuffmanNode* newNode = new HuffmanNode;
        newNode->sample = INTERNAL_NODE;
        newNode->frequency = left->frequency + right->frequency;
        newNode->left = left;
        newNode->right = right;
        nodes.append(newNode);

    }

    return nodes.first(); // The root of the Huffman tree
}
*/
struct CompareHuffmanNodes {
    bool operator()(HuffmanNode* a, HuffmanNode* b) {
        return a->frequency > b->frequency;
    }
};

HuffmanNode* buildHuffmanTree(const std::map<int16_t, int>& sampleCount) {
    // Create a priority queue to hold the Huffman nodes
    priority_queue<HuffmanNode*, std::vector<HuffmanNode*>, CompareHuffmanNodes> minHeap;

    // Initialize the priority queue with leaf nodes for each sample
    for (const auto& pair : sampleCount) {
        HuffmanNode* node = new HuffmanNode;
        node->sample = pair.first;
        node->frequency = pair.second;
        node->left = nullptr;
        node->right = nullptr;
        minHeap.push(node);
    }

    // Build the Huffman tree by repeatedly merging nodes
    while (minHeap.size() > 1) {
        HuffmanNode* left = minHeap.top();
        minHeap.pop();
        HuffmanNode* right = minHeap.top();
        minHeap.pop();

        // Create a new node as a parent of the two nodes
        HuffmanNode* parent = new HuffmanNode;
        parent->sample = '$';  // Non-leaf node, so sample is set to '$'
        parent->frequency = left->frequency + right->frequency;
        parent->left = left;
        parent->right = right;

        // Add the new node back to the priority queue
        minHeap.push(parent);
    }

    // The root of the Huffman tree is the only node left in the priority queue
    return minHeap.top();
}

map<int16_t, string> huffmanCodes;


void genHuffmanCodes(const HuffmanNode* root, const std::string& code) {
    if (root) {
        if (root->sample != '$') {
            // Only assign the code to the sample when it's a leaf node.
            huffmanCodes[root->sample] = code;
        }
        genHuffmanCodes(root->left, code + "0");
        genHuffmanCodes(root->right, code + "1");
    }
}



double calculateAverageCodeWordLength(const map<int16_t, int>& sampleCount, const map<int16_t, string>& huffmanCodes) {
    double totalCodeLength = 0;
    int totalSamples = 0;

    for (const auto& pair : sampleCount) {
        int16_t sample = pair.first;
        int frequency = pair.second;
        if (huffmanCodes.find(sample) != huffmanCodes.end()) {
            const string& code = huffmanCodes.at(sample);
            totalCodeLength += static_cast<double>(frequency) * code.length();
            totalSamples += frequency;
        }
    }

    if (totalSamples > 0) {
        return static_cast<double>(totalCodeLength) / totalSamples;
    } else {
        return 0.0; // Handle the case when there are no samples or codes.
    }
}

void visualizeHuffmanTree(const HuffmanNode* root, int depth = 0) {
    if (root) {
        for (int i = 0; i < depth; ++i) {
            std::cout << "  ";  // Indent for better visualization
        }

        if (root->sample != -1) {
            std::cout << "Sample: " << root->sample << " Frequency: " << root->frequency << std::endl;
        } else {
            std::cout << "Internal Node (Frequency: " << root->frequency << ")" << std::endl;
        }

        visualizeHuffmanTree(root->left, depth + 1);
        visualizeHuffmanTree(root->right, depth + 1);
    }
}

void MainWindow::on_pushButton_clicked() {
    QString filename = QFileDialog::getOpenFileName(
        this,
        tr("Open File"),
        "C://",
        "Wav file (*.wav)"
        );

    string wavFile = filename.toStdString();
    ifstream file(wavFile, ios::binary);

    if (!file.is_open()) {
        QMessageBox::critical(this, tr("File Error"), tr("Error opening the .wav file."));
        return;
    }

    wavHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(wavHeader));

    if (string(header.ChunkID, header.ChunkID + 4) != "RIFF" ||
        string(header.Format, header.Format + 4) != "WAVE" ||
        string(header.SubChunk1ID, header.SubChunk1ID + 4) != "fmt " ||
        header.AudioFormat != 1) {
        QMessageBox::warning(this, tr("Invalid File"), tr("Invalid or unsupported .wav file format."));
        file.close();
        return;
    }
    /*
    qDebug() << "ChunkID" << string(header.ChunkID, header.ChunkID +4);
    qDebug() << "ChunkSize" << header.ChunkSize;
    qDebug() << "Format" << string(header.Format, header.Format + 4);
    qDebug() << "SubChunk1ID" << string(header.SubChunk1ID, header.SubChunk1ID + 4);
    qDebug() << "SubChunk1Size" << header.Subchunk1Size;
    qDebug() << "Audio format " << header.AudioFormat;
    qDebug() << "Num Channel " << header.NumChannels;
    qDebug() << "SampleRate " << header.SampleRate;
    qDebug() << "ByteRate " << header.ByteRate;
    qDebug() << "Bit per sample " << header.BitsPerSample;
    qDebug() << "Subchunk2ID " << string(header.Subchunk2ID, header.Subchunk2ID + 4);
*/
    bool isLittleEndian = true;
    if (header.AudioFormat == 0x1000) {
        // Big-endian, so swap endianness
        isLittleEndian = false;
        // Swap the bytes for Subchunk2Size
        header.Subchunk2Size = qFromBigEndian(header.Subchunk2Size);
        //qDebug()<<"bigEndian";
    }
    //qDebug() << "Subchunk2Size = " << header.Subchunk2Size;
    //qDebug() << "Number of Channels = " << header.NumChannels;

    uint16_t numChannels = header.NumChannels;
    uint16_t bytesPerSample = header.BitsPerSample / 8;
    uint32_t totalSamples = header.Subchunk2Size / numChannels / bytesPerSample; //

    if (numChannels != 2) {
        QMessageBox::warning(this, tr("Invalid File"), tr("Only stereo WAV files are supported."));
        file.close();
        return;
    }

    // Read the interleaved stereo audio data
    vector<vector<int16_t>> audioData(numChannels, vector<int16_t>(totalSamples));
    for (int i = 0; i < totalSamples; ++i) {
        for (int ch = 0; ch < numChannels; ++ch) {
            uint16_t sample;
            file.read(reinterpret_cast<char*>(&sample), bytesPerSample);
            if (!isLittleEndian) {
                // Swap bytes for big-endian format
                sample = (sample >> 8) | (sample << 8);
            }
            audioData[ch][i] = static_cast<int16_t>(sample);
        }
    }
    double entropy = 0.0;
    //double totalCodeLength = 0.0;
    map<int16_t, int> sampleCount;


    for (int ch = 0; ch < numChannels; ++ch) {
        for (int i = 0; i < totalSamples; ++i) {
            sampleCount[audioData[ch][i]]++;
            //qDebug() << "audioData: " << audioData[ch][i];
        }
    }

/*
    //testing~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    sampleCount.clear();
    sampleCount[0] = 5;
    sampleCount[1] = 9;
    sampleCount[2] = 12;
    sampleCount[3] = 13;
    sampleCount[4] = 16;
    sampleCount[5] = 45;
    numChannels = 1;
    totalSamples = 100;
*/
    //double AccuProb = 0;
    for (const auto& pair : sampleCount) {
        double probability = static_cast<double>(pair.second) / (numChannels * totalSamples);

        //qDebug() << "numChannels: " << numChannels;
        //qDebug() << "Sample: " << pair.first << "freq: " << pair.second;
        //qDebug() << "Prob: " << probability;
        entropy -= probability * log2(probability);
        //AccuProb += probability;


    }
   // qDebug() << "AccuProb: " << AccuProb;


    HuffmanNode* huffmanTree = buildHuffmanTree(sampleCount);

    huffmanCodes.clear();
    std::vector<int> code; // To store the Huffman code
    genHuffmanCodes(huffmanTree, "");

/*
    for (const auto& pair : huffmanCodes) {
        qDebug() << "sample: " << pair.first << "code: " << pair.second;
    }
*/

    double averageLength = calculateAverageCodeWordLength(sampleCount, huffmanCodes);
    qDebug() << "entropy = " << entropy;
    qDebug() << "averageCodeLength = " << averageLength;




/*
    vector<bool> encodedData;
    for (int ch = 0; ch < numChannels; ++ch) {
        for (int i = 0; i < totalSamples; ++i) {
            int16_t sample = audioData[ch][i];
            const string& huffmanCode = huffmanCodes[sample];
            for (char bit : huffmanCode) {
                encodedData.push_back(bit == '1');
            }
        }
    }
    ofstream outputFile("encoded_audio.bin", ios::binary);
    int bitsWritten = 0;
    if (outputFile.is_open()) {
        for (bool bit : encodedData) {
            static uint8_t byte = 0;
            byte = (byte << 1) | (bit ? 1 : 0);
            if (++bitsWritten == 8) {
                outputFile.write(reinterpret_cast<const char*>(&byte), 1);
                bitsWritten = 0;
                byte = 0;
            }
        }
        outputFile.close();
    }
*/
    //qDebug() << "totalCodeLength = " << totalCodeLength;
    //qDebug() << "averageCodeLength = " << averageCodeLength;
    int width = 800;
    int height = 400;

    // Display the waveform
    //version1
    drawWaveform(ui->label_3, audioData, width, height, numChannels);
    //drawWaveform(ui->label_3, audioData, height, numChannels);

    ui->label->setText(QString("Total Samples: %1").arg(totalSamples));
    ui->label_2->setText(QString("Sampling Frequency: %1 Hz").arg(header.SampleRate));
    ui->label_4->setText(QString("Entropy: %1 bits per sample").arg(entropy));
    ui->label_5->setText(QString("Total Code Word Length: %1 bits per sample").arg(averageLength));

    file.close();
}




void MainWindow::on_pushButton_2_clicked()
{
    QApplication::quit();
}
