/*********************************************************************************************/
// class unordered_map:
// using vector as basic data structure
// using list to solve confliction
// including some basic functions of unordered_map:
//
// 1. iterator begin() const;
// 2. iterator end() const;
// 3. size_type size() const;
// 4. size_type bucket_size() const;
// 5. bool empty() const;
// 6. Value& operator[] (const Key& key);
// 7. iterator find(const Key& key);
// 8. void insert(const pair<Key, Value>& p);
// 9. void erase(const iterator ite);
// 10. void clear();

#ifndef EZSTL_UNORDERED_MAP_H
#define EZSTL_UNORDERED_MAP_H

#include "functional.h"
#include "list.h"
#include "utility.h"
#include "vector.h"
#include "test.h"

namespace ezSTL {

	// forward declaration of class unordered_map, which is to be used in __unordered_map_iterator
	template<typename Key, typename Value, typename Hash, typename KeyEqual> class unordered_map;

	// iterator of unordered_map
	template<typename Key, typename Value, typename Hash = hash<Key>, typename KeyEqual = equal_to<Key>>
		class __unordered_map_iterator: public iterator<forward_iterator_tag, pair<Key, Value>> {
		public:
			using size_type = unsigned int;
			using iterator = __unordered_map_iterator<Key, Value, Hash, KeyEqual>;
			using self = __unordered_map_iterator<Key, Value, Hash, KeyEqual>;
			using link_type = __list_node<pair<Key, Value>>*;
			using _list_iterator = __list_iterator<pair<Key, Value>>;
			using _list_pointer = list<pair<Key, Value>>*;
			using _um_pointer = unordered_map<Key, Value, Hash, KeyEqual>*;
			using difference_type = int;

			// use __list_node as internal storage node
			// list iterator
			_list_iterator list_iterator;
			// list pointer
			_list_pointer list_pointer;
			// unordered_map pointer
			_um_pointer um_pointer;
			// index of bucket in *um_pointer
			size_type bucket;

			// constructor
			__unordered_map_iterator() = default;
			__unordered_map_iterator(_list_iterator x, _list_pointer y, _um_pointer z, size_type n) : list_iterator(x),
				list_pointer(y), um_pointer(z), bucket(n) {}
			__unordered_map_iterator(const iterator& x) : list_iterator(x.list_iterator), list_pointer(x.list_pointer),
				um_pointer(x.um_pointer), bucket(x.bucket) {}

			self& operator= (const iterator& x) {
				if (this != &x) {
					list_iterator = x.list_iterator;
					list_pointer = x.list_pointer;
					um_pointer = x.um_pointer;
					bucket = x.bucket;
				}
				return *this;
			}

			bool operator== (const self& x) const {
				// if *this and x are both end()
				if (bucket >= um_pointer->buckets.size() && x.bucket >= x.um_pointer->buckets.size())
					return true;
				return list_iterator == x.list_iterator && bucket == x.bucket;
			}

			bool operator!= (const self& x) const {
				return !(*this == x);
			}

			Value& operator*() const {
				return (*list_iterator).second;
			}

			Value* operator->() const {
				return &(operator*());
			}

			self& operator++() {
				// if the next node in the same list
				if (list_iterator + 1 != list_pointer->end())
					++list_iterator;
				// if the next node not in the same list
				else {
					list_pointer = um_pointer->buckets[++bucket];
					// go to next bucket until the next bucket is not empty or overranging
					while (bucket < um_pointer->buckets.size() && list_pointer == nullptr)
						list_pointer = um_pointer->buckets[++bucket];
					// if it is not overanging
					if (bucket < um_pointer->buckets.size())
						list_iterator = list_pointer->begin();
				}
				return *this;
			}

			self& operator++(int) {
				self temp = *this;
				++(*this);
				return temp;
			}

			self operator+ (difference_type n) const {
				self temp = *this;
				advance(temp, n);
				return temp;
			}

			Key get_key() const {
				return (*list_iterator).first;
			}
	};

	// max load factor
	static const double max_load_factor = 0.5;
	// predetermined primes
	enum { num_primes = 28 };
	static const unsigned int prime_list[num_primes] =
	{
		53ul,         97ul,         193ul,       389ul,       769ul,
		1543ul,       3079ul,       6151ul,      12289ul,     24593ul,
		49157ul,      98317ul,      196613ul,    393241ul,    786433ul,
		1572869ul,    3145739ul,    6291469ul,   12582917ul,  25165843ul,
		50331653ul,   100663319ul,  201326611ul, 402653189ul, 805306457ul,
		1610612741ul, 3221225473ul, 4294967291ul
	};
	// get next prime greater than n, or return the last number in prime_list
	inline unsigned int next_prime(unsigned int n)
	{
		for (int i = 0; i < num_primes; ++i) {
			if (prime_list[i] > n) return prime_list[i];
		}
		return prime_list[num_primes - 1];
	}

	// class unordered_map
	template<typename Key, typename Value, typename Hash = hash<Key>, typename KeyEqual = equal_to<Key>>
		class unordered_map {
		public:
			using size_type = unsigned int;
			using value_type = pair<Key, Value>;
			using pointer = value_type*;
			using iterator = __unordered_map_iterator<Key, Value, Hash, KeyEqual>;
			using const_iterator = const iterator;
			using reference = value_type&;
			using const_reference = const pair<Key, Value>&;
			using difference_type = int;
			using _list_pointer = list<pair<Key, Value>>*;

		public:
			// using vector as base data structure
			vector<_list_pointer> buckets;
		protected:
			size_type __size;
			iterator end_iterator;

		public:
			// some common functions of unordered_map
			iterator begin() const {
				// find the first pair
				for (size_type i = 0; i < buckets.size(); ++i)
					if (buckets[i]) {
						iterator temp;
						temp.list_iterator = buckets[i]->begin();
						temp.list_pointer = buckets[i];
						temp.um_pointer = const_cast<unordered_map<Key, Value, Hash, KeyEqual>*>(this);
						temp.bucket = i;
						return temp;
					}
				return end();
			}
			iterator end() const {
				return end_iterator;
			}
			size_type size() const {
				return __size;
			}
			size_type bucket_size() const {
				return buckets.size();
			}
			bool empty() const {
				return __size == 0;
			}

			// operator []
			Value& operator[] (const Key& key) {
				return *find(key);
			}

			// find specific key, and return the iterator
			iterator find(const Key& key);
			// insert a pair
			void insert(const pair<Key, Value>& p);
			// earse a pair according the iterator
			void erase(const iterator ite);
			// clear the unordered_map
			void clear();

			// constructor
			unordered_map() : buckets(prime_list[0], nullptr), __size(0), end_iterator(nullptr, nullptr, this, buckets.size()) {}
			// copy constructor
			unordered_map(const unordered_map<Key, Value, Hash, KeyEqual>& um) : buckets(um.bucket_size(), nullptr), 
				__size(um.size()), end_iterator(nullptr, nullptr, this, um.bucket_size()) {
				for (size_type i = 0; i < bucket_size(); ++i) {
					if(um.buckets[i])
						buckets[i] = new list<pair<Key, Value>>(*um.buckets[i]);
				}
			}
			// move constructor
			unordered_map(unordered_map<Key, Value, Hash, KeyEqual>&& um) noexcept {
				buckets = move(um.buckets);
				__size = um.__size;
				end_iterator = um.end_iterator;
			}

			// copy assignemnt operator
			unordered_map<Key, Value, Hash, KeyEqual>& operator= (const unordered_map<Key, Value, Hash, KeyEqual>& um) {
				if (this != &um) {
					clear();
					buckets.resize(um.bucket_size());
					for (size_type i = 0; i < bucket_size(); ++i) {
						if (um.buckets[i])
							buckets[i] = new list<pair<Key, Value>>(*um.buckets[i]);
					}
					__size = um.size();
					end_iterator = um.end_iterator;
				}
				return *this;
			}
			// move assignemnt operators
			unordered_map<Key, Value, Hash, KeyEqual>& operator= (unordered_map<Key, Value, Hash, KeyEqual>&& um) noexcept {
				if (this != &um) {
					buckets = move(um.buckets);
					__size = um.__size;
					end_iterator = um.end_iterator;
				}
				return *this;
			}

			// destructor
			~unordered_map() {
				clear();
			}
	};

	template<typename Key, typename Value, typename Hash, typename KeyEqual>
	typename unordered_map<Key, Value, Hash, KeyEqual>::iterator
	unordered_map<Key, Value, Hash, KeyEqual>::find(const Key& key) {
		// finding index
		size_type index = Hash()(key) % buckets.size();
		if (!buckets[index]) return end();
		// searching the list and find key
		for(auto ite = buckets[index]->begin(); ite != buckets[index]->end(); ++ite)
			if (KeyEqual()(ite->first, key)) return iterator(ite, buckets[index], this, index);
		return end();
	}

	template<typename Key, typename Value, typename Hash, typename KeyEqual>
	void unordered_map<Key, Value, Hash, KeyEqual>::insert(const pair<Key, Value>& p) {
		auto ite = find(p.first);
		// if key does not exists
		if (ite == end()) {
			size_type index = Hash()(p.first) % buckets.size();
			if (!buckets[index])
				buckets[index] = new list<pair<Key, Value>>();
			buckets[index]->push_front(p);
			++__size;
		}
		// else replace the value
		else *ite = p.second;
		// if load_factor > max_load_factor, then rehash
		if (static_cast<double>(size()) / bucket_size() > max_load_factor) {
			// get next bucket size by predetermined primes
			size_type new_size = next_prime(bucket_size());
			// if not changed, return directly
			if (new_size == bucket_size()) return;
			else {
				unordered_map<Key, Value, Hash, KeyEqual> temp = *this;
				clear();
				// resize buckets
				buckets.resize(new_size);
				end_iterator.bucket = new_size;
				// insert each pair from old unordered_map to new one
				for (auto ite = temp.begin(); ite != temp.end(); ++ite)
					insert(make_pair(ite.get_key(), *ite));
			}
		}
	}

	template<typename Key, typename Value, typename Hash, typename KeyEqual>
	void unordered_map<Key, Value, Hash, KeyEqual>::erase(const iterator ite) {
		ite.list_pointer->erase(ite.list_iterator);
		// if the list is empty, then remove the list
		if (ite.list_pointer->empty()) {
			delete ite.um_pointer->buckets[ite.bucket];
			ite.um_pointer->buckets[ite.bucket] = nullptr;
		}
		--__size;
	}
	template<typename Key, typename Value, typename Hash, typename KeyEqual>
	void unordered_map<Key, Value, Hash, KeyEqual>::clear() {
		// clear the buckets
		for (size_type i = 0; i < bucket_size(); ++i) {
			if (buckets[i]) {
				delete buckets[i];
				buckets[i] = nullptr;
			}
		}
		__size = 0;
	}
}

#endif // !EZSTL_UNORDERED_MAP_H
