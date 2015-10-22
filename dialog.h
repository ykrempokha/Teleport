#ifndef DIALOG_H
#define DIALOG_H

#include <fstream>
#include <QDialog>
#include <QFileDialog>
#include <QTime>
#include <QFile>
#include <QString>

namespace Ui {
class dialog;
}

struct Code
{
    int value[256];
    int size;

    Code()
    {
        size = 0;
    }
    void push_back(int n)
    {
        value[size++] = n;
    }
};

struct Character
{
    unsigned char c;
    int amount;
    int empty;

    Code code;

    Character *left;
    Character *right;

    Character()
    {
        empty = 1;
        amount = 0;
        left = NULL;
        right = NULL;
    }
};

struct Information
{
    unsigned char c;
    int amount;
    double probability;
    double entropy;
    Information()
    {
        amount = 0;
        entropy = 0;
    }
};

class dialog : public QDialog
{
    Q_OBJECT

public:
    explicit dialog(QWidget *parent = 0);
    ~dialog();

private slots:
    void on_toolButton_clicked();
    void on_toolButton_2_clicked();
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();

private:
    Ui::dialog *ui;
    QTime time;

    Character *symbols;
    Character *root;
    Character *pointer[256];

    QFile inFile;
    QFile outFile;

    QString inFileName;
    QString outFileName;

    int buffer;
    int amount;

    int inFileSize;
    int outFileSize;
    int progress;

    void setFile(const char *in, const char *out);
    void preloadFile();
    void loadFile();
    void sort(Character *s[], int low, int high);
    Character *getRoot(Character *p[],int n);
    void buildCode(Character *r);
    void compressed();
    void buildTable();
    void decompressed();
    void getInformation();
};

#endif // DIALOG_H
