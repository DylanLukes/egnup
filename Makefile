REBAR := ./rebar

.PHONY: all deps doc test clean release

all: deps
	$(REBAR) compile

deps:
	$(REBAR) get-deps

doc:
	$(REBAR) doc skip_deps=true

test:
	$(REBAR) eunit skip_deps=true

tags:
	rm -fr TAGS
	find . -name "*.[eh]rl" -print | etags -

clean:
	$(REBAR) clean

release: all test
