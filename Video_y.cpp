/*
 * Video_y.cpp
 *
 *  Created on: 2009/10/07
 *      Author: YoshikazuTakeda
 */

#include "Video_y.h"
//#include "Image_y.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdlib.h>

ST_inputFormat::ST_inputFormat(){
	formatCtx = 0;
	codecCtx = 0;
	swsCtx = 0;
	streamIndex = -1;
}
ST_inputFormat::~ST_inputFormat(){
	if(swsCtx){
		sws_freeContext(swsCtx);
	}
	if(codecCtx){
		avcodec_close(codecCtx);
	}
	if(formatCtx){
		//av_close_input_file(formatCtx);
		avformat_close_input(&formatCtx);
	}
}

ST_outputFormat::ST_outputFormat(){
	format = 0;
	outFCtx = 0;
	stream = 0;
	codec = 0;
	codecCtx = 0;
	buf = 0;
}
ST_outputFormat::~ST_outputFormat(){
	if(format){
		if(!(format->flags & AVFMT_NOFILE)){
			//url_fclose(outFCtx->pb);
			avio_close(outFCtx->pb);
		}
	}

	/* コーデックを閉じる */
	if(stream){
		avcodec_close(stream->codec);
	}
	if(buf){
		av_freep(&buf);
	}
	if(outFCtx){
		for(unsigned int i=0; i<outFCtx->nb_streams; i++){
			av_freep(outFCtx->streams[i]->codec);
			av_freep(outFCtx->streams[i]);
		}
		av_freep(&outFCtx);
	}
}

Video_y::Video_y() {
	init_member();
	init_ffmpeg();
}

Video_y::Video_y(int debug) {
	init_member();
	init_ffmpeg();
	dodebug = debug;
}

Video_y::Video_y(int width, int height, int frameNum, int mode, int debug) {
	init_member();
	init_ffmpeg();
	this->width = width;
	this->height = height;
	frame = frameNum;
	this->mode = mode;
	dodebug = debug;
}

Video_y::~Video_y() {
	// TODO Auto-generated destructor stub
	if(debugF){
		fclose(debugF);
	}
}

void Video_y::init_ffmpeg(){
	/* FFmpegの初期化 */
	do_deinterlace = 0;
	pixf = PIX_FMT_YUV420P;
	avcodec_register_all();
	avdevice_register_all();
	av_register_all();

}

void Video_y::init_member(){
	width = 0;
	src_width = 0;
	height = 0;
	src_height = 0;
	decodedF = 0;
	frame = 0;
	fpsNume = 0;
	fpsDeno = 0;

	//movie_name.copy_from("debug_movie");

	mode = 1;

	debugF = 0;
	dodebug = 0;
}

void Video_y::dispose_inputFormat(){
	if(inf.swsCtx){
		sws_freeContext(inf.swsCtx);
	}
	if(inf.codecCtx){
		avcodec_close(inf.codecCtx);
	}
	if(inf.formatCtx){
		//av_close_input_file(inf.formatCtx);
		avformat_close_input(&inf.formatCtx);
	}
	if(debugF){
		fclose(debugF);
	}
	inf.swsCtx = 0;
	inf.codecCtx = 0;
	inf.formatCtx = 0;
	debugF = 0;
}

int Video_y::get_upperLog2(int num){
	int b;
	for(b=1; b<num; b<<=1){ }
	return b;
}


/**
 * ムービーファイルを読み込む
 * @param[in] force_width ; 0以外の場合は, この幅に強制設定する
 * @param[in] force_height ; 0以外の場合は, この高さに強制設定する
 * @return 成功で0, 失敗で-1 todo
 */
int Video_y::ffmpeg_load_movie(const char* filename, PixelFormat convertTo){
	AVCodec *codec = 0;
	AVStream *tmpSt;

	dispose_inputFormat();

	if(!filename){
		fprintf(stderr, "Video_y::load_movie(); please input movie filename!!\n");
		return -1;
	}
	
	/* ファイルのヘッダを読み, フォーマットを得る */
	/*if(av_open_input_file(&inf.formatCtx, filename, NULL, 0, NULL)){
		fprintf(stderr ,"Video_y::load_movie(): av_open_input_file()\n");
		return -1;
	}*/
	if(avformat_open_input(&inf.formatCtx, filename, NULL, NULL)){
			fprintf(stderr ,"Video_y::load_movie(): av_open_input_file()\n");
			return -1;
	}
	/* ファイルの中身からストリーム情報を得る */
	/*if(av_find_stream_info(inf.formatCtx) < 0){
		fprintf(stderr ,"Video_y::load_movie(): av_find_stream_info()\n");
		return -1;
	}*/
	if(avformat_find_stream_info(inf.formatCtx,NULL) < 0){
		fprintf(stderr ,"Video_y::load_movie(): av_find_stream_info()\n");
		return -1;
	}
	/* ビデオストリームを探す */
	for(unsigned int i=0; i<inf.formatCtx->nb_streams; i++){
		if(inf.formatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
			inf.streamIndex = i;
			break;
		}
	}
	if(inf.streamIndex < 0){
		fprintf(stderr, "Video_y::ffmpeg_load_movie: Can't find video stream\n");
		return -1;
	}
	tmpSt = inf.formatCtx->streams[inf.streamIndex];
	inf.codecCtx = tmpSt->codec;

	/* コーデックの検索 */
	if(!(codec = avcodec_find_decoder(inf.codecCtx->codec_id))){
		fprintf(stderr, "Video_y::ffmpeg_load_movie: avcodec_find_decoder()\n");
		return -1;
	}
	/* コーデックを開く */
	/*if(avcodec_open(inf.codecCtx, codec)){
		fprintf(stderr, "Video_y::ffmpeg_load_movie: avcodec_open()\n");
		return -1;
	}*/
	if(avcodec_open2(inf.codecCtx, codec,NULL)){
			fprintf(stderr, "Video_y::ffmpeg_load_movie: avcodec_open()\n");
			return -1;
	}
	/* 画像変換の設定 */
	pixf = convertTo;
	width = get_upperLog2(inf.codecCtx->width);
	height = get_upperLog2(inf.codecCtx->height);
	inf.swsCtx = sws_getContext(inf.codecCtx->width, inf.codecCtx->height, inf.codecCtx->pix_fmt, width, height, pixf, SWS_LANCZOS, 0, 0, 0);
	if(!inf.swsCtx){
		fprintf(stderr, "Video_y::ffmpeg_load_movie; sws_getContext\n");
		return -1;
	}
	/* フィールドの設定 */
	mode = 0;
	src_width = inf.codecCtx->width;
	src_height = inf.codecCtx->height;
	frame = tmpSt->nb_frames;
	fpsNume = tmpSt->r_frame_rate.num;
	fpsDeno = tmpSt->r_frame_rate.den;

	return 0;
}
//
//int Video_y::ffmpeg_save_frame_pgm(const AVFrame *frame, PixelFormat pict, int width, int height, const char *filename) {
//	FILE *f = 0;
//	char buf[64];
//
//	f = fopen(filename, "wb");
//	if(!f){
//		perror(0);
//		return -1;
//	}
//
//	switch(pict){
//	case PIX_FMT_GRAY8:
//		fwrite("P5\n# yoshi\n", sizeof(char), 11, f);
//		sprintf(buf, "%d %d\n%d", width, height, 255);
//		fwrite(buf, sizeof(char), strlen(buf), f);
//		fwrite(frame->data[0], sizeof(uint8_t), width*height, f);
//		fclose(f);
//		break;
//	case PIX_FMT_YUV420P:
//		fwrite("P5\n# yoshi\n", sizeof(char), 11, f);
//		sprintf(buf, "%d %d\n%d", width, height, 255);
//		fwrite(buf, sizeof(char), strlen(buf), f);
//		fwrite(frame->data[0], sizeof(uint8_t), width*height, f);
//		fclose(f);
//		break;
//	default:
//		break;
//	}
//	return 0;
//}

//int Video_y::ffmpeg_save_frame(const AVFrame *frame, const char *filename){
//	CodecID codec_id;
//	int ret, buf_size;
//	int out_size;
//	AVPacket packet;
//
//	/* 画像出力のフォーマットを準備 */
//	outf.format = guess_format(0, filename, 0);///
//	if(outf.format == 0){
//		fprintf(stderr, "Video_y::save_frame; guess_format()\n");
//		return -1;
//	}
//
//	outf.outFCtx = avformat_alloc_context();
//	if(outf.outFCtx == 0){
//		fprintf(stderr, "Video_y::save_frame; av_alloc_format_context()\n");
//		return -1;
//	}
//	outf.outFCtx->oformat = outf.format;
//	av_strlcpy(outf.outFCtx->filename, filename, sizeof(outf.outFCtx->filename));
//
//	/* 新たなストリームを追加 */
//	outf.stream = av_new_stream(outf.outFCtx, 0);
//	if(outf.stream == 0){
//		fprintf(stderr, "Video_y::save_frame; av_new_stream()\n");
//		return -1;
//	}
//
//	/* コーデックを予測 */
//	codec_id = av_guess_codec(outf.outFCtx->oformat, 0, outf.outFCtx->filename, 0, CODEC_TYPE_VIDEO);
//
//	/* コーデックを得る */
//	outf.codec = avcodec_find_encoder(codec_id);
//	// エンコーダが決まっている場合は avcodec_find_encoder_by_name();
//	if(outf.codec == 0){
//		fprintf(stderr, "Video_y::save_frame; avcodec_find_encoder()\n");
//		return -1;
//	}
//
//	/* ストリームにコーデックの情報を設定 */
//	outf.codecCtx = outf.stream->codec;
//	outf.codecCtx->codec_id = codec_id;
//	outf.codecCtx->codec_type = CODEC_TYPE_VIDEO;
//	outf.codecCtx->width = this->width;
//	outf.codecCtx->height = this->height;
//	outf.codecCtx->time_base.num = 1;///////////
//	outf.codecCtx->time_base.den = 60;///////////
//
//	/* コーデックに対応しているピクセルフォーマットを指定 */
//	outf.codecCtx->pix_fmt = PIX_FMT_NONE;
//	if(outf.codec && outf.codec->pix_fmts){
//		const PixelFormat *p = outf.codec->pix_fmts;
//		while(*p != -1){
//			if(*p == outf.codecCtx->pix_fmt){
//				break;
//			}
//			p++;
//		}
//
//		/* 非対応の場合は対応するものに差し替え */
//		if(*p == -1){
//			outf.codecCtx->pix_fmt = outf.codec->pix_fmts[0];
//		}
//	}
//
//	/* フォーマットにより、グローバルヘッダが要求される場合はコーデックにも伝える */
//	if((outf.format->flags & AVFMT_GLOBALHEADER)){
//		outf.codecCtx->flags |= CODEC_FLAG_GLOBAL_HEADER;
//	}
//
//	/* 出力のパラメータを指定 */
//	ret = av_set_parameters(outf.outFCtx, 0);
//	if(ret < 0){
//		fprintf(stderr, "Video_y::save_frame; av_set_parameters()\n");
//		return -1;
//	}
//
//
//	/* コーデックを開く */
//	ret = avcodec_open(outf.codecCtx, outf.codec);
//	if(ret != 0){
//		fprintf(stderr, "Video_y::save_frame; avcodec_open()\n");
//		return -1;
//	}
//
//	/* 出力ファイルを開く */
//	if(!(outf.format->flags & AVFMT_NOFILE)){
//		ret = url_fopen(&outf.outFCtx->pb, outf.outFCtx->filename, URL_WRONLY);
//		if(ret < 0){
//			fprintf(stderr, "Video_y::save_frame; url_fopen()\n");
//			return -1;
//		}
//	}
//
//	/* ファイルへ、ヘッダの書き込み */
//	av_write_header(outf.outFCtx);
//
//	/* エンコードに使用するバッファを用意する */
//	buf_size = outf.codecCtx->width * outf.codecCtx->height * 4;
//	outf.buf = (uint8_t*)av_malloc(buf_size);
//
//	/* ファイルにコンテンツを書き込む */
//	while(true){
//		/* コーデックでデータを符号化 */
//		/* フレームを符号化する */
//		out_size = avcodec_encode_video(outf.codecCtx, outf.buf, buf_size, frame);
//		/* 符号化がまだの場合は、次のフレームへ */
//		if(out_size == 0){
//			continue;
//		} else if(out_size < 0){
//			fprintf(stderr, "Video_y::save_frame; avcodec_encode_video()\n");
//			return -1;
//		}
//
//
//		/* 符号化したデータをパケットに詰め込む */
//		/* 書き込むパケットを準備 */
//		av_init_packet(&packet);
//		packet.stream_index = outf.stream->index;
//		packet.data = outf.buf;
//		packet.size = out_size;
//
//		/* コーデックのtime baseによるptsを, streamのtime baseによるものに修正 */
//		packet.pts = av_rescale_q(outf.codecCtx->coded_frame->pts, outf.codecCtx->time_base, outf.stream->time_base);
//		if(outf.codecCtx->coded_frame->key_frame){
//			packet.flags |= PKT_FLAG_KEY;
//		}
//
//
//		/* パケットを書き出す */
//		/* パケットを書き込む */
//		ret = av_interleaved_write_frame(outf.outFCtx, &packet);
//		if(ret != 0){
//			fprintf(stderr, "Video_y::save_frame; av_interleaved_write_frame()\n");
//			return -1;
//		}
//		/* 1フレームだけ書き込めばいいので終了 */
//		break;
//	}
//
//	/* ファイルを閉じる */
//	av_write_trailer(outf.outFCtx);
//
//	/* 以下はデストラクタへ */
////	if(!(sf.format->flags & AVFMT_NOFILE)){
////		ret = url_fclose(sf.outFCtx->pb);
////		if(ret < 0){
////			fprintf(stderr, "Video_y::save_frame; url_fclose()\n");
////			return -1;
////		}
////	}
////
////	/* コーデックを閉じる */
////	avcodec_close(sf.stream->codec);
////
////	av_freep(&sf.buf);
////	for(i=0; i<sf.outFCtx->nb_streams; i++){
////		av_freep(&sf.outFCtx->streams[i]->codec);
////		av_freep(&sf.outFCtx->streams[i]);
////	}
////	av_freep(&sf.outFCtx);
//	return 0;
//}

/**
 * デインタレースを行う(ffmpeg.c)
 * bufpは, 各フレームの処理後に毎回av_free()する必要がある.
 */
void Video_y::ffmpeg_pre_process_video_frame(AVCodecContext *dec, AVPicture *picture, void *&bufp) {
	AVPicture *picture2;
	AVPicture picture_tmp;
	uint8_t *buf = 0;

	/* deinterlace : must be done before any resize */
	if (do_deinterlace) {
		int size;

		/* create temporary picture */
		size = avpicture_get_size(dec->pix_fmt, dec->width, dec->height);
		buf = (uint8_t*)av_malloc(size);
		if (!buf) {
			perror(0);
			fprintf(stderr, "Video_y::ffmpeg_pre_process_video_frame()\n");
			return;
		}

		picture2 = &picture_tmp;
		avpicture_fill(picture2, buf, dec->pix_fmt, dec->width, dec->height);

		if (avpicture_deinterlace(picture2, picture, dec->pix_fmt, dec->width, dec->height) < 0) {
			/* if error, do not deinterlace */
			fprintf(stderr, "Video_y::ffmpeg_pre_process_video_frame(); Deinterlacing failed\n");
			av_free(buf);
			buf = 0;
			picture2 = picture;
		}
	} else {
		picture2 = picture;
	}

	if (picture != picture2){
		*picture = *picture2;
	}
	bufp = buf;
}

/**
 * @param[in] doRescale ; サイズ変換を行うかどうか. 1=行う, 0=行わない
 * @return 成功で1, 失敗で0を返す.
 */
int Video_y::ffmpeg_get_next_decoded_frame(AVFrame *dst, int doRescale){
	AVFrame tmpF;
	AVPacket packet;
	int isFinish = 0;
	int ret;
	void *buffer_to_free = 0;

	/* allocate picture correctly */
	avcodec_get_frame_defaults(&tmpF);

	/* ファイルからパケットを読み込む */
	while((ret=av_read_frame(inf.formatCtx, &packet)) >= 0){
		if(packet.stream_index != inf.streamIndex){
			/* 動画ストリーム以外は飛ばす */
			av_free_packet(&packet);
			continue;
		}

		/* パケットの複合 */
		avcodec_decode_video2(inf.codecCtx, &tmpF, &isFinish, &packet);
		if(isFinish){ break; }

		av_free_packet(&packet);
	}

	if(ret >= 0){
		/* 画像変換 */
		if(do_deinterlace){
			ffmpeg_pre_process_video_frame(inf.codecCtx, (AVPicture*)&tmpF, buffer_to_free);
		}

		if(doRescale){
			sws_scale(inf.swsCtx, tmpF.data, tmpF.linesize, 0, inf.codecCtx->height, dst->data, dst->linesize);
		} else {
			av_picture_copy((AVPicture*)dst, (AVPicture*)&tmpF, pixf, width, height);
		}
	}

	av_free(buffer_to_free);
	av_free_packet(&packet);
	return (ret >= 0) ? 1 : 0;
}

/**
 * start - 1 フレームまでデコードし, スキップする.
 * 次のgetNextによりstartフレームがデコードされる.
 * @return 成功0, 失敗-1
 */
int Video_y::skip_to(int start){
	MyAVFrame pict(width, height, pixf);
	int i = 0;

	switch(mode){
	case 0:	/* normal */
		if(!inf.codecCtx){
			fprintf(stderr, "Video_y::skip_to; movie file unknown!!\n");
			return -1;
		}
		while(i < start){
			if(!ffmpeg_get_next_decoded_frame(&pict.frame, 0)){
				fprintf(stderr, "Video_y::skip_to(); the video don't contain %d frames\n", start);
				return -1;
			}
			decodedF++;
			i++;
		}
		break;
	default:
		break;
	}
	return 0;
}


