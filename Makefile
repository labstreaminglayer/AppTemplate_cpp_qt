FILES = .travis.yml  CMakeLists.txt .appveyor.yml \
	main.cpp mainwindow.cpp mainwindow.h \
	sophisticated_recording_device.cpp sophisticated_recording_device.h
COPYFILES = mainwindow.ui .gitignore BestPracticesGUI.cfg LICENSE.txt
DOCS = $(patsubst %, doc/%.md, $(FILES))

all: $(DOCS) $(patsubst %, appskeleton/%, $(COPYFILES))

$(DOCS): doc/%.md: %
	perl split.pl $<

appskeleton/%: %
	cp $< $@
