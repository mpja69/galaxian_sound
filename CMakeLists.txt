cmake_minimum_required(VERSION 3.10)

project(test_sound)

add_executable(test_sound main.c sound.c pitch.c  lfo.c timer_555.c fire_sound.c lfsr.c miniaudio.h)

target_link_libraries(test_sound PRIVATE m pthread)
