#ifndef UTILITIES_DOLBAYOB_PTR_HPP_
#define UTILITIES_DOLBAYOB_PTR_HPP_

#include <iostream>

template <typename T>
class DolbayobPTR {
   public:
	DolbayobPTR() : _data(NULL), _ref_counter(NULL) {}

	DolbayobPTR(T* data)
		: _data(NULL),
		  _ref_counter(NULL)
	{
		set_data(data);
	}

	DolbayobPTR(const DolbayobPTR& copy)
		: _data(NULL), _ref_counter(NULL)
	{
		*this = copy;
	}

	~DolbayobPTR() { 
		unlink_ptr();
	}

	DolbayobPTR& operator=(const DolbayobPTR& copy) {
		if (this == &copy) {
			return *this;
		}

		unlink_ptr();

		_ref_counter = copy._ref_counter;
		_data = copy._data;
		if (copy._data) {
			++(*_ref_counter);
		}
		return *this;
	}

	T* get() { return _data; }

	void set_data(T* ptr) {
		unlink_ptr();

		_data = ptr;
		if (ptr != NULL) {
			_ref_counter = new size_t(1);
		}
	}

	void unlink_ptr() {
		if (_ref_counter == NULL) {
			return;
		}

		if ((*_ref_counter) == 1) {
			destroy_ptr();
			return;
		}

		--(*_ref_counter);
		_ref_counter = NULL;
		_data = NULL;
	}

   private:
	void destroy_ptr() {
		delete _data;
		_data = NULL;

		delete _ref_counter;
		_ref_counter = NULL;
	}

	T* _data;
	size_t* _ref_counter;
};

#endif // UTILITIES_DOLBAYOB_PTR_HPP_