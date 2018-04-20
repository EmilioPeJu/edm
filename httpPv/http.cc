#define __STDC_CONSTANT_MACROS
#include <sys/time.h>
#include <unistd.h>   
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <epicsThread.h>
#include "pv_factory.h"

extern "C" 
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/avutil.h"
}

static ProcessVariable::Type HTTP_string_type =
{ ProcessVariable::Type::text, 0, "text:0" };
   
static ProcessVariable::specificType HTTP_string_specific_type =
{ ProcessVariable::specificType::text, 0 };

static void run_me(void * usr);

class HTTP_ProcessVariable : public ProcessVariable
{
    int width;
    int height;
    int stopping;
    int _fcol;
    char * buffer;
    time_t time;
    char * url;
    AVFrame             *pFrame;
    AVFrame             *pFrameRGB;
    uint8_t             *destFrame;
    unsigned char * RainbowColorRGB;
    unsigned char * IronColorRGB;
public:
    bool is_valid() const
        {
            return true;
        }
    /* what? */
    const Type &get_type() const { return HTTP_string_type; }
    const specificType &get_specific_type() const { return HTTP_string_specific_type; }
    double get_double() const { return 0; }
    size_t get_dimension() const { return width * height; }
    const char *get_char_array() const 
        { 
            return (char *)destFrame;
        };
    const int *get_int_array() const { return NULL; }
    const double *get_double_array() const { return NULL; }
    time_t get_time_t() const { return time; };
    unsigned long get_nano() const { return 0; };
    short get_status() const { return 0; }
    short get_severity() const { return 0; }
    short get_precision() const { return 0; }
    double get_upper_disp_limit() const { return 0; }
    double get_lower_disp_limit() const { return 0; }
    double get_upper_alarm_limit() const { return 0; }
    double get_upper_warning_limit() const { return 0; }
    double get_lower_warning_limit() const { return 0; }
    double get_lower_alarm_limit() const { return 0; }
    double get_upper_ctrl_limit() const { return 0; }
    double get_lower_ctrl_limit() const { return 0; }
    bool put(double value) { return true; }
    bool put(const char *value) { return true; }
    bool put(int value) { return true; }
    bool putText(char *value) { return true; }
    bool putArrayText(char *value) { return true; }

    HTTP_ProcessVariable(const char * name) : 
        ProcessVariable(name)
        {
            printf("HTTP PV created: %s\n", name);
            width = 640;
            height = 480;
            stopping = 0;
            _fcol = 0;
            buffer = new char[1024 * 1024];
            destFrame = new uint8_t[1024 * 1024 * 3];
            for(int n = 0; n < width * height; n++)
            {
                buffer[n] = n;
            }
            
            pFrame=avcodec_alloc_frame();
            pFrameRGB=avcodec_alloc_frame();  

            AVFormatContext     *pFormatCtx;
            int                 videoStream;
            AVCodecContext      *pCodecCtx;
            AVCodec             *pCodec;
            AVPacket            packet;    
            int                 frameFinished, len, width = 0, height = 0;
            PixelFormat         pix_fmt = PIX_FMT_NONE;
            struct SwsContext   *ctx = NULL;
            bool                firstImage = true; 
            
            url = "http://i11-webcam1.diamond.ac.uk/mjpg/video.mjpg";
            
            epicsThreadCreate("low", 10, epicsThreadGetStackSize(epicsThreadStackBig), 
                              run_me, (void *)this);

            do_conn_state_callbacks();
            do_value_callbacks();

        }

    void run()
        {
            AVFormatContext     *pFormatCtx;
            int                 videoStream;
            AVCodecContext      *pCodecCtx;
            AVCodec             *pCodec;
            AVPacket            packet;    
            int                 frameFinished, len, width = 0, height = 0;
            PixelFormat         pix_fmt = PIX_FMT_NONE;
            struct SwsContext   *ctx = NULL;
            bool                firstImage = true; 

//    printf("ffThread started on URL %s\n",url);
            
            // Open video file
    if(av_open_input_file(&pFormatCtx, url, NULL, 0, NULL)!=0)
        return; // Couldn't open file
        
    // Find the first video stream
    videoStream=-1;
    for(unsigned int i=0; i<pFormatCtx->nb_streams; i++) {
        if(pFormatCtx->streams[i]->codec->codec_type==CODEC_TYPE_VIDEO) {
            videoStream=i;
            break;
        }
    }
    if(videoStream==-1)
        return; // Didn't find a video stream
        
    // Get a pointer to the codec context for the video stream
    pCodecCtx=pFormatCtx->streams[videoStream]->codec;

    // Find the decoder for the video stream
    pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
    if(pCodec==NULL) {
        fprintf(stderr, "Unsupported codec!\n");
        return; // Codec not found
    }

    // Open codec
    //ffmutex->lock();
    if(avcodec_open(pCodecCtx, pCodec)<0) {
        return; // Could not open codec
    }
    //ffmutex->unlock();
        
    while ((stopping!=1) && (av_read_frame(pFormatCtx, &packet)>=0)) {        
        // Is this a packet from the video stream?
        if(packet.stream_index==videoStream) {
        
            // Decode video frame
            len = avcodec_decode_video2(pCodecCtx, pFrame, 
                                        &frameFinished, &packet);      
            if(frameFinished) {              
                if((ctx == NULL) || (pix_fmt!=pCodecCtx->pix_fmt) || 
                   (width!=pCodecCtx->width) || (height!=pCodecCtx->height))  {    
                    // store pix_fmt, width and height
                    pix_fmt = pCodecCtx -> pix_fmt;
                    width = pCodecCtx -> width;
                    height = pCodecCtx -> height;          
                    // Create a context for software conversion to RGB / GRAY8                
                    ctx = sws_getCachedContext(ctx, width, height, pix_fmt,
                                               width, height, PIX_FMT_GRAY8, 
                                               SWS_BICUBIC, NULL, NULL, NULL);
                    // Assign appropriate parts of buffer to planes in pFrameRGB
                    avpicture_fill((AVPicture *) pFrameRGB, this->destFrame, PIX_FMT_GRAY8,
                                   width, height);                
                }
                if (this->_fcol) {
                    // Assume we have a YUV input,
                    // Throw away U and V, and use Y to generate RGB using
                    // colourmap
                    const unsigned char * colorMap;
                    switch(this->_fcol) {
                        case 1:
                            colorMap = RainbowColorRGB;
                            break;
                        case 2:
                            colorMap = IronColorRGB;
                            break;
                        default:
                            colorMap = RainbowColorRGB;
                            break;
                    }                            
                    for (int h=0; h<height; h++) {
                        for (int w=0; w<width; w++) {
                            memcpy(this->destFrame + 3*(h*width + w), colorMap + 3 * *(((unsigned char *)pFrame->data[0])+(pFrame->linesize[0]*h) + w), 3);
                        }
                    }
                } else {
                    // Do the software conversion to RGB / GRAY8
                    sws_scale(ctx, pFrame->data, pFrame->linesize, 0, height, 
                              pFrameRGB->data, pFrameRGB->linesize);
                }                              
                // Tell the GL widget that the picture is ready
                //emit updateSignal(width, height, firstImage);
                do_value_callbacks();
                printf("frame %d %d!\n", width, height);
                if (firstImage) firstImage = false;
            } else {
                printf("Frame not finished. Shouldn't see this...\n");
            }
        } else {
            printf("Non video packet. Shouldn't see this...\n");
        }
        // Free the packet
        av_free_packet(&packet);           
    }
    // tidy up    
    //ffmutex->lock();    
    
    avcodec_close(pCodecCtx);

    av_close_input_file(pFormatCtx);    
//    av_free(pCodecCtx);
    
//    av_free(pFormatCtx);
    pCodecCtx = NULL;

    pFormatCtx = NULL;
    
    //ffmutex->unlock();        
    
}

};

static void run_me(void * usr)
{
    HTTP_ProcessVariable * self = (HTTP_ProcessVariable *)usr;
    self->run();
}

static int ffinit = 0;

extern "C"
{
    ProcessVariable *create_HTTPPtr (const char * name)
    {
        if (ffinit==0) {
            ffinit = 1;
            // only display debug
            av_log_set_level(AV_LOG_DEBUG);
            // Register all formats and codecs
            av_register_all();
            
        }
        return new HTTP_ProcessVariable(name);
    }
}
