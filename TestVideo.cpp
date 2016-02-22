//============================================================================
// Name        : TestVideo.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include "ExtFeat.h"
#include "Video_y.h"
#include <string>

void main2(char* fname,config cfg) {
	ExtFeat f3;
	Video_y vid;

	//load video
	fprintf(stdout, "\ninput file is %s\n", fname);
	vid.ffmpeg_load_movie(fname, PIX_FMT_YUV420P);

	//get signal info
	list<size_t> lhash;
	list<size_t> lframeNuminShot;
	list<double> lShotLumAvg;
	double CbCrAvg[6];
	f3.extract_features(vid,cfg,&lhash,&lframeNuminShot,&lShotLumAvg,CbCrAvg);
	for(int i=0;i<6;i++){
		if(!CbCrAvg[i]){
			return;
		}
	}



	string str(fname);
	list<string> lstr;
	boost::split(lstr, str, boost::is_any_of("/."));

	if(cfg.out_Lum_info){
		string hashfname("hash/hash");
		char tmp[2];
		sprintf(tmp,"%d",cfg.ver_div);
		hashfname.append(tmp);
		hashfname.append("by");
		sprintf(tmp,"%d",cfg.hor_div);
		hashfname.append(tmp);
		//ofstreamクラスを使ってハッシュ値ファイルを書き込みオープン
		ofstream ofs(hashfname.c_str(),ios::app);
		if( !ofs ){
			cout << "Error: cannot open file(" << hashfname << ")" << endl;
			return ;
		}

		//動画情報を出力
		lstr.pop_back();
		ofs << lstr.back() << "\t"; //ファイル名
		ofs << (size_t)cfg.ver_div*cfg.hor_div << "\t"; //分割数
		ofs << cfg.min_frame << "\t"; //最小ショット長
		ofs << (size_t)lframeNuminShot.size() << "\t"; //ショット数

		BOOST_FOREACH(size_t hash,lhash){
			ofs << hash << "\t";
		}
		BOOST_FOREACH(size_t frameNuminShot,lframeNuminShot){
			ofs << frameNuminShot << "\t";
		}
		BOOST_FOREACH(double ShotLumAvg,lShotLumAvg){
			ofs << ShotLumAvg << "\t";
		}
		ofs << endl;
		ofs.close();
	}

	if(cfg.out_Col_info){
		//ofstreamクラスを使って色情報ファイルを書き込みオープン
		ofstream ofsC("CbCravg",ios::app);
		if( !ofsC ){
			cout << "Error: cannot open file(" << "CbCravg" << ")" << endl;
			return ;
		}

		//平均CbCrを出力
		//todo
		ofsC << lstr.back() << "\t";
		for(int i=0;i<6;i++){
			ofsC << CbCrAvg[i] << "\t" ;
		}
		ofsC << endl;

		ofsC.close();
	}
}

int main(int argc, char **argv) {

	config cfg;
	//初期化
	cfg.ver_div = 2;
	cfg.hor_div = 2;
	cfg.block_trans = 0.2;
	cfg.min_frame = 8;
	cfg.top_cut = 0;
	cfg.bottom_cut = 0;
	cfg.min_lum = 50;
	cfg.out_Col_info = false;
	cfg.out_Lum_info = false;

	int result;
	char fname[100];
	while((result=getopt(argc,argv,"f:v:h:cm:CL"))!=-1){
		switch(result){

		/* 値をとらないオプション */
		case 'c':	//上下のカットを行う
			cout << "top/bottom cut ON" <<endl;
			cfg.top_cut = 45;
			cfg.bottom_cut = 30;
			break;
		case 'f': /*getoptのクソ仕様のせいで
		'-'がついていない引数は全部弾かれるため
		入力動画ファイル名は"-f"の後に書く必要がある*/
			strcpy(fname,optarg);
			break;
		case 'v':/* 値を取る引数の場合は外部変数optargにその値を格納する. */
			cfg.ver_div = boost::lexical_cast<size_t>(optarg);
			break;
		case 'h':
			cfg.hor_div = boost::lexical_cast<size_t>(optarg);
			break;
		case 'm':
			cfg.min_frame = boost::lexical_cast<size_t>(optarg);
			break;
		case 'C':
			cfg.out_Col_info = true;
			break;
		case 'L':
			cfg.out_Lum_info = true;
			break;
			/* 以下二つのcaseは意味がないようだ.
		 getoptが直接エラーを出力してくれるから.
		 プログラムを終了するなら意味があるかも知れない */
		case ':':
			/* 値を取る引数に値がなかった場合:を返す. */
			cout << result <<" needs value"<<endl;
			break;

			/* getoptの引数で指定されなかったオプションを受け取ると?を返す. */
		case '?':
			cout << stdout<<"unknown"<<endl;;
			break;
		}
	}

	main2(fname,cfg);
	return 0;
}


