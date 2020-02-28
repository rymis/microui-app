
all:
	mkdir -p build
	(cd build; cmake ..)
	[ -f compile_commands.json ] || ln -sf ./build/compile_commands.json ./compile_commands.json
	cmake --build build -- -j 2

clean:
	rm -rf build

.PHONY: all clean
