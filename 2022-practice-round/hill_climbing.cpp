#include <algorithm>
#include <iostream>
#include <random>
#include <signal.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;

constexpr int pool_size = 1000;
constexpr double bit_flip_probability = 0.5;
constexpr double client_satisfaction_probability = 0.33;

bool running = true;
bool evolution_started = false;

void sigint_handler(int sig) {
	running = false;
	if (!evolution_started) {
		exit(0);
	}
}

typedef struct BitSet {
	vector<uint64_t> bits;
	BitSet(const vector<uint64_t>& bits) : bits(bits) {}
	BitSet(const size_t size) {
		const size_t blocks = (size >> 6) + (((size & 63) > 0) ? 1 : 0);
		bits = vector<uint64_t>(blocks, 0);
	}

	friend BitSet operator&(const BitSet& l, const BitSet& r) {
		vector<uint64_t> result;
		result.reserve(l.bits.size());
		for (size_t i = 0; i < l.bits.size(); ++i) result.push_back(l.bits[i] & r.bits[i]);
		return BitSet(result);
	}

	friend BitSet operator|(const BitSet& l, const BitSet& r) {
		vector<uint64_t> result;
		result.reserve(l.bits.size());
		for (size_t i = 0; i < l.bits.size(); ++i) result.push_back(l.bits[i] | r.bits[i]);
		return BitSet(result);
	}

	friend bool operator==(const BitSet& l, const BitSet& r) {
		for (int i = 0; i < l.bits.size(); ++i) {
			if (l.bits[i] != r.bits[i]) return false;
		}
		return true;
	}

	friend BitSet operator~(const BitSet& bits) {
		vector<uint64_t> result;
		result.reserve(bits.bits.size());
		for (size_t i = 0; i < bits.bits.size(); ++i) result.push_back(~bits.bits[i]);
		return BitSet(result);
	}

	bool get(const size_t& index) {
		const uint64_t block = bits[index >> 6];
		return ((block >> (index & 63)) & 1) == 1;
	}

	void set(const size_t index, const bool value) {
		if (value) {
			bits[index >> 6] |= (1 << (index & 63));
		}
		else {
			bits[index >> 6] &= ~(1 << (index & 63));
		}
	}

	void flip(const size_t index) {
		bits[index >> 6] ^= (1 << (index & 63));
	}

	bool empty() {
		for (uint64_t block : bits) {
			if (block != 0) return false;
		}
		return true;
	}
} BitSet;

typedef struct Gene {
	BitSet ingredients;
	int fitness;
	Gene(BitSet ingredients, int fitness) : ingredients(ingredients), fitness(fitness) {}
	friend constexpr bool operator<(const Gene& a, const Gene& b) { 
		if (a.fitness != b.fitness) return a.fitness < b.fitness; 
		for (size_t i = 0; i < a.ingredients.bits.size(); ++i) {
			if (a.ingredients.bits[i] != b.ingredients.bits[i]) return a.ingredients.bits[i] < b.ingredients.bits[i];
		}
		return false;
	}
} Gene;

const int evaluate_fitness(const BitSet& ingredients, const vector<BitSet>& clientLikes, const vector<BitSet>& clientDislikes) {
	int satisfied = 0;
	for (int client = 0; client < clientLikes.size(); ++client) {
		if ((ingredients & clientLikes[client]) == clientLikes[client]) {
			if ((ingredients & clientDislikes[client]).empty()) satisfied++;
		}
	}
	return satisfied;
}

template <class Generator>
const BitSet random_bitset(Generator& gen, size_t size) {
	uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);
	const size_t blocks = (size >> 6) + (((size & 63) > 0) ? 1 : 0);
	vector<uint64_t> bits; bits.reserve(blocks);
	for (size_t block = 0; block < blocks; ++block) {
		bits.push_back(dist(gen));
	}
	return BitSet(bits);
}

template <class Generator>
const BitSet flip_random_bit(Generator& gen, const BitSet& bits, const size_t num_bits) {
	BitSet new_bits(bits.bits);
	uniform_int_distribution<size_t> dist(0, num_bits - 1);
	uniform_real_distribution<double> real_dist(0, 1);
	do {
		new_bits.flip(dist(gen));
	} while (real_dist(gen) < bit_flip_probability);
	return new_bits;
}

template <class Generator>
const BitSet satisfy_random_client(Generator& gen, const BitSet& bits, const vector<BitSet>& client_likes, const vector<BitSet>& client_dislikes) {
	uniform_int_distribution<size_t> dist(0, client_likes.size() - 1);
	uniform_real_distribution<double> real_dist(0, 1);
	BitSet result = BitSet(bits.bits);
	do {
		const size_t client_index = dist(gen);
		result = result | client_likes[client_index];
		result = result & (~client_dislikes[client_index]);
	} while (real_dist(gen) < client_satisfaction_probability);
	return result;
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

unordered_set<int> leastDislikes(const vector<unordered_set<int>>& graph, const vector<vector<string>>& clientDislikes) {
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

unordered_set<int> fewestPreferences(const vector<unordered_set<int>>& graph, const vector<vector<string>>& clientLikes, const vector<vector<string>>& clientDislikes) {
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

BitSet ingredients_from_client_set(const unordered_set<int>& clients, const vector<BitSet>& client_likes, const int num_ingredients) {
	BitSet ingredients = BitSet(num_ingredients);
	for (int client : clients) {
		ingredients = ingredients | client_likes[client];
	}
	return ingredients;
}

int main() {

	signal(SIGINT, sigint_handler);

	random_device dev;
	mt19937_64 gen(dev());

	int C; cin >> C;

	vector<vector<string>> clientLikeNames; clientLikeNames.reserve(C);
	vector<vector<string>> clientDislikeNames; clientDislikeNames.reserve(C);

	unordered_map<string, int> ingredient_ids;
	vector<string> ingredient_names;

	for (int client = 0; client < C; ++client) {
		int L; cin >> L;
		vector<string> likes; likes.reserve(L);
		for (int i = 0; i < L; ++i) {
			string name; cin >> name;
			if (ingredient_ids.count(name) == 0) {
				ingredient_ids[name] = ingredient_names.size();
				ingredient_names.push_back(name);
			}
			likes.push_back(name);
		}	
		clientLikeNames.push_back(likes);

		int D; cin >> D;
		vector<string> dislikes; dislikes.reserve(D);
		for (int i = 0; i < D; ++i) {
			string name; cin >> name;
			if (ingredient_ids.count(name) == 0) {
				ingredient_ids[name] = ingredient_names.size();
				ingredient_names.push_back(name);
			}
			dislikes.push_back(name);
		}
		clientDislikeNames.push_back(dislikes);
	}

	vector<BitSet> clientLikes; clientLikes.reserve(C);
	vector<BitSet> clientDislikes; clientDislikes.reserve(C);

	for (vector<string> like_names : clientLikeNames) {
		BitSet likes(ingredient_names.size());
		for (string name : like_names) {
			likes.set(ingredient_ids[name], true);
		}
		clientLikes.push_back(likes);
	}

	for (vector<string> dislike_names : clientDislikeNames) {
		BitSet dislikes(ingredient_names.size());
		for (string name : dislike_names) {
			dislikes.set(ingredient_ids[name], true);
		}
		clientDislikes.push_back(dislikes);
	}

	vector<unordered_set<int>> conflictGraph;
	conflictGraph.reserve(C);
	for (int i = 0; i < C; ++i) conflictGraph.push_back(unordered_set<int>());

	for (int i = 0; i < C; ++i) {
		for (int j = 0; j < C; ++j) {
			if (i == j) continue;
			if (!(clientLikes[i] & clientDislikes[j]).empty()) {
				conflictGraph[i].insert(j);
				conflictGraph[j].insert(i);
			}
		}
	}

	cerr << "Creating initial gene pool..." << endl;

	vector<Gene> pool; pool.reserve(pool_size);

	BitSet most_conflicting = ingredients_from_client_set(removeMostConflicting(conflictGraph), clientLikes, ingredient_names.size());
	pool.push_back(Gene(most_conflicting, evaluate_fitness(most_conflicting, clientLikes, clientDislikes)));

	BitSet least_conflicting = ingredients_from_client_set(addLeastConflicting(conflictGraph), clientLikes, ingredient_names.size());
	pool.push_back(Gene(least_conflicting, evaluate_fitness(least_conflicting, clientLikes, clientDislikes)));

	BitSet least_dislikes = ingredients_from_client_set(leastDislikes(conflictGraph, clientDislikeNames), clientLikes, ingredient_names.size());
	pool.push_back(Gene(least_dislikes, evaluate_fitness(least_dislikes, clientLikes, clientDislikes)));

	BitSet fewest_preferred = ingredients_from_client_set(fewestPreferences(conflictGraph, clientLikeNames, clientDislikeNames), clientLikes, ingredient_names.size());
	pool.push_back(Gene(fewest_preferred, evaluate_fitness(fewest_preferred, clientLikes, clientDislikes)));

	for (int i = 0; i < pool_size - 4; ++i) {
		const BitSet ingredients = random_bitset(gen, ingredient_names.size());
		const int fitness = evaluate_fitness(ingredients, clientLikes, clientDislikes);
		pool.push_back(Gene(ingredients, fitness));
	}
	sort(pool.begin(), pool.end());

	evolution_started = true;
	int generation = 0;

	while (running) {
		generation++;
		cerr << "Generation " << generation << ". Best fitness: " << pool.back().fitness << endl;
		vector<Gene> new_pool; new_pool.reserve(3 * pool_size);
		for (Gene gene : pool) {
			new_pool.push_back(gene);
			const BitSet random_ingredient_flipped = flip_random_bit(gen, gene.ingredients, ingredient_names.size());
			new_pool.push_back(Gene(random_ingredient_flipped, evaluate_fitness(random_ingredient_flipped, clientLikes, clientDislikes)));
			const BitSet random_client_satisfied = satisfy_random_client(gen, gene.ingredients, clientLikes, clientDislikes);
			new_pool.push_back(Gene(random_client_satisfied, evaluate_fitness(random_client_satisfied, clientLikes, clientDislikes)));
		}
		sort(new_pool.begin(), new_pool.end());
		vector<Gene> filtered_pool; filtered_pool.reserve(3 * pool_size);
		filtered_pool.push_back(new_pool.front());
		for (Gene gene : new_pool) {
			if (!(gene.ingredients == filtered_pool.back().ingredients)) {
				filtered_pool.push_back(gene);
			}
		}
		while (filtered_pool.size() < pool_size) {
			const BitSet ingredients = random_bitset(gen, ingredient_names.size());
			filtered_pool.push_back(Gene(ingredients, evaluate_fitness(ingredients, clientLikes, clientDislikes)));
		}
		pool = vector<Gene>(filtered_pool.end() - pool_size, filtered_pool.end());
	}

	BitSet ingredients = pool.back().ingredients;
	size_t ingredient_count = 0;
	for (size_t i = 0; i < ingredient_names.size(); ++i) {
		if (ingredients.get(i)) ingredient_count++;
	}
	cout << ingredient_count;
	for (size_t i = 0; i < ingredient_names.size(); ++i) {
		if (ingredients.get(i)) cout << " " << ingredient_names[i];
	}
	cout << endl;

	return 0;
}