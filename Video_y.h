/*
 * Video_y.h
 *
 *  Created on: 2009/10/07
 *      Author: YoshikazuTakeda
 */

#ifndef VIDEO_Y_H_
#define VIDEO_Y_H_

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/avstring.h>
#include <libavdevice/avdevice.h>
}

#include <string>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <vector>
#include <time.h>

using namespace std;

/**
 * AVFrameのラッパー
 */
struct MyAVFrame {
	AVFrame frame;

	MyAVFrame(int width, int height, PixelFormat pixf){
		avcodec_get_frame_defaults(&frame);
		avpicture_alloc((AVPicture*)&frame, pixf, width, height);
	}
	~MyAVFrame(){
		avpicture_free((AVPicture*)&frame);
	}
};
//設定を格納する構造体
struct config{
	size_t ver_div; //縦方向の分割数
	size_t hor_div; //横方向の分割数
	double block_trans; //全ブロックのうち、変化したブロックの割合。これを越えると別のショットとして扱う。
	size_t min_frame; //この数だけショットの先頭と異なるフレームが続いたとき、そこから先は別のショットである。
	size_t top_cut; //動画情報をカットするために、上からTOPCUTピクセルを無視する。基本45
	size_t bottom_cut;//同上 30
	size_t min_lum; //この値より小さい輝度値を持つブロックは0に量子化される
	bool out_Col_info; //色情報を出力するかどうか
	bool out_Lum_info; //ハッシュ値による輝度情報を出力するかどうか
};

/**
 * フレームの1画素のビット数
 */
enum FrameBit {
	bit8, bit16
};

/**
 * 入力ファイル情報
 */
struct ST_inputFormat {
	AVFormatContext *formatCtx;
	int streamIndex;
	AVCodecContext *codecCtx;
	SwsContext *swsCtx;
	ST_inputFormat();
	virtual ~ST_inputFormat();
};

/**
 * ファイル出力用構造体
 */
struct ST_outputFormat {
	AVOutputFormat *format;
	AVFormatContext *outFCtx;
	AVStream *stream;
	AVCodec *codec;
	AVCodecContext *codecCtx;
	uint8_t *buf;
	ST_outputFormat();
	virtual ~ST_outputFormat();
};

/**
 * ビデオを扱うクラス
 */
class Video_y {
	int width;
	int src_width;
	int height;
	int src_height;

	int64_t frame;// 総フレーム数
	int fpsNume;// fpsの分子
	int fpsDeno;// fpsの分母

	int mode;// 擬似フレームモード=0, 実際の動画=1.


	FILE *debugF;
	int dodebug;

	int do_deinterlace;


	ST_outputFormat outf;

public:
	int64_t decodedF;// デコード済みのフレーム数
	PixelFormat pixf;
	ST_inputFormat inf;
	Video_y();
	Video_y(int debug);
	Video_y(int width, int height, int frameNum, int mode, int debug);
	virtual ~Video_y();

	int getWidth() const {
		return width;
	}
	int getHeight() const {
		return height;
	}
	int getDecodedF() const {
		return decodedF;
	}
	int getCurrentF() const {
		return decodedF - 1;
	}
	int getFrame() const {
		return frame;
	}
	int getFpsDeno() const {
		return fpsDeno;
	}
	int getFpsNume() const {
		return fpsNume;
	}
	void setMode(int mode) {
		this->mode = mode;
	}
	void setDodebug(int deb) {
		dodebug = deb;
	}
	void setDo_deinterlace(int deint){
		do_deinterlace = deint;
	}

public:
	void dispose_inputFormat();
	int ffmpeg_load_movie(const char* filename, PixelFormat convertTo);
	//int ffmpeg_save_frame_pgm(const AVFrame *frame, PixelFormat pict, const char *filename);
	//	int ffmpeg_save_frame(const AVFrame *frame, const char *filename);
	void ffmpeg_pre_process_video_frame(AVCodecContext *dec, AVPicture *picture, void *&bufp);
	int ffmpeg_get_next_decoded_frame(AVFrame *dst, int doRescale);
	int skip_to(int start);

private:
	void init_ffmpeg();
	void init_member();
	int get_upperLog2(int num);

};

#endif /* VIDEO_Y_H_ */
