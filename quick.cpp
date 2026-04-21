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

// Zoptymalizowany schemat partycjonowania Hoare'a (odporny na duplikaty)
int partition(vector<Movie>& arr, int low, int high) {
    double pivot = arr[low + (high - low) / 2].rating;
    int i = low - 1;
    int j = high + 1;
    
    while (true) {
        do {
            i++;
        } while (arr[i].rating < pivot);
        
        do {
            j--;
        } while (arr[j].rating > pivot);
        
        if (i >= j) return j;
        
        swap(arr[i], arr[j]);
    }
}

void quickSort(vector<Movie>& arr, int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);
        quickSort(arr, low, pi); 
        quickSort(arr, pi + 1, high);
    }
}

int main() {
    unordered_map<string, double> ratings;
    ifstream fileRatings("titleRatings.tsv");
    
    if (!fileRatings.is_open()) {
        cout << "BLAD: Nie mozna otworzyc pliku titleRasics.tsv!\n";
        return 1;
    }

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
    fileRatings.close();
    auto endSearch = high_resolution_clock::now();
    chrono::duration<double, std::milli> searchTime = endSearch - startSearch;
    cout << "Czas wczytywania ocen: " << searchTime.count() << " ms\n\n";

    vector<Movie> allMovies;
    ifstream fileData("titlebasics.tsv");
    
    if (!fileData.is_open()) {
        cout << "BLAD: Nie mozna otworzyc pliku data.tsv!\n";
        return 1;
    }

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
    fileData.close();

    if (allMovies.empty()) {
        cout << "BLAD: Nie wczytano zadnych danych.\n";
        return 1;
    }

    cout << "Wczytano filmow: " << allMovies.size() << "\n\n";
    
    vector<int> sizes = {10000, 100000, 500000, 1000000, (int)allMovies.size()};
    
    cout << setw(15) << "Rozmiar" << setw(20) << "Czas [ms]" << setw(15) << "Srednia" << setw(15) << "Mediana" << "\n";
    cout << "-----------------------------------------------------------------\n";

    for (int s : sizes) {
        if (s > allMovies.size()) s = allMovies.size();
        if (s == 0) continue;

        vector<Movie> dataSet(allMovies.begin(), allMovies.begin() + s);
        
        auto startSort = high_resolution_clock::now();
        quickSort(dataSet, 0, s - 1);
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
        string filename = "quick_sorted_" + to_string(s) + ".txt";
        ofstream outFile(filename);
        saveTree(root, outFile);
        freeTree(root);
        
        if (s == allMovies.size()) break;
    }

    return 0;
}