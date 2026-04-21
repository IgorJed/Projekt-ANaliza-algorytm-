#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <chrono>
#include <iomanip>

using namespace std;
using namespace std::chrono;

struct Movie {
    string tconst;
    double rating;
    string title;
};

struct TreeNode {
    double rating;
    string title;
    TreeNode* left;
    TreeNode* right;
    TreeNode(double r, string t) : rating(r), title(t), left(nullptr), right(nullptr) {}
};

TreeNode* buildBalanced(const vector<Movie>& arr, int start, int end) {
    if (start > end) return nullptr;
    int mid = start + (end - start) / 2;
    TreeNode* node = new TreeNode(arr[mid].rating, arr[mid].title);
    node->left = buildBalanced(arr, start, mid - 1);
    node->right = buildBalanced(arr, mid + 1, end);
    return node;
}

void saveTree(TreeNode* root, ofstream& out) {
    if (!root) return;
    saveTree(root->left, out);
    out << root->rating << "\t" << root->title << "\n";
    saveTree(root->right, out);
}

void freeTree(TreeNode* root) {
    if (!root) return;
    freeTree(root->left);
    freeTree(root->right);
    delete root;
}

void merge(vector<Movie>& arr, vector<Movie>& temp, int l, int m, int r) {
    int i = l;
    int j = m + 1;
    int k = l;

    while (i <= m && j <= r) {
        if (arr[i].rating <= arr[j].rating) {
            temp[k++] = arr[i++];
        } else {
            temp[k++] = arr[j++];
        }
    }

    while (i <= m) {
        temp[k++] = arr[i++];
    }

    while (j <= r) {
        temp[k++] = arr[j++];
    }

    for (int p = l; p <= r; p++) {
        arr[p] = temp[p];
    }
}

void mergeSort(vector<Movie>& arr, vector<Movie>& temp, int l, int r) {
    if (l < r) {
        int m = l + (r - l) / 2;
        mergeSort(arr, temp, l, m);
        mergeSort(arr, temp, m + 1, r);
        merge(arr, temp, l, m, r);
    }
}

int main() {
    unordered_map<string, double> ratings;
    ifstream fileRatings("titleRatings.tsv");
    string line;
    
    auto startSearch = high_resolution_clock::now();
    if (getline(fileRatings, line)) {
        while (getline(fileRatings, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string tconst, ratingStr, votes;
            getline(ss, tconst, '\t');
            getline(ss, ratingStr, '\t');
            getline(ss, votes, '\t');
            if (!ratingStr.empty() && ratingStr != "\\N") {
                try {
                    ratings[tconst] = stod(ratingStr);
                } catch(...) {}
            }
        }
    }
    auto endSearch = high_resolution_clock::now();
    chrono::duration<double, std::milli> searchTime = endSearch - startSearch;
    cout << "Czas wczytywania i filtrowania ocen: " << searchTime.count() << " ms\n\n";

    vector<Movie> allMovies;
    ifstream fileData("titlebasics.tsv");
    if (getline(fileData, line)) {
        while (getline(fileData, line)) {
            stringstream ss(line);
            string tconst, titleType, primaryTitle;
            getline(ss, tconst, '\t');
            getline(ss, titleType, '\t');
            getline(ss, primaryTitle, '\t');
            
            auto it = ratings.find(tconst);
            if (it != ratings.end()) {
                allMovies.push_back({tconst, it->second, primaryTitle});
            }
        }
    }

    vector<int> sizes = {10000, 100000, 500000, 1000000, (int)allMovies.size()};
    
    cout << setw(15) << "Rozmiar" << setw(20) << "Czas [ms]" << setw(15) << "Srednia" << setw(15) << "Mediana" << "\n";
    cout << "-----------------------------------------------------------------\n";

    for (int s : sizes) {
        if (s > allMovies.size()) s = allMovies.size();
        if (s == 0) continue;

        vector<Movie> dataSet(allMovies.begin(), allMovies.begin() + s);
        vector<Movie> temp(s);
        
        auto startSort = high_resolution_clock::now();
        mergeSort(dataSet, temp, 0, s - 1);
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
             
        TreeNode* root = buildBalanced(dataSet, 0, s - 1);
        string filename = "merge_sorted_" + to_string(s) + ".txt";
        ofstream outFile(filename);
        saveTree(root, outFile);
        freeTree(root);
    }

    return 0;
}