#ifndef GUTTER_BASE_H
#define GUTTER_BASE_H
/*
 * This is the base class for the two gutter classes,
 *	gutter_apply and gutter_retrieve. Both classes implement a
 *	heap-style binary-tree array structure storing the array elements as well as
 *	other redundant information.
 *
 * This base class provides the following members/methods:
 *	- an internal heap-style array
 *	- a constructor allocating the array to a size 2'n', where 'n' is the number
 *		of gutter elements
 *	- methods for traversing the binary tree structure of the array
 *		- parent index
 *		- left/right child index
 *		- 'k'th leaf index
 *	- methods for performing functor operations on specific collections of nodes
 *		- all ancestors of a given leaf (performed root-down/leaf-up)
 *		- all ancestors of a collection of sequential leaves (root-down/leaf-up)
 *		- a sequence of leaves
 *		- the set of "minimal covering ancestors" of a sequence of leaves
 *		  (i.e., the minimum collection of nodes such that
 *			- each node is an ancestor of at least one leaf
 *			- each leaf has at least one node that is an ancestor)
 *	- functors to use with above node-collection operation methods
 */

#include <cstddef>
#include <algorithm>
#include "BitTwiddles.h"

template <typename RESULT_T, typename FUNCTOR_T>
class gutter_base {
protected:
	typedef std::size_t INDEX_T;

	RESULT_T* const heap;
	const INDEX_T _size;
	FUNCTOR_T op;

	static inline INDEX_T index_parent(INDEX_T n) {
		return n/2;
	}
	static inline INDEX_T index_lbranch(INDEX_T n) {
		return 2*n;
	}
	static inline INDEX_T index_rbranch(INDEX_T n) {
		return 2*n+1;
	}
	static inline bool index_islbranch(INDEX_T index) {
		return !(index%2);
	}
	static inline INDEX_T index_first_of_row(INDEX_T index) {
		// = the greatest power of 2 less than index
		return msb(index);
	}
	inline INDEX_T index_nth_leaf(INDEX_T n) const {
		const INDEX_T index_deepest_lvl_1st = index_first_of_row(2*_size-1);
		return n + index_deepest_lvl_1st
				- ((n+index_deepest_lvl_1st < 2*_size) ? 0 : _size);
	}

	static inline INDEX_T index_ancestor_in_row(INDEX_T index, INDEX_T row_1st) {
		while (index_parent(index) >= row_1st)
			index = index_parent(index);
		return index;
	}


public:
	gutter_base(INDEX_T n, FUNCTOR_T functor)
		: op(functor), _size(n), heap((new RESULT_T[2*n-1])-1) {}
	~gutter_base() {
		delete[] (heap+1);
	}
	INDEX_T size() const {
		return _size;
	}
	/*
	void print() const {
		INDEX_T i=0;
		INDEX_T j;
		while (true) {
			for (j=(1<<i);j<2*_size && j<(2<<i);++j) {
				std::cout << heap[j] << ",\t";
			}
			std::cout << std::endl;
			if (j >= 2*_size) {
				break;
			}
			++i;
		}
	}
	//*/
protected:

	template<typename F>
	inline F act_on_all_ancestors_leafup(INDEX_T index, F functor) const {
		//index = index_nth_leaf(index);
		while (index > 0) {
			functor(index);
			index = index_parent(index);
		}
		return functor;
	}
	template<typename F>
	inline F act_on_all_ancestors_rootdown(INDEX_T index, F functor) const {
		for (INDEX_T i=1; i<index; i*=2) {
			functor(index_ancestor_in_row(index,i));
		}
		return functor;
	}
	template<typename F>
	inline F act_on_all_ancestors(INDEX_T index, F functor) const {
		return act_on_all_ancestors_leafup(index,functor);
	}

	// TODO test!
	template<typename F>
	inline F act_on_all_ancestors_leafup(INDEX_T i1, INDEX_T i2, F functor) const {
		if (i1>i2) {
			for (
					INDEX_T j=i1;
					j<=index_ancestor_in_row(2*_size-1,index_first_of_row(i1));
					++j
				) {
				functor(j);
			}
			i1 = index_parent(i1);
		}
		while (i1 > 0) {
			for (INDEX_T j=i1; j<=i2; ++j) {
				functor(j);
			}
			i1 = index_parent(i1);
			i2 = index_parent(i2);
		}
		return functor;
	}
	template<typename F>
	inline F act_on_all_ancestors_rootdown(INDEX_T i1, INDEX_T i2, F functor) const {
		for (INDEX_T i=1; i<=i1; i*=2) {
			for (
					INDEX_T j = index_ancestor_in_row(i1,i);
					j <= index_ancestor_in_row((i<=i2 ?i2 :2*_size-1), i);
					++j
				) {
				functor(j);
			}
		}
		return functor;
	}
	template<typename F>
	inline F act_on_all_ancestors(INDEX_T i1, INDEX_T i2, F functor) const {
		return act_on_all_ancestors_leafup(i1,i2,functor);
	}

	template<typename F>
	inline F act_on_min_covering_ancestors(INDEX_T i1, INDEX_T i2, F functor) const {
		//std::cout << "gutter_base::act_on_min_covering_ancestors("
		//		<<i1<<","<<i2<<") - begin..." << std::endl;
		if (i1>=i2) return functor;
		i1 = index_nth_leaf(i1);
		i2 = index_nth_leaf(i2-1);	// sets index2 as an inclusive bound
		while (true) {
			// Shift left bound up one generation
			//std::cout << "("<<i1<<","<<i2<<")" << std::endl;
			if (i1==i2) break;
			if (!index_islbranch(i1)) {
				functor(i1++);
			//	std::cout << "("<<i1<<","<<i2<<")" << std::endl;
				if (i1==i2) break;
			}
			i1 = index_parent(i1);
			// Shift right bound up one generation
			//std::cout << "("<<i1<<","<<i2<<")" << std::endl;
			if (i1==i2) break;
			if (index_islbranch(i2)) {
				functor(i2--);
			//	std::cout << "("<<i1<<","<<i2<<")" << std::endl;
				if (i1==i2) break;
			}
			i2 = index_parent(i2);
		}
		// Left and right bounds have reached a common ancestor
		functor(i1);
		//std::cout << "gutter_base::act_on_min_covering_ancestors - done!" << std::endl;
		return functor;
	}

	template<typename F>
	inline F act_on_leaves_in_order(INDEX_T i1, INDEX_T i2, F functor) const {
		INDEX_T i = i1;
		++i2;	// make i2 an exclusive bound
		// (Assume there is at least one leaf in range
		do {
			if (i == 2*_size)
				i = index_parent(i);
			functor(i++);
		} while (i != i2);
		return functor;
	}

	// Stores/returns the functor operation among all values over the given range
	class functor_get {
	private:
		FUNCTOR_T op;
		const RESULT_T* const binarray;
		RESULT_T res;
	public:
		functor_get(const gutter_base& ro_ba)
				: op(ro_ba.op), binarray(ro_ba.heap), res(ro_ba.op()) {}
		void operator()(INDEX_T in) {
			res = op(res, binarray[in]);
		}
		const RESULT_T& result() {return res;}
	};
	// Assigns to each value, the functor operation between said value and a pre-stored element
	class functor_apply {
	private:
		FUNCTOR_T op;
		RESULT_T* const binarray;
		const RESULT_T input;
	public:
		functor_apply(gutter_base& ro_ba, const RESULT_T &in)
				: op(ro_ba.op), binarray(ro_ba.heap), input(in) {}
		void operator()(INDEX_T out) const {
			binarray[out] = op(binarray[out], input);
		}
	};

	// Outputs elements in structure to iterable
	// TODO instead output an iterator to the elements passed
	template <typename ITER_T>
	class functor_get_to_iter {
	private:
		FUNCTOR_T op;
		const RESULT_T* const binarray;
		ITER_T iter;
	public:
		functor_get_to_iter(const gutter_base& ro_ba, ITER_T it)
				: op(ro_ba.op), binarray(ro_ba.heap), iter(it) {}
		void operator()(INDEX_T index) {
			*(iter++) = binarray[index];
		}
		ITER_T iterator() {
			return iter;
		}
	};
	// Sets elements in structure from iterable
	template <typename ITER_T>
	class functor_set_from_iter {
	private:
		FUNCTOR_T op;
		RESULT_T* const binarray;
		ITER_T iter;
	public:
		functor_set_from_iter(gutter_base& ro_ba, ITER_T it)
				: op(ro_ba.op), binarray(ro_ba.heap), iter(it) {}
		void operator()(INDEX_T index) {
			binarray[index] = *(iter++);
		}
		ITER_T iterator() {
			return iter;
		}
	};
};

#include <limits>

template <typename T>
struct add {
	inline T operator()() const {
		return 0;
	}
	inline T operator()(T arg1, T arg2) const {
		return arg1 + arg2;
	}
};
template <typename T>
struct mult {
	inline T operator()() const {
		return 1;
	}
	inline T operator()(T arg1, T arg2) const {
		return arg1 * arg2;
	}
};
template <typename T>
struct min {
	inline T operator()() const {
		return std::numeric_limits<T>::max();
	}
	inline T operator()(T arg1, T arg2) const {
		return std::min(arg1, arg2);
	}
};
template <typename T>
struct max {
	inline T operator()() const {
		return std::numeric_limits<T>::min();
	}
	inline T operator()(T arg1, T arg2) const {
		return std::max(arg1, arg2);
	}
};
/*
template <typename T>
struct gcd {
	inline T operator()() const {
		return 0;
	}
	inline T operator()(T arg1, T arg2) const {
		return std::gcd(arg1, arg2);
	}
};
template <typename T>
struct lcm {
	inline T operator()() const {
		return 1;
	}
	inline T operator()(T arg1, T arg2) const {
		return std::lcm(arg1, arg2);
	}
};
//*/
#endif
