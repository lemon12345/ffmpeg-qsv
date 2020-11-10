#include "widget.h"
#include "ui_widget.h"
#include <QDebug>
extern "C"
{
#include"libavcodec/avcodec.h"
#include "libavdevice/avdevice.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/avutil.h"
#include "libavutil/hwcontext_qsv.h"
}
#include "sdl/SDL.h"

#define SFM_REFRESH_EVENT  (SDL_USEREVENT + 1)

#define SFM_BREAK_EVENT  (SDL_USEREVENT + 2)

int thread_exit = 0;
int thread_pause = 0;
int sdl_reflesh_thread(void* opaque)
{
	thread_exit = 0;
	thread_pause = 0;
	while (!thread_exit)
	{
		if (!thread_pause)
		{
			SDL_Event event;
			event.type = SFM_REFRESH_EVENT;
			SDL_PushEvent(&event);
		}
		SDL_Delay(40);
	}
	thread_exit = 0;
	thread_pause = 0;
	//Break
	SDL_Event event;
	event.type = SFM_BREAK_EVENT;
	SDL_PushEvent(&event);
	return 0;
}
Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
 //   imageWidget = new showImageWidget();
  //  rcodec = new rvideodecode();
  //  connect(rcodec,&rvideodecode::sigShowFrame,imageWidget,&showImageWidget::loadImage,Qt::QueuedConnection);
//	connect(this, &Widget::sigstart, rcodec, &rvideodecode::slot_start, Qt::QueuedConnection);
}

Widget::~Widget()
{
    delete ui;
}
void Widget::showVideo()
{
	AVFormatContext* fmt_ctx =NULL;
	AVCodecContext   *decode_ctx = NULL;
	AVCodec*     decodec = NULL;
	AVPacket    packet;
	AVInputFormat *inputfmt = NULL;
	AVDictionary * dicy = NULL;
	AVStream* stream = NULL;
	AVFrame   *frame = av_frame_alloc();
	AVFrame   *frameyuv = av_frame_alloc();
	std::string filepath = QApplication::applicationDirPath().toStdString() + "/www.mp4";
	int ret = avformat_open_input(&fmt_ctx, filepath.c_str(), NULL,NULL);
	if (ret < 0)
	{
		qDebug() << "open file failed";
		return;
	}
	ret = avformat_find_stream_info(fmt_ctx, NULL);
	if (ret < 0)
	{
		qDebug() << "failed to avformat_find_stream_info";
		return;
	}
	//av_dump_format(fmt_ctx, 0, filepath.c_str(), 1);
	for (int i = 0; i < fmt_ctx->nb_streams; i++)
	{
		if (fmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			stream = fmt_ctx->streams[i];
		}
	}
	decodec =  avcodec_find_decoder_by_name("h264_qsv");
	if (!decodec)
	{
		qDebug() << "can not find decoder";
		return;
	}
	decode_ctx =  avcodec_alloc_context3(decodec);
	
	if (!decode_ctx)
	{
		qDebug() << "failed to create context3";
		return;
	}

	av_opt_set(decode_ctx->priv_data, "tune", "zerolatency", 0);
	if (avcodec_copy_context(decode_ctx, stream->codec) != 0)
	{
		qDebug() << "copy codec context failed";
	}

	ret = avcodec_open2(decode_ctx, decodec, NULL);
	if (ret < 0)
	{
		qDebug() << "failed to open codec";
	}

	int numsize = avpicture_get_size(decode_ctx->pix_fmt, decode_ctx->width, decode_ctx->height);
	uint8_t* buff = (uint8_t*)av_malloc(numsize*sizeof(uint8_t));
	avpicture_fill((AVPicture*)frameyuv, buff, decode_ctx->pix_fmt, decode_ctx->width, decode_ctx->height);
	SwsContext *sctx = sws_getContext(decode_ctx->width, decode_ctx->height, AV_PIX_FMT_NV12, decode_ctx->width, decode_ctx->height, AV_PIX_FMT_YUV420P, SWS_BILINEAR, NULL, NULL, NULL);
	if (!sctx)
	{
		qDebug() << "failed to sws_getContext";
		return;
	}
	SDL_Rect sdlRect;
	SDL_Renderer *sdlRender;
	SDL_Texture* sdlTexture;
	SDL_Window* sdlWindow;
	SDL_Event event;
	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_TIMER))
	{
		qDebug() << "failed to sdl_init";
		return;
	}
	sdlWindow = SDL_CreateWindow("play video", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, decode_ctx->width, decode_ctx->height, SDL_WINDOW_OPENGL);
	if (!sdlWindow)
	{
		qDebug() << "failed to create sdl window";
	}
	sdlRender = SDL_CreateRenderer(sdlWindow, -1, 0);
	if (!sdlRender)
	{
		qDebug() << "failed to create sdl render";
	}
	sdlTexture = SDL_CreateTexture(sdlRender, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, decode_ctx->width, decode_ctx->height);
	if (!sdlTexture)
	{
		qDebug() << "failed to create sdl texture";
	}
	sdlRect.x = 0;
	sdlRect.y = 0;
	sdlRect.w = decode_ctx->width;
	sdlRect.h = decode_ctx->height;
	SDL_CreateThread(sdl_reflesh_thread, NULL, NULL);
	int gotfinished;
	for (;;)
	{
		SDL_WaitEvent(&event);
		if (event.type == SFM_REFRESH_EVENT)
		{
			while (1)
			{
				ret = av_read_frame(fmt_ctx, &packet);
				if (ret < 0)
				{
					thread_exit = 1;
				}
				break;
			}
			ret = avcodec_decode_video2(decode_ctx, frame, &gotfinished, &packet);
			if (ret < 0)
			{
				qDebug() << "failed to decode video";
				//return;
			}
			if (gotfinished)
			{
				sws_scale(sctx, (uint8_t const * const *)frame->data, frame->linesize, 0, decode_ctx->height, frameyuv->data, frameyuv->linesize);
				SDL_UpdateTexture(sdlTexture, NULL, frameyuv->data[0], frameyuv->linesize[0]);
				SDL_RenderClear(sdlRender);
				SDL_RenderCopy(sdlRender, sdlTexture, NULL, NULL);
				SDL_RenderPresent(sdlRender);
			}
			av_packet_unref(&packet);
		}
		else if (event.type == SDL_KEYDOWN)
		{
			if (event.key.keysym.sym == SDLK_SPACE)
				thread_pause = !thread_pause;
		}
		else if (event.type == SDL_QUIT){
			thread_exit = 1;
		}
		else if (event.type == SFM_BREAK_EVENT){
			break;
		}

	}
	sws_freeContext(sctx);
	SDL_Quit();
	av_frame_free(&frame);
	av_frame_free(&frameyuv);
	avcodec_close(decode_ctx);
	avformat_close_input(&fmt_ctx);
}
void Widget::start()
{
	rcodec->decodecing();
}
