#include "rvideodecode.h"
#include <QDebug>
#include <QApplication>
#include <QImage>
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

rvideodecode::rvideodecode(QObject *parent) :
    QObject(parent)
    ,pthread(new QThread)
{
   this->moveToThread(pthread);
}

void rvideodecode::decodecing()
{
    AVFormatContext *input_ctx = NULL;
    AVStream *video_st = NULL;
    AVCodecContext *decoder_ctx = NULL;
    const AVCodec *decoder;

    AVPacket pkt = { 0 };
    AVFrame *frame = NULL, *sw_frame = NULL;
    //DecodeContext decode = { NULL };
    AVIOContext *output_ctx = NULL;

    int ret, i;
    std::string path = QApplication::applicationDirPath().toStdString()+"/www.mp4";
    std::string outpath = QApplication::applicationDirPath().toStdString() + "/mm.yuv";
    FILE *yuvFile;
    yuvFile = fopen(outpath.c_str(), "wb+");
    /* open the input file */

    ret = avformat_open_input(&input_ctx, path.c_str(), NULL, NULL);

    if (ret < 0)
    {
        qDebug() << "failed to open input file ";
        return;
    }

    ret = avformat_find_stream_info(input_ctx, NULL);

    if (ret < 0)
    {
        qDebug() << "failed find stream info ";
        return;
    }
    //打印视频信息
    av_dump_format(input_ctx, 0, path.c_str(), 0);

    for (int i = 0; i < input_ctx->nb_streams; i++)
    {
        if (input_ctx->streams[i]->codec->codec_id == AV_CODEC_ID_H264)
            video_st = input_ctx->streams[i];
    }
    if (!video_st)
    {
        qDebug() << "failed to h264 video";
        return;
    }
    AVBufferRef* avbuf;
    //加载硬解码器
//    ret = av_hwdevice_ctx_create(&avbuf, AV_HWDEVICE_TYPE_QSV, "auto", NULL, 0);
  //  if (ret < 0)
  //  {
   //     qDebug() << "failed to create hwdevice";
   //     return;
    //}
    decoder = avcodec_find_decoder_by_name("h264_qsv");
    if (!decoder)
    {
        qDebug() << " can not find h264_qsv";
        return;
    }

    decoder_ctx = avcodec_alloc_context3(decoder);
    if (!decoder_ctx)
    {
        qDebug() << "failed to create decodec contex3";
        return;
    }
    //拷贝视频编码上下文参数
    av_opt_set(decoder_ctx->priv_data, "tune", "zerolatency", 0);
    if (avcodec_copy_context(decoder_ctx, video_st->codec) != 0)
    {
        qDebug() << "Could not copy codec context!";
        return;
    }
    ret = avcodec_open2(decoder_ctx, decoder, NULL);
    if (ret < 0)
    {
        qDebug() << "failed to open codec";
        return;
    }
    int numbytes = 0;
    uint8_t* buffer;
    AVFrame* pFrame = nullptr;
    AVFrame* pFrameRGB = nullptr;

    pFrame = av_frame_alloc();
    pFrameRGB = av_frame_alloc();
	numbytes = avpicture_get_size(AV_PIX_FMT_YUV420P, decoder_ctx->width, decoder_ctx->height);
    buffer = (uint8_t*)av_malloc(numbytes*sizeof(uint8_t));
	avpicture_fill((AVPicture*)pFrameRGB, buffer, AV_PIX_FMT_YUV420P, decoder_ctx->width, decoder_ctx->height);
	SwsContext* sws_ctx = sws_getContext(decoder_ctx->width, decoder_ctx->height, AV_PIX_FMT_NV12, decoder_ctx->width, decoder_ctx->height, AV_PIX_FMT_YUV420P, SWS_BILINEAR, NULL, NULL, NULL);
    AVPacket packet;
   // ret = av_read_frame(input_ctx, &packet);
	SDL_Window *screen;
	SDL_Renderer *sdlRenderer;
	SDL_Texture* sdlTexture;
	SDL_Rect sdlRect;

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);
	screen = SDL_CreateWindow("play window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, decoder_ctx->width, decoder_ctx->height, SDL_WINDOW_OPENGL);
	sdlRenderer = SDL_CreateRenderer(screen, -1, 0);
	sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, decoder_ctx->width, decoder_ctx->height);
	sdlRect.x = 0;
	sdlRect.y = 0;
	sdlRect.h = decoder_ctx->height;
	sdlRect.w = decoder_ctx->width;



    int frameFinished;
	while ((ret = av_read_frame(input_ctx, &packet))>= 0)
    {
        avcodec_decode_video2(decoder_ctx, pFrame, &frameFinished, &packet);
        if (frameFinished)
        {
            sws_scale(sws_ctx, (uint8_t const * const *)pFrame->data, pFrame->linesize, 0, decoder_ctx->height, pFrameRGB->data, pFrameRGB->linesize);
           // QImage img((uchar *)buffer, decoder_ctx->width, decoder_ctx->height, QImage::Format_RGB32);
			
          //  if (!img.isNull())
            {
                 // emit sigShowFrame(img);
            }
			SDL_UpdateTexture(sdlTexture,0, pFrameRGB->data[0], pFrame->linesize[0]);
			SDL_RenderClear(sdlRenderer);
			SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &sdlRect);
			SDL_RenderPresent(sdlRenderer);
			SDL_Delay(40);
            av_free_packet(&packet);
        }
    }


}

void rvideodecode::slot_start()
{
    decodecing();
}
