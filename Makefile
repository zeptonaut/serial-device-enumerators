run: mac
	./a.out

mac:
	clang++ -framework IOKit -framework CoreFoundation -g serial_device_enumerator_mac.cc
