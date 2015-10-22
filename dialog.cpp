#include "dialog.h"
#include "ui_dialog.h"
#include <math.h>

#include <QDebug>

dialog::dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dialog)
{
    ui->setupUi(this);
    ui->pushButton->setEnabled(false);
    ui->pushButton_2->setEnabled(false);
    ui->pushButton_3->setEnabled(false);
    ui->frame->setVisible(false);
}

dialog::~dialog()
{
    delete ui;
}
void dialog::on_toolButton_clicked()
{
    inFileName = QFileDialog::getOpenFileName(this, tr("Открыть файл"),"", tr("Files (*.*)"));
    if(inFileName != "")
    {
        ui->lineEdit->setText(inFileName);
        ui->pushButton_3->setEnabled(true);
        if(inFileName.right(3) == ".tp")
        {
            ui->pushButton->setEnabled(false);
            ui->pushButton_2->setEnabled(true);
            outFileName = inFileName.left(inFileName.length()-3);
            ui->lineEdit_2->setText(outFileName);
        }
        else
        {
            ui->pushButton->setEnabled(true);
            ui->pushButton_2->setEnabled(false);
            outFileName = inFileName+".tp";
            ui->lineEdit_2->setText(outFileName);
        }
        preloadFile();
    }
}
void dialog::on_toolButton_2_clicked()
{
    outFileName = QFileDialog::getSaveFileName(this, tr("Сохранить файл"),"",tr("Teleport (*.tp)"));
    ui->lineEdit_2->setText(outFileName);
}
void dialog::on_pushButton_clicked()
{
    ui->frame->setVisible(false);
    ui->pushButton_3->setChecked(false);
    if(!(inFile.isOpen()))
    {
       preloadFile();
    }
    loadFile();
}
void dialog::on_pushButton_2_clicked()
{
    ui->frame->setVisible(false);
    ui->pushButton_3->setChecked(false);
    if(!(inFile.isOpen()))
    {
       preloadFile();
    }
    decompressed();
}
void dialog::on_pushButton_3_clicked()
{
    if(ui->frame->isVisible())
    {
        ui->frame->setVisible(false);
    }
    else
    {
        getInformation();
        ui->frame->setVisible(true);
    }
}
void dialog::preloadFile()
{
    inFile.setFileName(inFileName);
    inFile.open(QFile::ReadOnly);

    inFileSize = inFile.size();
    outFileSize = 0;

    symbols = new Character[256];
    root = NULL;
    amount = 0;
    buffer = 2<<16;

    ui->progressBar->setValue(0);
    ui->label_7->setText(QString::number(inFileSize/1000)+" KB");
    ui->label_8->setText("0 KB");
    ui->label_10->setText("0 ms");
}
void dialog::getInformation()
{
    QFile f;
    Information *i = new Information[256];
    int c;
    int fs;
    int s = buffer;
    double e = 0;
    int a = 0;
    int ai;
    double li;
    char *t = new char[buffer];

    f.setFileName(inFileName);
    f.open(QFile::ReadOnly);
    fs = f.size();

    while(f.pos() < fs)
    {
        if (fs - f.pos() < s)
        {
            s = fs - f.pos();
        }

        f.read(t,s);

        for(int j=0; j<s; j++)
        {
            c = (unsigned char)t[j];

            if(i[c].amount == 0)
            {
                i[c].c = t[j];
                a++;
            }
            i[c].amount++;
        }
    }
    for(int j = 0; j<256; j++)
    {
        if(i[j].amount !=0)
        {
            i[j].probability = 1.0*i[j].amount/fs;
            i[j].entropy = i[j].probability*(log(1.0/i[j].probability)/log(2.0));
        }
        e+=i[j].entropy;
    }
    ai = e*fs/8.0;
    li = (1.0-1.0*ai/fs)*100.0;

    ui->label_14->setText(QString::number(fs)+" B");
    ui->label_15->setText(QString::number(e));
    ui->label_16->setText(QString::number(a));
    ui->label_17->setText(QString::number(ai));
    ui->label_18->setText(QString::number(li)+" %");

    f.close();
}
void dialog::loadFile()
{
    ui->progressBar->setMaximum(1+(inFileSize/buffer)*3);

    time.start();

    int size = buffer;
    int position = 0;

    int symbol;

    char *input = new char[size];

    while(position < inFileSize)
    {
        if (inFileSize - position < size)
        {
            size = inFileSize - position;
        }

        inFile.read(input,size);

        for(int i=0; i<size; i++)
        {
            symbol = (unsigned char)input[i];

            if(symbols[symbol].amount == 0)
            {
                symbols[symbol].c = input[i];
                symbols[symbol].empty = 0;
                amount++;
            }
            symbols[symbol].amount++;
        }
        position = inFile.pos();

        ui->label_10->setText(QString::number(time.elapsed())+" ms");
        ui->progressBar->setValue(ui->progressBar->value()+1);

    }
    for(int i = 0; i<256; i++)
    {
        pointer[i] = &symbols[i];
    }

    sort(pointer,0,255);
    root = getRoot(pointer,amount);
    buildCode(root);
    compressed();
}
void dialog::sort(Character *s[], int low, int high)
{
    int i = low, j = high, x = s[(low + high) / 2]->amount;

    do
    {
        while (s[i]->amount > x)
        {
            i++;
        }
        while (s[j]->amount < x)
        {
            j--;
        }

        if(i <= j)
        {
            if (i < j)
            {
                Character *tmp = s[i];
                s[i] = s[j];
                s[j] = tmp;
            }
            i++;
            j--;
        }
    } while (i <= j);

    if (i < high)
    {
        sort(s, i, high);
    }
    if (low < j)
    {
        sort(s, low,j);
    }
}
Character *dialog::getRoot(Character *p[],int n)
{
    int a = p[n-1]->amount+p[n-2]->amount;

    Character *s = new Character;

    s->amount=a;
    s->empty=0;
    s->left=p[n-1];
    s->right=p[n-2];

    if(n==2)
    {
        return s;
    }
    else
    {
        for(int i=0; i<n; i++)
        {
            if (a>p[i]->amount)
            {
                for(int j=n-1; j>i; j--)
                {
                    p[j]=p[j-1];
                }
                p[i]=s;
                break;
            }
        }
    }
    return getRoot(p,n-1);
}
void dialog::buildCode(Character *r)
{
    if(r->left)
    {
        r->left->code = r->code;
        r->left->code.push_back(0);
        buildCode(r->left);
    }
    if(r->right)
    {
        r->right->code = r->code;
        r->right->code.push_back(1);
        buildCode(r->right);
    }
}
void dialog::compressed()
{
    QByteArray header;

    int bits = 7;
    int bytes = 0;
    unsigned char b = 0;

    int size = buffer;
    int position = 0;

    int s;
    int n;

    char *input = new char[size];
    char *output = new char[size];

    inFile.seek(0);
    outFile.setFileName(outFileName);
    outFile.open(QFile::WriteOnly);

    for(int i=0; i<256; i++)
    {
        if(symbols[i].amount != 0)
        {
            output[bytes++] = symbols[i].c;
            n = symbols[i].code.size;
            for(int j=0; j<n; j++)
            {
                output[bytes++] = symbols[i].code.value[j]+'0';
            }
            output[bytes++] = 32;
        }
    }

    header.setNum(bytes);
    header.push_back(32);
    header.push_back(QByteArray().setNum(inFileSize));
    header.push_back(32);
    header.append(output,bytes);
    outFileSize+=header.size();
    outFile.write(header);

    bytes = 0;

    while(position < inFileSize)
    {
        if (inFileSize - position < size)
        {
            size = inFileSize - position;
        }

        inFile.read(input,size);

        for(int i=0; i<size; i++)
        {
            s = (unsigned char)input[i];
            n = symbols[s].code.size;

            for(int j=0; j<n; j++)
            {
                if (bits < 0)
                {
                    output[bytes++] = b;
                    bits=7;

                    if(bytes>=size)
                    {
                        outFileSize+=bytes;
                        outFile.write(output,bytes);
                        bytes=0;
                    }
                    b=0;
                }
                b += symbols[s].code.value[j]<<bits;
                bits--;
            }
        }

        position = inFile.pos();

        ui->progressBar->setValue(ui->progressBar->value()+2);
        ui->label_10->setText(QString::number(time.elapsed())+" ms");
    }

    output[bytes] = b;
    outFile.write(output,bytes+1);
    outFileSize+=bytes;

    ui->label_10->setText(QString::number(time.elapsed())+" ms");
    ui->label_8->setText(QString::number(outFileSize/1000)+" KB");
    ui->progressBar->setValue(ui->progressBar->maximum());

    outFile.close();
    inFile.close();
}
void dialog::decompressed()
{
    time.start();

    root = new Character();
    Character *node;

    int offset = 0;
    int length = 0;
    int size = buffer;
    int bits = 8;
    int bytes = 0;
    int ibp = 0;
    int obp = 0;

    char c;

    char *input = new char[size];
    char *output = new char[size];

    inFile.seek(0);
    outFile.setFileName(outFileName);
    outFile.open(QFile::WriteOnly);

    inFile.read(input,size);

    while(input[offset++]!=32)
    {
        length = length*10 + (input[offset-1]-'0');
    }
    while(input[offset++]!=32)
    {
        outFileSize = outFileSize*10 + (input[offset-1]-'0');
    }
    for(int i = offset; i<length+offset; i++)
    {
        c = input[i];
        node = root;
        i++;
        while(input[i]!=32)
        {
            if(input[i]==48)
            {
                if(node->left == NULL)
                {
                    node->left = new Character();
                }
                node = node->left;
            }
            else if(input[i]==49)
            {
                if(node->right == NULL)
                {
                    node->right = new Character;
                }
                node = node->right;
            }
            i++;
        }
        node->c = c;
        node->empty = 0;
    }

    ui->progressBar->setMaximum(1+outFileSize/buffer);
    ibp = offset-1+length;

    while(bytes<outFileSize)
    {
        node = root;
        while(node->empty)
        {
            if (bits >= 8)
            {
                bits = 0;
                ibp++;
                if (ibp >= size)
                {
                    ibp = 0;
                    if (inFile.size() - inFile.pos() >= buffer)
                    {
                        inFile.read(input,buffer);
                    }
                    else
                    {
                        inFile.read(input,inFile.size() - inFile.pos());
                    }
                }
                c = input[ibp];
            }
            if (((c << bits) & 128) != 0)
            {
                node=node->right;
            }
            else
            {
                node=node->left;
            }
            bits++;
        }
        bytes++;
        output[obp++] = node->c;

        if(obp>=buffer)
        {
            outFile.write(output,obp);
            obp = 0;
            ui->progressBar->setValue(ui->progressBar->value()+1);
            ui->label_10->setText(QString::number(time.elapsed())+" ms");
        }
    }
    if(obp!=0)
    {
        outFile.write(output,obp);
    }

    ui->label_10->setText(QString::number(time.elapsed())+" ms");
    ui->label_8->setText(QString::number(outFileSize/1000)+" KB");
    ui->progressBar->setValue(ui->progressBar->maximum());

    outFile.close();
    inFile.close();
}
