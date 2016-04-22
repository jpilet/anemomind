
BRUNCH = ./node_modules/.bin/brunch
DEST= user@server.ch:www/to/dist/

install:
	npm install && bower install

clean:
	rm -rf dist/* bower_components/  node_modules/

sync:
	cp app/assets/robots.txt.prod dist/robots.txt
	cd build && rsync -avu --delete -e 'ssh -p22' . $(DEST)


publish:
	$(BRUNCH) build --production
	cp app/assets/robots.txt.devel dist/robots.txt
	cd build && rsync -avu --delete -e 'ssh -p22' . $(DEST)


.PHONY: install clean sync publish
