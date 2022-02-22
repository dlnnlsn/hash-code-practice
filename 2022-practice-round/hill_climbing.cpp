#include <algorithm>
#include <iostream>
#include <random>
#include <signal.h>
#include <unordered_map>
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
	friend constexpr bool operator<(const Gene& a, const Gene& b) { return a.fitness < b.fitness; }
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
const BitSet flip_random_bit(Generator& gen, const BitSet& bits) {
	BitSet new_bits(bits.bits);
	uniform_int_distribution<size_t> dist(0, bits.bits.size() - 1);
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

	cerr << "Creating initial gene pool..." << endl;

	vector<Gene> pool; pool.reserve(pool_size);
	for (int i = 0; i < pool_size; ++i) {
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
			const BitSet random_ingredient_flipped = flip_random_bit(gen, gene.ingredients);
			new_pool.push_back(Gene(random_ingredient_flipped, evaluate_fitness(random_ingredient_flipped, clientLikes, clientDislikes)));
			const BitSet random_client_satisfied = satisfy_random_client(gen, gene.ingredients, clientLikes, clientDislikes);
			new_pool.push_back(Gene(random_client_satisfied, evaluate_fitness(random_client_satisfied, clientLikes, clientDislikes)));
		}
		sort(new_pool.begin(), new_pool.end());
		pool = vector<Gene>(new_pool.end() - pool_size, new_pool.end());
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