#include "engine.h"
int save_jpeg(AVFrame *pFrame,char *outfile, int width, int height) 
{

    AVFormatContext *pFormatCtx = avformat_alloc_context();
    pFormatCtx->oformat = av_guess_format("mjpeg", NULL, NULL);

    if (avio_open(&pFormatCtx->pb, outfile, AVIO_FLAG_READ_WRITE) < 0) {
        return -1;
    }

    AVStream *pAVStream = avformat_new_stream(pFormatCtx, 0);
    if (pAVStream == NULL) {
        return -1;
    }

    AVCodecContext *pCodecCtx = pAVStream->codec;

    pCodecCtx->codec_id = pFormatCtx->oformat->video_codec;
    pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    pCodecCtx->pix_fmt = AV_PIX_FMT_YUVJ420P;
    pCodecCtx->width = width;
    pCodecCtx->height = height;
    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = 25;

    av_dump_format(pFormatCtx, 0, outfile, 1);

    AVCodec *pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
    if (!pCodec) {
        return -1;
    }
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        return -1;
    }

    avformat_write_header(pFormatCtx, NULL);/*write head in file*/

    int y_size = pCodecCtx->width * pCodecCtx->height;

    AVPacket pkt;
    av_new_packet(&pkt, y_size * 3);

    int got_picture = 0;
	/*from decode to get frame, use frame set in pkt about encode*/
    int ret = avcodec_encode_video2(pCodecCtx, &pkt, pFrame, &got_picture);/*encode frame*/
    if (ret < 0) {
        return -1;
    }
    if (got_picture == 1) {
        ret = av_write_frame(pFormatCtx, &pkt);/*write frame in file*/
    }

    av_free_packet(&pkt);

    av_write_trailer(pFormatCtx);/*write trailer in file*/

    if (pAVStream) {
        avcodec_close(pAVStream->codec);
    }
    avio_close(pFormatCtx->pb);
    avformat_free_context(pFormatCtx);
    return 0;
}
