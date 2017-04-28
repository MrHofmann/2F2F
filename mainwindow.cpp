#include <QFileDialog>
#include <QInputDialog>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "playback_thread.h"
#include "init.h"

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
    connect(ui->RadioEquilizer, SIGNAL(clicked()),
            this, SLOT(radio_equilizer_clicked()));
    connect(ui->RadioLow, SIGNAL(clicked()),
            this, SLOT(radio_low_clicked()));
    connect(ui->RadioHigh, SIGNAL(clicked()),
            this, SLOT(radio_high_clicked()));
    connect(ui->RadioCustom, SIGNAL(clicked()),
            this, SLOT(radio_custom_clicked()));
    connect(ui->CheckEnableFilter, SIGNAL(clicked()),
            this, SLOT(enable_filter()));
    connect(ui->KnobMaster, SIGNAL(valueChanged(int)),
            this, SLOT(knob_master_changed()));
    connect(ui->KnobBalance, SIGNAL(valueChanged(int)),
            this, SLOT(knob_balance_changed()));
    connect(ui->SliderGain, SIGNAL(valueChanged(int)),
            this, SLOT(slider_gain_changed()));

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
 /*   connect(ui->LineFMax, SIGNAL(clicked()),
            this, SLOT(line_fmax_clicked()));
    connect(ui->LineFMin, SIGNAL(clicked()),
            this, SLOT(line_fmin_clicked()));
*/
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::button_open_clicked()
{

    PlaybackThread *thread=new PlaybackThread();
    AVDictionaryEntry *tag = NULL;
    string artist, title;

    QString filename = QFileDialog::getOpenFileName(
            this, "Select File");
    if (!filename.isEmpty())
    {
        std::cout << filename.toStdString() << std::endl;
    }
    else
        return;

    open_file(filename.toStdString().c_str());

    while ((tag = av_dict_get(format_context->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
        if(strcmp(tag->key,"artist")==0)
            artist=tag->value;
        else if(strcmp(tag->key,"title")==0)
            title=tag->value;

    ui->LineNowPlaying->setText((artist + " - " + title).c_str());

    track_loaded=true;
    ui->ButtonOpen->setEnabled(false);


//INIT SDL PLAYBACK - prikupljanje informacija za sdl_openAudio
    wanted_spec.freq = codec_context->sample_rate;
    wanted_spec.format = AUDIO_S16SYS;
    wanted_spec.channels = codec_context->channels;
    wanted_spec.silence = 0;
    wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE;
    wanted_spec.callback = audio_callback;
    wanted_spec.userdata = codec_context;

//    cout << wanted_spec.freq << endl;
//    cout << wanted_spec.format << endl;
//    cout << wanted_spec.channels << endl;
//    cout << wanted_spec.silence << endl;
//    cout << wanted_spec.samples << endl;
//    cout << spec.callback << endl;
//    cout << spec.userdata << endl;
//    cout << "----------------------------" << endl;

    if(SDL_OpenAudio(&wanted_spec, &spec) < 0)
    {
        cerr << "SDL_OpenAudio: " << SDL_GetError() << endl;
        exit(-2);
    }

//    cout << spec.freq << endl;
//    cout << spec.format << endl;
//    cout << spec.channels << endl;
//    cout << spec.silence << endl;
//    cout << spec.samples << endl;
//    cout << spec.callback << endl;
//    cout << spec.userdata << endl;

//    packet_queue_init();


    connect(thread, SIGNAL(played()),
            this, SLOT(playback_finished()));

    thread->start();
}

void MainWindow::playback_finished()
{
    //avcodec_free_context(&codec_context);
    avcodec_close(codec_context);
    avformat_close_input(&format_context);
    SDL_CloseAudio();
//    SDL_Quit();

    track_loaded=false;
    ui->ButtonPlay->setText("Play");
    ui->ButtonOpen->setEnabled(true);
    ui->LineNowPlaying->setText("");

    cout << "Playback Finished Successfuly" << endl;
}

void MainWindow::button_play_clicked()
{
    if(track_loaded)
    {
        if(playback_enable)
        {
            ui->ButtonPlay->setText("Play");
            playback_enable=0;
        }
        else
        {
            ui->ButtonPlay->setText("Pause");
            playback_enable=1;
        }
    }
}

void MainWindow::button_stop_clicked()
{
    sdl_quit = 1;
    playback_stop=true;
}

void MainWindow::button_exit_clicked()
{
    exit(1);
}

void MainWindow::radio_custom_clicked()
{
    //bool b=false;
    filter_mode=CUSTOM;

    ui->LineFMax->setEnabled(true);
    ui->LineFMin->setEnabled(true);
    ui->SliderGain->setEnabled(true);
   // ui->KnobMaster->setEnabled(true);
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

    int f0=QInputDialog::getInt(this, "Set FMin", "F_Min");
    while(f0<20)
        f0=QInputDialog::getInt(this, "Set FMin", "F_Min");

    int f1=QInputDialog::getInt(this, "Set FMax", "F_Max");
    while(f1<f0 || f1>20000)
        f1=QInputDialog::getInt(this, "Set FMax", "F_Max");

    ui->LineFMin->setText(QString::number(f0));
    ui->LineFMax->setText(QString::number(f1));

}

void MainWindow::radio_equilizer_clicked()
{

    filter_mode=EQUILIZER;

    ui->LineFMax->setEnabled(false);
    ui->LineFMin->setEnabled(false);
    ui->SliderGain->setEnabled(false);
    //ui->KnobMaster->setEnabled(false);
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

    filter_mode=LOW_PASS;

    ui->LineFMax->setEnabled(false);
    ui->LineFMin->setEnabled(false);
    ui->SliderGain->setEnabled(false);
    //ui->KnobMaster->setEnabled(false);

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

    filter_mode=HIGH_PASS;

    ui->LineFMax->setEnabled(false);
    ui->LineFMin->setEnabled(false);
    ui->SliderGain->setEnabled(false);
    //ui->KnobMaster->setEnabled(false);

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

void MainWindow::enable_filter()
{
    filter_enable=!filter_enable;
}

void MainWindow::knob_master_changed()
{
    master_volume=ui->KnobMaster->value();

    //cout << master_volume << " ";
}

void MainWindow::knob_balance_changed()
{
    balance=ui->KnobBalance->value();
}

void MainWindow::slider_gain_changed()
{
    c_gain=ui->SliderGain->value();
}

void MainWindow::slider_gain0_changed()
{
    f_gain[0]=ui->SliderGain0->value();
}

void MainWindow::slider_gain1_changed()
{
    f_gain[1]=ui->SliderGain1->value();
}

void MainWindow::slider_gain2_changed()
{
    f_gain[2]=ui->SliderGain2->value();
}

void MainWindow::slider_gain3_changed()
{
    f_gain[3]=ui->SliderGain3->value();
}

void MainWindow::slider_gain4_changed()
{
    f_gain[4]=ui->SliderGain4->value();
}

void MainWindow::slider_gain5_changed()
{
    f_gain[5]=ui->SliderGain5->value();
}

void MainWindow::slider_gain6_changed()
{
    f_gain[6]=ui->SliderGain6->value();
}

void MainWindow::slider_gain7_changed()
{
    f_gain[7]=ui->SliderGain7->value();
}

void MainWindow::slider_gain8_changed()
{
    f_gain[8]=ui->SliderGain8->value();
}

void MainWindow::slider_gain9_changed()
{
    f_gain[9]=ui->SliderGain9->value();
}

void MainWindow::line_fmin_clicked()
{
    if(ui->RadioCustom->isChecked())
    {
        int f1=ui->LineFMax->text().toInt();
        int f0=QInputDialog::getInt(this, "Set FMin", "F_Min");

        while(f0>f1)
            f0=QInputDialog::getInt(this, "Set FMin", "F_Min");
    }
}

void MainWindow::line_fmax_clicked()
{

    int f0=ui->LineFMin->text().toInt();
    int f1=QInputDialog::getInt(this, "Set FMax", "F_Max");

    while(f1<f0)
        f1=QInputDialog::getInt(this, "Set FMax", "F_Max");
}
