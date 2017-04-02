
build: core.pyd

clean: 
	rm ./build/* -rf
	rm ./core.pyd

run:
	python main.py

test: test.pyd
	cd ./build/lib.mingw-2.7 && python -m unittest --verbose test

core.pyd: *.pyx *.pxd *.c *.h ../serialframe.c ../serialframe.h ../main.h
	python setup.py build
	cp ./build/lib.mingw-2.7/core.pyd ./core.pyd

test.pyd: *.pyx *.pxd *.c *.h ../*.c ../*.h
	python setup_test.py build
