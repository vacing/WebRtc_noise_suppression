// WebRtc_official.cpp: 定义控制台应用程序的入口点。
//	用于测试 google 官方提供的 webrtc 库中的 noise suppression 模块
//	所用文件均与官方程序库中的保持一致

#include "stdafx.h"
#include <iostream>
#include <vector>
#include "AudioFile.cpp"		// .cpp

#include "noise_suppression.c"
#include "ns_core.c"
#include "defines.h"
#include "typedefs.h"
#include "checks.cc"

#ifndef nullptr
#define nullptr 0
#endif

#ifndef MIN
#define MIN(A, B)        ((A) < (B) ? (A) : (B))
#endif

#define WEBRTC_WIN
enum nsLevel {
	kLow,
	kModerate,
	kHigh,
	kVeryHigh
};

NsHandle* nsInit(int sample_rate,nsLevel);
vector<double> nsProcess(NsHandle* nsHandle,AudioFile<double> audio);
using namespace std;	// 重要

int main()
{
	bool isMono = true;
	vector<vector<double>> input;
	vector<vector<double>> output(2);				// 后续使用 [] 进行访问前，需要指定vec的大小
	AudioFile<double> af;
	af.load("music_with_noise_48k_16bit_21db.wav");					// 用于测试高采样率下的情况
	
	af.printSummary();
	int sample_rate = af.getSampleRate();
	int total_samples = af.getNumSamplesPerChannel();
	input.push_back(af.samples[0]);		
	if (af.getNumChannels() > 1) {					// 提取双声道数据
		isMono = false;
		input.push_back(af.samples[1]);
	}
	// load noise suppression module
	size_t samples = MIN(160, sample_rate / 100);	// 最高支持160个点
	if (samples == 0) return -1;
	const int maxSamples = 320; 
	int num_bands = 2;								// 1~2
	size_t total_frames = (total_samples / samples);		// 处理的帧数
	// 以下为初始化
	NsHandle *nsHandle = WebRtcNs_Create();
	int status = WebRtcNs_Init(nsHandle, sample_rate);
	if (status != 0) {
		printf("WebRtcNs_Init fail\n");
		return -1;
	}
	status = WebRtcNs_set_policy(nsHandle, kVeryHigh);
	if (status != 0) {
		printf("WebRtcNs_set_policy fail\n");
		return -1;
	}
	// 主处理函数（帧处理)
	for (int i = 0; i < total_frames; i++) {
		float data_in[maxSamples];
		float data_out[maxSamples];
		float data_in2[maxSamples];
		float data_out2[maxSamples];
		//  input the signal to process,input points <= 160 (10ms)
		for (int n = 0; n != samples; ++n) {
			data_in[n] = input[0][ samples * i + n];	
			data_in2[n] = input[1][ samples * i + n];	
		}
		float *input_buffer[2] = { data_in ,data_in2 };			//ns input buffer [band][data]   band:1~2
		float *output_buffer[2] = { data_out,data_out2 };		//ns output buffer [band][data] band:1~2
																//声明p是一个指针，它指向一个具有2个元素的数组
		WebRtcNs_Analyze(nsHandle, input_buffer[0]);			
		WebRtcNs_Process(nsHandle, (const float *const *)input_buffer, isMono?1:2, output_buffer);
		// output the processed signal
		for (int n = 0; n != samples; ++n) {
			output[0].push_back(output_buffer[0][n]);		// Lift band	
			if(!isMono)
				output[1].push_back(output_buffer[1][n]);	// Right band

		}

	}
	WebRtcNs_Free(nsHandle);
	std::cout << output.size() << endl;
	af.setAudioBuffer(output);
	af.save("music_noise_suppressed_48k_21db_p3.wav");


    return 0;
}

NsHandle * nsInit(int sample_rate, nsLevel)
{
	return nullptr;
}

vector<double> nsProcess(NsHandle * nsHandle, AudioFile<double> audio)
{
	return vector<double>();
}
