PLUGIN_NAME=hyprcontexts

.PHONY: all clean

all:
	$(CXX) -shared -fPIC -O3 -I/usr/include/hyprland -I/usr/include/hyprland/protocols -I/usr/include/hyprland/src/includes -std=c++23 main.cpp -o ${PLUGIN_NAME}.so `pkg-config --cflags pixman-1 libdrm hyprlang`

clean:
	rm -f ${PLUGIN_NAME}.so
