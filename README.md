# Simple Baseline JPEG Encoder
This goal of this repository is to provide simple codes of high readibility for fully understanding the encoding process of baseline JPEG. The implememtation was inspired by [TheIllusionistMirage](https://github.com/TheIllusionistMirage)'s [JPEG decoder](https://github.com/TheIllusionistMirage/simple-jpeg-decoder) repository. Image processing related operations use (OpenCV)[https://github.com/opencv/opencv] library for higher readibilty. You can refer to it's [official documentation](https://docs.opencv.org) for more details.

# Building
Go to the repostiry directory and execute the following commands in shell:
```
$ mkdir build && cd build
$ cmake ..
$ make
```
# Usage
### print help
```
$ ./cppeg -h
```
### Compress Image into JPEG File
```
$ ./kpeg input_img_path [optional_output_path]
```
Suppose our input image's path is `sample.jpg` and we don't denote the output path of the compressed JPEG file. Then, the output JPEG file will have the name `sample_compressed.jpg`.
# Reference
[1] Recommendation T.81 (09/92): Information technology—Digital compression and coding of continuous-tone still images—Requirements and guidelines

[2] Koushtav Chakrabarty's JPEG Decoder Guide [[blog](https://koushtav.me/jpeg/tutorial/c++/decoder/2019/03/02/lets-write-a-simple-jpeg-library-part-2/)]

[3] OpenCV documentation [[website](https://docs.opencv.org/3.4/d2/de8/group__core__array.html#ga191389f8a0e58180bb13a727782cd461)]