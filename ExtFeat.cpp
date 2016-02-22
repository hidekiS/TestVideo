/*
 * FFT3D.cpp
 *
 *  Created on: 2009/10/24
 *      Author: yoshi
 */

#include "ExtFeat.h"
#define _USE_MATH_DEFINES
#include <iostream>
#include <math.h>
#include <stdlib.h>
using namespace std;
/**
 * @param[in] start_frame : このフレーム番号からFFT3Dを開始する
 * @param[in] end_frame : このフレーム番号まで（dim3interによっては, 端数が切り捨てられることもある）FFT3Dを行う.
 *                        0の場合は最終フレームまでFFT3Dを行う.
 * @param[in] forceDiv : 強制的にこの数でファイルを分割（HDDを用いる）, 0ではメモリ量から動的に求める
 * @param[in] outFile : 周波数データ出力先
 * @return 成功した場合0, 失敗で-1
 */

void ExtFeat::extract_features(Video_y& video,
		config cfg,
		list<size_t> *lhash,
		list<size_t> *lframeNuminShot,
		list<double> *lShotLumAvg,
		double *CbCrAvg){

	/*read frame*/
	AVFrame tmpF;
	AVPacket packet;
	int isFinish = 0;
	int ret;
	double uavg=0,vavg=0,yavg=0;
	double umax=0,umin=0,vmax=0,vmin=0;
	size_t nbright_frame = 0;
	//void *buffer_to_free = 0;
	unsigned char* buffer ;
	AVFrame avframe;

	avcodec_alloc_frame();
	int numBytes = avpicture_get_size(video.pixf,video.getWidth(), video.getHeight());

	buffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));
	if(buffer == NULL) return ;


	/* バッファをフレームへセット */
	avpicture_fill((AVPicture *) &avframe , buffer, video.pixf,
			video.getWidth(), video.getHeight());

	/* allocate picture correctly */
	avcodec_get_frame_defaults(&tmpF);

	list<vector<double> > lydivframe; //ydivframeのリスト
	list<double> lframeLumAvg;	//各フレームの平均輝度値を格納
	double vyavg=0; //ビデオ全体の平均輝度値

	clock_t start,end;
	start = clock();

	for(size_t i=0;true;i++){

		/* ファイルからパケットを読み込む */
		while((ret=av_read_frame(video.inf.formatCtx, &packet)) >= 0){
			if(packet.stream_index != video.inf.streamIndex){
				/* 動画ストリーム以外は飛ばす */
				av_free_packet(&packet);
				continue;
			}
			/* パケットの複合 */
			avcodec_decode_video2(video.inf.codecCtx, &tmpF, &isFinish, &packet);
			if(isFinish){ break; }
			av_free_packet(&packet);
		}
		video.decodedF++;
		if(ret >= 0){
			/* 画像変換 */
			av_picture_copy((AVPicture*)&avframe, (AVPicture*)&tmpF, video.pixf, video.getWidth(), video.getHeight());
		}

		vector<double> ydivframe(cfg.ver_div*cfg.hor_div); //ブロックごとの輝度値を格納

		//todo
		size_t width = tmpF.width;
		size_t height = tmpF.height-cfg.top_cut-cfg.bottom_cut;
		size_t bwidth = width/cfg.ver_div;
		size_t bheight = height/cfg.hor_div;
		size_t skip_ver=2; //ピクセルのスキップ数(縦軸)
		size_t skip_hor=2; //(横軸)
		yavg = 0;

		/*ブロックごとの平均輝度値を取得*/
		for(size_t y=0;y<cfg.ver_div;y++){
			for(size_t x=0;x<cfg.hor_div;x++){
				for(size_t j=0;j<bheight;j+=skip_ver){
					for(size_t i=0;i<bwidth;i+=skip_hor){
						double tmpy =  (double)tmpF.data[0][(cfg.top_cut + y*bheight + j)*tmpF.linesize[0] + x*bwidth + i];
						ydivframe[y*cfg.ver_div+x] += tmpy;
						yavg += tmpy;
					}
				}
				ydivframe[y*cfg.ver_div+x] /= (bwidth/skip_hor*bheight/skip_ver);
			}
		}

		yavg /= (width/skip_hor*height/skip_ver);
		lframeLumAvg.push_back(yavg);
		lydivframe.push_back(ydivframe);	//リストへpush

		//debug
		/*
		for(size_t y=0;y<cfg.hor_div;y++){
			for(size_t x=0;x<cfg.ver_div;x++){
				cout << lydivframe.back()[y*cfg.ver_div+x] << "\t";
			}
			cout << endl;
		}*/
		/*
		cout << (int)tmpF.linesize[0]<< "\t"; //384
		cout << tmpF.width << "\t" << tmpF.height << endl; // 352 240
		cout << width << "\t" << height << endl; // 352 240
		cout << bwidth << "\t" << bheight << endl;
		cout << yavg <<endl;
		 */
		vyavg += yavg;


		/*平均輝度値が50以上のフレーム*/
		if(yavg >= 50){
			//CbCrを取得
			uavg = 0;
			vavg = 0;
			nbright_frame++;
			for(size_t i=0;i<height/2;i++){
				for(size_t j=0;j<width/2;j++){
					uavg += (double)tmpF.data[1][j + i*tmpF.linesize[1]];
					vavg += (double)tmpF.data[2][j + i*tmpF.linesize[2]];
				}
			}
			uavg = uavg/(height/2*width/2) -128;
			vavg = vavg/(height/2*width/2) -128;
			//最大CbCrの更新
			if(umax == 0){umax = uavg;}	if(umin == 0){umin = uavg;}
			if(vmax == 0){vmax = vavg;}	if(vmin == 0){vmin = vavg;}

			if(uavg > umax){umax = uavg;}
			else if(uavg < umin){umin = uavg;}
			if(vavg > vmax){vmax = vavg;}
			else if(vavg < vmin){vmin = vavg;}
			CbCrAvg[0] += uavg;
			CbCrAvg[1] += vavg;
		}
		if(ret<0){
			vyavg /= i+1;
			cout << "framenum:"<<i+1<<"\t"<<"break" <<endl;
			break;
		}
	}

	end = clock();
	cout << (double)(end - start)/CLOCKS_PER_SEC <<"[s]"<<endl;


	//debug
	cout <<"avg:"<<vyavg <<endl;
	list<vector<unsigned int> > lbitdivframe; //divframeのリスト
	//輝度値の正規化
	normalize_divframe(cfg,lydivframe,&lbitdivframe,vyavg);

	//ハッシュ値の格納
	store_hash_value(cfg,lbitdivframe,lhash,lframeNuminShot);
	//各ショットの平均輝度値を格納
	store_shot_lum_avg(lframeLumAvg,lShotLumAvg,lframeNuminShot);

	//debug
	/*
	list<size_t>::iterator lframeNuminShotite = lframeNuminShot->begin();
	BOOST_FOREACH(size_t hash,*lhash){
		cout <<"hash:"<<hash <<"\t"<<"framenum:"<<*lframeNuminShotite<<endl;
		lframeNuminShotite++;
	}
	*/

	//動画の平均CbCrをCbCrAvgに格納
	cout << "nbright_frame:" << nbright_frame <<endl;
	if(nbright_frame == 0){return;}
	CbCrAvg[0] /= nbright_frame;
	CbCrAvg[1] /= nbright_frame;
	CbCrAvg[2] = umax; CbCrAvg[3] = umin; CbCrAvg[4] = vmax; CbCrAvg[5] = vmin;

	av_freep(&avframe);
	//	av_freep(&tmpF);
	return ;
}
//フレームの輝度情報を01に量子化
void ExtFeat::normalize_divframe(
		config cfg,
		list<vector<double> > lydivframe,
		list<vector<unsigned int> > *lbitdivframe,
		double myu
){
	BOOST_FOREACH(vector<double> ydivframe,lydivframe){
		vector<unsigned int> bitdivframe(cfg.ver_div*cfg.hor_div); //ブロックごとのハッシュ値[0,1]を格納
		size_t ite=0;
		BOOST_FOREACH(double Ixy,ydivframe){
			if((Ixy - myu)>=0 && Ixy >= cfg.min_lum){
				bitdivframe[ite++] = 1;
			}else{
				bitdivframe[ite++] = 0;
			}
		}
		lbitdivframe->push_back(bitdivframe);
	}
	return;
}

/*lhashにハッシュ値列を、lframeNuminShotにショットに含まれるフレーム数列を格納*/
void ExtFeat::store_hash_value(
		config cfg,
		list<vector<unsigned int> > lbitdivframe,
		list<size_t> *lhash,
		list<size_t> *lframeNuminShot){
	vector<unsigned int> headofShot(cfg.ver_div*cfg.hor_div); //ショットの先頭フレームのハッシュ値
	vector<unsigned int> tmpframe(cfg.ver_div*cfg.hor_div); //次ショットの先頭フレームの一時的格納
	headofShot = lbitdivframe.front();
	size_t frameite = 0, //動画中でのフレーム番号
			blockite, //フレーム中でのブロック番号
			headofShotite; //ショットの先頭の位置
	size_t frameInshot = 0,
			count, //変化したブロックの数
			transfnum = 0; //変化量がしきい値を越えたフレームの数
	bool countflag = false;
	headofShotite = 1;

	BOOST_FOREACH(vector<unsigned int> bitdivframe,lbitdivframe){
		frameite++;
		frameInshot++;
		blockite = 0;
		count = 0;
		if(frameite > 1){
		//	cout << frameite <<":";
			BOOST_FOREACH(unsigned int Ixy,bitdivframe){
				//cout << Ixy <<"\ ";
				if(headofShot[blockite] != Ixy){
					count++;
				}
				blockite++;
			}
			//cout <<"rate:"<<(double)count/(cfg.ver_div*cfg.hor_div)<<endl;
			if((double)count/(cfg.ver_div*cfg.hor_div) >= cfg.block_trans){
				if(!countflag){
					tmpframe = bitdivframe;
					countflag = true;
				}
				transfnum++;
			//	cout << transfnum <<endl;
			}else if(countflag && (double)count/(cfg.ver_div*cfg.hor_div) < cfg.block_trans){
				transfnum--;
				if(transfnum == 0){
					countflag = false;
				}
			}

			if(transfnum >= cfg.min_frame || frameite >= lbitdivframe.size()){
			//	cout <<"shot change"<<endl;
				lhash->push_back(toInt(cfg,headofShot));
				lframeNuminShot->push_back(frameInshot - transfnum);
				headofShot = tmpframe;
				headofShotite = frameite-transfnum-1;
				frameInshot = transfnum;
				transfnum = 0;
				countflag = false;
			}
		}
	}
}

//ショットの平均輝度値をlShotLumAvgに格納
void ExtFeat::store_shot_lum_avg(list<double> lframeLumAvg,
		list<double> *lShotLumAvg,
		list<size_t> *lframeNuminShot){
	list<double>::iterator lframeLumAvgite = lframeLumAvg.begin();

	BOOST_FOREACH(size_t fnum,*lframeNuminShot){
		lShotLumAvg->push_back(*lframeLumAvgite);
		advance(lframeLumAvgite,fnum);
	}
}

//vector<unsigned int> に格納された01列を10進数に変換して返す
unsigned int ExtFeat::toInt(config cfg,vector<unsigned int> headofShot){
	size_t digit=cfg.ver_div*cfg.hor_div;
	size_t hash=0;
	size_t N=1;
	for(size_t i=0;i<digit-1;i++){
		N *= 2;
	}
	BOOST_FOREACH(unsigned int Ixy,headofShot){
		if(Ixy == 1){
			hash += N;
		}
		N /= 2;
	}
	return hash;
}



