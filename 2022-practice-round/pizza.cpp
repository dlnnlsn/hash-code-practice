#include <iostream>
#include <unordered_set>
#include <vector>

using namespace std;

int main() {

	int C; cin >> C;

	vector<unordered_set<string>> clientLikes;
	vector<unordered_set<string>> clientDislikes;

	for (int client = 0; client < C; ++client) {
		int L; cin >> L;
		unordered_set<string> likes;
		for (int i = 0; i < L; ++i) {
			string ingredient; cin >> ingredient;
			likes.insert(ingredient);
		}
		clientLikes.push_back(likes);
		int D; cin >> D;
		unordered_set<string> dislikes;
		for (int i = 0; i < D; ++i) {
			string ingredient; cin >> ingredient;
			dislikes.insert(ingredient);
		}
		clientDislikes.push_back(dislikes);
	}

	vector<unordered_set<int>> conflictGraph;

	for (int i = 0; i < C; ++i) {
		unordered_set<int> neighbours;
		for (int j = 0; j < C; ++j) {
			for (auto it = clientLikes[i].begin(); it != clientLikes[i].end(); ++it) {
				if (clientDislikes[j].count(*it) != 0) neighbours.insert(j);
			}
			for (auto it = clientDislikes[i].begin(); it != clientDislikes[i].end(); ++it) {
				if (clientLikes[j].count(*it) != 0) neighbours.insert(j);
			}
		}
		conflictGraph.push_back(neighbours);
	}

	return 0;
}