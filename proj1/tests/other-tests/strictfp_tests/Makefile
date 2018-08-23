.PHONY: check clean really-clean

APYC = ../apyc

PYTHON = python3

GOODTEST = $(PYTHON) test-proj1 

BADTEST = $(PYTHON) test-proj1 --errors

check: 
	$(MAKE) -C .. compile
	@echo Testing correct programs...
	$(RM) errflag
	-$(GOODTEST) --compiler=$(APYC) correct/*.py || touch errflag
	@echo
	@echo Testing erroneous programs...
	-$(BADTEST) --compiler=$(APYC) error/*.py || touch errflag
	@if test -f errflag; then code=1; else code=0; fi; \
        $(RM) errflag; exit $$code

clean:
	$(RM) errflag *~ */*~ 

really-clean: clean
