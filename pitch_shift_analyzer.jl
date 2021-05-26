using PyPlot
using FFTW
#using DSP
include("./functions.jl")

#include("D:/Github Repos/VSTs_CSF376/pitch_shift_analyzer.jl")

#how many samples in the buffer (power of two for easy FFT)
N = 1<<11;

#sampling rate (in Hertz)
rate = 44100;

#time period of one sample
t_sample = 1/rate;

t0 = 0;
tmax = t0 + (N-1)*t_sample;

#the frequency that will create a wavetable
fullwave_freq = 1/tmax;
println("Fullwave freq is ", fullwave_freq);

#the time array
t = t0:t_sample:tmax;

#the signal (in time spectrum)
signal = BL_saw(t, fullwave_freq, rate>>1, 4000);\
#signal = white_noise_bin(length(t))

#signal = sin.(2*pi * t/tmax);
mult = 200;
new_signal = change_freq(signal, mult);
new_t = t_sample .* (  0:1:(length(new_signal)-1)  );
println("Changed frequency: ", fullwave_freq*mult);

#F_ = fft(signal)
#the signal in frequency spectrum
F = fftshift(fft(signal));
F1 = fftshift(fft(new_signal));
#the frequency array
freqs = rate.*(-(N>>1):1:(N>>1)-1)./N;
new_N = length(new_t);
new_freqs = rate.*(-(new_N>>1):1:(new_N>>1)-1)./new_N;

figure(1);
plot(t, signal);
title("Original wave");
figure(2);
plot(freqs, abs.(F));
title("Original wave in Freq spectrum");
xlabel("Frequency (Hz)")
ylabel("Intensity")
figure(3);
plot(t, new_signal);
title("Interpolated wave");
#xlabel("Time (s)")
#ylabel("Wave value")
figure(4);
plot(freqs, abs.(F1));
title("Interpolated wave in Freq spectrum");
#xlabel("Frequencies (Hz)")
#ylabel("Intensity of given frequency")