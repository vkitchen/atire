/*
 * UNISEG_PLUGIN.C
 *
 *  Created on: Jul 10, 2009
 *      Author: monfee
 */

#include "uniseg.h"
#include "seg.h"
#include "qfreq.h"
#include "encoding_factory.h"
#include "uniseg_settings.h"
#include "utilities.h"

#include <iostream>

using namespace std;

UNISEG_uniseg::UNISEG_uniseg() : uniseg_plugin_interface()
{
    //output_ = 0;
    count_ = 0;
    //seger = NULL;
    enc_ = UNISEG_encoding_factory::instance().get_encoding();

    // load the string frequency table
    cerr << "##################### initializing UNISEG segmentation module ######################" << endl;
    //seger.load_frqs();
    QFreq::instance().load_freq();
    cerr << "############################# finished initialization ##############################" << endl;
}

UNISEG_uniseg::~UNISEG_uniseg()
{
    cleanup();
}

void UNISEG_uniseg::cleanup()
{
}

const unsigned char *UNISEG_uniseg::do_segmentation(unsigned char *c, int length)
{
	if (!c || strlen((char *)c) == 0 || length == 0)
		return NULL;

	/**
	 * segment a few characters a time
	 */
	unsigned char *current = c;
	unsigned char *next = current;
	unsigned char *end = c + length;
	const long step = UNISEG_settings::instance().max;
	long segmented_len = 0;
	long how_far = 0;
	long distance = 0;
	long count = 0;
	bool flag = false;
	bool stop = false;
	bool need_eligibility_check = QFreq::instance().need_eligibility_check();
	output_.clear();
	count_ = 0;

	while (current < end) {
		count = 0;

		if (output_.length() > 0)
			output_.append(" ");

		while (current < end && isspace(*current))
			output_.push_back(*current++);

		next = current;
		while (next < end) {
			enc_->test_char(next);
			if (enc_->lang() == uniseg_encoding::CHINESE)
				while (next < end && enc_->lang() == uniseg_encoding::CHINESE && count < step) {
					next += enc_->howmanybytes();
					enc_->test_char(next);
					count++;
				}

			if (count > 0)
				break;

//			if (*next & 80) {
//				current = next += enc_->howmanybytes();
//				enc_->test_char(next);
//				while (next < end && enc_->lang() != uniseg_encoding::CHINESE && enc_->lang() != uniseg_encoding::SPACE) {
//					next += enc_->howmanybytes();
//					output_.append((const char *)current, enc_->howmanybytes());
//					enc_->test_char(next);
//					current = next;
//				}
//			}
//			else {
				string_array temp_word_array;
				unsigned char *pre = next;
				next += enc_->howmanybytes();
				temp_word_array.push_back(string_type(pre, next));
				enc_->test_char(next);

				while (next < end && enc_->lang() != uniseg_encoding::CHINESE && enc_->lang() != uniseg_encoding::SPACE) {
					pre = next;
					next += enc_->howmanybytes();
					temp_word_array.push_back(string_type(pre, next));
					enc_->test_char(next);
				}

				while (next < end && enc_->lang() == uniseg_encoding::CHINESE) {
					pre = next;
					next += enc_->howmanybytes();
					temp_word_array.push_back(string_type(pre, next));
					string_type word_str;
					arraytostring(temp_word_array, word_str);
					if (QFreq::instance().fuzzy_search_dic(word_str)) {
						next += enc_->howmanybytes();
						enc_->test_char(next);
					}
					else {
						temp_word_array.pop_back();
						break;
					}
				}
				while (temp_word_array.size() > 1 && (*temp_word_array.back().c_str() & 0x80)) {
					string_type word_str;
					arraytostring(temp_word_array, word_str);
					if (!QFreq::instance().is_word(word_str))
						temp_word_array.pop_back();
					else
						break;
				}
				string_type non_all_chinese_word;
				arraytostring(temp_word_array, non_all_chinese_word);
				output_.append(non_all_chinese_word);
				current += non_all_chinese_word.length();
			//}
		}

		how_far = next - current;

	    flag = next < end;
	    if (how_far > 0 && count > 1) {
			seger_.input(current, how_far);
			seger_.start();
			const array_type& words_list = seger_.best_words();
			long i = 0;
			long size = (flag && words_list.size() > 1) ? words_list.size() - 1 : words_list.size();
			if (size > 0)
				for (; i < size; i++) {
					bool has_word_pair = words_list[i]->has_word_pair() && !words_list[i]->is_word();
					if (i > 0 /*&& *(output_.end()--) != ' '*/)
						output_.append(" ");
					if (words_list[i]->size() == 1
							|| has_word_pair
							|| (need_eligibility_check && QFreq::instance().eligibility_check(words_list[i]))) {
						string_type& word = words_list[i]->chars();
						if (has_word_pair)
							output_.append(words_list[i]->left()->chars() + " " + words_list[i]->right()->chars());
						else
							output_.append(word);
						segmented_len += word.length();
					}
					else {
						word_ptr_type lparent = words_list[i]->lparent();
						assert(lparent != NULL);
						while (lparent->size() > 1 && !lparent->is_word()) {
							if (lparent->lparent() != NULL)
								lparent = lparent->lparent();
							else
								break;
						}
						output_.append(lparent->chars());
						segmented_len += lparent->chars().length();
					}
				}
			else
				segmented_len = how_far;
			// put the last segment back to the remaining characters
			current += segmented_len;
			segmented_len = 0;
			count_ += (words_list.size() == size) ? 0 : count - words_list[size - 1]->size();
	    }
	    else if (how_far > 0) {
			//how_far = how_far > 0 ? how_far : enc_->howmanybytes();
			output_.append((const char *)current, (size_t)how_far);
			current += how_far;
	    }
	}

	return get_output();
}

const unsigned char *UNISEG_uniseg::get_input()
{
	return (unsigned char *)input_.c_str();
}

const unsigned char *UNISEG_uniseg::get_output()
{
	return (unsigned char *)output_.c_str();
}

int UNISEG_uniseg::get_count() { return count_; }


#ifdef  __cplusplus
extern "C" {
#endif
	uniseg_plugin_interface *maker() {
	   return new UNISEG_uniseg;
	}

#ifdef WITH_ANT_PLUGIN
	class proxy
	{
	public:
	   proxy() {
		   std::cerr << "Registering UNISEG to the plugin factory..." << std::endl;
		  // register the maker with the factory
		   plugin_factory[uniseg_plugin_interface::SEGMENTATION].maker = maker;
	   }
	};
	// our one instance of the proxy
	proxy p;
#endif

#ifdef  __cplusplus
}
#endif
