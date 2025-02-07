#include<iostream>
#include<fstream>
#include<string>
#include<vector>
#include<stack>
#include<queue>

using namespace std;

class btree {
public:
    fstream bfile;

    btree() {
        ok = false;
    };

    ~btree() {
        bfile.close();
    };

    vector<int> findblock(int key);

    void creation(int blocksize, const char *filename);

    void insertion(int key, int value);

    int insertleaf(vector<int> curblock, int ket, int value);

    int insertnonleaf(int key, int value);

    void readHeader(const char *filename);

    vector<pair<int, int>> RangeSearch(int strat, int end);

    int PointSearch(int key);

    vector<int> print(int D);

    bool ok; //path를 저장할지 말지 정하는 변수
    int blocksize;
    int rootID;
    int Depth;
    int totalnode;
    stack<int> path;
};

vector<int> btree::findblock(int key) {
    if (ok) path.push(rootID);
    int findDepth = 1;
    vector<int> cur(blocksize / sizeof(int));
    bfile.seekg(12 + (rootID - 1) * blocksize, ios::beg);
    bfile.read(reinterpret_cast<char *>(&cur[0]), blocksize);

    while (Depth >= findDepth) {
        int nextBIDptr = 0;

        for (int i = 0; i < (blocksize - 4) / 8; i++) {
            if (cur[2 * i + 1] == 0)break;
            if (cur[i * 2 + 1] > key)break;
            nextBIDptr += 2;
        }

        if (ok) {
            path.push(cur[nextBIDptr]);
        }
        bfile.seekg(12 + (cur[nextBIDptr] - 1) * blocksize, ios::beg);
        bfile.read(reinterpret_cast<char *>(&cur[0]), blocksize);


        findDepth++;
    }

    return cur;
}

void btree::creation(int blocksize, const char *filename) {
    this->bfile.open(filename, ios::binary | ios::out);
    this->blocksize = blocksize;
    vector<int> header{blocksize, 0, 0};
    bfile.write(reinterpret_cast<char *>(&header[0]), 12);
}

int btree::insertleaf(vector<int> curblock, int key, int value) {
    int M = (blocksize - 4) / 8; // block안에 들어갈수있는 총 데이터의 개수
    bool spilt = true;
    vector<pair<int, int>> result;


    for (int i = 0; i < M; i++) {
        if (curblock[2 * i] == 0) {
            spilt = false;
            break;
        }
        result.push_back({curblock[2 * i], curblock[2 * i + 1]});
    }

    result.push_back({key, value});
    std::sort(result.begin(), result.end());


    if (spilt) { // spilt을 할때 오른쪽 노드에 더 많은 데이터를 할당한다.
        totalnode += 1;
        vector<int> left; // 왼쪽 노드
        vector<int> right; //오른쪽 노드

        for (int i = 0; i <= M; i++) {
            if (((M + 1) / 2) > i) { // 왼쪽 노드에 저장될 값들
                left.push_back(result[i].first);
                left.push_back(result[i].second);
            } else {
                right.push_back(result[i].first);
                right.push_back(result[i].second);
            }
        }
        for (int i = left.size(); i < ((blocksize - 4) / 4); i++) {
            left.push_back(0);
        }
        for (int i = right.size(); i < ((blocksize - 4) / 4); i++) {
            right.push_back(0);
        }
        left.push_back(totalnode);
        right.push_back(curblock[(blocksize / 4) - 1]);

        bfile.seekp(12 + (path.top() - 1) * blocksize, ios::beg);
        bfile.write(reinterpret_cast<char *>(&left[0]), blocksize);
        bfile.seekp(12 + (totalnode - 1) * blocksize, ios::beg);
        bfile.write(reinterpret_cast<char *>(&right[0]), blocksize);


        return right[0];
    } else {   // spilt을 하지 않을때

        for (int i = result.size(); i < M; i++) {
            result.push_back({0, 0});
        }
        for (int i = 0; i < M; i++) {
            curblock[2 * i] = result[i].first;
            curblock[2 * i + 1] = result[i].second;
        }
        bfile.seekp(12 + (path.top() - 1) * blocksize, ios::beg);
        bfile.write(reinterpret_cast<char *>(&curblock[0]), blocksize);


        return -1;
    }

}

int btree::insertnonleaf(int BID, int key) {
    int M = (blocksize - 4) / 8; // block안에 들어갈수있는 총 데이터의 개수
    bool spilt = true;
    vector<pair<int, int>> result;

    vector<int> curblock(blocksize / 4);
    bfile.seekg(12 + (path.top() - 1) * blocksize, ios::beg);
    bfile.read(reinterpret_cast<char *>(&curblock[0]), blocksize);


    for (int i = 0; i < M; i++) {
        if (curblock[2 * i + 1] == 0) {
            spilt = false;
            break;
        }
        result.push_back({curblock[2 * i + 1], curblock[2 * i + 2]});
    }
    result.push_back({key, BID});
    std::sort(result.begin(), result.end());
    if (spilt) {
        totalnode += 1;
        vector<int> left;
        vector<int> right;
        left.push_back(curblock[0]);
        right.push_back(result[(M + 1) / 2].second);
        for (int i = 0; i <= M; i++) {
            if ((M + 1) / 2 > i) { //왼쪽 노드에 split
                left.push_back(result[i].first);
                left.push_back(result[i].second);
            } else if ((M + 1) / 2 + 1 <= i) {
                right.push_back(result[i].first);
                right.push_back(result[i].second);
            }
        }
        for (int i = left.size(); i < blocksize / 4; i++) {
            left.push_back(0);
        }
        for (int i = right.size(); i < blocksize / 4; i++) {
            right.push_back(0);
        }

        bfile.seekp(12 + (path.top() - 1) * blocksize, ios::beg);
        bfile.write(reinterpret_cast<char *>(&left[0]), blocksize);
        bfile.seekp(12 + (totalnode - 1) * blocksize, ios::beg);
        bfile.write(reinterpret_cast<char *>(&right[0]), blocksize);

        return result[((M + 1) / 2)].first;

    } else {
        for (int i = result.size(); i < M; i++) {
            result.push_back({0, 0});
        }
        for (int i = 0; i < M; i++) {
            curblock[2 * i + 1] = result[i].first;
            curblock[2 * i + 2] = result[i].second;
        }
        bfile.seekp(12 + (path.top() - 1) * blocksize, ios::beg);
        bfile.write(reinterpret_cast<char *>(&curblock[0]), blocksize);

        return -1;
    }
}

void btree::insertion(int key, int value) {
    if (totalnode == 0) {
        totalnode += 1;
        Depth = 0;
        rootID = 1;
        vector<int> cur(blocksize / sizeof(int));
        cur[0] = key;
        cur[1] = value;
        bfile.seekp(12 + (totalnode - 1) * blocksize, ios::beg);
        bfile.write(reinterpret_cast<char *>(&cur[0]), blocksize);
        bfile.seekp(4, ios::beg);
        bfile.write(reinterpret_cast<char *>(&rootID), 4);
    } else {
        vector<int> curleafblock(blocksize / sizeof(int));
        ok = true;
        curleafblock = findblock(key);
        ok = false;
        int spilt = insertleaf(curleafblock, key, value);
        path.pop();

        if (spilt == -1) {
            while (!path.empty()) {
                path.pop();
            }
        } else { //spilt이 일어났을때
            while (spilt != -1) {
                if (path.empty()) {
                    vector<int> rootblock{rootID, spilt, totalnode};
                    for (int i = rootblock.size(); i < blocksize / 4; i++) {
                        rootblock.push_back(0);
                    }
                    totalnode += 1;
                    rootID = totalnode;
                    Depth += 1;
                    bfile.seekp(12 + (rootID - 1) * blocksize, ios::beg);
                    bfile.write(reinterpret_cast<char *>(&rootblock[0]), blocksize);
                    break;
                }

                spilt = insertnonleaf(totalnode, spilt);
                path.pop();
            }
            while (!path.empty()) {
                path.pop();
            }
            bfile.seekp(4, ios::beg);
            bfile.write(reinterpret_cast<char *>(&rootID), sizeof(int));
            bfile.write(reinterpret_cast<char *>(&Depth), sizeof(int));
        }

    }
}

void btree::readHeader(const char *filename) {
    bfile.open(filename, ios::binary | ios::out | ios::in);

    bfile.seekg(0, ios::beg);
    bfile.read(reinterpret_cast<char *>(&blocksize), sizeof(int));
    bfile.read(reinterpret_cast<char *>(&rootID), sizeof(int));
    bfile.read(reinterpret_cast<char *>(&Depth), sizeof(int));

    bfile.seekg(0, ios::end);
    totalnode = bfile.tellg();

    totalnode = (totalnode - 12) / blocksize;
}

int btree::PointSearch(int key) {
    vector<int> curblock(blocksize / 4);
    curblock = findblock(key);
    for (int i = 0; i < ((blocksize / 4) - 1); i++) {
        if (curblock[i] == key)return curblock[i + 1];
    }
    return -1;
}

vector<pair<int, int>> btree::RangeSearch(int strat, int end) {
    vector<pair<int, int>> result; //key, value
    vector<int> curblock(blocksize/sizeof(int));
    curblock = findblock(strat);
    int nxt = 0;

    for (int i = 0; i < (blocksize - 4) / 8; i++) {
        if (curblock[2 * i] >= strat) {
            nxt = i;
            break;
        }
    }

    while (curblock[2 * nxt] <= end) {
        if (curblock[2 * nxt] != 0) {
            result.push_back({curblock[2 * nxt], curblock[2 * nxt + 1]});
        }
        nxt++;
        if (nxt == (blocksize - 4) / 8) {
            bfile.seekg(12 + (curblock[blocksize / 4 - 1] - 1) * blocksize, ios::beg);
            bfile.read(reinterpret_cast<char *>(&curblock[0]), blocksize);
            nxt = 0;
        }
    }

    return result;
}

vector<int> btree::print(int D){
    queue<int>level;
    vector<int>result;
    vector<int>curblock(blocksize/ sizeof(int));
    bfile.seekg(12+(rootID-1)*blocksize,ios::beg);
    bfile.read(reinterpret_cast<char*>(&curblock[0]),blocksize);

    if(D==0){
        for(int i=0; i<(blocksize-4)/8; i++){
            if(curblock[2*i+1]!=0){
                result.push_back(curblock[2*i+1]);
            }
        }
        return result;
    }
    else{
        level.push(curblock[0]);
        for(int i=0; i<(blocksize-4)/8; i++){
            if(curblock[2*i+1]==0)break;
            level.push(curblock[2*i+2]);
        }
        while(!level.empty()){
            bfile.seekg(12+(level.front()-1)*blocksize,ios::beg);
            bfile.read(reinterpret_cast<char*>(&curblock[0]),blocksize);
            level.pop();
            for(int i=0; i<(blocksize-4)/8; i++){
                if(curblock[2*i+1]==0)continue;
                result.push_back(curblock[2*i+1]);
            }
        }
        return result;
    }

}



int main(int argc, char *argv[]) {
    char command = argv[1][0];
    ofstream outfile;
    ifstream infile;

    btree *_btree = new btree();

    switch (command) {
        case 'c':
            _btree->creation(stoi(argv[3]), argv[2]);
            break;
        case 'i': {
            infile.open(argv[3]);
            _btree->readHeader(argv[2]);
            if (!infile.is_open()) {
                cerr << "오류발생 해당 파일이 존재하지않음\n";
                return -1;
            }
            string key;
            string value;
            while (getline(infile, key, '|') && getline(infile, value)) {
                _btree->insertion(stoi(key), stoi(value));
            }
            infile.close();
            break;
        }
        case 's': {
            infile.open(argv[3]);
            if (infile.fail()) {
                cerr << "해당 search 파일을 찾을 수 없습니다.\n";

                return -1;
            }
            outfile.open(argv[4]);
            _btree->readHeader(argv[2]);

            string key;

            while (getline(infile, key)) {
                int value = _btree->PointSearch(stoi(key));
                outfile<<stoi(key)<<"|"<<value<<'\n';
            }

            infile.close();
            outfile.close();
            break;
        }
        case 'r': {
            infile.open(argv[3]);
            if (infile.fail()) {
                cerr << "해당 search 파일을 찾을 수 없습니다.\n";

                return -1;
            }
            outfile.open(argv[4]);
            _btree->readHeader(argv[2]);
            string start;
            string end;
            while (getline(infile, start, '-')) {
                if (!getline(infile, end)) {
                    cerr << "txt형식이 잘못되었습니다.\n";
                    return -1;
                }
                vector<pair<int, int>> result;
                result = _btree->RangeSearch(stoi(start), stoi(end));
                for (int i = 0; i < result.size(); i++) {
                    outfile << result[i].first << "|" << result[i].second << "\t";
                }
                outfile << '\n';
            }
            infile.close();
            outfile.close();
            break;
        }
        case 'p':
            outfile.open(argv[3]);
            _btree->readHeader(argv[2]);
            for(int i=0; i<2; i++){
                vector<int> result=_btree->print(i);
                outfile<<"<"<<i<<">\n";
                for(int j=0; j<result.size()-1; j++){
                    outfile<<result[j]<<",";
                }
                outfile<<result[result.size()-1]<<'\n';
            }
            break;
    }

    delete _btree;

    return 0;

}
