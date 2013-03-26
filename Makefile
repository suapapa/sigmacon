build:
	scons

install: build
	cp sigmacon /usr/local/bin/
	cp sigmacond /etc/init.d/
	ln -s /etc/init.d/sigmacond /etc/rcS.d/S15sigmacond
	@echo "all done"

uninstall:
	rm -f /usr/local/bin/sigmacon
	rm -f /etc/init.d/sigmacond
	rm -f /etc/rcS.d/S15sigmacond
	@echo "all done"
