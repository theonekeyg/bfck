TESTS!= echo *.sh

.PHONY: all $(TESTS)
.NOTPARALLEL: $(TESTS)

all: $(TESTS)

$(TESTS):
	sh $*.sh
	@echo test \`$*\` passed
