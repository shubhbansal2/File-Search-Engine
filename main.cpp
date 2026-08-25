#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <chrono>
#include <cctype>
#include <memory>
#include <cstdlib>

using namespace std;

template <typename K, typename V>
using Map = unordered_map<K, V>;

class FileChunkReader{
private:
    ifstream inFile;
    size_t chunkSize;
    vector<char> storage;

public:
    FileChunkReader(const string &path, size_t sizeKB){
        if (sizeKB < 256 || sizeKB > 1024)
            throw invalid_argument("Buffer must be between 256KB and 1024KB");
        chunkSize = sizeKB * 1024;
        storage.resize(chunkSize);
        inFile.open(path, ios::binary);
        if (!inFile)
            throw runtime_error("Failed to open file: " + path);
    }
    bool next(string &out){
        if (!inFile.good())
            return false;
        inFile.read(storage.data(), chunkSize);
        streamsize count = inFile.gcount();
        if (count <= 0)
            return false;
        out.assign(storage.data(), count);
        return true;
    }
};

class WordParser{
private:
    string carry;

public:
    vector<string> split(const string &text){
        vector<string> result;
        string current = carry;
        for (char c : text)
        {
            if (isalnum(c))
                current.push_back(tolower(c));
            else
            {
                if (!current.empty())
                {
                    result.push_back(current);
                    current.clear();
                }
            }
        }
        carry = current;
        return result;
    }
    string finish(){
        string tmp = carry;
        carry.clear();
        return tmp;
    }
};

class TextIndex{
private:
    string label;
    Map<string, size_t> freq;

public:
    TextIndex(const string &name) : label(name) {}
    void insert(const string &w) { ++freq[w]; }
    size_t count(const string &w) const{
        auto it = freq.find(w);
        if (it == freq.end())
            return 0;
        return it->second;
    }
    const Map<string, size_t> &data() const { return freq; }
    string name() const { return label; }
};

class QueryTask{
public:
    virtual void run() = 0;
    virtual ~QueryTask() {}
};

class SingleWordQuery : public QueryTask{
private:
    const TextIndex &idx;
    string target;

public:
    SingleWordQuery(const TextIndex &i, const string &w) : idx(i), target(w) {}
    void run() override
    {
        cout << "Version: " << idx.name() << endl;
        cout << "Frequency of '" << target << "': " << idx.count(target) << endl;
    }
};

class TopWordsQuery : public QueryTask{
private:
    const TextIndex &idx;
    size_t k;

public:
    TopWordsQuery(const TextIndex &i, size_t top) : idx(i), k(top) {}
    void run() override
    {
        cout << "Version: " << idx.name() << endl;
        vector<pair<string, size_t>> vec(idx.data().begin(), idx.data().end());
        sort(vec.begin(), vec.end(), [](auto &a, auto &b)
             { return a.second > b.second; });
        cout << "Top " << k << " words" << endl;
        for (size_t i = 0; i < min(k, vec.size()); i++)
            cout << vec[i].first << ": " << vec[i].second << endl;
    }
};

class CompareQuery : public QueryTask{
private:
    const TextIndex &a;
    const TextIndex &b;
    string word;

public:
    CompareQuery(const TextIndex &v1, const TextIndex &v2, const string &w) : a(v1), b(v2), word(w) {}
    void run() override{
        long diff = (long)a.count(word) - (long)b.count(word);
        cout << "Difference for '" << word << "' between " << a.name() << " and " << b.name() << ": " << diff << endl;
    }
};

void createIndex(const string &path, size_t bufferKB, TextIndex &index){
    FileChunkReader reader(path, bufferKB);
    WordParser parser;
    string piece;
    while (reader.next(piece))
    {
        auto tokens = parser.split(piece);
        for (const auto &t : tokens)
            index.insert(t);
    }
    string last = parser.finish();
    if (!last.empty())
        index.insert(last);
}

int main(int argc, char *argv[]){
    try{
        auto start = chrono::high_resolution_clock::now();
        string file, fileA, fileB;
        string version, v1, v2;
        string queryType, word;
        size_t bufferKB = 512;
        size_t topK = 10;

        for (int i = 1; i < argc; i++){
            string arg = argv[i];
            if (arg == "--file")
                file = argv[++i];
            else if (arg == "--file1")
                fileA = argv[++i];
            else if (arg == "--file2")
                fileB = argv[++i];
            else if (arg == "--version")
                version = argv[++i];
            else if (arg == "--version1")
                v1 = argv[++i];
            else if (arg == "--version2")
                v2 = argv[++i];
            else if (arg == "--buffer")
                bufferKB = stoul(argv[++i]);
            else if (arg == "--query")
                queryType = argv[++i];
            else if (arg == "--word")
                word = argv[++i];
            else if (arg == "--top")
                topK = stoul(argv[++i]);
        }

        unique_ptr<QueryTask> task;
        TextIndex idx(version), idx1(v1), idx2(v2);

        if (queryType == "word"){
            transform(word.begin(), word.end(), word.begin(), ::tolower);
            createIndex(file, bufferKB, idx);
            task = make_unique<SingleWordQuery>(idx, word);
        }
        else if (queryType == "top"){
            createIndex(file, bufferKB, idx);
            task = make_unique<TopWordsQuery>(idx, topK);
        }
        else if (queryType == "diff"){
            createIndex(fileA, bufferKB, idx1);
            createIndex(fileB, bufferKB, idx2);
            task = make_unique<CompareQuery>(idx1, idx2, word);
        }
        else
            throw invalid_argument("Unknown query type");

        task->run();

        auto end = chrono::high_resolution_clock::now();
        double time = chrono::duration<double>(end - start).count();

        cout << "Buffer size: " << bufferKB << "KB\n";
        cout << "Execution time: " << time << " seconds\n";
    }
    catch (exception &e){
        cerr << "Error: " << e.what() << endl;
    }
    return 0;
}