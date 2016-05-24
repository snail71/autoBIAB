gcc -Wall -g -o autoBIAB autoBIAB.c rs232.c utils.c recipe.c equipment.c controller.c `xml2-config --cflags --libs` `pkg-config --cflags --libs gtk+-3.0`
