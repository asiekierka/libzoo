## Process this file with automake to generate Makefile.in

ACLOCAL_AMFLAGS = -I m4
AM_CPPFLAGS = -I$(srcdir)/src

include_HEADERS = src/zoo.h
nodist_include_HEADERS = zoo_config.h
lib_LTLIBRARIES = libzoo.la

libzoo_la_SOURCES = \
	src/zoo.c \
	src/zoo.h \
	src/zoo_element_defs.inc \
	src/zoo_elements.c \
	src/zoo_game.c \
	src/zoo_game_io.c \
	src/zoo_oop.c \
	src/zoo_sound.c \
	src/zoo_window.c
