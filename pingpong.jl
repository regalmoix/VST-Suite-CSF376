using PyPlot

rate = 44100;#Hz

function nextSample(a, b, target, f_1, f_2)
	
	return b/(rate^2)*target + (2 - a/rate - b/(rate^2))*f_1 + (a/rate - 1)*f_2;
	
end

max_t = 2;
t = 0:1/rate:max_t;
len = length(t);
vals = zeros(len);
vals[1] = 440;
vals[2] = 440;
omega = 100;
zeta = 0.1;
for i = 3:1:len
	vals[i] = nextSample(2*zeta*omega, omega^2, 550, vals[i-1], vals[i-2]);
end

figure(1);
plot(t, vals);