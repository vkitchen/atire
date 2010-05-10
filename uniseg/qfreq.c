/*
 * qfreq.cpp
 *
 *  Created on: Jan 15, 2009
 *      Author: monfee
 */

#include "qfreq.h"
#include "uniseg_settings.h"
#include "freq_loader.h"
#include "convert.h"
#include "freq_file.h"
#include "index_file.h"

#include <iostream>

using namespace std;


QFreq::QFreq() {
	k_ = -1;
	loaded_ = false;
}

QFreq::~QFreq() {

}

void QFreq::load_freq(int n, bool force) {

	if (k_ > -1 && !force)
		return;

	/************************************
	 *  LOAD FREQUENCY TABLE IN MEMORY
	 ***********************************/
	cerr << "loading frequency files " << endl;

	//FreqLoader loader(freq_);

	if (UNISEG_settings::instance().freqs_path.length() <= 0) {
		cerr << "cann't find the frequency table path " << endl;
		exit(-1);
	}

	//loader.load(UNISEG_settings::instance().freqs_path, n);
	k_ = 1;
	//	int num = 0;
	cerr << "Loading files from " << UNISEG_settings::instance().freqs_path << endl;
	UNISEG_settings::instance().load = true;

	while (k_ <= n) {
		string name = stringify(k_);

		FreqFile *freq_file = new FreqFile(name, freq_);
		freq_file->path(UNISEG_settings::instance().freqs_path);

		if (!freq_file->exist()) {
			k_--;
			std::cerr << "No such file:" << freq_file->fullpathname() << std::endl;
			delete freq_file;
			break;
		}

		freq_file->wlen(k_);
		freq_file->read_in_memory();
		freq_files_.push_back(freq_file);

//		if (k_ > 1) {
//			/************************************
//			 * LOAD THE INDEX OF TERMS ONLY FIRST
//			 ************************************/
//			//freq_file->read_with_index();
//			IndexFile idxf(name);
//			idxf.path(UNISEG_settings::instance().freqs_path);
//			idxf.wlen(k_);
//			idxf.read(freq_);
//		}
//		else
		if (k_ == 1)
			freq_file->read();
		else
			freq_file->load_index();

		k_++;
	}

	freq_.cal_sum_n_avg();
	//k_ = loader.count();

	loaded_ = true;
}

word_ptr_type QFreq::find(string_type word) {
	string_array ca;
	to_string_array(word, ca);

	int min = 1, max = ca.size(); //freq_.array_size();
	//int len = ca.size();

	for (int i = min; i <= max; i++) {
		for (int j = 0; j < (max - i) + 1; j++) {
			//tmp_str.append(ca[j]);
			string_type tmp_str("");
			int k = j;
			while (k < (j + i)) {
				tmp_str.append(ca[k]);
				++k;
			}

			word_ptr_type tmp_word = freq_.find(tmp_str);
			load(tmp_word);
			//word_ptr_type tmp_word = freq_.find();
		}
	}
	return freq_.find(word);
}

void QFreq::load(word_ptr_type word)
{
	int len = 1;
	if (word != NULL && (len = word->size()) <= freq_files_.size()) {
		assert(len > 0);
		freq_files_[len]->read_term(word);
	}
}

void QFreq::load(int index)
{
	if (index > 1) {
		if (index > freq_files_.size())
			index = freq_files_.size();

		--index;
		freq_files_[index]->load();
	}
	else if (index == -1) {
		for (int i = 1; i < freq_files_.size(); i++)
			freq_files_[i]->load();
	}
}

QFreq& QFreq::instance() {
	static QFreq inst;
	return inst;
}
