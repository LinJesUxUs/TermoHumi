NAME = bin/main.hex

SRCS = src/main.cpp src/AHT10.cpp

HEADER = src/AHT10.h src/structs.h src/includes.h

OBJ = obj/main.o

C = avr-g++
COBJ = avr-objcopy
FLAGS = -mmcu=atmega8
OFLAGS = -j .text -j .data -O ihex

.PHONY : check flash verification all clean fclean re folders

check : $(NAME)
	sudo avrdude -c usbasp -p m8

flash : $(NAME)
	sudo avrdude -c usbasp -p m8 -U flash:w:$(NAME)

verification : $(NAME)
	sudo avrdude -c usbasp -p m8 -U flash:v:$(NAME)

all : $(NAME)

$(NAME) : $(OBJ) $(HEADER)
	$(COBJ) $(OFLAGS) $(OBJ) $(NAME)
# ar rcs $(NAME) $?

obj/%.o : src/%.cpp $(HEADER) folders
	$(C) $(FLAGS) -Os $(SRCS) -o $(OBJ)

clean:
	rm -rf $(OBJ)

fclean : clean
	$(RM) $(NAME)

re : fclean all

folders : obj bin

obj :
	mkdir obj

bin :
	mkdir bin
