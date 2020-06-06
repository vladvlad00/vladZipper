#include <Windows.h>
#include <iostream>
#include <fstream>
#include <random>
#include <string>

using namespace std;

int getSize(ifstream& f)
{
    f.seekg(0,ios::end);
    int size = f.tellg();
    f.seekg(0);
    return size;
}

bool compareFiles(ifstream& f, ifstream& g)
{
    constexpr int BUFFER_SIZE = 1024;
    char buffer1[BUFFER_SIZE], buffer2[BUFFER_SIZE];

    while (f.read(buffer1,BUFFER_SIZE))
    {
        g.read(buffer2,BUFFER_SIZE);
        for (int i=0;i<BUFFER_SIZE;i++)
            if (buffer1[i] != buffer2[i])
            {
                cout << buffer1[i] << ' ' << buffer2[i] << '\n';
                return false;
            }
    }
    g.read(buffer2,BUFFER_SIZE);
    int remaining1 = f.gcount();
    int remaining2 = g.gcount();
    f.clear();
    g.clear();
    if (remaining1 != remaining2)
        return false;
    for (int i=0;i<remaining1;i++)
        if (buffer1[i] != buffer2[i])
            return false;
    return true;
}

int main()
{
    constexpr int TESTE = 100;
    constexpr int MAX_FILE_LENGTH = 100000;
    random_device seeder;
    mt19937 rnd(seeder());
    string alfabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789\n,.-";
    int lg = alfabet.length();
    for (int i=1;i<=TESTE;i++)
    {
        string inFileName = "./inFiles/test" + to_string(i) + ".in";
        string outFileName = "./outFiles/test" + to_string(i) + ".out";
        string zippedFileName = "./zippedFiles/test" + to_string(i) + ".vlad";
        string winZippedFileName = "./winZippedFiles/test" + to_string(i) + ".zip";
        ofstream fout(inFileName);
        int fileLength = rnd() % MAX_FILE_LENGTH;
        for (int j=0;j<fileLength;j++)
            fout << alfabet[rnd()%lg];
        fout.close();

        system(("./vladZIP.exe zip " + inFileName + " " + zippedFileName).c_str());
        system(("./vladZIP.exe unzip " + zippedFileName + " " + outFileName).c_str());
        system(("powershell Compress-Archive " + inFileName + " " + winZippedFileName).c_str());

        ifstream in(inFileName,ios::in | ios::binary);
        ifstream zip(zippedFileName,ios::in | ios::binary);
        ifstream out(outFileName,ios::in | ios::binary);
        ifstream winZip(winZippedFileName,ios::in | ios::binary);

        int inSize = getSize(in);
        int zipSize = getSize(zip);
        int outSize = getSize(out);
        int winZipSize = getSize(winZip);

        cout << "Test " << i << ": ";
        if (inSize != outSize)
            cout << "Different in and out files\n";
        else
        {
            if (compareFiles(in,out))
            {
                double compression = ((double)zipSize / (double)inSize) * 100.0;
                cout << "Compression ratio: " << compression << "%, ";
                if (zipSize == winZipSize)
                    cout << "as good as Windows\n";
                else if (zipSize > winZipSize)
                    cout << "worse than Windows\n";
                else
                {
                    double aux = (((double)winZipSize / (double)zipSize) - 1.0)*100.0;
                    cout << aux << "% better than Windows\n";
                }

            }
            else
                cout << "Different in and out files\n";
        }

        in.close();
        zip.close();
        out.close();
    }
    return 0;
}