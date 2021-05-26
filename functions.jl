using Random

#interpolation functions

function change_freq( signal, mult )
	
	sig_len = length(signal);
	
	#new_sig_len = Int64(floor(sig_len/mult));
	new_sig = zeros(sig_len);
	
	delta = mult;
	currentIndex = 1.0;
	
	for i = 1:1:sig_len
		index0 = Int64(floor(currentIndex));
		index1 = index0 + 1;
		if index1 > sig_len
			index1 -= sig_len;
		end
		
		frac = currentIndex - index0;
		
		value0 = signal[index0];
		value1 = signal[index1];
		
		#linear interpolation
		new_sig[i] = value0 + frac * (value1 - value0);
		
		currentIndex += delta;
		
		if currentIndex > sig_len
			currentIndex -= sig_len;
			currentIndex += 1;
		end
	end
	
	return new_sig;
end

#naive functions

function white_noise(t)
	return Random.rand!(zeros(length(t)))[1] .- 0.5;
end

function white_noise_bin(tlen)
	x = Random.rand!(zeros(tlen));
	for i = 1:1:length(x)
		if x[i] > 0.5
			x[i] = 1
		else
			x[i] = -1
		end
	end
	return x
end

function saw(phi)
	return (phi%(2*pi))/pi - 1;	
end
function square(phi)
	phi_ = phi%(2*pi);
	return phi_ > pi ? -1 : 1;
end
function triangle(phi)
	phi_ = phi%(2*pi)
	return phi_ > pi ? -2*phi_/pi + 3 : 2*phi_/pi - 1
end

#band-limited functions

function BL_triangle(t, freq, limit_freq, harmonics)
	sum_val = zeros(length(t));
	i = 0;
	n = 1;
	while n*freq <= limit_freq && i < harmonics
		
		sum_val += (-1)^i * n^(-2) * sin.(2*pi * freq*n * t);
		
		i += 1;
		n = 2*i + 1;
	end
	
	return 8*sum_val / (pi^2);
end

function BL_square(t, freq, limit_freq, harmonics)
	sum_val = zeros(length(t));
	i = 1;
	n = 1;
	while n*freq <= limit_freq && i <= harmonics
		sum_val += sin.(2*pi * n * freq * t)/n;
		
		i += 1;
		n = 2*i - 1;
	end
	
	return 4*sum_val/pi;
end

function BL_saw(t, freq, limit_freq, harmonics)
	sum_val = zeros(length(t));
	k = 1;
	
	while k*freq <= limit_freq && k <= harmonics
		sum_val += sin.(2*pi * k * freq * t)/k;
		
		k += 1;
	end
	
	return 2*sum_val/pi;
	
end