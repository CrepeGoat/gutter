#ifndef GUTTER_RETRIEVE_H
#define GUTTER_RETRIEVE_H

#include "gutter_base.h"
#include <algorithm>
#include <initializer_list>

/*
 * This class effectively stores an array of elements, but optimizes to
 * return a given associative operation (i.e., sum/prod, min/max, gcd/lcm)
 * over a continuous range (i.e., [a1,a2]) of elements quickly.
 *
 * OPERATIONS & COMPLEXITY
 * 	- computing the associative operation over 'k' sequential elements
 *		-> O(log(n))
 *	- setting a new value to the 'i'th element
 *		-> O(log(n))
 *	- setting a collection of 'k' sequential elements
 *		-> O(k+log(n)-log(k))
 */
template <typename RESULT_T, typename FUNCTOR_T>
class gutter_retrieve
		: public gutter_base<RESULT_T, FUNCTOR_T> {

	typedef gutter_base<RESULT_T, FUNCTOR_T> base;
	typedef typename base::INDEX_T INDEX_T;

	class functor_update_parent {
	private:
		RESULT_T* const binarray;
		FUNCTOR_T op;
	public:
		functor_update_parent(RESULT_T* array, FUNCTOR_T operation)
			: op(operation), binarray(array) {}
		void operator()(INDEX_T index) {
			binarray[index] = op(
					binarray[base::index_lbranch(index)],
					binarray[base::index_rbranch(index)]
				);
		}
	};

public:
	// Constructor
	gutter_retrieve(INDEX_T n, FUNCTOR_T functor=FUNCTOR_T())
			: base(n,functor) {
		std::fill(this->heap+1,this->heap+(2*this->_size), this->op());
	}
	// Access Method (run in O(1) time)
	RESULT_T operator[](INDEX_T leaf_no) const {
		return this->heap[this->index_nth_leaf(leaf_no)];
	}
	// Access Methods (run in O(log(n)) time)
	void assign(INDEX_T leaf_no, RESULT_T& x) {
		leaf_no = this->index_nth_leaf(leaf_no);
		this->heap[leaf_no] = x;
		base::template act_on_all_ancestors_leafup(
				this->index_parent(leaf_no),
				functor_update_parent(this->heap, this->op)
			);
	}
	void apply(INDEX_T leaf_no, RESULT_T& x) {
		leaf_no = this->index_nth_leaf(leaf_no);
		this->heap[leaf_no] = this->op(this->heap[leaf_no], x);
		base::template act_on_all_ancestors_leafup(
				this->index_parent(leaf_no),
				functor_update_parent(this->heap, this->op)
			);
	}
	RESULT_T accumulate(INDEX_T leaf1, INDEX_T leaf2) const {
		return base::template act_on_min_covering_ancestors(
				leaf1,leaf2, typename base::functor_get(*this)
			).result();
	}
	// Access Method (runs in O(k+log(n)-log(k)) time)
	template <typename ITER_T>
	ITER_T assign(INDEX_T i1, INDEX_T i2, ITER_T input) {
		if (i1>i2) //error?
			return input;
		else if (i1==i2) return input;
		// Get Leaf positions
		i1 = this->index_nth_leaf(i1);
		i2 = this->index_nth_leaf(i2-1);	// make i2 an inclusive bound
		// Input results from iterator
		input = base::template act_on_leaves_in_order(
				i1,i2, typename base::template functor_set_from_iter<ITER_T>(*this,input)
			).iterator();
		// Update affected ancestors
		i1 = this->index_parent(i1);
		i2 = this->index_parent(i2);
		base::template act_on_all_ancestors_leafup(
				i1,i2, functor_update_parent(this->heap, this->op)
			);
		// Return iterator to end of copy location
		return input;
	}
	// Constructor
	gutter_retrieve(std::initializer_list<RESULT_T> source,
			FUNCTOR_T functor=FUNCTOR_T()) : base(source.size(),functor) {
		assign(0,source.size(), source.begin());
	}
};


#endif
