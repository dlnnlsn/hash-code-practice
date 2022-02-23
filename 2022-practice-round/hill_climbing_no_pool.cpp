#include <algorithm>
#include <bitset>
#include "heuristics.h"
#include <iostream>
#include <random>
#include <signal.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;

constexpr int pool_size = 1000;
constexpr double bit_flip_probability = 0.5;
constexpr double client_satisfaction_probability = 0.5;

bool running = true;
bool evolution_started = false;

void sigint_handler(int sig) {
	running = false;
	if (!evolution_started) exit(0);
}

struct seed {
	typedef unsigned int result_type;
	random_device dev;
	template <class RandomAccessIterator>
	void generate(RandomAccessIterator begin, RandomAccessIterator end) {
		for (RandomAccessIterator item = begin; item != end; ++item) {
			*item = dev();
		}
	}
	seed() : dev{} {}
};

typedef bitset<10000> bits;

typedef struct Gene {
	bits ingredients;
	size_t fitness;
	Gene(bits ingredients, size_t fitness) : ingredients(ingredients), fitness(fitness) {}
	friend constexpr bool operator<(const Gene& a, const Gene& b) {
		if (a.fitness != b.fitness) return a.fitness < b.fitness;
		for (size_t i = 0; i < 10000; ++i) {
			if (a.ingredients[i] != b.ingredients[i]) return a.ingredients[i] < b.ingredients[i];
		}
		return false;
	}
} Gene;

const size_t evaluate_fitness(const bits& ingredients, const vector<bits>& client_likes, const vector<bits>& client_dislikes) {
	size_t satisfied = 0;
	for (size_t client = 0; client < client_likes.size(); ++client) {
		if ((ingredients & client_likes[client]) == client_likes[client]) {
			if ((ingredients & client_dislikes[client]).none()) {
				++satisfied;
			}
		}
	}
	return satisfied;
}

template <class Generator>
const bits random_bitset(Generator& generator, size_t num_ingredients) {
	bits result;
	bernoulli_distribution dist(0.5);

	for (size_t i = 0; i < num_ingredients; ++i) {
		result[i] = dist(generator);
	}

	return result;
}

const bits ingredients_from_client_set(const unordered_set<size_t>& clients, const vector<bits>& client_likes) {
	bits ingredients;
	for (size_t client: clients) {
		ingredients |= client_likes[client];
	}
	return ingredients;
}

template <class Generator>
const bits flip_random_bits(Generator& generator, const bits current_bits, const size_t num_ingredients) {
	bits new_bits(current_bits);
	uniform_int_distribution<size_t> dist(0, num_ingredients - 1);
	uniform_real_distribution<double> real_dist(0, 1);
	do {
		new_bits.flip(dist(generator));
	} while (real_dist(generator) < bit_flip_probability);
	return new_bits;
}

template <class Generator>
const bits satisfy_random_clients(Generator& generator, const bits current_bits, const vector<bits>& client_likes, const vector<bits>& client_dislikes) {
	uniform_int_distribution<size_t> dist(0, client_likes.size() - 1);
	uniform_real_distribution<double> real_dist(0, 1);
	bits result = bits(current_bits);
	do {
		const size_t client_index = dist(generator);
		result |= client_likes[client_index];
		result &= (~client_dislikes[client_index]);
	} while (real_dist(generator) < client_satisfaction_probability);
	return result;
}

int main() {

	struct seed seeder;
	mt19937_64 generator(seeder);

	signal(SIGINT, sigint_handler);

	size_t num_clients; cin >> num_clients;

	unordered_map<string, size_t> ingredient_ids;
	vector<string> ingredient_names;
	size_t num_ingredients = 0;

	vector<bits> client_likes;
	client_likes.reserve(num_clients);
	vector<bits> client_dislikes;
	client_dislikes.reserve(num_clients);

	for (int client = 0; client < num_clients; ++client) {
		int num_likes; cin >> num_likes;
		bits current_likes;
		for (int i = 0; i < num_likes; ++i) {
			string name; cin >> name;
			if (ingredient_ids.count(name) == 0) {
				ingredient_ids[name] = num_ingredients++;
				ingredient_names.push_back(name);
			}
			current_likes[ingredient_ids[name]] = true;
		}
		client_likes.push_back(current_likes);

		int num_dislikes; cin >> num_dislikes;
		bits current_dislikes;
		for (int i = 0; i < num_dislikes; ++i) {
			string name; cin >> name;
			if (ingredient_ids.count(name) == 0) {
				ingredient_ids[name] = num_ingredients++;
				ingredient_names.push_back(name);
			}
			current_dislikes[ingredient_ids[name]] = true;
		}
		client_dislikes.push_back(current_dislikes);
	}

	vector<unordered_set<size_t>> conflict_graph;
	conflict_graph.reserve(num_clients);
	for (size_t i = 0; i < num_clients; ++i) conflict_graph.push_back(unordered_set<size_t>());

	for (size_t i = 0; i < num_clients; ++i) {
		for (size_t j = 0; j < num_clients; ++j) {
			if (i == j) continue;
			if (!(client_likes[i] & client_dislikes[j]).none()) {
				conflict_graph[i].insert(j);
				conflict_graph[j].insert(i);
			}
		}
	}

	bits best_so_far;
	size_t best_fitness_so_far = evaluate_fitness(best_so_far, client_likes, client_dislikes);

	unordered_set<size_t> most_conflicting = removeMostConflicting(conflict_graph);
	bits most_conflicting_ingredients = ingredients_from_client_set(most_conflicting, client_likes);
	const size_t most_conflicting_fitness = evaluate_fitness(most_conflicting_ingredients, client_likes, client_dislikes);
	cerr << "Most conflicting heuristic: " << most_conflicting_fitness << endl;
	if (most_conflicting_fitness > best_fitness_so_far) {
		best_fitness_so_far = most_conflicting_fitness;
		best_so_far = most_conflicting_ingredients;
	}	

	unordered_set<size_t> least_conflicting = addLeastConflicting(conflict_graph);
	bits least_conflicting_ingredients = ingredients_from_client_set(least_conflicting, client_likes);
	const size_t least_conflicting_fitness = evaluate_fitness(least_conflicting_ingredients, client_likes, client_dislikes);
	cerr << "Least conflicting heuristic: " << least_conflicting_fitness << endl;
	if (least_conflicting_fitness > best_fitness_so_far) {
		best_fitness_so_far = least_conflicting_fitness;
		best_so_far = least_conflicting_ingredients;
	}

	evolution_started = true;
	size_t generation = 0;
	size_t epoch = 0;

	while (running) {
		++generation;
		if (generation == 1000) {
			generation = 0;
			++epoch;
			cerr << "Epoch: " << epoch << ". Best fitness: " << best_fitness_so_far << endl;
		}
		
		const bits random_ingredients_flipped = flip_random_bits(generator, best_so_far, num_ingredients);
		const size_t random_ingredients_flipped_fitness = evaluate_fitness(random_ingredients_flipped, client_likes, client_dislikes);
		if (random_ingredients_flipped_fitness > best_fitness_so_far) {
			best_fitness_so_far = random_ingredients_flipped_fitness;
			best_so_far = random_ingredients_flipped;
		}

		const bits random_clients_satisfied = satisfy_random_clients(generator, best_so_far, client_likes, client_dislikes);
		const size_t random_clients_satisfied_fitness = evaluate_fitness(random_clients_satisfied, client_likes, client_dislikes);
		if (random_clients_satisfied_fitness > best_fitness_so_far) {
			best_fitness_so_far = random_clients_satisfied_fitness;
			best_so_far = random_clients_satisfied;
		}
	}

	cerr << "Writing best solution found..." << endl;
	cout << best_so_far.count();
	for (size_t i = 0; i < num_ingredients; ++i) {
		if (best_so_far[i]) cout << " " << ingredient_names[i];
	}
	cout << endl;

	return 0;
}