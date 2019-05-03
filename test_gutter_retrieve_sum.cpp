
#include "gutter_retrieve.h"
#include <iostream>

template <typename RESULT_T>
class test_gutter_retrieve_sum {
private:
	typedef std::size_t INDEX_T;

	const add<RESULT_T> functor;
	gutter_retrieve<RESULT_T,add<RESULT_T> > rsh;
	RESULT_T *values;
	const INDEX_T size;
public:
	bool should_print_internals;

	test_gutter_retrieve_sum(INDEX_T length)
	: size(length), functor(), rsh(length,functor), values(new RESULT_T[length]) {
		for (INDEX_T i=0;i<size;++i) {
			values[i] = functor();
		}
	}
	~test_gutter_retrieve_sum() {
		delete[] values;
	}

	void test_add_to(INDEX_T index, RESULT_T delta, bool should_print_results=false) {
		if (should_print_results)
			std::cout << index << " <+ " << delta << std::endl;
		rsh.apply(index, delta);
		values[index]=functor(values[index],delta);
		//if (should_print_internals) {
		//
		//}
	}
	bool test_sum_range(INDEX_T index1, INDEX_T index2, bool should_print_results=false) {
		if (should_print_results)
			std::cout << '[' << index1 << ", " << index2 << ')' << std::endl;
		RESULT_T tmp1 = rsh.accumulate(index1,index2);
		RESULT_T tmp2=0;
		while (index1<index2) {
			tmp2+=values[index1];
			++index1;
		}
		if (tmp1 != tmp2) {
			std::cout << "FAILURE";
			if (!should_print_results)
				std::cout << " - [" << index1 << ", " << index2 << ')' << std::endl;
			else
				std::cout << std::endl;
			std::cout << "alg: \t" << tmp1 << std::endl;
			std::cout << "true:\t" << tmp2 << std::endl;

		} else if (should_print_results) {
			std::cout << "alg = true = " << tmp1 << std::endl;
		}
		return tmp1==tmp2;
	}

	void stress_test() {
		std::cout << "Test suite:\tgutter_retrieve<T,+> class" << std::endl;
		std::cout << "\ttarget:\tapply(T), accumulate(I,I) methods" << std::endl;
		std::cout << "\ttype:\tstress test" << std::endl;

		std::cout << "Beginning Test." << std::endl;
		while (true) {
			// Change values
			const INDEX_T index = rand()%size;
			test_add_to(index,(rand()%200000)+1, true);
			// Check sums
			for (INDEX_T i=0;i<=index;++i) {
				for (INDEX_T j=index;j<=size;++j) {
					if (!test_sum_range(i,j)) {
						return;
					}
				}
			}
		}
	}
};

int main() {
	test_gutter_retrieve_sum<int>(1000).stress_test();
	return 0;
}
