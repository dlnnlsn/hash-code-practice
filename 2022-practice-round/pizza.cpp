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

	return 0;
}