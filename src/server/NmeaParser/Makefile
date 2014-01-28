ARDUINO_LIBS_PATH=../../device/Arduino/libraries
NMEA_PARSER_PATH=$(ARDUINO_LIBS_PATH)/NmeaParser
IRENE_TARGET_SPEED_PATH=$(ARDUINO_LIBS_PATH)/IreneTargetSpeed

CXXFLAGS=-g -Wall -I$(ARDUINO_LIBS_PATH)


all: nmea2matlab nmea2json

clean:
	rm -f nmea2matlab.o nmeaparser.o nmea2matlab nmea2json.o nmea2json IreneTargetSpeed.o

nmea2matlab: nmea2matlab.o nmeaparser.o IreneTargetSpeed.o

nmeaparser.o: $(NMEA_PARSER_PATH)/NmeaParser.cpp $(NMEA_PARSER_PATH)/NmeaParser.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

IreneTargetSpeed.o: $(IRENE_TARGET_SPEED_PATH)/IreneTargetSpeed.cpp $(IRENE_TARGET_SPEED_PATH)/IreneTargetSpeed.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

nmea2matlab.o: nmea2matlab.cpp $(NMEA_PARSER_PATH)/NmeaParser.h $(IRENE_TARGET_SPEED_PATH)/IreneTargetSpeed.h

nmea2json: nmea2json.o nmeaparser.o IreneTargetSpeed.o

nmea2json.o: nmea2json.cpp $(NMEA_PARSER_PATH)/NmeaParser.h $(IRENE_TARGET_SPEED_PATH)/IreneTargetSpeed.h

