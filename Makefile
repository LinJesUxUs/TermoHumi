BINDIR := bin

NAME := $(addprefix $(BINDIR)/,main.hex)

BIN := $(addprefix $(BINDIR)/,main.elf)

SRCDIR := src

SRCS := $(addprefix $(SRCDIR)/,main.cpp AHT10.cpp)

HEADER := $(addprefix $(SRCDIR)/,AHT10.h common.h)

OBJDIR := obj

OBJ := $(addprefix $(OBJDIR)/,main.o AHT10.o)



MCU = atmega8

OPT = s

DEBUG = dwarf-2

EXTRAINCDIRS = .
CSTANDARD = -std=gnu++11
EXTRA_CDEFS =

EXTRA_CUNDEFS =
EXTRA_COPTIONS = -Wstrict-prototypes
EXTRA_CPPOPTIONS =

GENDEPFLAGS = -MD -MP -MF .deppp/$(@F).d


C = avr-g++
CC = avr-g++
COBJ = avr-objcopy
FLAGS = -mmcu=atmega8 -I. -gdwarf-2 -Os -Wcpp -Wa,-adhlns=$<.lst -I. -gdwarf-2 -MD -MP -MF .deppp/$(notdir $@).d
OFLAGS = -j .text -j .data -O ihex

CFLAGS =
CFLAGS = -g$(DEBUG)
CFLAGS += -O$(OPT)
# CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -Wall
CFLAGS += -Wa,-adhlns=$<.lst
CFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS))
CFLAGS += $(CSTANDARD)
CFLAGS += $(EXTRA_COPTIONS)

CPPFLAGS =
CPPFLAGS = -g$(DEBUG)
CPPFLAGS += -O$(OPT)
# CPPFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CPPFLAGS += -Wcpp
CPPFLAGS += -Wa,-adhlns=$<.lst
CPPFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS))
CPPFLAGS += $(EXTRA_CPPOPTIONS)

PRINTF_LIB = # full functionality except but no floating point
SCANF_LIB = # full functionality except floating point, max. width 255
MATH_LIB = -lm
EXTMEMOPTS =

LDFLAGS = -Wl,-Map=main.map,--cref
LDFLAGS += $(EXTMEMOPTS)
LDFLAGS += $(PRINTF_LIB) $(SCANF_LIB) $(MATH_LIB)

ALL_CFLAGS = -mmcu=$(MCU) -I. $(CFLAGS) $(GENDEPFLAGS)
ALL_CPPFLAGS = -mmcu=$(MCU) -I. $(CPPFLAGS) $(GENDEPFLAGS)

.PHONY : check flash verification all clean fclean re folders

check : $(NAME)
	sudo avrdude -c usbasp -p m8

flash : $(NAME)
	sudo avrdude -c usbasp -p m8 -U flash:w:$(NAME)

verification : $(NAME)
	sudo avrdude -c usbasp -p m8 -U flash:v:$(NAME)

all : $(NAME)

$(NAME) : $(BIN)
	$(COBJ) $(OFLAGS) $(BIN) $(NAME)
# ar rcs $(NAME) $?

.SECONDARY : $(BIN)
.PRECIOUS : $(OBJ)
$(BIN) : $(OBJ)
	$(CC) $(ALL_CFLAGS) $(OBJ) -o $@ $(LDFLAGS)

$(OBJDIR)/%.o : $(SRCDIR)/%.cpp folders
	$(C) -c $(ALL_CPPFLAGS) $< -o $@

clean:
	rm -rf $(OBJ) $(SRCDIR)/*.lst

fclean : clean
	$(RM) $(BIN) $(NAME) main.map
	rm -rf .deppp

re : fclean all

folders : obj bin deppp

obj :
	mkdir -p obj

bin :
	mkdir -p bin

deppp :
	mkdir -p .deppp
