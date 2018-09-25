l = seriallist
l(3)
myserialport = serial("/dev/cu.wchusbserial1410", "BaudRate", 9600)
fopen(myserialport)
binNums = [0:127];
bins = zeros(1,128);
try
    start = fscanf(myserialport,"%s");
    for i = 1:128
        bins(i) = fscanf(myserialport,"%d")
    end
catch ME
    warning('error occured');
end
fclose(myserialport)
plot(binNums, bins)