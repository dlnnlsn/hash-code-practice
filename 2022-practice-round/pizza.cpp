#include <iostream>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;

unordered_set<int> randomResolution(const vector<unordered_set<int>>& graph) {
	random_device dev;
	mt19937 gen(dev());
	uniform_int_distribution<int> random_bool(0, 1);

	unordered_set<int> satisfied;
	for (int i = 0; i < graph.size(); ++i) satisfied.insert(i);
	bool hasConflict = true;
	while (hasConflict) {
		hasConflict = false;
		for (auto a : satisfied) {
			for (auto b : satisfied) {
				if (graph[a].count(b) != 0) {
					bool removeA = random_bool(gen);
					satisfied.erase(removeA ? a : b);
					hasConflict = true;
					break;
				}
			}
			if (hasConflict) break;
		}
	}
	return satisfied;
}

unordered_set<int> uniformRandomResolution(const vector<unordered_set<int>>& graph) {
	random_device dev;
	mt19937 gen(dev());

	unordered_set<int> satisfied;
	for (int i = 0; i < graph.size(); ++i) satisfied.insert(i);
	while (true) {
		vector<int> conflicts;
		for (auto a : satisfied) {
			for (auto b : satisfied) {
				if (graph[a].count(b) != 0) {
					conflicts.push_back(a);
					break;
				}
			}
		}
		if (conflicts.size() == 0) break;
		uniform_int_distribution<int> dist(0, conflicts.size() - 1);
		satisfied.erase(conflicts[dist(gen)]);
	}
	return satisfied;
}

unordered_set<int> removeMostConflicting(vector<unordered_set<int> > graph) {
	unordered_set<int> satisfied;
	for (int i = 0; i < graph.size(); ++i) satisfied.insert(i);
	while (true) {
		int maxConflicts = 0;
		int mostConflictingPerson = -1;
		for (auto person : satisfied) {
			int numConflicts = graph[person].size();
			if (numConflicts > maxConflicts) {
				maxConflicts = numConflicts;
				mostConflictingPerson = person;
			}
		}
		if (maxConflicts == 0) break;
		satisfied.erase(mostConflictingPerson);
		for (auto& node : graph) node.erase(mostConflictingPerson);
	}
	return satisfied;
}

unordered_set<int> addLeastConflicting(const vector<unordered_set<int>>& graph) {
	unordered_set<int> satisfied;
	unordered_set<int> potential;
	for (int i = 0; i < graph.size(); ++i) potential.insert(i);
	while (potential.size() > 0) {
		int leastConflicts = graph.size() + 1;
		int leastConflictingPerson = -1;
		for (auto person : potential) {
			int numConflicts = graph[person].size();
			if (numConflicts < leastConflicts) {
				leastConflicts = numConflicts;
				leastConflictingPerson = person;
			}
		}
		satisfied.insert(leastConflictingPerson);
		for (auto person : graph[leastConflictingPerson]) potential.erase(person);
		potential.erase(leastConflictingPerson);
	}
	return satisfied;
}

unordered_set<int> leastDislikes(const vector<unordered_set<int>>& graph, const vector<unordered_set<string>>& clientDislikes) {
	unordered_set<int> satisfied;
	unordered_set<int> potential;
	for (int i = 0; i < clientDislikes.size(); ++i) potential.insert(i);
	while (potential.size() > 0) {
		int leastDislikes = -1;
		int leastFussyPerson = -1;
		for (int person : potential) {
			if (leastDislikes == -1 || clientDislikes[person].size() < leastDislikes) {
				leastDislikes = clientDislikes[person].size();
				leastFussyPerson = person;
			}
		}
		satisfied.insert(leastFussyPerson);
		potential.erase(leastFussyPerson);
		for (int person : graph[leastFussyPerson]) potential.erase(person);
	}
	return satisfied;
}

unordered_set<int> fewestPreferences(const vector<unordered_set<int>>& graph, const vector<unordered_set<string>>& clientLikes, const vector<unordered_set<string>>& clientDislikes) {
	unordered_set<int> satisfied;
	unordered_set<int> potential;
	for (int i = 0; i < clientDislikes.size(); ++i) potential.insert(i);
	while (potential.size() > 0) {
		int fewestPreferences = -1;
		int leastFussyPerson = -1;
		for (int person : potential) {
			if (fewestPreferences== -1 || clientDislikes[person].size() + clientLikes[person].size() < fewestPreferences) {
				fewestPreferences = clientDislikes[person].size() + clientLikes[person].size();
				leastFussyPerson = person;
			}
		}
		satisfied.insert(leastFussyPerson);
		potential.erase(leastFussyPerson);
		for (int person : graph[leastFussyPerson]) potential.erase(person);
	}
	return satisfied;
}


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