
BRUNCH = ./node_modules/.bin/brunch
DEST= user@server.ch:www/to/dist/

clean:
	rm -f build --production

syncprod:
	cp app/assets/robots.txt.prod build/robots.txt
	cd build && rsync -avu --delete -e 'ssh -p22' . $(DEST)

syncdev:
	cp app/assets/robots.txt.devel build/robots.txt
	cd build && rsync -avu --delete -e 'ssh -p22' . $(DEST)

devel:
	$(BRUNCH) build --production
	cp app/assets/robots.txt.devel build/robots.txt
	cd build && rsync -avu --delete -e 'ssh -p22' . $(DEST)

prod:
	$(BRUNCH) build --production
	cp app/assets/robots.txt.prod build/robots.txt
	cd build && rsync -avu --delete -e 'ssh -p22' . $(DEST)



.PHONY: docs clean publish
