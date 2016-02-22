/*
 * FFT3D.h
 *
 *  Created on: 2009/10/24
 *      Author: yoshi
 */

#ifndef FFT3D_H_
#define FFT3D_H_

#include <stdio.h>
#include "Video_y.h"
//using namespace std;
/**
 * 特徴量抽出機能関連のクラス
 */
class ExtFeat {

public:
	ExtFeat() {}
	virtual ~ExtFeat(){}

	void extract_features(Video_y& video,
			config cfg,
			list<size_t> *lhash,
			list<size_t> *lframeNuminShot,
			list<double> *lShotLumAvg,
			double *CbCrAvg);

	int picture_out(Video_y& video, int start_frame, int end_frame, int dim3, int merge, int do_ffmpeg);
	int trim_freqdata(const char *freq_data, int start_seg, int end_seg, const char *out_freq_data);
	int cut_out_freqdata(const char *freq_data, int start_seg, int end_seg, const char *out_from_data, const char *out_to_data);
	int merge_out(const char *freq_in, const char *merged_out, int merge);

	void normalize_divframe(
			config cfg,
			list<vector<double> > lydivframe,
			list<vector<unsigned int> > *lbitdivframe,
			double myu
	);
	void store_hash_value(
			config cfg,
			list<vector<unsigned int> > lbitdivframe,
			list<size_t> *lhash,
			list<size_t> *lframeNuminShot
	);
	void store_shot_lum_avg(list<double> lframeLumAvg,
			list<double> *lShotLumAvg,
			list<size_t> *lframeNuminShot
	);
	unsigned int toInt(config cfg,vector<unsigned int> headofShot);
};

#endif /* FFT3D_H_ */
