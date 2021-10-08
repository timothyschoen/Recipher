
# Project Name
TARGET = BandSynth

CPP_STANDARD = --std=gnu++17
OPT ?= -O2

# Sources
CPP_SOURCES = src/main.cpp src/FilterSynth.cpp src/SynthVoice.cpp src/Envelope.cpp src/Oscillator.cpp src/PhaseVocoder.cpp
C_SOURCES = src/fft/kiss_fft.c src/fft/kiss_fftr.c

# Library Locations
LIBDAISY_DIR ?= lib/libdaisy
DAISYSP_DIR ?= lib/DaisySP

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
