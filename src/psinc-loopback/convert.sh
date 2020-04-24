#!/bin/bash

# Convert stream to YUV format for applications that don't support RGB24
ffmpeg -y -i /dev/video0 -pix_fmt yuyv422 -f v4l2 /dev/video1
#ffmpeg -i /dev/video0 -pix_fmt yuv420p -f v4l2 /dev/video1

