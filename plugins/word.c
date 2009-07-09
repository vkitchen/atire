/*
	 WORD.c
	 ------
 */

#include "word.h"
#include <climits>
#include <cassert>
#include <algorithm>
#include <iostream>
#include <limits>
#include <math.h>

#include "uniseg_settings.h"
#include "uniseg_types.h"

using namespace std;

Word::Word(string_type chars, int size) : chars_(chars), size_(size) {
	freq_ =  1;
	init();
}

Word::Word(string_type chars, int freq, int size) :
	chars_(chars), freq_(freq), size_(size) {
	init();
}

Word& Word::operator++() {
	//freq_++;
	increase();
	return (*this);
}

Word Word::operator++(int) {
	Word word = *this;
	++(*this);
	return word;
}

bool Word::operator<(Word* w2) {
	return this->freq() < w2->freq();
}

bool Word::operator()(int value) {
	return this->freq() <= value;
}

void Word::init() {
	address_ = INVALID;
	lparent_ = NULL;
	rparent_ = NULL;
	side_ = UNKNOWN;

	p_ = 0.0;
	//a_ = 0.0;
	right_a_ = 0.0;
	left_a_ = 0.0;

	lcounter_ = 0;
	rcounter_ = 0;

	is_word_ = false;
	seged_ = false;
}

bool Word::cmp_just_freq(Word *w1, Word *w2) {
	int s1 = w1->size();
	int s2 = w2->size();

	if (s1 == s2)
		return w1->freq() > w2->freq();
	return s1 > s2;
}

bool Word::cmp_freq(Word *w1, Word *w2) {
	int h1, h2;
	if (!w1->has_parent() || !w2->has_parent())
	{
		h1 = w1->freq();
		h2 = w2->freq();

		assert(w1->size() == 1);
		assert(w2->size() == 1);
		if (h1 == h2)
			{
			return uniseg_encoding_utf8::to_codepoint((unsigned char *)w1->chars().c_str())
				< uniseg_encoding_utf8::to_codepoint((unsigned char *)w2->chars().c_str());
			}
		return h1 > h2;
	}

	h1 = w1->highest_freq();
	h2 = w2->highest_freq();
	//int h1 = p1.second;
	//int h2 = p2.second;

	// if having the same highest frequency, then
	// test the address, the lower address the higher priority
	if (h1 == h2) {
		//assert(p1.first != INVALID);
		//assert(p2.first != INVALID);
		assert(w1->parent());
		assert(w2->parent());

		int a1 = INVALID, a2 = INVALID;
		if (w1->parent()->chars() != w2->parent()->chars()) {
			a1 = w1->parent()->address();
			a2 = w2->parent()->address();

			assert( a1 != INVALID);
			assert( a2 != INVALID);

			return a1 < a2;
		}
		Side s1 = w1->side();
		Side s2 = w2->side();

		if (s1 == s2) {

			a1 = w1->parent2()->address();
			a2 = w2->parent2()->address();

			assert( a1 != INVALID);
			assert( a2 != INVALID);

			return a1 < a2;
		}
		return s1 < s2;
	}
	return h1 > h2;
}

unsigned int Word::highest_freq() {
	//if (arr_.size() <= 0)
	//	return std::make_pair((int)INVALID, freq_);

	if (lparent_ != NULL && rparent_ != NULL) {

		//return std::max(arr_[0]->freq(), arr_[1]->freq());
		//int idx = 0;
		side_ = LEFT;
		int h = lparent_->freq();

		//for (int i = 1; i < arr_.size(); i++) {
			if (h < (int)rparent_->freq()) {
				h = rparent_->freq();
				side_ = RIGHT;
			}
		//}
		return h;
	}
	return freq_;
}

/*
Word* Word::word(int idx) {
	assert(idx <= arr_.size());

	if (idx == INVALID)
		return this;

	return arr_[idx];
}
*/

unsigned int Word::children_freq() {
	unsigned int freq = 0;
	for (int i = 0; i < (int)children_.size(); i++)
		freq += children_[i]->freq();
	return freq;
}

const Word* Word::parent() const{
	//if (side_ == UNKNOWN)
	//	this->highest_freq();

	if (side_ == LEFT)
		return lparent_;
	else if (side_ == RIGHT)
		return rparent_;
	return NULL;
}

const Word* Word::parent2() const {
	//if (side_ == UNKNOWN)
	//	this->highest_freq();

	if (side_ == LEFT)
		return rparent_;
	else if (side_ == RIGHT)
		return lparent_;
	return NULL;
}

std::pair<Word*, Word*> Word::family() {
	if (side_ == UNKNOWN)
		highest_freq();

	if (side_ == LEFT)
		return std::make_pair(lparent_, rchar_);
	else if (side_ == RIGHT)
		return std::make_pair(rparent_, lchar_);
	return std::make_pair(this, this);
}

bool Word::has_parent() {
	return (lparent_ != NULL) || (rparent_ != NULL);
}

void Word::counter(Side side, unsigned int count) {
	if (side == LEFT)
		lcounter(count);
	else if (side == RIGHT)
		rcounter(count);
}

int Word::counter(Side side) {
	if (side == LEFT)
		return lcounter();
	else if (side == RIGHT)
		return rcounter();
	return 0;
}

Word::string_type Word::array_to_string(array_type& wa) {
	return array_to_string(wa, 0, wa.size());
}

Word::string_type Word::array_to_string(array_type& wa, int idx, int len) {
	string_type tmp_str;
	assert(idx >= 0);
	assert(len <= (int)wa.size());
	for (int i = idx; i < (idx + len); i++)
		tmp_str.append(wa[i]->chars());
	return tmp_str;
}

Word::string_array Word::array_to_array(const array_type& wa) {
	string_array ca;
	for (int i = 0; i < (int)wa.size(); i++)
		ca.push_back(wa[i]->chars());
	return ca;
}

void Word::array(std::vector<Word*> arr) {
	arr_.clear();
	for (int i = 0; i < (int)arr.size(); i++)
		arr_.push_back(arr[i]);

	//std::cout << "size (" << size_ << ":" << arr.size() << ")"std::endl;
	assert(size() == (int)arr.size());
	//std::copy(arr_.begin(), arr_.end(), arr.begin());
}

Word::string_type Word::subchars(int idx, int len) {
	string_type substr("");
	subchars(substr, idx, len);
	return substr;
}

void Word::subchars(string_type& substr, int idx, int len) {
	assert(substr.length() == 0);
	assert((idx + len) <= size());

	int count = 0;
	for (int i = idx; i < (idx + len); i++) {
		string_type str = arr_[i]->chars();
		substr.append(str);
		count += str.length();
	}
	assert(count == substr.length());
}

Word::array_type Word::subarray(int idx, int len) {
	assert((idx + len) <= size());

	array_type ca(len);
	for (int i = 0; i < len; i++)
		ca.push_back(arr_[idx + i]);
	return ca;
}

/**
 * looking for a sub-word
 */
Word* Word::subword(int idx, int len) {
	Word* w_ptr = this;

	for (int i = 0; i < idx; i++)
		w_ptr = w_ptr->rparent();

	int n = w_ptr->size() - len;
	while (n > 0) {
		w_ptr = w_ptr->lparent();
		n--;
	}
	assert(w_ptr != NULL);
	return w_ptr;
}

void Word::is_word(bool b) {
	is_word_ = b;
	if (b) seged_ = b;

	word_ptr_type lp_ptr = lparent_;

	while (lp_ptr != NULL && !lp_ptr->is_word()) {
		if (lp_ptr->freq() == freq_)
			lp_ptr->seged(true);
		else
			break;
		lp_ptr = lp_ptr->lparent();
	}
}

void Word::seged_if(unsigned int freq) {
	if (freq == freq_)
		seged_ = true;
}

Word* Word::find_rchild(string_type a_char) {
	std::map<std::string, Word*>::const_iterator it = rcchar_.find(a_char);
	if (it != rcchar_.end())
		return it->second;
	return NULL;
}

void Word::children_do(void (*function_ptr)(Word*)) {
	//(*function_ptr_) = (void*)function_ptr_;
	for (int i = 0; i < children_.size(); i++) {
		(*function_ptr)(children_[i]);
		children_[i]->children_do(function_ptr);
	}
}

void Word::justify(unsigned int min) {
	if (lparent() != NULL)
		lparent()->justify();


	if (rparent() != NULL)
		rparent()->justify();

	if (lparent() != NULL)
		lparent()->reduce_freq(freq(), min);


	if (rparent() != NULL)
		rparent()->reduce_freq(freq(), min);
}

void Word::reduce_freq(unsigned int freq, unsigned int min) {
	if (freq_ == min)
		return;

	freq_ = (freq_ > freq) ? freq_ - freq : 0;
}

void Word::cal_p(double base) {
/*	if (base_ == 0.0)
		base_ = base;

	assert(base_ != 0.0);
	p_ = static_cast<double>(freq())/base_;*/
#ifdef DEBUG
		cout << "Calculating probabilty of "
			<< chars() << " with "
			<< freq() << " over "
			<< base << endl;
#endif

	assert(base != 0.0);

	//if (p == 0.0)
	p_ = static_cast<double>(freq())/base;

}

void Word::cal_a() {
	right_a_ = left_a_ = 0.0;

	if (size_ == 1)
		return;

	if (size_ == 2) {
		assert(rchar_ != NULL);
		assert(lparent_ != NULL);
		assert(lparent_->size() == 1);
		left_a_ = log(p_ / (lparent_->p()*rchar_->p()));
	} else {
//		int start = size_/2;
//
//		for (int i = start; i > 0; i--) {
//			for (int j = start; j < size_; j++) {
//				Word* lw = this->subword(i - 1, start - i + 1);
//				Word* rw = this->subword(start, j - start + 1);
//				assert(lw != NULL);
//				assert(rw != NULL);
//
//				double mi = log(p_ / (lw->p()*rw_->p()));
//				left_a_ += mi;
//			}
//		}
		if (UNISEG_settings::instance().debug)
						cout << endl << "calculating overall association score for " << this->chars() << endl;
		left_a_ = cal_a(size_/2);
		if ((size_ % 2) == 1)
			right_a_ = cal_a((size_/2) + 1);

	}
}

double Word::cal_a(int start) {
	double a = 0.0;

	if (UNISEG_settings::instance().mean == 5
				&& UNISEG_settings::instance().mi == 2)
		a = std::numeric_limits<double>::max();
	else if (UNISEG_settings::instance().mean == 5
			&& UNISEG_settings::instance().mi == 3)
		a = std::numeric_limits<double>::min();

	int count = 0;
	for (int i = start; i > 0; i--) {
		for (int j = start; j < size_; j++) {
			count++;
			Word* lw = this->subword(i - 1, start - i + 1);
			Word* rw = this->subword(start, j - start + 1);

			assert(lw != NULL);
			assert(rw != NULL);

			int ww_len = j - i + 2;
			assert((lw->size() + rw->size()) == ww_len);
			Word* ww = this->subword(i -1, ww_len);
			assert(ww != NULL);

			double mi = log(ww->p() / (lw->p()*rw->p()));
			double sign = (mi > 0) ? 1.0 : -1.0;
			double tmp = sign * mi * mi;

			if (UNISEG_settings::instance().debug)
				cout << "calculating association score for log "
					<< ww->chars() << "(" << ww->p() << ") over "
					<< lw->chars() << "(" << lw->p() << ") "
					<< " "
					<< rw->chars() << "(" << rw->p() << ") "
					<< " and get "
					<< tmp
					<< endl;

			if (mi < 0)
				assert(sign == -1.0);

			if (UNISEG_settings::instance().mean == 5) {
				switch (UNISEG_settings::instance().mi) {
					case 1: // sum
					case 4: // mean, average
						a += tmp;
						break;
					case 2: // choose minimum
					{
						if (tmp < a)
							a = tmp;
						break;
					}
					case 3:
					{
						if (tmp > a)
							a = tmp;
						break;
					}
					default:
						break;
				}
			}

		}
	}

	if (UNISEG_settings::instance().mean == 5
			&& UNISEG_settings::instance().mi == 4)
		a /= count;

	return a;
}