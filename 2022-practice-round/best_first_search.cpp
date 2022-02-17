#include <iostream>
#include <queue>
#include <signal.h>
#include <unordered_set>
#include <vector>

using namespace std;

bool running = true;

void sigint_handler(int sig) {
	cerr << "Stopping search..." << endl;
	running = false;
}

typedef struct Context {
	int person;
	int estimated_value;
	vector<int> included;
	unordered_set<int> potential;
	Context(int person, int estimated_value, vector<int> included, unordered_set<int> potential) : person(person), estimated_value(estimated_value), included(included), potential(potential) {}
	friend constexpr bool operator<(const Context& l, const Context& r) {
		return l.estimated_value < r.estimated_value;
	} 
} Context;

int heuristic(const vector<unordered_set<int>>& graph, unordered_set<int> potential) {
	int numSatisfied = 0;
	while (potential.size() > 0) {
		int leastConflicts = graph.size() + 1;
		int leastConflictingPerson = -1;
		for (int person : potential) {
			int conflicts = 0;
			if (potential.size() < graph[person].size()) {
				for (int potential_neighbour : potential) {
					conflicts += graph[person].count(potential_neighbour);
				}
			}
			else {
				for (int neighbour : graph[person]){
					conflicts += potential.count(neighbour);
				}
			}
			if (conflicts < leastConflicts) {
				leastConflicts = conflicts;
				leastConflictingPerson = person;
			}
		}
		numSatisfied++;
		for (int neighbour : graph[leastConflictingPerson]) potential.erase(neighbour);
		potential.erase(leastConflictingPerson);
	}
	return numSatisfied;
}

vector<int> best_first_search(const vector<unordered_set<int>>& graph) {
	vector<int> best_so_far;

	priority_queue<Context> to_visit;
	unordered_set<int> potential;
	for (int i = 0; i < graph.size(); ++i) potential.insert(i);
	to_visit.push(Context(0, 0, vector<int>(), potential));

	while (running && to_visit.size() > 0) {
		Context frame = to_visit.top();
		to_visit.pop();

		const int bound = frame.included.size() + frame.potential.size() + 1;
		if (bound <= best_so_far.size()) continue;

		if (frame.included.size() > best_so_far.size()) {
			best_so_far = frame.included;
			cerr << "Best so far: " << best_so_far.size() << endl;
		}

		if (frame.person == graph.size()) continue;
		 
		const bool has_current_person = frame.potential.count(frame.person) != 0;
		frame.potential.erase(frame.person);

		const int left_bound = frame.included.size() + frame.potential.size(); 
		if (left_bound > best_so_far.size()) {
			to_visit.push(Context(frame.person + 1, frame.included.size() + heuristic(graph, frame.potential), frame.included, frame.potential));
		}

		if (!has_current_person) continue;
		frame.included.push_back(frame.person);
		for (int neighbour : graph[frame.person]) frame.potential.erase(neighbour);

		const int right_bound = frame.included.size() + frame.potential.size();
		if (right_bound > best_so_far.size()) {
			to_visit.push(Context(frame.person + 1, frame.included.size() + heuristic(graph, frame.potential), frame.included, frame.potential));
		}
	}

	return best_so_far;
}

int main() {

	int C; cin >> C;
	vector<unordered_set<string>> clientLikes;
	vector<unordered_set<string>> clientDislikes;
	clientLikes.reserve(C);
	clientDislikes.reserve(C);

	for (int i = 0; i < C; ++i) {
		int L; cin >> L;
		unordered_set<string> likes;
		for (int j = 0; j < L; ++j) {
			string ingredient; cin >> ingredient;
			likes.insert(ingredient);
		}
		clientLikes.push_back(likes);
		int D; cin >> D;
		unordered_set<string> dislikes;
		for (int j = 0; j < D; ++j) {
			string ingredient; cin >> ingredient;
			dislikes.insert(ingredient);
		}
		clientDislikes.push_back(dislikes);
	}

	vector<unordered_set<int>> graph;
	graph.reserve(C);
	for (int i = 0; i < C; ++i) graph.push_back(unordered_set<int>());

	for (int i = 0; i < C; ++i) {
		for (int j = 0; j < C; ++j) {
			if (i == j) continue;
			for (string ingredient : clientLikes[i]) {
				if (clientDislikes[j].count(ingredient) != 0) {
					graph[i].insert(j);
					graph[j].insert(i);
					break;
				}
			}
		}
	}

	signal(SIGINT, sigint_handler);

	const vector<int> clients = best_first_search(graph);
	cerr << "Best First Search: " << clients.size() << endl;
	unordered_set<string> ingredients;
	for (int person : clients) {
		for (string ingredient : clientLikes[person]) {
			ingredients.insert(ingredient);
		}
	}
	cout << ingredients.size();
	for (string ingredient : ingredients) {
		cout << " " << ingredient;
	}
	cout << endl;

	return 0;
}