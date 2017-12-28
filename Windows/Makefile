PREFIX=/usr/local
default: TopCoderHackHelper
TopCoderHackHelper/TopCoderHackHelper.cpp.o: $(shell $(CXX) -MM TopCoderHackHelper.cpp -std=c++11 | tr '\n' ' ' | tr '\\' ' ' | perl -pe 's/.*://')
	@mkdir -p `dirname $@`
	@echo "Compile $<"
	@$(CXX) -c -o $@ $< -std=c++11 -Ofast
TopCoderHackHelper: .output/TopCoderHackHelper
.output/TopCoderHackHelper: TopCoderHackHelper/TopCoderHackHelper.cpp.o
	@mkdir -p `dirname $@`
	@echo "Link $@"
	@$(CXX) -o $@ TopCoderHackHelper/TopCoderHackHelper.cpp.o -lpcap -lpthread -Ofast
install.TopCoderHackHelper: TopCoderHackHelper
	@mkdir -p $(PREFIX)/bin
	@echo "Install .output/TopCoderHackHelper"
	@install -m 0755 .output/TopCoderHackHelper $(PREFIX)/bin/`basename .output/TopCoderHackHelper`
clean:
	@echo "Remove objects"
	@-rm -f TopCoderHackHelper/TopCoderHackHelper.cpp.o
	@echo "Remove outputs"
	@-rm -f .output/TopCoderHackHelper
.PHONY: clean
install: install.TopCoderHackHelper
.PHONY: install
viewcompiler:
	@echo "c compiler: $(CC)"
	@echo "c++ compiler: $(CXX)"
	@echo "archive linker: $(AR)"
.PHONY: viewcompiler
