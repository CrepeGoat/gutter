#ifndef GUTTER_APPLY_H
#define GUTTER_APPLY_H

#include "gutter_base.h"
#include <algorithm>

/*
 * This class effectively stores an array of elements, but optimizes to
 * quickly apply values via a given associative operation (i.e. sum/prod,
 * min/max, gcd/lcm) over a continuous range of elements (i.e., [a1,a2]).
 *
 * OPERATIONS & COMPLEXITY
 * 	- computing the 'i'th element
 * 		-> O(log(n))
 *	- applying a value via the given operation to 'k' sequential elements
 *	  (e.g., setting a[i] = x + a[i] for i = 1, ..., k)
 *		-> O(log(n))
 *	- setting new values for 'k' sequential elements
 *		-> O(k+log(n)-log(k))
 */
template <typename RESULT_T, typename FUNCTOR_T>
class gutter_apply
		: public gutter_base<RESULT_T, FUNCTOR_T> {

	typedef gutter_base<RESULT_T, FUNCTOR_T> base;
	typedef typename base::INDEX_T INDEX_T;

	inline void consolidate_to_children(INDEX_T index) {
		this->heap[this->index_lbranch(index)]
			= this->op(this->heap[this->index_lbranch(index)],this->heap[index]);
		this->heap[this->index_rbranch(index)]
			= this->op(this->heap[this->index_rbranch(index)],this->heap[index]);
		this->heap[index] = this->op();
	}
	class functor_consolidate {
	private:
		RESULT_T* const binarray;
		FUNCTOR_T op;
	public:
		functor_consolidate(RESULT_T* array, FUNCTOR_T operation)
			: binarray(array), op(operation) {}
		void operator()(INDEX_T in) {
			binarray[base::index_lbranch(index)]
				= op(binarray[base::index_lbranch(index)],binarray[index]);
			binarray[base::index_rbranch(index)]
				= op(binarray[base::index_rbranch(index)],binarray[index]);
			binarray[index] = op();
		}
	};

public:
	// Constructor
	gutter_apply(INDEX_T n, FUNCTOR_T functor=FUNCTOR_T())
			: base(n,functor) {
		std::fill(this->heap+1,this->heap+(2*this->_size), this->op());
	}
	// Access Method (run in O(1) time)
	void apply(INDEX_T leaf_no, RESULT_T& x) {
		this->heap[this->index_nth_leaf(leaf_no)]
			= this->op(this->heap[this->index_nth_leaf(leaf_no)], x);
	}
	// Access Methods (run in O(log(n)) time)
	void assign(INDEX_T leaf_no, RESULT_T& x) {
		leaf_no = this->index_nth_leaf(leaf_no);
		base::template act_on_all_ancestors_rootdown<auto>(
			this->index_parent(leaf_no), functor_consolidate(this->heap, this->op));
		//for (INDEX_T i=1; i<this->_size; i*=2) {
		//	consolidate_to_children(this->index_ancestor_in_row(leaf_no,i));
		//}
		this->heap[leaf_no] = this->op(this->heap[leaf_no], x);
	}
	RESULT_T operator[](INDEX_T leaf_no) const {
		return base::template act_on_all_ancestors<auto>(
			this->index_nth_leaf(leaf_no), typename base::functor_get(*this)
		).result();
	}
	void apply(INDEX_T i1, INDEX_T i2, RESULT_T& x) {
		base::template act_on_min_covering_ancestors<auto>(
			i1,i2, typename base::functor_apply(*this,x)
		);
	}
	// Access Method (runs in O(k+log(n)-log(k)) time)
	template <typename ITER_T>
	ITER_T copy(INDEX_T i1, INDEX_T i2, ITER_T output) {
		if (i1>i2) //error?
			return output;
		else if (i1==i2) return output;
		// Get Leaf positions
		i1 = this->index_nth_leaf(i1);
		i2 = this->index_nth_leaf(i2-1);	// make i2 an inclusive bound
		// Consolidate all values in range to the bottom row
		base::template act_on_all_ancestors_rootdown<auto>(
			this->index_parent(i1),
			this->index_parent(i2),
			functor_consolidate(this->heap, this->op)
		);
		// Output results to iterator
		return base::template act_on_leaves_in_order<auto>(
			i1,i2, typename base::functor_get_to_iter<ITER_T>(*this,output)
		).iterator();
	}
};


#endif
