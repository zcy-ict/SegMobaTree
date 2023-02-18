# ZCY Makefile

OBPATH = objects/
OBJECTS = $(shell find -name "*.cpp")
OBJECTS_O = $(OBJECTS:./%.cpp=$(OBPATH)%.o)
OBJECTS_D = $(OBJECTS:./%.cpp=$(OBPATH)%.d)

CXX = g++
CXXFLAGS = -g -std=c++14 -fpermissive -O3 -fopenmp -mpopcnt

main: $(OBJECTS_O)
	@$(CXX) $(CXXFLAGS) -o main $(OBJECTS_O)
	@echo $(CXX) $(CXXFLAGS) -o main *.o

-include $(OBJECTS_D)

$(OBJECTS_D) : $(OBPATH)%.d : %.cpp
	@mkdir -p $(dir $@); \
	$(CXX) -MM $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,$(notdir $*.o):,$(OBPATH)$*.o:,g' $@.$$$$ > $@; \
	echo "	$(CXX) $(CXXFLAGS) -o $(OBPATH)$*.o -c $<" >> $@; \
	rm -f $@.$$$$

.PHONY: clean

clean:
	rm -rf $(OBPATH)
	rm -rf main
	rm -rf output
