#include "widget.h"
#include <QApplication>
#include <QDebug>
#include <stdio.h>
#include <Windows.h>
extern "C"
{
#include"libavcodec/avcodec.h"
#include "libavdevice/avdevice.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/avutil.h"
#include "libavutil/hwcontext_qsv.h"
}
#include "rvideodecode.h"
//#include "sdl/SDL.h"
//SDL_Window *screen;
//SDL_Renderer* render;
//SDL_Texture* texture;
//typedef unsigned int u32;
//typedef unsigned short u8;
//typedef unsigned char u_char;
//
//SDL_Rect sdlRect;
uint8_t *out_buffer;
uint8_t *src_data[4];
uint8_t *dst_data[4];
int src_linesize[4], dst_linesize[4];
struct SwsContext *img_Convert_ctx;
FILE *yuvFile;

typedef struct DecodeContext
{
	AVBufferRef *hw_device_ref;
} DecodeContext;

static AVPixelFormat get_format(AVCodecContext *avctx, const enum AVPixelFormat *pix_fmts)
{
	while (*pix_fmts != AV_PIX_FMT_NONE)
	{
		if (*pix_fmts == AV_PIX_FMT_QSV)
		{
			DecodeContext *decode = (DecodeContext *)avctx->opaque;
			AVHWFramesContext  *frames_ctx;
			AVQSVFramesContext *frames_hwctx;
			int ret;

			/* create a pool of surfaces to be used by the decoder */
			avctx->hw_frames_ctx = av_hwframe_ctx_alloc(decode->hw_device_ref);
			if (!avctx->hw_frames_ctx)
				return AV_PIX_FMT_NONE;
			frames_ctx = (AVHWFramesContext*)avctx->hw_frames_ctx->data;
			frames_hwctx = (AVQSVFramesContext*)frames_ctx->hwctx;

			frames_ctx->format = AV_PIX_FMT_QSV;
			frames_ctx->sw_format = avctx->sw_pix_fmt;
			frames_ctx->width = FFALIGN(avctx->coded_width, 32);
			frames_ctx->height = FFALIGN(avctx->coded_height, 32);
			frames_ctx->initial_pool_size = 32;

			frames_hwctx->frame_type = MFX_MEMTYPE_VIDEO_MEMORY_DECODER_TARGET;

			ret = av_hwframe_ctx_init(avctx->hw_frames_ctx);

			if (ret < 0)
				return AV_PIX_FMT_NONE;

			return AV_PIX_FMT_QSV;
		}

		pix_fmts++;
	}

	fprintf(stderr, "The QSV pixel format not offered in get_format()\n");

	return AV_PIX_FMT_NONE;
}

static int decode_packet(DecodeContext *decode, AVCodecContext *decoder_ctx,
	AVFrame *frame, AVFrame *sw_frame,
	AVPacket *pkt, AVIOContext *output_ctx)
{
	int ret = 0;
	int len = 0;
	DWORD time = GetTickCount();
	ret = avcodec_send_packet(decoder_ctx, pkt);

	if (ret < 0)
	{
		fprintf(stderr, "Error during decoding\n");
		return ret;
	}

	while (ret >= 0)
	{
		int i, j, temp, k;
		unsigned char *nv12Data = NULL;
	/*	sdlRect.x = 0;
		sdlRect.y = 0;
		sdlRect.w = decoder_ctx->width;
		sdlRect.h = decoder_ctx->height;*/

		ret = avcodec_receive_frame(decoder_ctx, frame);


		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			break;

		else if (ret < 0)
		{
			fprintf(stderr, "Error during decoding\n");
			return ret;
		}

		/* A real program would do something useful with the decoded frame here.
		* We just retrieve the raw data and write it to a file, which is rather
		* useless but pedagogic. */
//		ret = av_hwframe_transfer_data(sw_frame, frame, 0);
		//printf("当前耗时:%d ms\n", GetTickCount() - time);

		if (ret < 0)
		{
			fprintf(stderr, "Error transferring the data to system memory\n");
			goto fail;
		}

		nv12Data = new unsigned char[decoder_ctx->width * decoder_ctx->height * 1.5];

		for (i = 0; i < FF_ARRAY_ELEMS(sw_frame->data) && sw_frame->data[i]; i++)//
		{
			for (j = 0; j < (sw_frame->height >> (i > 0)); j++)//
			{
				memcpy(nv12Data + i* sw_frame->width*sw_frame->height + sw_frame->width*j, sw_frame->data[i] + j * sw_frame->linesize[i], sw_frame->width);
			}
		}

		//fwrite(nv12Data, 1, decoder_ctx->width * decoder_ctx->height * 1.5, yuvFile);
		//SDL_UpdateTexture(texture, NULL, nv12Data, sw_frame->linesize[0]);
		//SDL_UpdateTexture(texture, NULL, sw_frame->data[0], sw_frame->linesize[0]);
		//SDL_RenderClear(render);
		//SDL_RenderCopy(render, texture, NULL, &sdlRect);
		//SDL_RenderPresent(render);

	fail:
		av_frame_unref(sw_frame);
		av_frame_unref(frame);
		delete nv12Data;

		if (ret < 0)
			return ret;
	}

	return 0;
}

void read_frame()
{
	AVFormatContext *input_ctx = NULL;
	AVStream *video_st = NULL;
	AVCodecContext *decoder_ctx = NULL;
	const AVCodec *decoder;

	AVPacket pkt = { 0 };
	AVFrame *frame = NULL, *sw_frame = NULL;
	DecodeContext decode = { NULL };
	AVIOContext *output_ctx = NULL;

	int ret, i;
	/*const char* path = "邓紫棋.mp4";
	const char *outPath = "输出.yuv";*/
	std::string path = QApplication::applicationDirPath().toStdString() + "/www.mp4";
	std::string outpath = QApplication::applicationDirPath().toStdString() + "/mm.yuv";
	fopen_s(&yuvFile, outpath.c_str(), "wb+");
	/* open the input file */

	ret = avformat_open_input(&input_ctx, path.c_str(), NULL, NULL);

	if (ret < 0)
	{
		//fprintf(stderr, "Cannot open input file '%s': ", argv[1]);
		goto finish;
	}

	if ((ret = avformat_find_stream_info(input_ctx, NULL)) < 0)
	{
		//return ret;
	};

	/* find the first H.264 video stream */
	for (i = 0; i < input_ctx->nb_streams; i++)
	{
		AVStream *st = input_ctx->streams[i];

		if (st->codecpar->codec_id == AV_CODEC_ID_H264 && !video_st)
			video_st = st;
		else
			st->discard = AVDISCARD_ALL;
	}

	if (!video_st)
	{
		//fprintf(stderr, "No H.264 video stream in the input file\n");
		goto finish;
	}

	/* open the hardware device */
	ret = av_hwdevice_ctx_create(&decode.hw_device_ref, AV_HWDEVICE_TYPE_QSV,
		"auto", NULL, 0);

	if (ret < 0) //加载硬解
	{
	//	fprintf(stderr, "Cannot open the hardware device\n");
		goto finish;
	}

	/* initialize the decoder */
	decoder = avcodec_find_decoder_by_name("h264_qsv");

	if (!decoder)
	{
		//fprintf(stderr, "The QSV decoder is not present in libavcodec\n");
		goto finish;
	}

	decoder_ctx = avcodec_alloc_context3(decoder);
	if (!decoder_ctx)
	{
		ret = AVERROR(ENOMEM);
		goto finish;
	}
	decoder_ctx->codec_id = AV_CODEC_ID_H264;

	if (video_st->codecpar->extradata_size)
	{
		decoder_ctx->extradata = (uint8_t*)av_mallocz(video_st->codecpar->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);

		if (!decoder_ctx->extradata)
		{
			ret = AVERROR(ENOMEM);
			goto finish;
		}

		memcpy(decoder_ctx->extradata, video_st->codecpar->extradata,
			video_st->codecpar->extradata_size);
		decoder_ctx->extradata_size = video_st->codecpar->extradata_size;
	}

	decoder_ctx->opaque = &decode;
	//decoder_ctx->get_format = get_format;

	if ((ret = avcodec_parameters_to_context(decoder_ctx, video_st->codecpar)) < 0)
	{
		//return ret;
	}

	ret = avcodec_open2(decoder_ctx, NULL, NULL);
	if (ret < 0)
	{
		fprintf(stderr, "Error opening the decoder: ");
		goto finish;
	}

	//if (SDL_Init(SDL_INIT_VIDEO))
	//{
	//	printf("初始化SDL失败\n");
	//}

	//screen = SDL_CreateWindow("test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
	//	decoder_ctx->width, decoder_ctx->height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	//if (screen <0)
	//{
	//	printf("创建屏幕失败\n");
	//}

	//render = SDL_CreateRenderer(screen, -1, SDL_RENDERER_ACCELERATED);

	//if (!render)
	//{
	//	printf("创建渲染器失败\n");
	//}

	//texture = SDL_CreateTexture(render, SDL_PIXELFORMAT_NV12, SDL_TEXTUREACCESS_STREAMING, decoder_ctx->width, decoder_ctx->height);

	//if (!texture)
	//{
	//	printf("创建纹理失败\n");
	//}

	/* open the output stream */
	ret = avio_open(&output_ctx, outpath.c_str(), AVIO_FLAG_WRITE);//打开文件路径

	if (ret < 0)
	{
		fprintf(stderr, "Error opening the output context: ");
		//goto finish;
	}

	frame = av_frame_alloc();
	sw_frame = av_frame_alloc();

	if (!frame || !sw_frame)
	{
		ret = AVERROR(ENOMEM);
		goto finish;
	}

	//img_Convert_ctx = sws_getContext(decoder_ctx->width, decoder_ctx->height, decoder_ctx->pix_fmt, decoder_ctx->width, decoder_ctx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

	/* actual decoding */
	ret = 1;
	while (ret >= 0)
	{
		av_read_frame(input_ctx, &pkt);

		if (ret < 0)
		{
			printf("读取一帧错误\n");
		//	break;
		}

		if (pkt.stream_index == video_st->index)
		{
			ret = decode_packet(&decode, decoder_ctx, frame, sw_frame, &pkt, output_ctx);
		}

		av_packet_unref(&pkt);
	}

	/* flush the decoder */
	pkt.data = NULL;
	pkt.size = 0;
	ret = decode_packet(&decode, decoder_ctx, frame, sw_frame, &pkt, output_ctx);
	printf("解码完成！\n");

finish:
	if (ret < 0)
	{
		char buf[1024];
		av_strerror(ret, buf, sizeof(buf));
		fprintf(stderr, "%s\n", buf);
	}

	avformat_close_input(&input_ctx);

	av_frame_free(&frame);
	av_frame_free(&sw_frame);

	avcodec_free_context(&decoder_ctx);

	av_buffer_unref(&decode.hw_device_ref);

	avio_close(output_ctx);
}
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
	//w.start();
	//read_frame();
	//decodecing();
	w.showVideo();
    w.show();

    return a.exec();
}
