
build: core.pyd

clean: 
	rm ./build/* -rf
	rm ./core.pyd

run:
	python main.py

core.pyd: *.pyx *.pxd *.c *.h ../serialframe.c ../serialframe.h ../main.h
	python setup.py build
	cp ./build/lib.mingw-2.7/core.pyd ./core.pyd

