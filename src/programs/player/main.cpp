#include <iostream>

#include <SDL2/SDL.h>

// 音频回调函数，负责将音频数据拷贝给音频设备进行播放
static void audio_callback(void* userdata, Uint8* stream, int len) {
    // userdata中存放了音频数据相关的信息
    struct AudioData {
        Uint8* pos;
        Uint32 length;
    };
    
    AudioData* audio = (AudioData*) userdata;

    if (audio->length == 0) {
        return;
    }
    
    // 实际需要拷贝给音频设备的字节数
    Uint32 to_copy = (len > (int)audio->length) ? audio->length : (Uint32)len;
    SDL_memcpy(stream, audio->pos, to_copy);

    // 更新音频数据指针与剩余长度
    audio->pos += to_copy;
    audio->length -= to_copy;

    printf("audio_callback: %p %d\n", stream, len);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "No audio specified." << std::endl;
        return 0;
    }

    // 初始化SDL音频子系统
    printf("SDL_Init\n");
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    // 加载WAV文件
    const char* filename = argv[1];
    SDL_AudioSpec wav_spec;
    Uint8* wav_buffer;
    Uint32 wav_length;

    printf("SDL_LoadWAV\n");
    if (SDL_LoadWAV(filename, &wav_spec, &wav_buffer, &wav_length) == NULL) {
        std::cerr << "Failed to load WAV file: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    // 准备用户数据，用于回调函数读取音频缓冲
    struct AudioData {
        Uint8* pos;
        Uint32 length;
    } audio_data;

    audio_data.pos = wav_buffer;
    audio_data.length = wav_length;

    // 设置回调函数与用户数据
    wav_spec.callback = audio_callback;
    wav_spec.userdata = &audio_data;

    // 打开音频设备
    printf("SDL_OpenAudio\n");
    if (SDL_OpenAudio(&wav_spec, NULL) < 0) {
        std::cerr << "Failed to open audio: " << SDL_GetError() << std::endl;
        SDL_FreeWAV(wav_buffer);
        SDL_Quit();
        return -1;
    }

    // 开始播放音频
    SDL_PauseAudio(0);

    // 等待直到音频数据播放完毕
    int loop = 0;
    while (audio_data.length > 0) {
        printf("LOOP: %d\n", ++loop);
        if (loop > 30) {
            break;
        }
        SDL_Delay(100);
    }

    // 停止并关闭音频设备
    SDL_CloseAudio();

    // 释放WAV缓冲区
    SDL_FreeWAV(wav_buffer);

    // 关闭SDL
    SDL_Quit();

    return 0;
}