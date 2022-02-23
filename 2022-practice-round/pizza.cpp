#include "heuristics.h"
#include <iostream>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;

void printIngredients(string label, const unordered_set<int>& clients, const vector<unordered_set<string>>& clientLikes) {
	cerr << label << ": " << clients.size() << endl;
	unordered_set<string> ingredients;
	for (auto person : clients) {
		for (auto ingredient : clientLikes[person]) {
			ingredients.insert(ingredient);
		}
	}
	cout << ingredients.size();
	for (auto ingredient : ingredients) {
		cout << " " << ingredient;
	}
	cout << endl;
}

int main() {

	int C; cin >> C;

	vector<unordered_set<string>> clientLikes;
	vector<unordered_set<string>> clientDislikes;
	clientLikes.reserve(C);
	clientDislikes.reserve(C);
	for (int i = 0; i < C; ++i) {
		clientLikes.push_back(unordered_set<string>());
		clientDislikes.push_back(unordered_set<string>());
	}

	for (int client = 0; client < C; ++client) {
		int L; cin >> L;
		for (int i = 0; i < L; ++i) {
			string ingredient; cin >> ingredient;
			clientLikes[client].insert(ingredient);
		}
		int D; cin >> D;
		for (int i = 0; i < D; ++i) {
			string ingredient; cin >> ingredient;
			clientDislikes[client].insert(ingredient);
		}
	}

	vector<unordered_set<int>> conflictGraph;
	conflictGraph.reserve(C);
	for (int i = 0; i < C; ++i) conflictGraph.push_back(unordered_set<int>());

	for (int i = 0; i < C; ++i) {
		for (int j = 0; j < C; ++j) {
			if (i == j) continue;
			for (auto ingredient : clientLikes[i]) {
				if (clientDislikes[j].count(ingredient) != 0) {
					conflictGraph[i].insert(j);
					conflictGraph[j].insert(i);
					break;
				}
			}
		}
	}

	unordered_set<int> mostConflictingHeuristic = removeMostConflicting(conflictGraph);
	printIngredients("Most Conflicting Heuristic", mostConflictingHeuristic, clientLikes);

	unordered_set<int> leastConflictingHeuristic = addLeastConflicting(conflictGraph);
	printIngredients("Least Conflicting Heuristic", leastConflictingHeuristic, clientLikes);

	unordered_set<int> randomResolutionHeuristic = randomResolution(conflictGraph);
	printIngredients("Random Resolution Heuristic", randomResolutionHeuristic, clientLikes);

	unordered_set<int> uniformRandomResolutionHeuristic = uniformRandomResolution(conflictGraph);
	printIngredients("Uniform Random Resolution Heuristic", uniformRandomResolutionHeuristic, clientLikes);

	unordered_set<int> leastDislikesHeuristic = leastDislikes(conflictGraph, clientDislikes);
	printIngredients("Least Dislikes Heuristic", leastDislikesHeuristic, clientLikes);

	unordered_set<int> fewestPreferencesHeuristic = fewestPreferences(conflictGraph, clientLikes, clientDislikes);
	printIngredients("Fewest Preferences Heuristic", fewestPreferencesHeuristic, clientLikes);

	return 0;
}