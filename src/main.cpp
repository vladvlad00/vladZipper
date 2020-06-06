#include <iostream>
#include <fstream>
#include <set>
#include <unordered_map>
#include <vector>
#include <cstring>
#include <stack>

using namespace std;

constexpr int BUFFER_SIZE = 1024;

struct node
{
    int inf;
    char ch;
    node *left, *right;

    node(int inf, char ch, node* left=nullptr, node* right=nullptr) : inf(inf), ch(ch), left(left), right(right)
    { }
};

struct cmp
{
    bool operator()(const node* a, const node* b) const
    {
        return (a->inf == b->inf ? a->ch < b->ch : a->inf < b->inf);
    }
};


void getFrequences(ifstream& fin, ofstream& fout, unordered_map<char,int>& fr)
{
    char readBuffer[BUFFER_SIZE];
    unsigned fileSize = 0;

    while (fin.read(readBuffer, BUFFER_SIZE))
    {
        for (char c : readBuffer)
            fr[c]++;
        fileSize += BUFFER_SIZE;
    }
    int remaining = fin.gcount();
    fileSize += remaining;
    fout.write((char*)(&fileSize),sizeof(int));
    fin.clear();
    for (int i=0;i<remaining;i++)
        fr[readBuffer[i]]++;
}

void DFS(node* root, unordered_map<char,vector<bool>>& codes, vector<bool>& bits)
{
    if (!root->left)
    {
        codes[root->ch] = bits;
        return;
    }
    bits.push_back(false);
    DFS(root->left,codes,bits);
    bits.back().flip();
    DFS(root->right,codes,bits);
    bits.pop_back();
}

int a()
{
    return 250+5-5;
}

void getCodes(const unordered_map<char, int>& fr, unordered_map<char,vector<bool>>& codes, node*&root)
{
    multiset<node*,cmp> S;
    for (auto[ch,freq] : fr)
        S.insert(new node(freq,ch));
    const int n = S.size();
    for (int i=1;i<=n-1;i++)
    {
        if (i == 250)
        {
            i = a();
        }
        auto node1 = *S.begin();
        S.erase(S.begin());
        auto node2 = *S.begin();
        S.erase(S.begin());
        S.insert(new node(node1->inf+node2->inf,0,node1,node2));
    }
    root = *S.begin();
    vector<bool> bits;
    DFS(root, codes, bits);
    /*for (auto&[ch,code] : codes)
    {
        cout << ch << " : ";
        for (auto i : code)
            cout << i;
        cout << '\n';
    }*/
}

void writeHuffmanTree(ofstream& fout, node* root, char* writeBuffer, int& pos, int& bitPos)
{
    if (root->left != nullptr)
    {
        bitPos++;
        if (bitPos == 8)
        {
            pos++;
            bitPos = 0;
        }
        if (pos == BUFFER_SIZE)
        {
            fout.write(writeBuffer,BUFFER_SIZE);
            pos = 0;
            memset(writeBuffer,0,BUFFER_SIZE);
        }
        writeHuffmanTree(fout,root->left,writeBuffer,pos,bitPos);
        writeHuffmanTree(fout,root->right,writeBuffer,pos,bitPos);
    }
    else
    {
        writeBuffer[pos] |= (1 << bitPos);
        bitPos++;
        if (bitPos == 8)
        {
            pos++;
            bitPos = 0;
        }
        if (pos == BUFFER_SIZE)
        {
            fout.write(writeBuffer,BUFFER_SIZE);
            pos = 0;
            memset(writeBuffer,0,BUFFER_SIZE);
        }
        for (int i=0;i<8;i++)
        {
            if ((root->ch >> i) & 1)
                writeBuffer[pos] |= (1 << bitPos);
            bitPos++;
            if (bitPos == 8)
            {
                pos++;
                bitPos = 0;
            }
            if (pos == BUFFER_SIZE)
            {
                fout.write(writeBuffer,BUFFER_SIZE);
                pos = 0;
                memset(writeBuffer,0,BUFFER_SIZE);
            }
        }
    }
}

void writeCompressedFile(ifstream& fin, ofstream& fout, unordered_map<char,vector<bool>>& codes)
{
    fin.seekg(0);
    char readBuffer[BUFFER_SIZE] = {0};
    char writeBuffer[BUFFER_SIZE] = {0};
    int bitPos = 0;
    int pos = 0;
    while (fin.read(readBuffer, BUFFER_SIZE))
    {
        for (char c : readBuffer)
        {
            for (auto bit : codes[c])
            {
                if (bitPos == 8)
                {
                    bitPos = 0;
                    pos++;
                }
                if (pos == BUFFER_SIZE)
                {
                    fout.write(writeBuffer,BUFFER_SIZE);
                    pos = 0;
                    memset(writeBuffer,0,BUFFER_SIZE);
                }
                if (bit)
                    writeBuffer[pos] |= (1<<bitPos);
                bitPos++;
            }
        }
    }
    int remaining = fin.gcount();
    fin.clear();
    for (int i=0;i<remaining;i++)
    {
        for (auto bit : codes[readBuffer[i]])
        {
            if (bitPos == 8)
            {
                bitPos = 0;
                pos++;
            }
            if (pos == BUFFER_SIZE)
            {
                fout.write(writeBuffer,BUFFER_SIZE);
                pos = 0;
                memset(writeBuffer,0,BUFFER_SIZE);
            }
            writeBuffer[pos] |= (bit * (1<<bitPos));
            bitPos++;
        }
    }
    fout.write(writeBuffer,pos+1);
}

void zip(ifstream& fin, ofstream& fout)
{
    /* Format of the compressed file:
     * First 4 bytes : the number of bytes compressed
     * The Huffman Tree (0 is internal node, 1 is leaf and the next 8 bits describe the character compressed)
     * The compressed file
     */

    unordered_map<char, int> fr;

    getFrequences(fin,fout,fr);

    unordered_map<char, vector<bool>> codes;
    node* root;
    getCodes(fr,codes,root);

    char writeBuffer[BUFFER_SIZE] = {0};
    int pos = 0;
    int bitPos = 0;
    writeHuffmanTree(fout,root,writeBuffer,pos,bitPos);
    fout.write(writeBuffer,pos+1);
    writeCompressedFile(fin,fout,codes);
}

node* getHuffmanTree(ifstream& fin)
{
    stack<node*> S;
    char readBuffer[BUFFER_SIZE];
    int lastByte = 4;

    int deCitit = 0;
    while (fin.read(readBuffer, BUFFER_SIZE))
    {
        for (char c : readBuffer)
        {
            lastByte++;
            for (int i=0;i<8;i++)
            {
                bool bit = (c >> i) & 1;
                if (deCitit)
                {
                    if (bit)
                        S.top()->ch |= (1 << (8-deCitit));
                    deCitit--;
                    if (deCitit == 0)
                    {
                        auto child = S.top();
                        S.pop();
                        if (S.empty())
                        {
                            fin.seekg(lastByte);
                            return child;
                        }
                        if (!S.top()->left)
                            S.top()->left = child;
                        else
                            S.top()->right = child;
                    }
                }
                else
                {
                    if (bit)
                        deCitit = 8;
                    S.push(new node(0,0));
                }
                while (S.top()->right)
                {
                    auto child = S.top();
                    S.pop();
                    if (S.empty())
                    {
                        fin.seekg(lastByte);
                        return child;
                    }
                    if (!S.top()->left)
                        S.top()->left = child;
                    else
                        S.top()->right = child;
                }
            }
        }
    }
    int remaining = fin.gcount();
    fin.clear();
    for (int j=0;j<remaining;j++)
    {
        lastByte++;
        char c = readBuffer[j];
        for (int i=0;i<8;i++)
        {
            bool bit = (c >> i) & 1;
            if (deCitit)
            {
                if (bit)
                    S.top()->ch |= (1 << (8-deCitit));
                deCitit--;
                if (deCitit == 0)
                {
                    auto child = S.top();
                    S.pop();
                    if (S.empty())
                    {
                        fin.seekg(lastByte);
                        return child;
                    }
                    if (!S.top()->left)
                        S.top()->left = child;
                    else
                        S.top()->right = child;
                }
            }
            else
            {
                if (bit)
                    deCitit = 8;
                S.push(new node(0,0));
            }
            while (S.top()->right)
            {
                auto child = S.top();
                S.pop();
                if (S.empty())
                {
                    fin.seekg(lastByte);
                    return child;
                }
                if (!S.top()->left)
                    S.top()->left = child;
                else
                    S.top()->right = child;
            }
        }
    }
    return nullptr; //should never reach this
}

void unzip(ifstream& fin, ofstream& fout)
{
    int fileSize;
    fin.read((char*)(&fileSize), sizeof(int));

    auto root = getHuffmanTree(fin);
    //cout << fin.tellg();

    vector<bool> bits;
    unordered_map<char, vector<bool>> codes;
    DFS(root, codes, bits);

    char readBuffer[BUFFER_SIZE];
    char writeBuffer[BUFFER_SIZE] = {0};
    int pos = 0;
    auto aux = root;
    int written = 0;
    while (written < fileSize && fin.read(readBuffer, BUFFER_SIZE))
    {
        for (char c : readBuffer)
        {
            for (int i = 0; i < 8 && written < fileSize; i++)
            {
                if ((c >> i) & 1)
                    aux = aux->right;
                else
                    aux = aux->left;
                if (!aux->left)
                {
                    writeBuffer[pos++] = aux->ch;
                    written++;
                    aux = root;
                    if (pos == BUFFER_SIZE)
                    {
                        fout.write(writeBuffer, BUFFER_SIZE);
                        pos = 0;
                    }
                }
            }
            if (written == fileSize)
                break;
        }
    }
    int remaining = fin.gcount();
    fin.clear();
    for (int j=0;j<remaining;j++)
    {
        char c = readBuffer[j];
        for (int i = 0; i < 8 && written < fileSize; i++)
        {
            if ((c >> i) & 1)
                aux = aux->right;
            else
                aux = aux->left;
            if (!aux->left)
            {
                writeBuffer[pos++] = aux->ch;
                written++;
                aux = root;
                if (pos == BUFFER_SIZE)
                {
                    fout.write(writeBuffer, BUFFER_SIZE);
                    pos = 0;
                }
            }
        }
        if (written == fileSize)
            break;
    }
    fout.write(writeBuffer,pos);
}

int main(int argc, char* argv[])
{
    if (argc < 4)
    {
        cout << "Parameters: zip/unzip, fromPath, toPath\n";
        return 0;
    }
    string op = argv[1];
    if (op != "zip" && op != "unzip")
    {
        cout << "First parameter must be zip/unzip\n";
        return 0;
    }
    string fromPath = argv[2];
    string toPath = argv[3];
    ifstream fin(fromPath, ios::in | ios::binary);
    if (!fin)
    {
        cout << "Error opening " << fromPath << '\n';
        return 0;
    }
    ofstream fout(toPath, ios::out | ios::binary);
    if (!fout)
    {
        cout << "Error opening " << toPath << '\n';
        return 0;
    }
    if (op == "zip")
        zip(fin,fout);
    else
        unzip(fin,fout);
    return 0;
}