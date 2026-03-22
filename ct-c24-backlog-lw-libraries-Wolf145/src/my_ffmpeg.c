#include "../include/my_ffmpeg.h"

#include "../return_codes.h"
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#include <stdint.h>
#include <stdlib.h>

static uint8_t receive_packets(AVCodecContext* codecContext, AVFrame* frame, size_t channelId, double** outData, size_t* sz, size_t* capacity)
{
	int32_t err = SUCCESS;
	do
	{
		err = avcodec_receive_frame(codecContext, frame);
		switch (err)
		{
		case 0:
			break;
		case AVERROR_EOF:
			fprintf(stderr, "Failed to receive frame: encoder has been flushed\n");
			av_frame_free(&frame);
			return ERROR_UNKNOWN;
		case AVERROR(EAGAIN):
			fprintf(stderr, "Failed to receive frame: input is not accepted\n");
			av_frame_free(&frame);
			return ERROR_UNKNOWN;
		case AVERROR(EINVAL):
			fprintf(stderr, "Failed to receive frame: codec not opened\n");
			av_frame_free(&frame);
			return ERROR_DATA_INVALID;
		case AVERROR(ENOMEM):
			fprintf(stderr, "Failed to receive frame: unable to add packet\n");
			av_frame_free(&frame);
			return ERROR_NOTENOUGH_MEMORY;
		default:
			fprintf(stderr, "Failed to receive frame\n");
			av_frame_free(&frame);
			return ERROR_UNKNOWN;
		}
		size_t num_samples = frame->nb_samples;
		if ((*sz) + num_samples > (*capacity))
		{
			(*capacity) = ((*sz) + num_samples) * 2;
			double* tmp = NULL;
			*outData = (double*)realloc(tmp = (*outData), (*capacity) * sizeof(double));
			if (!*outData)
			{
				fprintf(stderr, "Failed to realloc\n");
				free(tmp);
				av_frame_free(&frame);
				return ERROR_NOTENOUGH_MEMORY;
			}
		}
		switch (frame->format)
		{
		case AV_SAMPLE_FMT_FLT:
		case AV_SAMPLE_FMT_FLTP:
			for (size_t i = 0; i < num_samples; ++i)
				(*outData)[(*sz) + i] = (double)((float*)frame->extended_data[channelId])[i];
			break;
		default:
			fprintf(stderr, "Invalid sample format\n");
			av_frame_free(&frame);
			return ERROR_DATA_INVALID;
		}
		(*sz) += num_samples;
	} while (err != 0);
	return SUCCESS;
}
static uint8_t
	receive_frames(AVFormatContext* formatContext, AVCodecContext* codecContext, size_t channelId, double** outData, size_t* outSz)
{
	AVPacket packet;
	packet.data = NULL;
	packet.size = 0;
	AVFrame* frame = av_frame_alloc();
	if (!frame)
	{
		fprintf(stderr, "Failed to allocate AVFrame\n");
		av_frame_free(&frame);
		return ERROR_NOTENOUGH_MEMORY;
	}

	size_t sz = 0;
	size_t capacity = 512;
	(*outData) = (double*)malloc(capacity * sizeof(double));
	if (!(*outData))
	{
		fprintf(stderr, "Failed to realloc\n");
		av_frame_free(&frame);
		return ERROR_NOTENOUGH_MEMORY;
	}
	while (av_read_frame(formatContext, &packet) >= 0)
	{
		int32_t err = avcodec_send_packet(codecContext, &packet);
		if (err < 0)
		{
			switch (err)
			{
			case AVERROR_EOF:
				fprintf(stderr, "Failed to accept packets\n");
				av_frame_free(&frame);
				return ERROR_UNKNOWN;
			case AVERROR(ENOMEM):
				fprintf(stderr, "Failed to open file\n");
				av_frame_free(&frame);
				return ERROR_NOTENOUGH_MEMORY;
			default:
				fprintf(stderr, "Failed to send packet to codec\n");
				av_frame_free(&frame);
				return ERROR_UNKNOWN;
			}
		}
		err = receive_packets(codecContext, frame, channelId, outData, &sz, &capacity);
		if (err)
		{
			av_frame_free(&frame);
			return (uint8_t)err;
		}
		av_packet_unref(&packet);
	}
	(*outSz) = sz;
	av_frame_free(&frame);
	return SUCCESS;
}
uint8_t decode_audio(char* file, size_t channelId, size_t maxChannels, double** outData, int32_t* samplRate, size_t* outSz)
{
	av_log_set_level(AV_LOG_QUIET);
	uint8_t err = SUCCESS;
	AVFormatContext* formatContext = NULL;
	AVCodecContext* codecContext = NULL;
	const AVCodec* codec;

	formatContext = avformat_alloc_context();
	if (!formatContext)
	{
		fprintf(stderr, "Failed to allocate format context\n");
		return ERROR_NOTENOUGH_MEMORY;
	}
	int32_t opens = avformat_open_input(&formatContext, file, NULL, NULL);
	if (opens < 0)
	{
		switch (opens)
		{
		case AVERROR(ENOMEM):
			err = ERROR_NOTENOUGH_MEMORY;
			fprintf(stderr, "Failed to open file(not enough memory)\n");
			goto cleaning;
		case AVERROR(ENOENT):
			err = ERROR_CANNOT_OPEN_FILE;
			fprintf(stderr, "File not found\n");
			goto cleaning;
		default:
			err = ERROR_UNKNOWN;
			fprintf(stderr, "Failed to open file\n");
			goto cleaning;
		}
	}
	int32_t streams = avformat_find_stream_info(formatContext, NULL);
	if (streams < 0)
	{
		err = ERROR_FORMAT_INVALID;
		fprintf(stderr, "Failed to get stream\n");
		goto cleaning;
	}
	int32_t streamId = av_find_best_stream(formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	if (streamId < 0)
	{
		err = ERROR_FORMAT_INVALID;
		fprintf(stderr, "No audio stream in file");
		goto cleaning;
	}
	if (formatContext->streams[streamId]->codecpar->ch_layout.nb_channels < maxChannels)
	{
		err = ERROR_FORMAT_INVALID;
		fprintf(stderr, "Number of channels is less than %lu\n", maxChannels);
		goto cleaning;
	}
	*samplRate = formatContext->streams[streamId]->codecpar->sample_rate;
	codec = avcodec_find_decoder(formatContext->streams[streamId]->codecpar->codec_id);
	if (!codec)
	{
		err = ERROR_ARGUMENTS_INVALID;
		fprintf(stderr, "Decoder not found\n");
		goto cleaning;
	}
	switch (formatContext->streams[streamId]->codecpar->codec_id)
	{
	case AV_CODEC_ID_MP2:
		break;
	case AV_CODEC_ID_MP3:
		break;
	case AV_CODEC_ID_FLAC:
		break;
	case AV_CODEC_ID_OPUS:
		break;
	case AV_CODEC_ID_AAC:
		break;
	default:
		err = ERROR_FORMAT_INVALID;
		fprintf(stderr, "Invalid file format");
		goto cleaning;
	}
	codecContext = avcodec_alloc_context3(codec);
	if (codecContext == NULL)
	{
		err = ERROR_FORMAT_INVALID;
		fprintf(stderr, "Failed to allocate audio codec context\n");
		goto cleaning;
	}
	if (avcodec_open2(codecContext, codec, NULL) < 0)
	{
		err = ERROR_FORMAT_INVALID;
		fprintf(stderr, "Failed to open codec\n");
		goto cleaning;
	}
	err = receive_frames(formatContext, codecContext, channelId, outData, outSz);
cleaning:
	avcodec_free_context(&codecContext);
	avformat_close_input(&formatContext);
	avformat_free_context(formatContext);
	return err;
}
