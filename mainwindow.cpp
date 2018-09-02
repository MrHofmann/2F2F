#include "mainwindow.h"
#include "ui_mainwindow.h"

//#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <QFileDialog>
#include <QInputDialog>
#include <iostream>

#include "mainthread.h"
#include "decodethread.h"
#include "utils.h"
#include "renderarea.h"


MainWindow::MainWindow() :
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->ButtonOpen,SIGNAL(clicked()),
            this, SLOT(button_open_clicked()));
    connect(ui->ButtonPlay,SIGNAL(clicked()),
            this, SLOT(button_play_clicked()));
    connect(ui->ButtonStop,SIGNAL(clicked()),
            this, SLOT(button_stop_clicked()));
    connect(ui->ButtonExit,SIGNAL(clicked()),
            this, SLOT(button_exit_clicked()));
    connect(ui->RadioEnable, SIGNAL(clicked()),
            this, SLOT(radio_enable_clicked()));
    connect(ui->RadioDisable, SIGNAL(clicked()),
            this, SLOT(radio_disable_clicked()));
    connect(ui->RadioEqualizer, SIGNAL(clicked()),
            this, SLOT(radio_equalizer_clicked()));
    connect(ui->RadioLow, SIGNAL(clicked()),
            this, SLOT(radio_low_clicked()));
    connect(ui->RadioHigh, SIGNAL(clicked()),
            this, SLOT(radio_high_clicked()));
    connect(ui->RadioBandPass, SIGNAL(clicked()),
            this, SLOT(radio_band_pass_clicked()));
    connect(ui->RadioBandStop, SIGNAL(clicked()),
            this, SLOT(radio_band_stop_clicked()));

    connect(ui->KnobMaster, SIGNAL(valueChanged(int)),
            this, SLOT(knob_master_changed()));
    connect(ui->KnobBalance, SIGNAL(valueChanged(int)),
            this, SLOT(knob_balance_changed()));
    connect(ui->KnobOrder, SIGNAL(valueChanged(int)),
            this, SLOT(knob_order_changed()));
    connect(ui->KnobWidth, SIGNAL(valueChanged(int)),
            this, SLOT(knob_width_changed()));

    connect(ui->SliderGain0, SIGNAL(valueChanged(int)),
            this, SLOT(slider_gain0_changed()));
    connect(ui->SliderGain1, SIGNAL(valueChanged(int)),
            this, SLOT(slider_gain1_changed()));
    connect(ui->SliderGain2, SIGNAL(valueChanged(int)),
            this, SLOT(slider_gain2_changed()));
    connect(ui->SliderGain3, SIGNAL(valueChanged(int)),
            this, SLOT(slider_gain3_changed()));
    connect(ui->SliderGain4, SIGNAL(valueChanged(int)),
            this, SLOT(slider_gain4_changed()));
    connect(ui->SliderGain5, SIGNAL(valueChanged(int)),
            this, SLOT(slider_gain5_changed()));
    connect(ui->SliderGain6, SIGNAL(valueChanged(int)),
            this, SLOT(slider_gain6_changed()));
    connect(ui->SliderGain7, SIGNAL(valueChanged(int)),
            this, SLOT(slider_gain7_changed()));
    connect(ui->SliderGain8, SIGNAL(valueChanged(int)),
            this, SLOT(slider_gain8_changed()));
    connect(ui->SliderGain9, SIGNAL(valueChanged(int)),
            this, SLOT(slider_gain9_changed()));


    QPalette palette = ui->centralWidget->palette();
//    palette.setColor(QPalette::Background, QColor(149, 0, 171));
//    palette.setColor(QPalette::Background, QColor(149, 97, 171));
//    palette.setColor(QPalette::Background, QColor(62, 0, 118));
//    palette.setColor(QPalette::Background, QColor(89, 0, 170));
//    palette.setColor(QPalette::Background, QColor(129, 0, 244));
    palette.setColor(QPalette::Background, QColor(159, 53, 255));

    ui->page_2->setAutoFillBackground(true);
    ui->page_2->setPalette(palette);

//    palette.setColor(QPalette::Background, QColor(103, 100, 168));
//    ui->centralWidget->setAutoFillBackground(true);
//    ui->centralWidget->setPalette(palette);

    QColor color1 = qvariant_cast<QColor>("mediumslateblue");
    QColor color2 = qvariant_cast<QColor>("cornsilk");

    QBoxLayout* renderBoxLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    _canvas_eq = new RenderArea(ui->page_2, &_filter_state);
    _canvas_eq->setObjectName("eq");
    _canvas_eq->setFillGradient(color1, color2);
    renderBoxLayout->setContentsMargins(0, 0, 0, 0);
    renderBoxLayout->addWidget(_canvas_eq);
    ui->page_2->setLayout(renderBoxLayout);

    _playback_enable = false;
    _playback_stop = false;
    _track_loaded = false;

    _track_state.audio_sample_size = 0;
    _track_state.audio_buf_index = 0;
    _track_state.audio_buf_size = 0;
    _track_state.audio_hw_buf_size = 0;
    _track_state.audio_st = nullptr;
    _track_state.av_index = -1;
    _track_state.codec_context = nullptr;
    _track_state.cond = nullptr;
    _track_state.format_context = nullptr;
    _track_state.mutex = nullptr;
    _track_state.sdl_quit = 0;

    _filter_state.filter_enabled = false;
    _filter_state.filter_method = Filter::NONE;
    _filter_state.f_gain = std::vector<int>(10, 100);
    _filter_state.filters.push_back(new Equalizer(1024, 44100, &_filter_state.f_gain));
    _filter_state.filter_type = Filter::EQUALIZER;
    _filter_state.order = 50;
    _filter_state.cutoff = 1000.0;
    _filter_state.width = 50.0;
    _filter_state.dc_gain = 1.5;
    _filter_state.master_volume = 100;
    _filter_state.balance = 100;
    _filter_state.cutoff1 = 1500.0;
    _filter_state.cutoff2 = 500.0;

    _user_data.track_state = &_track_state;
    _user_data.filter_state = &_filter_state;

    MainThread *main_thread = new MainThread(&_track_state);
    connect(main_thread, SIGNAL(finished()), this, SLOT(main_finished()));
    main_thread->start();
}

MainWindow::~MainWindow()
{
    delete ui;
}


int MainWindow::stream_component_open(int stream_index)
{
    AVFormatContext *format_ctx = _track_state.format_context;
    AVCodecContext *codec_ctx = nullptr;
//    AVCodecParameters *codec_param = nullptr;
    AVCodec *codec = nullptr;
    SDL_AudioSpec wanted_spec, spec;

    if(stream_index < 0 || stream_index >= format_ctx->nb_streams)
        return -1;

    //FIND DECODER - nalazi odgovarajuci decoder
    codec = avcodec_find_decoder(format_ctx->streams[stream_index]->codecpar->codec_id);
    if(!codec)
    {
        std::cerr << "Unsupported codec" << std::endl;
        return -1;
    }

    codec_ctx = avcodec_alloc_context3(codec);
    if(!codec_ctx)
    {
        std::cerr << "Could not allocate decoding context" << std::endl;
        return -1;
    }

//    codec_param = format_ctx->streams[stream_index]->codecpar;
    if(avcodec_parameters_to_context(codec_ctx, format_ctx->streams[stream_index]->codecpar) < 0)
    {
        std::cerr << "Could not copy codec context" << std::endl;
        return -1;
    }

//    if(codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO)
//        SDL_SetWindowSize(screen, codec_ctx->width, codec_ctx->height);

    if(codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO)
    {
        wanted_spec.freq = codec_ctx->sample_rate;
        wanted_spec.format = AUDIO_F32SYS;
        wanted_spec.channels = codec_ctx->channels;
        wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE;
        wanted_spec.callback = audio_callback;
        wanted_spec.userdata = &_user_data;
        wanted_spec.silence = 0;

        if(SDL_OpenAudio(&wanted_spec, &spec) < 0)
        {
            std::cerr << "SDL_OpenAudio: " << SDL_GetError() << std::endl;
            return -1;
        }

        _track_state.audio_hw_buf_size = spec.size;
    }

    //INIT DEKODER - prikupljanje informacija o kodeku
    if(avcodec_open2(codec_ctx, codec, nullptr) < 0)
    {
        std::cerr << "Unsupported codec" << std::endl;
        return -1;
    }

//    av_opt_set_int(codec_ctx, "refcounted_frames", 1, 0);

    switch(codec_ctx->codec_type)
    {
        case AVMEDIA_TYPE_AUDIO:
            _track_state.av_index = stream_index;
            _track_state.audio_st = format_ctx->streams[stream_index];
            _track_state.codec_context = codec_ctx;            
            _track_state.audio_sample_size = av_get_bytes_per_sample(codec_ctx->sample_fmt);
            _track_state.audio_buf_size = 0;
            _track_state.audio_buf_index = 0;
            memset(&_track_state.audio_pkt, 0, sizeof(_track_state.audio_pkt));
            packet_queue_init(&(_track_state.audio_queue));

//            SDL_PauseAudio(0);
            break;

        case AVMEDIA_TYPE_VIDEO:/*
            videoState->videoStream = stream_index;
            videoState->video_st = pFormatCtx->streams[stream_index];
            videoState->video_ctx = codecCtx;

            videoState->frame_timer = (double)av_gettime() / 1000000.0;
            videoState->frame_last_delay = 40e-3;

            packet_queue_init(&videoState->videoq);

            videoState->video_tid = SDL_CreateThread(video_thread, "video_thread", videoState);
            videoState->sws_ctx = sws_getContext(videoState->video_ctx->width, videoState->video_ctx->height,
                videoState->video_ctx->pix_fmt, videoState->video_ctx->width,
                videoState->video_ctx->height, AV_PIX_FMT_YUV420P,
                SWS_BILINEAR, nullptr, nullptr, nullptr);*/
            break;

        default:
            break;
    }

    return 0;
}

int MainWindow::load_track(std::string path)
{
    AVIOInterruptCB int_cb; // = { decode_interrupt_cb, nullptr };
    AVFormatContext *format_ctx = nullptr;

    int audio_index = -1;
    int video_index = -1;
    _track_state.av_index = -1;

//OPEN FILE
    if(avformat_open_input(&format_ctx, path.c_str(), nullptr, nullptr) < 0)
    {
        std::cerr << "Error open file" << std::endl;
        return -1;
    }
    _track_state.format_context = format_ctx;
    _track_state.format_context->interrupt_callback = int_cb; //NE ZNAM DA LI JE OVO BITNO

//FIND STREAM - prikuplja podatke o ulaznom fajlu (broj strimova...)
    if(avformat_find_stream_info(format_ctx, nullptr) < 0)
    {
        std::cerr << "Error file stream info" << std::endl;
        return -1;
    }

//ISPISIVANJE INFORMACIJA O FORMATU
    av_dump_format(format_ctx, 0, path.c_str(), false);


//NALAZI STREAM
//    audio_index = av_find_best_stream(_track_state->format_context, AVMEDIA_TYPE_AUDIO, -1, -1, &audio_codec, 0);
//    video_index = av_find_best_stream(_track_state->format_context, AVMEDIA_TYPE_VIDEO, -1, -1, &video_codec, 0);

    for(int i=0; i<format_ctx->nb_streams; i++)
    {
        if(format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && video_index < 0)
            video_index = i;

        if(format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && audio_index < 0)
            audio_index = i;
    }

    stream_component_open(audio_index);
//    stream_component_open(video_index);


    if(_track_state.av_index < 0)
    {
        std::cerr << "Could not find codec" << std::endl;

        SDL_Event event;
        event.type = SDL_QUIT;
        event.user.data1 = &_track_state;
        SDL_PushEvent(&event);

        return -1;
    }

    std::cout << std::endl << "FILE OPEN SUCCESS" << std::endl << std::endl;
    return 0;
}


void MainWindow::button_open_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, "Select File", "../FFT-Equalizer/test/", "*.*");
    if (!filename.isEmpty())
        std::cout << filename.toStdString() << std::endl;
    else
        return;

    load_track(filename.toStdString());

    AVFormatContext *format_ctx = _track_state.format_context;
    AVDictionaryEntry *tag = nullptr;
    std::string artist, title;
    while ((tag = av_dict_get(format_ctx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
        if(strcmp(tag->key, "artist") == 0)
            artist = tag->value;
        else if(strcmp(tag->key, "title")==0)
            title = tag->value;

    ui->LineNowPlaying->setText((artist + " - " + title).c_str());

    _track_loaded = true;
    ui->ButtonOpen->setEnabled(false);    

    DecodeThread *decode_thread = new DecodeThread(&_track_state);
    connect(decode_thread, SIGNAL(decoded()), this, SLOT(decode_finished()));
    decode_thread->start();
}

void MainWindow::button_play_clicked()
{
    if(_track_loaded)
    {
        if(_playback_enable)
        {
            ui->ButtonPlay->setText("Play");
            _playback_enable = false;
            SDL_PauseAudio(1);
        }
        else
        {
            ui->ButtonPlay->setText("Pause");
            _playback_enable = true;
            SDL_PauseAudio(0);
        }
    }
}

void MainWindow::button_stop_clicked()
{

    _playback_stop = true;
    _playback_enable = false;
    _track_loaded = false;

//    avcodec_close(_track_state.codec_context);
//    avformat_close_input(&_track_state.format_context);
//    SDL_Quit();
//    SDL_CloseAudio();

    _track_state.sdl_quit = 1;
    ui->ButtonPlay->setText("Play");
    ui->ButtonOpen->setEnabled(true);
    ui->LineNowPlaying->setText("");

}

void MainWindow::button_exit_clicked()
{
    exit(1);
}


void MainWindow::radio_enable_clicked()
{
    _filter_state.filter_enabled = true;
}

void MainWindow::radio_disable_clicked()
{
    _filter_state.filter_enabled = false;
}


void MainWindow::radio_equalizer_clicked()
{
    _filter_state.filter_type = Filter::EQUALIZER;
    _filter_state.filter_method = Filter::NONE;
    ui->StackedWidget->setCurrentIndex(0);

    for(unsigned i=0; i<_filter_state.filters.size(); i++)
        delete _filter_state.filters[i];
    _filter_state.filters.clear();

    Filter *equalizer = new Equalizer(1024, 44100, &_filter_state.f_gain);
//    equalizer->update_kernel();
    _filter_state.filters.push_back(equalizer);

    ui->SliderGain0->setEnabled(true);
    ui->SliderGain1->setEnabled(true);
    ui->SliderGain2->setEnabled(true);
    ui->SliderGain3->setEnabled(true);
    ui->SliderGain4->setEnabled(true);
    ui->SliderGain5->setEnabled(true);
    ui->SliderGain6->setEnabled(true);
    ui->SliderGain7->setEnabled(true);
    ui->SliderGain8->setEnabled(true);
    ui->SliderGain9->setEnabled(true);
}

void MainWindow::radio_low_clicked()
{
    _filter_state.filter_type = Filter::LOW_PASS;
    _filter_state.filter_method = Filter::OA_FFT;
    ui->StackedWidget->setCurrentIndex(1);

    for(unsigned i=0; i<_filter_state.filters.size(); i++)
        delete _filter_state.filters[i];
    _filter_state.filters.clear();

    ui->SliderGain0->setEnabled(false);
    ui->SliderGain1->setEnabled(false);
    ui->SliderGain2->setEnabled(false);
    ui->SliderGain3->setEnabled(false);
    ui->SliderGain4->setEnabled(false);
    ui->SliderGain5->setEnabled(false);
    ui->SliderGain6->setEnabled(false);
    ui->SliderGain7->setEnabled(false);
    ui->SliderGain8->setEnabled(false);
    ui->SliderGain9->setEnabled(false);
}

void MainWindow::radio_high_clicked()
{
    _filter_state.filter_type = Filter::HIGH_PASS;
    _filter_state.filter_method = Filter::OA_FFT;
    ui->StackedWidget->setCurrentIndex(1);

    for(unsigned i=0; i<_filter_state.filters.size(); i++)
        delete _filter_state.filters[i];
    _filter_state.filters.clear();

    ui->SliderGain0->setEnabled(false);
    ui->SliderGain1->setEnabled(false);
    ui->SliderGain2->setEnabled(false);
    ui->SliderGain3->setEnabled(false);
    ui->SliderGain4->setEnabled(false);
    ui->SliderGain5->setEnabled(false);
    ui->SliderGain6->setEnabled(false);
    ui->SliderGain7->setEnabled(false);
    ui->SliderGain8->setEnabled(false);
    ui->SliderGain9->setEnabled(false);
}

void MainWindow::radio_band_pass_clicked()
{
    _filter_state.filter_type = Filter::BAND_PASS;
    _filter_state.filter_method = Filter::OA_FFT;
    ui->StackedWidget->setCurrentIndex(1);

    for(unsigned i=0; i<_filter_state.filters.size(); i++)
        delete _filter_state.filters[i];
    _filter_state.filters.clear();

    ui->SliderGain0->setEnabled(false);
    ui->SliderGain1->setEnabled(false);
    ui->SliderGain2->setEnabled(false);
    ui->SliderGain3->setEnabled(false);
    ui->SliderGain4->setEnabled(false);
    ui->SliderGain5->setEnabled(false);
    ui->SliderGain6->setEnabled(false);
    ui->SliderGain7->setEnabled(false);
    ui->SliderGain8->setEnabled(false);
    ui->SliderGain9->setEnabled(false);
}

void MainWindow::radio_band_stop_clicked()
{
    _filter_state.filter_type = Filter::BAND_STOP;
    _filter_state.filter_method = Filter::OA_FFT;
    ui->StackedWidget->setCurrentIndex(1);

    for(unsigned i=0; i<_filter_state.filters.size(); i++)
        delete _filter_state.filters[i];
    _filter_state.filters.clear();

    ui->SliderGain0->setEnabled(false);
    ui->SliderGain1->setEnabled(false);
    ui->SliderGain2->setEnabled(false);
    ui->SliderGain3->setEnabled(false);
    ui->SliderGain4->setEnabled(false);
    ui->SliderGain5->setEnabled(false);
    ui->SliderGain6->setEnabled(false);
    ui->SliderGain7->setEnabled(false);
    ui->SliderGain8->setEnabled(false);
    ui->SliderGain9->setEnabled(false);
}


void MainWindow::knob_master_changed()
{
    _filter_state.master_volume = ui->KnobMaster->value();
}

void MainWindow::knob_balance_changed()
{
    _filter_state.balance = ui->KnobBalance->value();
}

void MainWindow::knob_order_changed()
{
    _filter_state.order = ui->KnobOrder->value();
    if(_filter_state.filters.size() > 0)
    {
        _filter_state.filters[0]->update_kernel();
        _canvas_eq->update();
    }
}

void MainWindow::knob_width_changed()
{
    _filter_state.width = ui->KnobWidth->value();
    if(_filter_state.filters.size() > 0)
    {
        _filter_state.filters[0]->update_kernel();
        _canvas_eq->update();
    }
}


void MainWindow::slider_gain0_changed()
{
    _filter_state.f_gain[0] = ui->SliderGain0->value();
}

void MainWindow::slider_gain1_changed()
{
    _filter_state.f_gain[1] = ui->SliderGain1->value();
}

void MainWindow::slider_gain2_changed()
{
    _filter_state.f_gain[2] = ui->SliderGain2->value();
}

void MainWindow::slider_gain3_changed()
{
    _filter_state.f_gain[3] = ui->SliderGain3->value();
}

void MainWindow::slider_gain4_changed()
{
    _filter_state.f_gain[4] = ui->SliderGain4->value();
}

void MainWindow::slider_gain5_changed()
{
    _filter_state.f_gain[5] = ui->SliderGain5->value();
}

void MainWindow::slider_gain6_changed()
{
    _filter_state.f_gain[6] = ui->SliderGain6->value();
}

void MainWindow::slider_gain7_changed()
{
    _filter_state.f_gain[7] = ui->SliderGain7->value();
}

void MainWindow::slider_gain8_changed()
{
    _filter_state.f_gain[8] = ui->SliderGain8->value();
}

void MainWindow::slider_gain9_changed()
{
    _filter_state.f_gain[9] = ui->SliderGain9->value();
}



void MainWindow::main_finished()
{
    _track_loaded = false;
    _track_state.sdl_quit = 1;
    ui->ButtonPlay->setText("Play");
    ui->ButtonOpen->setEnabled(true);
    ui->LineNowPlaying->setText("");

    std::cout << "Playback Finished Successfuly" << std::endl;
//    avcodec_free_context(&codec_context);
    avcodec_close(_track_state.codec_context);
    avformat_close_input(&_track_state.format_context);
//    avcodec_free_context(&_track_state.codec_context);
    SDL_CloseAudio();
//    SDL_Quit();
}

void MainWindow::decode_finished()
{
    std::cout << "DECODE FINISHED" << std::endl;
}

