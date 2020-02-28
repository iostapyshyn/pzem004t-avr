BIN = pzem

PORT ?= /dev/tty.usbserial-FA1400
MCU_TARGET ?= atmega16
F_CPU ?= 8000000UL

AVRDUDE_TARGET ?= m16
PROGRAMMER ?= avrftdi

-include ../avr-templates/avr.mk
