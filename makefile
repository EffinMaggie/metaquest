-include ef.gy/base.mk include/ef.gy/base.mk

NAME:=metaquest

DATAHEADERS:=include/data/female.first.h include/data/male.first.h include/data/all.last.h
MAXLINES:=5000

# gather source data
data/female.first.h: include/data/female.first.h
data/male.first.h: include/data/male.first.h
data/all.last.h: include/data/all.last.h

data/census/dist.%.census.gov:
	mkdir -p $(dir $@) || true
	$(CURL) 'http://www2.census.gov/topics/genealogy/1990surnames/dist.$*' > $@

include/data/%.h: data/census/dist.%.census.gov makefile
	mkdir -p $(dir $@) || true
	echo '#include <array>' > $@
	echo '#include <tuple>' >> $@
	echo 'namespace data {' >> $@
	echo "static const std::array<std::tuple<const char*,long>,$$(cat $< | head -n $(MAXLINES) | wc -l | sed 's/^ *//' | cut -d ' ' -f 1)> $$(echo $* | tr '.' '_') {{" >> $@
	awk '{print " std::tuple<const char*,long>(\"" $$1 "\"," ($$2*1000+1) "),"}' < $< | head -n $(MAXLINES) >> $@
	echo '    }};' >> $@
	echo '};' >> $@
