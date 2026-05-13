#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <algorithm>

using namespace std;
using namespace std::chrono;

struct RatingRecord {
    string tconst;
    double rating;
};

struct TitleRecord {
    string tconst;
    string title;
};

struct TitleNode {
    string tconst;
    string title;
    TitleNode* left;
    TitleNode* right;
    TitleNode(const string& id, const string& t) : tconst(id), title(t), left(nullptr), right(nullptr) {}
};

TitleNode* buildBalancedTitleTree(const vector<TitleRecord>& arr, int start, int end) {
    if (start > end) return nullptr;
    int mid = start + (end - start) / 2;
    TitleNode* node = new TitleNode(arr[mid].tconst, arr[mid].title);
    node->left = buildBalancedTitleTree(arr, start, mid - 1);
    node->right = buildBalancedTitleTree(arr, mid + 1, end);
    return node;
}

string searchTitle(TitleNode* root, const string& tconst) {
    TitleNode* curr = root;
    while (curr != nullptr) {
        if (tconst == curr->tconst) {
            return curr->title;
        } else if (tconst < curr->tconst) {
            curr = curr->left;
        } else {
            curr = curr->right;
        }
    }
    return "Brak tytulu w bazie";
}

void freeTitleTree(TitleNode* root) {
    if (!root) return;
    freeTitleTree(root->left);
    freeTitleTree(root->right);
    delete root;
}

void bucketSort(vector<RatingRecord>& arr) {
    int n = arr.size();
    if (n <= 0) return;
    
    vector<vector<RatingRecord>> b(101);
    
    for (int i = 0; i < n; i++) {
        int bi = arr[i].rating * 10.0;
        if (bi > 100) bi = 100;
        if (bi < 0) bi = 0;
        b[bi].push_back(move(arr[i]));
    }
    
    int index = 0;
    for (int i = 0; i < 101; i++) {
        for (size_t j = 1; j < b[i].size(); j++) {
            RatingRecord key = move(b[i][j]);
            int k = j - 1;
            while (k >= 0 && b[i][k].rating > key.rating) {
                b[i][k + 1] = move(b[i][k]);
                k--;
            }
            b[i][k + 1] = move(key);
        }
        for (size_t j = 0; j < b[i].size(); j++) {
            arr[index++] = move(b[i][j]);
        }
    }
}

void parseTitleLine(const string& line, string& tconst, string& title) {
    size_t pos1 = line.find('\t');
    if (pos1 == string::npos) return;
    size_t pos2 = line.find('\t', pos1 + 1);
    if (pos2 == string::npos) return;
    size_t pos3 = line.find('\t', pos2 + 1);
    
    tconst = line.substr(0, pos1);
    title = line.substr(pos2 + 1, pos3 == string::npos ? string::npos : pos3 - pos2 - 1);
}

void parseRatingLine(const string& line, string& tconst, string& ratingStr) {
    size_t pos1 = line.find('\t');
    if (pos1 == string::npos) return;
    size_t pos2 = line.find('\t', pos1 + 1);
    
    tconst = line.substr(0, pos1);
    ratingStr = line.substr(pos1 + 1, pos2 == string::npos ? string::npos : pos2 - pos1 - 1);
}

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    vector<TitleRecord> allTitles;
    ifstream fileData("titlebasics.tsv");
    
    if (!fileData.is_open()) {
        cout<<"ERROR";
        return 1;

    }

    cout << "Wczytywanie tytulow...\n";
    string line;
    auto startTitles = high_resolution_clock::now();
    if (getline(fileData, line)) {
        string tconst, title;
        while (getline(fileData, line)) {
            if (line.empty()) continue;
            parseTitleLine(line, tconst, title);
            allTitles.push_back({tconst, title});
        }
    }
    fileData.close();

    TitleNode* titleTreeRoot = buildBalancedTitleTree(allTitles, 0, allTitles.size() - 1);
    
    auto endTitles = high_resolution_clock::now();
    chrono::duration<double, std::milli> titlesTime = endTitles - startTitles;
    cout << "Czas wczytywania i budowy drzewa tytulow: " << titlesTime.count() << " ms\n";

    allTitles.clear();
    allTitles.shrink_to_fit();

    vector<RatingRecord> allRatings;
    ifstream fileRatings("titleRatings.tsv");
    
    if (!fileRatings.is_open()) {
        freeTitleTree(titleTreeRoot);
        return 1;
    }

    auto startSearch = high_resolution_clock::now();
    if (getline(fileRatings, line)) {
        string tconst, ratingStr;
        while (getline(fileRatings, line)) {
            if (line.empty()) continue;
            parseRatingLine(line, tconst, ratingStr);
            if (!ratingStr.empty() && ratingStr != "\\N") {
                try {
                    allRatings.push_back({tconst, stod(ratingStr)});
                } catch(...) {}
            }
        }
    }
    fileRatings.close();
    
    auto endSearch = high_resolution_clock::now();
    chrono::duration<double, std::milli> searchTime = endSearch - startSearch;
    cout << "Czas wczytywania ocen: " << searchTime.count() << " ms\n";

    if (allRatings.empty()) {
        freeTitleTree(titleTreeRoot);
        return 1;
    }

    cout << "Wczytano ocen do sortowania: " << allRatings.size() << "\n\n";
    
    vector<int> sizes = {10000, 100000, 500000, 1000000, (int)allRatings.size()};
    
    cout << setw(15) << "Rozmiar" << setw(20) << "Czas [ms]" << setw(15) << "Srednia" << setw(15) << "Mediana" << "\n";
    cout << "-----------------------------------------------------------------\n";

    for (int s : sizes) {
        if (s > allRatings.size()) s = allRatings.size();
        if (s == 0) continue;

        vector<RatingRecord> dataSet(allRatings.begin(), allRatings.begin() + s);
        
        auto startSort = high_resolution_clock::now();
        bucketSort(dataSet);
        auto endSort = high_resolution_clock::now();
        
        chrono::duration<double, std::milli> sortTime = endSort - startSort;
        
        double sum = 0;
        for (const auto& m : dataSet) sum += m.rating;
        double mean = sum / s;
        double median = dataSet[s / 2].rating;
        
        cout << setw(15) << s 
             << setw(20) << fixed << setprecision(3) << sortTime.count() 
             << setw(15) << fixed << setprecision(2) << mean 
             << setw(15) << median << "\n";
             
        string filename = "bucket_sorted_" + to_string(s) + ".txt";
        ofstream outFile(filename);
        
        for (const auto& rec : dataSet) {
            outFile << rec.rating << "\t" << searchTitle(titleTreeRoot, rec.tconst) << "\n";
        }
        
        if (s == allRatings.size()) break;
    }

    freeTitleTree(titleTreeRoot);
    return 0;
}
