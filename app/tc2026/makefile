# EPOS Application Makefile

include ../../makedefs

OBJS := tc2026.o rijndael_enc.o rijndael_enc_libc.o aes.o input_small.o kalman.o h264_decinput.o

all: install

$(APPLICATION): $(OBJS) $(LIB)/*
		$(ALD) $(ALDFLAGS) -o $@ $(OBJS)

%.o: %.cc
		$(ACC) $(ACCFLAGS) -c -o $@ $<

install: $(APPLICATION)
		$(INSTALL) $(APPLICATION) $(IMG)

clean:
		$(CLEAN) *.o $(APPLICATION)
