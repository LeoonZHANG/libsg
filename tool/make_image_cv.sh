#g++ -Iusr/include/opencv2 -lopencv_core -lopencv_highgui -o image_cv image.cpp
#g++ -Iusr/include/opencv2 ../util/number.c image_cv.cpp $(pkg-config opencv --libs) -o image_cv
g++ -Wall -pedantic -O3 -shared -fPIC ../util/number.c ../imgutil/image_cv.cpp -lopencv_core -o image_cv.so
